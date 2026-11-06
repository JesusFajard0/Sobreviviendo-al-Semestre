#pragma once

#include <SFML/Graphics.hpp>

#include <functional>

#include "world/Interactuable.hpp"

// Puerta de salida de una escena. Al interactuar ejecuta una acción
// (abrir el menú de destinos).
class Puerta : public Interactuable
{
public:
    Puerta(const sf::FloatRect& area, std::function<void()> accion);

    void interactuar() override;

    void dibujar(sf::RenderWindow& ventana,
                 const sf::Font& fuente) const override;

private:
    std::function<void()> accion_;
    sf::RectangleShape forma_;
};