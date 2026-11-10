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

        const std::size_t comentario = linea.find('#');
        if (comentario != std::string::npos)
            linea.erase(comentario);

        std::istringstream ss(linea);
        std::string escena, bloquesTxt, diasTxt, sobrante;

        if (!(ss >> escena))
            continue;

        if (!(ss >> bloquesTxt >> diasTxt) || (ss >> sobrante))
        {
            std::cerr << "[Fases] Linea " << numeroLinea
                      << ": se esperaban 3 columnas (escena bloques dias)\n";
            todoBien = false;
            continue;
        }

        Regla regla{escena, 0, 0};

        if (!leerMascara(bloquesTxt, static_cast<int>(Bloque::Cantidad), leerBloque, regla.bloques))
        {
            std::cerr << "[Fases] Linea " << numeroLinea << ": bloques invalidos '" << bloquesTxt << "'\n";
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

bool Fases::permitidaEn(const std::string& escena, Bloque bloque, Dia dia) const
{
    const Regla sonda{escena, 0, 0};
    const Regla* regla = reglas_.search(sonda);

    if (regla == nullptr)
        return true;

    const bool bloqueAbierto = (regla->bloques & (1 << static_cast<int>(bloque))) != 0;
    const bool diaAbierto    = (regla->dias    & (1 << static_cast<int>(dia)))    != 0;
    return bloqueAbierto && diaAbierto;
}

bool Fases::escenaPermitida(const std::string& escena) const
{
    if (tiempo_.enFinDeSemana() || tiempo_.semestreTerminado())
        return false;

    return permitidaEn(escena, tiempo_.bloque(), tiempo_.dia());
}

void Fases::limpiar()
{
    reglas_.clear();
}