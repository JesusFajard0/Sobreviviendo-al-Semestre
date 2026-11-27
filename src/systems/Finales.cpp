#include "systems/Finales.hpp"

#include <cctype>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>

namespace
{
    constexpr int COLUMNAS_FINAL  = 6;   // final | id | nombre | imagen | condiciones | texto
    constexpr int COLUMNAS_UMBRAL = 3;   // umbral | nombre | valor

    // Las unicas metricas que se pueden usar en las condiciones y en los textos.
    const char* const NOMBRES_METRICAS[] = {
        "aprobadas", "no_presentadas", "promedio",
        "nota_calculo", "nota_fisica", "nota_programacion",
        "salud_mental", "dopamina", "dinero", "energia", "estres",
        "asistencias", "faltas", "fiestas", "platos", "noches_sin_dormir",
        "estadisticas_buenas"
    };

    // Umbrales que usa el calculo de "estadisticas_buenas".
    const char* const UMBRAL_SALUD    = "salud_buena";
    const char* const UMBRAL_DOPAMINA = "dopamina_buena";
    const char* const UMBRAL_NOCHES   = "noches_maximas";

    bool metricaValida(const std::string& nombre)
    {
        for (const char* n : NOMBRES_METRICAS)
        {
            if (nombre == n)
                return true;
        }
        return false;
    }

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

    // Quita un comentario al final de la linea ("valor  # nota").
    std::string sinComentario(const std::string& s)
    {
        const std::size_t pos = s.find('#');
        return recortar(pos == std::string::npos ? s : s.substr(0, pos));
    }

    // Divide por el separador conservando los campos vacios.
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

    // En el archivo el salto de linea se escribe \n (dos caracteres).
    std::string conSaltos(const std::string& s)
    {
        std::string r;
        for (std::size_t i = 0; i < s.size(); ++i)
        {
            if (s[i] == '\\' && i + 1 < s.size() && s[i + 1] == 'n')
            {
                r += '\n';
                ++i;
            }
            else
            {
                r += s[i];
            }
        }
        return r;
    }

    bool leerNumero(const std::string& texto, double& valor)
    {
        if (texto.empty())
            return false;
        char* fin = nullptr;
        const double v = std::strtod(texto.c_str(), &fin);
        if (fin == texto.c_str() || *fin != '\0')
            return false;
        valor = v;
        return true;
    }

    bool esNombre(const std::string& s)
    {
        if (s.empty())
            return false;
        for (const char c : s)
        {
            if (!std::isalnum(static_cast<unsigned char>(c)) && c != '_')
                return false;
        }
        return true;
    }

    // "salud_mental<=15" o "promedio>=promedio_alto" -> metrica, operador y
    // valor (numero o nombre de umbral). Los operadores de dos caracteres se
    // buscan primero para no confundir ">=" con ">".
    bool leerCondicion(const std::string& texto, Finales::Condicion& c)
    {
        struct Op { const char* simbolo; Finales::Operador operador; };
        static const Op OPERADORES[] = {
            {">=", Finales::Operador::MayorIgual}, {"<=", Finales::Operador::MenorIgual},
            {"==", Finales::Operador::Igual},      {"!=", Finales::Operador::Distinto},
            {">",  Finales::Operador::Mayor},      {"<",  Finales::Operador::Menor}
        };

        for (const Op& op : OPERADORES)
        {
            const std::size_t pos = texto.find(op.simbolo);
            if (pos == std::string::npos)
                continue;

            c.metrica  = recortar(texto.substr(0, pos));
            c.operador = op.operador;
            c.valor    = 0.0;
            c.umbral.clear();

            const std::string derecha = recortar(texto.substr(pos + std::string(op.simbolo).size()));
            if (!metricaValida(c.metrica))
                return false;

            if (leerNumero(derecha, c.valor))
                return true;

            c.umbral = derecha;   // no es un numero: es el nombre de un umbral
            return esNombre(c.umbral);
        }
        return false;
    }
}

// ---------------------------------------------------------------
// Carga
// ---------------------------------------------------------------

