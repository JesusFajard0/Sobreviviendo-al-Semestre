#include "activities/GestorActividades.hpp"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <utility>

namespace
{
    constexpr int N_COLUMNAS = 12;

    std::string recortar(const std::string& s)
    {
        std::size_t ini = 0;
        std::size_t fin = s.size();
        while (ini < fin && std::isspace(static_cast<unsigned char>(s[ini])))
            ++ini;
        while (fin > ini && std::isspace(static_cast<unsigned char>(s[fin - 1])))
            --fin;
        return s.substr(ini, fin - ini);
    }

    std::string minusculas(std::string s)
    {
        std::transform(s.begin(), s.end(), s.begin(),
                       [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        return s;
    }

    // Divide por el separador conservando los campos vacíos.
    std::vector<std::string> dividir(const std::string& texto, char separador)
    {
        std::vector<std::string> partes;
        std::size_t inicio = 0;
        while (true)
        {
            const std::size_t pos = texto.find(separador, inicio);
            if (pos == std::string::npos)
            {
                partes.push_back(recortar(texto.substr(inicio)));
                break;
            }
            partes.push_back(recortar(texto.substr(inicio, pos - inicio)));
            inicio = pos + 1;
        }
        return partes;
    }

    bool leerFloat(const std::string& texto, float& valor)
    {
        if (texto.empty())
            return false;
        char* fin = nullptr;
        const double v = std::strtod(texto.c_str(), &fin);
        if (fin == texto.c_str() || *fin != '\0')
            return false;
        valor = static_cast<float>(v);
        return true;
    }

    bool leerEntero(const std::string& texto, int& valor)
    {
        if (texto.empty())
            return false;
        char* fin = nullptr;
        const long v = std::strtol(texto.c_str(), &fin, 10);
        if (fin == texto.c_str() || *fin != '\0')
            return false;
        valor = static_cast<int>(v);
        return true;
    }

    bool leerBloque(const std::string& texto, int& indice)
    {
        const std::string t = minusculas(texto);
        for (int i = 0; i < static_cast<int>(Bloque::Cantidad); ++i)
        {
            if (t == minusculas(Tiempo::nombre(static_cast<Bloque>(i))))
            {
                indice = i;
                return true;
            }
        }
        return false;
    }

    bool leerDia(const std::string& texto, int& indice)
    {
        const std::string t = minusculas(texto);
        for (int i = 0; i < static_cast<int>(Dia::Cantidad); ++i)
        {
            if (t == minusculas(Tiempo::nombre(static_cast<Dia>(i))))
            {
                indice = i;
                return true;
            }
        }
        return false;
    }

    // "*" = todos; si no, una lista separada por comas.
    template <typename Leer>
    bool leerMascara(const std::string& texto, int total, Leer leer, std::uint8_t& mascara)
    {
        if (texto == "*")
        {
            mascara = static_cast<std::uint8_t>((1 << total) - 1);
            return true;
        }

        mascara = 0;
        for (const std::string& parte : dividir(texto, ','))
        {
            int indice = 0;
            if (!leer(parte, indice))
                return false;
            mascara = static_cast<std::uint8_t>(mascara | (1 << indice));
        }
        return mascara != 0;
    }
}

GestorActividades::GestorActividades()
    : tabla_(IgualId(), &hashEntrada)
{
}

// ---------------------------------------------------------------
// Carga
// ---------------------------------------------------------------

bool GestorActividades::cargarDesdeArchivo(const std::string& ruta)
{
    std::ifstream archivo(ruta);
    if (!archivo.is_open())
    {
        std::cerr << "[GestorActividades] No se pudo abrir '" << ruta << "'\n";
        return false;
    }

    bool todoBien = true;
    int numeroLinea = 0;
    std::string linea;

    while (std::getline(archivo, linea))
    {
        ++numeroLinea;

        linea = recortar(linea);
        if (linea.empty() || linea[0] == '#')
            continue;

        const std::vector<std::string> c = dividir(linea, '|');
        if (static_cast<int>(c.size()) != N_COLUMNAS)
        {
            std::cerr << "[GestorActividades] Linea " << numeroLinea << ": se esperaban "
                      << N_COLUMNAS << " columnas y hay " << c.size() << "\n";
            todoBien = false;
            continue;
        }

        DatosActividad d{};
        d.id     = c[0];
        d.nombre = c[1];

        int gasta = 0;
        const bool bien =
            !d.id.empty() && !d.nombre.empty() &&
            leerMascara(c[2], static_cast<int>(Bloque::Cantidad), leerBloque, d.bloques) &&
            leerMascara(c[3], static_cast<int>(Dia::Cantidad), leerDia, d.dias) &&
            leerEntero(c[4], gasta) && (gasta == 0 || gasta == 1) &&
            leerFloat(c[5],  d.deltaEnergia) &&
            leerFloat(c[6],  d.deltaConocimiento) &&
            leerFloat(c[7],  d.deltaSaludMental) &&
            leerFloat(c[8],  d.deltaEstres) &&
            leerFloat(c[9],  d.deltaDopamina) &&
            leerFloat(c[10], d.deltaDinero) &&
            leerEntero(c[11], d.limiteDiario);

        if (!bien)
        {
            std::cerr << "[GestorActividades] Linea " << numeroLinea
                      << ": valores invalidos en '" << c[0] << "'\n";
            todoBien = false;
            continue;
        }

        d.gastaBloque = (gasta == 1);

        if (buscarEntrada(d.id) != nullptr)
        {
            std::cerr << "[GestorActividades] Linea " << numeroLinea << ": la actividad '"
                      << d.id << "' esta repetida\n";
            todoBien = false;
            continue;
        }

        ids_.push_back(d.id);
        tabla_.insert(Entrada{std::move(d), 0});
    }

    return todoBien;
}

// ---------------------------------------------------------------
// Consultas
// ---------------------------------------------------------------

const GestorActividades::Entrada* GestorActividades::buscarEntrada(const std::string& id) const
{
    DatosActividad vacia{};
    vacia.id = id;
    const Entrada sonda{std::move(vacia), 0};
    return tabla_.search(sonda);
}

const DatosActividad* GestorActividades::buscar(const std::string& id) const
{
    const Entrada* e = buscarEntrada(id);
    return e != nullptr ? &e->datos : nullptr;
}

int GestorActividades::usosHoy(const std::string& id) const
{
    const Entrada* e = buscarEntrada(id);
    return e != nullptr ? e->usosHoy : 0;
}

EstadoActividad GestorActividades::validar(const Entrada& entrada,
                                           const Estadisticas& estadisticas,
                                           const Tiempo& tiempo) const
{
    if (tiempo.semestreTerminado() || tiempo.enFinDeSemana())
        return EstadoActividad::SinTiempo;

    const DatosActividad& d = entrada.datos;

    const bool bloqueAbierto = (d.bloques & (1 << static_cast<int>(tiempo.bloque()))) != 0;
    const bool diaAbierto    = (d.dias    & (1 << static_cast<int>(tiempo.dia())))    != 0;
    if (!bloqueAbierto || !diaAbierto)
        return EstadoActividad::FueraDeHorario;

    if (d.limiteDiario >= 0 && entrada.usosHoy >= d.limiteDiario)
        return EstadoActividad::LimiteAlcanzado;

    // Las actividades que cuestan dinero no se pueden hacer sin tener suficiente.
    if (d.deltaDinero < 0.f &&
        estadisticas.obtener(Stat::Dinero) + d.deltaDinero < 0.f)
        return EstadoActividad::SinDinero;

    return EstadoActividad::Disponible;
}

EstadoActividad GestorActividades::puedeHacer(const std::string& id,
                                              const Estadisticas& estadisticas,
                                              const Tiempo& tiempo) const
{
    const Entrada* e = buscarEntrada(id);
    if (e == nullptr)
        return EstadoActividad::NoExiste;

    return validar(*e, estadisticas, tiempo);
}

// ---------------------------------------------------------------
// Ejecución
// ---------------------------------------------------------------

ResultadoActividad GestorActividades::ejecutar(const std::string& id,
                                               Estadisticas& estadisticas,
                                               Tiempo& tiempo,
                                               Materia materiaActiva)
{
    ResultadoActividad resultado;

    const Entrada* e = buscarEntrada(id);
    if (e == nullptr)
    {
        std::cerr << "[GestorActividades] Actividad inexistente: '" << id << "'\n";
        resultado.estado = EstadoActividad::NoExiste;
        return resultado;
    }

    const EstadoActividad estado = validar(*e, estadisticas, tiempo);
    if (estado != EstadoActividad::Disponible)
    {
        resultado.estado = estado;
        return resultado;
    }

    const DatosActividad& d = e->datos;

    // Mismo orden que el enum Stat.
    static_assert(static_cast<int>(Stat::Cantidad) == 5,
                  "Cambio el numero de estadisticas: actualizar este arreglo");
    const std::array<float, static_cast<int>(Stat::Cantidad)> deltas = {{
        d.deltaSaludMental,   // Stat::SaludMental
        d.deltaEnergia,       // Stat::Energia
        d.deltaEstres,        // Stat::Estres
        d.deltaDopamina,      // Stat::Dopamina
        d.deltaDinero         // Stat::Dinero
    }};

    for (int i = 0; i < static_cast<int>(Stat::Cantidad); ++i)
        resultado.cambios[i] = estadisticas.modificar(static_cast<Stat>(i), deltas[i]);

    if (d.deltaConocimiento != 0.f)
        resultado.cambioConocimiento =
            estadisticas.modificarConocimiento(materiaActiva, d.deltaConocimiento);

    ++e->usosHoy;
    resultado.estado = EstadoActividad::Hecha;

    // Una actividad que gasta bloque avanza el tiempo. Si cambió el día
    // (o llegó el fin de semana o el fin del semestre) se reinician los contadores.
    if (d.gastaBloque)
    {
        resultado.evento = tiempo.avanzarBloque();
        if (resultado.evento != EventoTiempo::Ninguno)
            reiniciarContadores();
    }

    return resultado;
}

void GestorActividades::reiniciarContadores()
{
    for (const std::string& id : ids_)
    {
        const Entrada* e = buscarEntrada(id);
        if (e != nullptr)
            e->usosHoy = 0;
    }
}

// ---------------------------------------------------------------
// Extras
// ---------------------------------------------------------------

float GestorActividades::aplicarMesada(Estadisticas& estadisticas)
{
    return estadisticas.modificar(Stat::Dinero, MESADA);
}

const char* GestorActividades::mensaje(EstadoActividad estado)
{
    switch (estado)
    {
        case EstadoActividad::FueraDeHorario:   return "No se puede hacer a esta hora";
        case EstadoActividad::LimiteAlcanzado:  return "Ya lo hiciste el maximo de veces hoy";
        case EstadoActividad::SinDinero:        return "No tienes dinero suficiente";
        case EstadoActividad::SinTiempo:        return "No hay tiempo ahora";
        case EstadoActividad::NoExiste:         return "Actividad desconocida";
        default:                                return "";
    }
}