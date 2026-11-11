#pragma once

#include <SFML/Graphics.hpp>

#include <functional>
#include <string>

#include "world/Interactuable.hpp"

// Objeto de una escena que permite hacer una actividad (cama, computadora...).
// Al interactuar avisa con el id de la actividad; quien lo creó decide qué
// pasa (igual que Puerta).
class ObjetoActividad : public Interactuable
{
public:
    ObjetoActividad(const sf::FloatRect& area,
                    const std::string& idActividad,
                    const std::string& etiqueta,
                    std::function<void(const std::string&)> alUsar);

    void interactuar() override;

    void dibujar(sf::RenderWindow& ventana,
                 const sf::Font& fuente) const override;

    const std::string& idActividad() const { return idActividad_; }

private:
    std::string idActividad_;
    std::string etiqueta_;
    std::function<void(const std::string&)> alUsar_;
    sf::RectangleShape forma_;
};