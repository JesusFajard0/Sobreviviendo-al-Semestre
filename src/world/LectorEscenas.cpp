
#include "world/LectorEscenas.hpp"

#include <cstdint>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace
{
    std::string limpiar(const std::string& texto)
    {
        const std::size_t inicio =
            texto.find_first_not_of(" \t\r\n");

        if (inicio == std::string::npos)
            return "";

        const std::size_t final =
            texto.find_last_not_of(" \t\r\n");

        return texto.substr(inicio, final - inicio + 1);
    }

    std::vector<std::string> separar(
        const std::string& linea,
        char separador
    )
    {
        std::vector<std::string> partes;
        std::stringstream flujo(linea);
        std::string parte;

        while (std::getline(flujo, parte, separador))
            partes.push_back(limpiar(parte));

        if (!linea.empty() && linea.back() == separador)
            partes.push_back("");

        return partes;
    }

    bool convertirEntero(const std::string& texto, int& resultado)
    {
        try
        {
            std::size_t posicion = 0;
            resultado = std::stoi(texto, &posicion);

            return posicion == texto.size();
        }
        catch (const std::exception&)
        {
            return false;
        }
    }

    bool convertirFloat(const std::string& texto, float& resultado)
    {
        try
        {
            std::size_t posicion = 0;
            resultado = std::stof(texto, &posicion);

            return posicion == texto.size();
        }
        catch (const std::exception&)
        {
            return false;
        }
    }

    bool leerColor(const std::string& texto, sf::Color& color)
    {
        const auto componentes = separar(texto, ',');

        if (componentes.size() != 3)
            return false;

        int rojo, verde, azul;

        if (!convertirEntero(componentes[0], rojo) ||
            !convertirEntero(componentes[1], verde) ||
            !convertirEntero(componentes[2], azul))
        {
            return false;
        }

        if (rojo < 0 || rojo > 255 ||
            verde < 0 || verde > 255 ||
            azul < 0 || azul > 255)
        {
            return false;
        }

        color = sf::Color(
            static_cast<std::uint8_t>(rojo),
            static_cast<std::uint8_t>(verde),
            static_cast<std::uint8_t>(azul)
        );

        return true;
    }

    bool leerEscena(
        const std::vector<std::string>& campos,
        DatosEscena& escena
    )
    {
        if (campos.size() != 10)
            return false;

        escena.id = campos[1];
        escena.nombre = campos[2];
        escena.padre = campos[3];
        escena.tipo = campos[5];
        escena.fondo = campos[6];

        if (escena.id.empty() || escena.nombre.empty())
            return false;

        if (!convertirEntero(campos[4], escena.faseMinima) ||
            escena.faseMinima < 1)
        {
            return false;
        }

        if (escena.tipo != "normal" &&
            escena.tipo != "animacion")
        {
            return false;
        }

        if (!leerColor(campos[7], escena.color))
            return false;

        if (!convertirFloat(campos[8], escena.spawn.x) ||
            !convertirFloat(campos[9], escena.spawn.y))
        {
            return false;
        }

        return true;
    }

    bool leerRectangulo(
        const std::vector<std::string>& campos,
        DatosRectangulo& rectangulo
    )
    {
        if (campos.size() != 6)
            return false;

        rectangulo.tipo = campos[0];
        rectangulo.idEscena = campos[1];

        if (rectangulo.idEscena.empty())
            return false;

        if (rectangulo.tipo != "rect_colision" &&
            rectangulo.tipo != "rect_salida")
        {
            return false;
        }

        if (!convertirFloat(campos[2], rectangulo.x) ||
            !convertirFloat(campos[3], rectangulo.y) ||
            !convertirFloat(campos[4], rectangulo.ancho) ||
            !convertirFloat(campos[5], rectangulo.alto))
        {
            return false;
        }

        if (rectangulo.ancho <= 0 || rectangulo.alto <= 0)
            return false;

        return true;
    }
}

bool LectorEscenas::cargarArchivo(
    const std::string& ruta,
    std::vector<DatosEscena>& escenas,
    std::vector<DatosRectangulo>& rectangulos
)
{
    std::ifstream archivo(ruta);

    if (!archivo.is_open())
    {
        std::cerr << "[LectorEscenas] No se pudo abrir: "
                  << ruta << '\n';
        return false;
    }

    std::vector<DatosEscena> escenasLeidas;
    std::vector<DatosRectangulo> rectangulosLeidos;

    std::string linea;
    int numeroLinea = 0;

    while (std::getline(archivo, linea))
    {
        ++numeroLinea;
        linea = limpiar(linea);

        if (linea.empty() || linea[0] == '#')
            continue;

        const auto campos = separar(linea, '|');

        bool correcto = false;

        if (!campos.empty() && campos[0] == "escena")
        {
            DatosEscena escena;
            correcto = leerEscena(campos, escena);

            if (correcto)
                escenasLeidas.push_back(escena);
        }
        else if (!campos.empty() &&
                 (campos[0] == "rect_colision" ||
                  campos[0] == "rect_salida"))
        {
            DatosRectangulo rectangulo;
            correcto = leerRectangulo(campos, rectangulo);

            if (correcto)
                rectangulosLeidos.push_back(rectangulo);
        }

        if (!correcto)
        {
            std::cerr << "[LectorEscenas] Registro invalido en linea "
                      << numeroLinea << ": " << linea << '\n';
            return false;
        }
    }

    if (escenasLeidas.empty())
    {
        std::cerr << "[LectorEscenas] El archivo no contiene escenas.\n";
        return false;
    }

    escenas = std::move(escenasLeidas);
    rectangulos = std::move(rectangulosLeidos);

    std::cout << "[LectorEscenas] Escenas leidas: "
              << escenas.size() << '\n';

    std::cout << "[LectorEscenas] Rectangulos leidos: "
              << rectangulos.size() << '\n';

    return true;
}
