
#pragma once

#include <string>
#include <SFML/Graphics.hpp>

struct DatosEscena
{
    std::string id;
    std::string nombre;
    std::string padre;

    int faseMinima;

    std::string tipo;
    std::string fondo;

    sf::Color color;

    sf::Vector2f spawn;
};

struct DatosRectangulo
{
    std::string tipo;
    std::string idEscena;

    float x;
    float y;
    float ancho;
    float alto;
};