bool Finales::cargarDesdeArchivo(const std::string& ruta)
{
    std::ifstream archivo(ruta);
    if (!archivo.is_open())
    {
        std::cerr << "[Finales] No se pudo abrir '" << ruta << "'\n";
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

        // ---- umbral | nombre | valor ----
        if (c[0] == "umbral")
        {
            double valor = 0.0;
            if (static_cast<int>(c.size()) != COLUMNAS_UMBRAL || !esNombre(c[1]) ||
                !leerNumero(sinComentario(c[2]), valor))
            {
                std::cerr << "[Finales] Linea " << numeroLinea
                          << ": se esperaba 'umbral | nombre | numero'\n";
                todoBien = false;
                continue;
            }
            if (umbrales_.count(c[1]) != 0)
            {
                std::cerr << "[Finales] Linea " << numeroLinea << ": el umbral '"
                          << c[1] << "' esta repetido\n";
                todoBien = false;
                continue;
            }
            umbrales_[c[1]] = valor;
            continue;
        }

        // ---- final | id | nombre | imagen | condiciones | texto ----
        if (c[0] != "final" || static_cast<int>(c.size()) != COLUMNAS_FINAL)
        {
            std::cerr << "[Finales] Linea " << numeroLinea
                      << ": se esperaba 'umbral' con " << COLUMNAS_UMBRAL
                      << " columnas o 'final' con " << COLUMNAS_FINAL << "\n";
            todoBien = false;
            continue;
        }

        Regla regla;
        regla.id     = c[1];
        regla.nombre = conSaltos(c[2]);
        regla.imagen = c[3];
        regla.texto  = conSaltos(c[5]);

        bool bien = esNombre(regla.id) && !regla.nombre.empty() && !regla.texto.empty();

        // "-" o vacio = siempre; si no, condiciones separadas por &.
        if (bien && !c[4].empty() && c[4] != "-")
        {
            for (const std::string& parte : dividir(c[4], '&'))
            {
                Condicion cond{};
                if (!leerCondicion(parte, cond))
                {
                    bien = false;
                    break;
                }
                regla.condiciones.push_back(cond);
            }
        }

        if (!bien)
        {
            std::cerr << "[Finales] Linea " << numeroLinea << ": valores invalidos en '"
                      << c[1] << "'\n";
            todoBien = false;
            continue;
        }

        bool repetido = false;
        for (const Regla& r : reglas_)
        {
            if (r.id == regla.id)
                repetido = true;
        }
        if (repetido)
        {
            std::cerr << "[Finales] Linea " << numeroLinea << ": el final '"
                      << regla.id << "' esta repetido\n";
            todoBien = false;
            continue;
        }

        reglas_.push_back(std::move(regla));
    }

    // Una regla que nombra un umbral que no existe no se puede evaluar: se descarta.
    for (auto it = reglas_.begin(); it != reglas_.end();)
    {
        std::string faltante;
        for (const Condicion& cond : it->condiciones)
        {
            if (!cond.umbral.empty() && umbrales_.count(cond.umbral) == 0)
                faltante = cond.umbral;
        }

        if (!faltante.empty())
        {
            std::cerr << "[Finales] El final '" << it->id << "' usa el umbral '"
                      << faltante << "' que no esta definido: se descarta\n";
            todoBien = false;
            it = reglas_.erase(it);
        }
        else
        {
            ++it;
        }
    }

    // "estadisticas_buenas" necesita estos tres umbrales.
    for (const char* nombre : {UMBRAL_SALUD, UMBRAL_DOPAMINA, UMBRAL_NOCHES})
    {
        if (umbrales_.count(nombre) == 0)
        {
            std::cerr << "[Finales] Falta el umbral '" << nombre
                      << "' (hace falta para saber si las estadisticas son buenas)\n";
            todoBien = false;
        }
    }

    // Cada partida debe caer en exactamente un final.
    std::string ejemplo;
    const int malas = revisarCobertura(&ejemplo);
    if (malas > 0)
    {
        std::cerr << "[Finales] Aviso: " << malas << " combinaciones no caen en exactamente "
                  << "un final. Primera: " << ejemplo << "\n";
        todoBien = false;
    }

    return todoBien;
}

// ---------------------------------------------------------------
// Evaluacion
// ---------------------------------------------------------------

double Finales::umbral(const std::string& nombre) const
{
    const auto it = umbrales_.find(nombre);
    return it != umbrales_.end() ? it->second : 0.0;
}

