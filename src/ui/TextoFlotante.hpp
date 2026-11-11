#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <cstdint>

struct TextoFlotante {
    sf::Text texto;
    sf::Vector2f posicion;
    float duracion;
    float velocidadY;
    sf::Color colorBase;

    TextoFlotante(const std::string& msg, sf::Vector2f pos, const sf::Font& fuente, sf::Color color, unsigned tamaño = 14)
        // Inicializamos el texto directamente aquí para SFML 3
        : texto(fuente, msg, tamaño), 
          posicion(pos), 
          duracion(1.8f), 
          velocidadY(-25.f), 
          colorBase(color) 
    {
        // Ya no seteamos fuente, string y size aquí porque ya se hizo arriba.
        // Solo falta el color y la posición.
        texto.setFillColor(color);
        texto.setPosition(posicion);
    }

    bool actualizar(float dt) {
        duracion -= dt;
        if (duracion <= 0.f) return false;

        posicion.y += velocidadY * dt;
        texto.setPosition(posicion);

        // Desvanecimiento gradual (fade out)
        int alpha = static_cast<int>((duracion / 1.8f) * 255.f);
        if (alpha < 0) alpha = 0;
        sf::Color c = colorBase;
        c.a = static_cast<uint8_t>(alpha);
        texto.setFillColor(c);

        return true;
    }
};