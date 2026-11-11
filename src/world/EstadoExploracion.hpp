#pragma once

#include <SFML/Graphics.hpp>

#include <memory>
#include <string>

#include "core/GestorRecursos.hpp"
#include "core/State.hpp"
#include "world/Escena.hpp"
#include "world/MapaEscenas.hpp"
#include "world/Personaje.hpp"
#include "systems/Estadisticas.hpp"
#include "systems/Tiempo.hpp"
#include "ui/Hud.hpp"

// Estado donde el jugador camina por las escenas.
class EstadoExploracion : public State
{
public:
        EstadoExploracion(StateStack* pila, GestorRecursos& recursos, Estadisticas& estadisticas, Tiempo& tiempo);

    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;

private:
    void cambiarEscena(const std::string& id);
    void abrirMenuDestinos();
    const sf::Font& fuente() const;
        // TEMPORAL (S1-7): borrar antes del Pull Request.
    void teclaDepuracion(sf::Keyboard::Key tecla, bool fino);

    GestorRecursos& recursos_;
    Estadisticas&   estadisticas_;
    Tiempo&         tiempo_;
    MapaEscenas mapa_;
    std::unique_ptr<Escena> escena_;
    Personaje personaje_;

    Hud hud_;

    // Ventana donde se dibujó por última vez: hace falta para convertir
    // la posición del ratón (píxeles) a coordenadas lógicas del HUD.
    sf::RenderWindow* ventana_ = nullptr;


    // Fase actual del semestre (1 = Cálculo, 2 = Física, 3 = Programación).
    // Provisional: fija en 1 hasta que exista el sistema de tiempo.
    int fase_;
};