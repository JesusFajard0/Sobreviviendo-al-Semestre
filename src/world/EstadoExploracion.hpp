#pragma once

#include <SFML/Graphics.hpp>

#include <memory>
#include <string>

#include "core/GestorRecursos.hpp"
#include "core/State.hpp"
#include "world/Escena.hpp"
#include "world/MapaEscenas.hpp"
#include "world/Personaje.hpp"

// Estado donde el jugador camina por las escenas.
class EstadoExploracion : public State
{
public:
    EstadoExploracion(StateStack* pila, GestorRecursos& recursos);

    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;

private:
    void cambiarEscena(const std::string& id);
    void abrirMenuDestinos();
    const sf::Font& fuente() const;

    GestorRecursos& recursos_;
    MapaEscenas mapa_;
    std::unique_ptr<Escena> escena_;
    Personaje personaje_;

    // Fase actual del semestre (1 = Cálculo, 2 = Física, 3 = Programación).
    // Provisional: fija en 1 hasta que exista el sistema de tiempo.
    int fase_;
};