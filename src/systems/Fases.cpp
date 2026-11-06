#include "systems/Fases.hpp"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <sstream>
#include <utility>
#include <vector>

namespace
{
    std::string minusculas(std::string s)
    {
        std::transform(s.begin(), s.end(), s.begin(),
                       [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        return s;
    }

    std::vector<std::string> separar(const std::string& texto, char separador)
    {
        std::vector<std::string> partes;
        std::string actual;
        std::istringstream ss(texto);
        while (std::getline(ss, actual, separador))
            partes.push_back(actual);
        return partes;
    }

    bool leerFase(const std::string& texto, int& indice)
    {
        const std::string t = minusculas(texto);
        for (int i = 0; i < static_cast<int>(Fase::Cantidad); ++i)
        {
            if (t == minusculas(Tiempo::nombre(static_cast<Fase>(i))))
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
        for (const std::string& parte : separar(texto, ','))
        {
            int indice = 0;
            if (!leer(parte, indice))
                return false;
            mascara = static_cast<std::uint8_t>(mascara | (1 << indice));
        }
        return mascara != 0;
    }
}

Fases::Fases(const Tiempo& tiempo)
    : tiempo_(tiempo),
      reglas_(IgualEscena(), &hashRegla)
{
}

// ---------------------------------------------------------------
// Carga
// ---------------------------------------------------------------

bool Fases::cargarDesdeArchivo(const std::string& ruta)
{
    std::ifstream archivo(ruta);
    if (!archivo.is_open())
    {
        std::cerr << "[Fases] No se pudo abrir '" << ruta << "'\n";
        return false;
    }

    bool todoBien = true;
    int numeroLinea = 0;
    std::string linea;

    while (std::getline(archivo, linea))
    {
        ++numeroLinea;

        // Se ignora lo que venga después de '#'.
        const std::size_t comentario = linea.find('#');
        if (comentario != std::string::npos)
            linea.erase(comentario);

        std::istringstream ss(linea);
        std::string escena, fasesTxt, diasTxt, sobrante;

        if (!(ss >> escena))
            continue;   // línea vacía o solo comentario

        if (!(ss >> fasesTxt >> diasTxt) || (ss >> sobrante))
        {
            std::cerr << "[Fases] Linea " << numeroLinea
                      << ": se esperaban 3 columnas (escena fases dias)\n";
            todoBien = false;
            continue;
        }

        Regla regla{escena, 0, 0};

        if (!leerMascara(fasesTxt, static_cast<int>(Fase::Cantidad), leerFase, regla.fases))
        {
            std::cerr << "[Fases] Linea " << numeroLinea << ": fases invalidas '" << fasesTxt << "'\n";
            todoBien = false;
            continue;
        }

        if (!leerMascara(diasTxt, static_cast<int>(Dia::Cantidad), leerDia, regla.dias))
        {
            std::cerr << "[Fases] Linea " << numeroLinea << ": dias invalidos '" << diasTxt << "'\n";
            todoBien = false;
            continue;
        }

        if (reglas_.search(regla) != nullptr)
        {
            std::cerr << "[Fases] Linea " << numeroLinea << ": la escena '" << escena
                      << "' ya tiene regla\n";
            todoBien = false;
            continue;
        }

        reglas_.insert(std::move(regla));
    }

    return todoBien;
}

// ---------------------------------------------------------------
// Consultas
// ---------------------------------------------------------------

bool Fases::permitidaEn(const std::string& escena, Fase fase, Dia dia) const
{
    const Regla sonda{escena, 0, 0};
    const Regla* regla = reglas_.search(sonda);

    // Sin regla: siempre abierta.
    if (regla == nullptr)
        return true;

    const bool faseAbierta = (regla->fases & (1 << static_cast<int>(fase))) != 0;
    const bool diaAbierto  = (regla->dias  & (1 << static_cast<int>(dia)))  != 0;
    return faseAbierta && diaAbierto;
}

bool Fases::escenaPermitida(const std::string& escena) const
{
    if (tiempo_.enFinDeSemana() || tiempo_.semestreTerminado())
        return false;

    return permitidaEn(escena, tiempo_.fase(), tiempo_.dia());
}

void Fases::limpiar()
{
    reglas_.clear();
}