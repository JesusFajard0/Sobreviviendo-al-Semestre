#pragma once

#include <SFML/Graphics.hpp>

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "world/DatosEscena.hpp"
#include "world/Puerta.hpp"

// Un escenario jugable, construido a partir de sus datos y rectángulos.
class Escena
{
public:
    Escena(const DatosEscena& datos,
           const std::vector<DatosRectangulo>& rectangulos,
           std::function<void()> alUsarPuerta);

    const std::string& id() const { return id_; }
    const sf::Vector2f& spawn() const { return spawn_; }

    const std::vector<sf::RectangleShape>& obstaculos() const
    {
        return obstaculos_;
    }

    // Devuelve la puerta de salida, o nullptr si la escena no tiene.
    Puerta* puerta() { return puerta_.get(); }

    void dibujar(sf::RenderWindow& ventana, const sf::Font& fuente) const;

private:
    std::string id_;
    sf::Vector2f spawn_;
    sf::RectangleShape fondo_;
    std::vector<sf::RectangleShape> obstaculos_;
    std::unique_ptr<Puerta> puerta_;
};