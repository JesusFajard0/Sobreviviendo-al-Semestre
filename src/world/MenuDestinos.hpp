#pragma once

#include <SFML/Graphics.hpp>

#include <functional>
#include <string>
#include <vector>

#include "core/State.hpp"
#include "world/MapaEscenas.hpp"

// Menú superpuesto con los destinos de la puerta.
// W/S o flechas: mover. Enter: elegir. Esc: cerrar.
class MenuDestinos : public State
{
public:
    MenuDestinos(StateStack* pila,
                 const sf::Font& fuente,
                 std::vector<Destino> destinos,
                 std::function<void(const std::string&)> alElegir);

    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;

private:
    void elegir();

    const sf::Font& fuente_;
    std::vector<Destino> destinos_;
    std::function<void(const std::string&)> alElegir_;
    int seleccion_;
};