double Finales::valorDe(const Condicion& c) const
{
    return c.umbral.empty() ? c.valor : umbral(c.umbral);
}

bool Finales::cumple(const Condicion& c, const Metricas& m) const
{
    const auto it = m.find(c.metrica);
    if (it == m.end())
        return false;

    const double v = it->second;
    const double limite = valorDe(c);

    switch (c.operador)
    {
        case Operador::Mayor:      return v >  limite;
        case Operador::MayorIgual: return v >= limite;
        case Operador::Menor:      return v <  limite;
        case Operador::MenorIgual: return v <= limite;
        case Operador::Igual:      return v == limite;
        case Operador::Distinto:   return v != limite;
    }
    return false;
}

Finales::Metricas Finales::metricas(const Notas& notas, const Estadisticas& est,
                                    const DatosPartida& datos) const
{
    int noPresentadas = 0;
    for (int i = 0; i < ResumenSemestre::N; ++i)
    {
        if (notas.estado(static_cast<Materia>(i)) == EstadoExamen::NoPresento)
            ++noPresentadas;
    }

    const double salud    = est.obtener(Stat::SaludMental);
    const double dopamina = est.obtener(Stat::Dopamina);

    Metricas m = {
        {"aprobadas",         static_cast<double>(notas.materiasAprobadas())},
        {"no_presentadas",    static_cast<double>(noPresentadas)},
        {"promedio",          static_cast<double>(notas.promedio())},
        {"nota_calculo",      static_cast<double>(notas.nota(Materia::Calculo))},
        {"nota_fisica",       static_cast<double>(notas.nota(Materia::Fisica))},
        {"nota_programacion", static_cast<double>(notas.nota(Materia::Programacion))},
        {"salud_mental",      salud},
        {"dopamina",          dopamina},
        {"dinero",            static_cast<double>(est.obtener(Stat::Dinero))},
        {"energia",           static_cast<double>(est.obtener(Stat::Energia))},
        {"estres",            static_cast<double>(est.obtener(Stat::Estres))},
        {"asistencias",       static_cast<double>(datos.asistencias)},
        {"faltas",            static_cast<double>(datos.faltas)},
        {"fiestas",           static_cast<double>(datos.fiestas)},
        {"platos",            static_cast<double>(datos.platos)},
        {"noches_sin_dormir", static_cast<double>(datos.nochesSinDormir)}
    };

    // Las tres cosas a la vez; si falla cualquiera, las estadisticas son malas.
    const bool buenas = salud    >= umbral(UMBRAL_SALUD) &&
                        dopamina >= umbral(UMBRAL_DOPAMINA) &&
                        datos.nochesSinDormir <= umbral(UMBRAL_NOCHES);
    m["estadisticas_buenas"] = buenas ? 1.0 : 0.0;

    return m;
}

int Finales::coincidencias(const Metricas& m) const
{
    int n = 0;
    for (const Regla& regla : reglas_)
    {
        bool todas = true;
        for (const Condicion& c : regla.condiciones)
        {
            if (!cumple(c, m))
            {
                todas = false;
                break;
            }
        }
        if (todas)
            ++n;
    }
    return n;
}

const Finales::Regla* Finales::elegir(const Metricas& m) const
{
    for (const Regla& regla : reglas_)
    {
        bool todas = true;
        for (const Condicion& c : regla.condiciones)
        {
            if (!cumple(c, m))
            {
                todas = false;
                break;
            }
        }

        if (todas)
            return &regla;
    }
    return nullptr;
}

