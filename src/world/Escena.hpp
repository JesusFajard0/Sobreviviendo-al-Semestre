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

    // Área por la que puede caminar el personaje.
    sf::FloatRect limites() const;

    const std::vector<sf::RectangleShape>& obstaculos() const
    {
        return obstaculos_;
    }

    // Devuelve la puerta de salida, o nullptr si la escena no tiene.
    Puerta* puerta() { return puerta_.get(); }

    // Pone la imagen de fondo (assets/backgrounds/<id>.png).
    void ponerFondo(const sf::Texture& textura);

    // Con verColisiones = true se dibujan encima del arte las colisiones y la puerta.
    void dibujar(sf::RenderWindow& ventana, const sf::Font& fuente,
                 bool verColisiones = false) const;

private:
    std::string id_;
    sf::Vector2f spawn_;
    sf::RectangleShape fondo_;
    bool tieneFondo_ = false;
    std::vector<sf::RectangleShape> obstaculos_;
    std::unique_ptr<Puerta> puerta_;
};