#include "systems/FinDeSemana.hpp"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <vector>

namespace
{
    constexpr int   N_COLUMNAS = 11;
    constexpr int   N_TIPOS    = static_cast<int>(TipoFinDeSemana::Cantidad);

    // Valores de partida para probar; se pueden balancear.
    constexpr float ORDEN_INICIAL      = 80.f;
    constexpr float ORDEN_POR_DIA_SUCIO = -10.f;   // dia sin lavar platos
    constexpr float ORDEN_AL_LAVAR     = 30.f;

    // Cuanto pesa cada estadistica en el sorteo (0..100 se reparte en estos tramos).
    constexpr float TRAMO_STAT      = 25.f;   // dopamina / orden / conocimiento: hasta +4
    constexpr float TRAMO_ESTRES    = 50.f;   // estres: hasta +2 en el aseo
    constexpr float TRAMO_SABER     = 50.f;   // conocimiento: hasta +2 en el estudio (mas bajo = estudiar domina)
    constexpr float EXTRA_DISCOTECA = 2.f;    // fue a la discoteca el viernes -> rumba
    constexpr float EXTRA_EXAMEN    = 2.f;    // hay examen la semana siguiente -> estudio

    std::string recortar(const std::string& s)
    {
        std::size_t ini = 0, fin = s.size();
        while (ini < fin && std::isspace(static_cast<unsigned char>(s[ini]))) ++ini;
        while (fin > ini && std::isspace(static_cast<unsigned char>(s[fin - 1]))) --fin;
        return s.substr(ini, fin - ini);
    }

    std::vector<std::string> dividir(const std::string& texto, char sep)
    {
        std::vector<std::string> partes;
        std::size_t inicio = 0;
        while (true)
        {
            const std::size_t pos = texto.find(sep, inicio);
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
        if (texto.empty()) return false;
        char* fin = nullptr;
        const double v = std::strtod(texto.c_str(), &fin);
        if (fin == texto.c_str() || *fin != '\0') return false;
        valor = static_cast<float>(v);
        return true;
    }

    int indicePorId(const std::string& id)
    {
        if (id == "rumba")   return static_cast<int>(TipoFinDeSemana::Rumba);
        if (id == "aseo")    return static_cast<int>(TipoFinDeSemana::Aseo);
        if (id == "estudio") return static_cast<int>(TipoFinDeSemana::Estudio);
        return -1;
    }
}

FinDeSemana::FinDeSemana()
    : azar_(std::random_device{}())
{
    reiniciar();
}

void FinDeSemana::reiniciar()
{
    ordenCasa_    = ORDEN_INICIAL;
    lavoHoy_      = false;
    fueDiscoteca_ = false;
}

// ---------------------------------------------------------------
// Carga
// ---------------------------------------------------------------

bool FinDeSemana::cargarDesdeArchivo(const std::string& ruta)
{
    std::ifstream archivo(ruta);
    if (!archivo.is_open())
    {
        std::cerr << "[FinDeSemana] No se pudo abrir '" << ruta << "'\n";
        return false;
    }

    cargado_.fill(false);

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
            std::cerr << "[FinDeSemana] Linea " << numeroLinea << ": se esperaban "
                      << N_COLUMNAS << " columnas y hay " << c.size() << "\n";
            todoBien = false;
            continue;
        }

        const int i = indicePorId(c[0]);
        DatosFinDeSemana d;
        d.id     = c[0];
        d.nombre = c[1];
        d.texto  = c[10];

        const bool bien =
            i >= 0 && !d.nombre.empty() &&
            leerFloat(c[2],  d.deltaEnergia) &&
            leerFloat(c[3],  d.deltaConocimiento) &&
            leerFloat(c[4],  d.deltaSaludMental) &&
            leerFloat(c[5],  d.deltaEstres) &&
            leerFloat(c[6],  d.deltaDopamina) &&
            leerFloat(c[7],  d.deltaDinero) &&
            leerFloat(c[8],  d.deltaOrden) &&
            leerFloat(c[9],  d.pesoBase) &&
            d.pesoBase > 0.f;

        if (!bien)
        {
            std::cerr << "[FinDeSemana] Linea " << numeroLinea
                      << ": valores invalidos en '" << c[0] << "'\n";
            todoBien = false;
            continue;
        }

        if (cargado_[i])
        {
            std::cerr << "[FinDeSemana] Linea " << numeroLinea << ": '" << c[0]
                      << "' esta repetida\n";
            todoBien = false;
            continue;
        }