int Finales::revisarCobertura(std::string* ejemplo) const
{
    // Valores alrededor de cada umbral (justo debajo, en el y justo encima) para
    // que un "mayor o igual" mal puesto se note.
    const double salud0 = umbral(UMBRAL_SALUD);
    const double dopa0  = umbral(UMBRAL_DOPAMINA);
    const double noch0  = umbral(UMBRAL_NOCHES);

    const std::vector<double> saluds   = {0, salud0 - 1, salud0, salud0 + 1, 100};
    const std::vector<double> dopaminas = {0, dopa0 - 1, dopa0, dopa0 + 1, 100};
    const std::vector<double> noches   = {0, noch0 - 1, noch0, noch0 + 1, 30};
    const std::vector<double> fiestas  = {0, 1, 2, 3, 4, 5, 6, 7, 10, 30};

    int malas = 0;

    for (int aprobadas = 0; aprobadas <= ResumenSemestre::N; ++aprobadas)
    {
        // Con las 3 aprobadas el promedio no baja del minimo de aprobar.
        const int promedioDesde = (aprobadas == ResumenSemestre::N)
            ? static_cast<int>(umbral("promedio_minimo")) : 0;

        for (int promedio = promedioDesde; promedio <= Notas::NOTA_MAXIMA; ++promedio)
        for (const double s : saluds)
        for (const double d : dopaminas)
        for (const double n : noches)
        for (const double f : fiestas)
        {
            Metricas m = {
                {"aprobadas", static_cast<double>(aprobadas)}, {"promedio", static_cast<double>(promedio)},
                {"salud_mental", s}, {"dopamina", d}, {"noches_sin_dormir", n}, {"fiestas", f},
                {"no_presentadas", 0}, {"nota_calculo", 0}, {"nota_fisica", 0}, {"nota_programacion", 0},
                {"dinero", 0}, {"energia", 0}, {"estres", 0}, {"asistencias", 0}, {"faltas", 0}, {"platos", 0}
            };
            m["estadisticas_buenas"] = (s >= salud0 && d >= dopa0 && n <= noch0) ? 1.0 : 0.0;

            const int hay = coincidencias(m);
            if (hay != 1)
            {
                if (malas == 0 && ejemplo != nullptr)
                {
                    *ejemplo = "aprobadas=" + std::to_string(aprobadas) +
                               " promedio=" + std::to_string(promedio) +
                               " salud=" + std::to_string(std::lround(s)) +
                               " dopamina=" + std::to_string(std::lround(d)) +
                               " noches=" + std::to_string(std::lround(n)) +
                               " fiestas=" + std::to_string(std::lround(f)) +
                               " -> " + std::to_string(hay) + " finales";
                }
                ++malas;
            }
        }
    }

    return malas;
}

std::string Finales::rellenar(const std::string& texto, const Metricas& m)
{
    std::string resultado;
    std::size_t i = 0;

    while (i < texto.size())
    {
        if (texto[i] == '{')
        {
            const std::size_t cierre = texto.find('}', i + 1);
            if (cierre != std::string::npos)
            {
                const auto it = m.find(texto.substr(i + 1, cierre - i - 1));
                if (it != m.end())
                {
                    resultado += std::to_string(std::lround(it->second));
                    i = cierre + 1;
                    continue;
                }
            }
        }
        resultado += texto[i];
        ++i;
    }

    return resultado;
}

ResumenSemestre Finales::resumir(const Notas& notas, const Estadisticas& est,
                                 const DatosPartida& datos) const
{
    ResumenSemestre r;

    for (int i = 0; i < ResumenSemestre::N; ++i)
    {
        const Materia m = static_cast<Materia>(i);
        r.nota[i]   = notas.nota(m);
        r.estado[i] = notas.estado(m);
        r.aprobada[i] = notas.aprobada(m);
    }

    r.promedio        = notas.promedio();
    r.aprobadas       = notas.materiasAprobadas();
    r.saludMental     = est.obtener(Stat::SaludMental);
    r.dopamina        = est.obtener(Stat::Dopamina);
    r.dinero          = est.obtener(Stat::Dinero);
    r.asistencias     = datos.asistencias;
    r.faltas          = datos.faltas;
    r.fiestas         = datos.fiestas;
    r.platos          = datos.platos;
    r.nochesSinDormir = datos.nochesSinDormir;

    const Metricas m = metricas(notas, est, datos);
    const Regla* regla = elegir(m);

    if (regla != nullptr)
    {
        r.idFinal     = regla->id;
        r.nombreFinal = rellenar(regla->nombre, m);
        r.textoFinal  = rellenar(regla->texto, m);
        r.imagenFinal = regla->imagen;
    }
    else
    {
        // Sin ninguna regla que cuadre (archivo vacio o mal hecho).
        r.nombreFinal = "Fin del semestre";
        r.textoFinal  = "Terminaste las semanas del semestre.";
    }

    return r;
}
