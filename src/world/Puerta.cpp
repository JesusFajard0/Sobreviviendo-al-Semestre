#include "world/Puerta.hpp"

#include <utility>

#include "core/CompatSFML.hpp"

Puerta::Puerta(const sf::FloatRect& area, std::function<void()> accion)
    : Interactuable(area),
      accion_(std::move(accion))
{
    forma_.setSize(tamanoRect(area));
    forma_.setPosition(posicionRect(area));
    forma_.setFillColor(sf::Color(110, 70, 30));
}

void Puerta::interactuar()
{
    if (accion_)
        accion_();
}

void Puerta::dibujar(sf::RenderWindow& ventana,
                     const sf::Font& fuente) const
{
    ventana.draw(forma_);

    // Dibuja el aviso "[E]" si el jugador está cerca.
    Interactuable::dibujar(ventana, fuente);
}