        datos_[i]   = std::move(d);
        cargado_[i] = true;
    }

    for (int i = 0; i < N_TIPOS; ++i)
    {
        if (!cargado_[i])
        {
            std::cerr << "[FinDeSemana] Falta la opcion '"
                      << nombre(static_cast<TipoFinDeSemana>(i)) << "' en '" << ruta << "'\n";
            todoBien = false;
        }
    }

    return todoBien;
}

// ---------------------------------------------------------------
// Durante la semana
// ---------------------------------------------------------------

void FinDeSemana::registrarActividad(const std::string& idActividad)
{
    if (idActividad == "lavar_platos")
    {
        lavoHoy_   = true;
        ordenCasa_ = std::clamp(ordenCasa_ + ORDEN_AL_LAVAR, 0.f, 100.f);
    }
    else if (idActividad == "discoteca")
    {
        fueDiscoteca_ = true;
    }
}

void FinDeSemana::cerrarDia()
{
    if (!lavoHoy_)
        ordenCasa_ = std::clamp(ordenCasa_ + ORDEN_POR_DIA_SUCIO, 0.f, 100.f);
    lavoHoy_ = false;
}

// ---------------------------------------------------------------
// Sorteo
// ---------------------------------------------------------------

std::array<float, N_TIPOS>
FinDeSemana::pesos(const Estadisticas& est, int semanaQueViene, Materia materiaSiguiente) const
{
    // Hay examen la semana que viene?
    bool examenCerca = false;
    for (int m = 0; m < static_cast<int>(Materia::Cantidad); ++m)
        if (Tiempo::semanaExamen(static_cast<Materia>(m)) == semanaQueViene)
            examenCerca = true;

    const float dopamina = est.obtener(Stat::Dopamina);
    const float estres   = est.obtener(Stat::Estres);
    const float saber    = est.conocimiento(materiaSiguiente);

    std::array<float, N_TIPOS> w{};

    w[static_cast<int>(TipoFinDeSemana::Rumba)] =
        datos_[static_cast<int>(TipoFinDeSemana::Rumba)].pesoBase
        + (100.f - dopamina) / TRAMO_STAT
        + (fueDiscoteca_ ? EXTRA_DISCOTECA : 0.f);

    w[static_cast<int>(TipoFinDeSemana::Aseo)] =
        datos_[static_cast<int>(TipoFinDeSemana::Aseo)].pesoBase
        + (100.f - ordenCasa_) / TRAMO_STAT
        + estres / TRAMO_ESTRES;

    w[static_cast<int>(TipoFinDeSemana::Estudio)] =
        datos_[static_cast<int>(TipoFinDeSemana::Estudio)].pesoBase
        + (100.f - saber) / TRAMO_SABER
        + (examenCerca ? EXTRA_EXAMEN : 0.f);

    return w;
}

ResultadoFinDeSemana FinDeSemana::resolver(Estadisticas& est, int semanaQueViene,
                                           Materia materiaSiguiente)
{
    const auto w = pesos(est, semanaQueViene, materiaSiguiente);
    std::discrete_distribution<int> dado(w.begin(), w.end());
    const int i = dado(azar_);
    const DatosFinDeSemana& d = datos_[i];

    ResultadoFinDeSemana r;
    r.tipo   = static_cast<TipoFinDeSemana>(i);
    r.nombre = d.nombre;
    r.texto  = d.texto;

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

    for (int s = 0; s < static_cast<int>(Stat::Cantidad); ++s)
        r.cambios[s] = est.modificar(static_cast<Stat>(s), deltas[s]);

    if (d.deltaConocimiento != 0.f)
        r.cambioConocimiento = est.modificarConocimiento(materiaSiguiente, d.deltaConocimiento);

    const float ordenAntes = ordenCasa_;
    ordenCasa_      = std::clamp(ordenCasa_ + d.deltaOrden, 0.f, 100.f);
    r.cambioOrden   = ordenCasa_ - ordenAntes;

    fueDiscoteca_ = false;   // ya cuento para este sorteo
    lavoHoy_      = false;
    return r;
}

const char* FinDeSemana::nombre(TipoFinDeSemana t)
{
    switch (t)
    {
        case TipoFinDeSemana::Rumba:   return "rumba";
        case TipoFinDeSemana::Aseo:    return "aseo";
        case TipoFinDeSemana::Estudio: return "estudio";
        default:                       return "?";
    }
}
