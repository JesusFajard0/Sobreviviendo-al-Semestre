#include "world/LectorNpcs.hpp"

#include <cctype>
#include <cstdlib>
#include <fstream>
#include <iostream>

namespace
{
    constexpr int N_COLUMNAS = 8;   // npc | escena | nombre | dialogo | x | y | ancho | alto

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
}

bool LectorNpcs::cargarArchivo(const std::string& ruta, std::vector<DatosNpc>& npcs)
{
    std::ifstream archivo(ruta);
    if (!archivo.is_open())
    {
        std::cerr << "[LectorNpcs] No se pudo abrir '" << ruta << "'\n";
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

        if (static_cast<int>(c.size()) != N_COLUMNAS || c[0] != "npc")
        {
            std::cerr << "[LectorNpcs] Linea " << numeroLinea
                      << ": se esperaba 'npc | escena | nombre | dialogo | x | y | ancho | alto'\n";
            todoBien = false;
            continue;
        }

        DatosNpc d{};
        d.escena  = c[1];
        d.nombre  = c[2];
        d.dialogo = c[3];

        const bool bien =
            !d.escena.empty() && !d.nombre.empty() && !d.dialogo.empty() &&
            leerFloat(c[4], d.x) &&
            leerFloat(c[5], d.y) &&
            leerFloat(c[6], d.ancho) &&
            leerFloat(c[7], d.alto) &&
            d.ancho > 0.f && d.alto > 0.f;

        if (!bien)
        {
            std::cerr << "[LectorNpcs] Linea " << numeroLinea << ": valores invalidos\n";
            todoBien = false;
            continue;
        }

        npcs.push_back(d);
    }

    return todoBien;
}
