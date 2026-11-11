#include "world/LectorObjetos.hpp"

#include <cctype>
#include <cstdlib>
#include <fstream>
#include <iostream>

namespace
{
    constexpr int N_COLUMNAS = 7;

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
}

bool LectorObjetos::cargarArchivo(
    const std::string& ruta,
    std::vector<DatosObjeto>& objetos)
{
    std::ifstream archivo(ruta);
    if (!archivo.is_open())
    {
        std::cerr << "[LectorObjetos] No se pudo abrir '" << ruta << "'\n";
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

        if (static_cast<int>(c.size()) != N_COLUMNAS || c[0] != "objeto")
        {
            std::cerr << "[LectorObjetos] Linea " << numeroLinea
                      << ": se esperaba 'objeto | escena | actividad | x | y | ancho | alto'\n";
            todoBien = false;
            continue;
        }

        DatosObjeto d{};
        d.escena    = c[1];
        d.actividad = c[2];

        const bool bien =
            !d.escena.empty() && !d.actividad.empty() &&
            leerFloat(c[3], d.x) &&
            leerFloat(c[4], d.y) &&
            leerFloat(c[5], d.ancho) &&
            leerFloat(c[6], d.alto) &&
            d.ancho > 0.f && d.alto > 0.f;

        if (!bien)
        {
            std::cerr << "[LectorObjetos] Linea " << numeroLinea
                      << ": valores invalidos\n";
            todoBien = false;
            continue;
        }

        objetos.push_back(d);
    }

    return todoBien;
}