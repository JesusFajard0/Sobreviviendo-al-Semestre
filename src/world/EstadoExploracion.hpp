#pragma once

#include <SFML/Graphics.hpp>

#include <memory>
#include <string>
#include <vector>

#include "core/GestorRecursos.hpp"
#include "core/Partida.hpp"
#include "core/State.hpp"
#include "systems/Estadisticas.hpp"
#include "systems/Tiempo.hpp"
#include "ui/Hud.hpp"
#include "world/DatosObjeto.hpp"
#include "world/Escena.hpp"
#include "world/MapaEscenas.hpp"
#include "world/ObjetoActividad.hpp"
#include "world/Personaje.hpp"

// Estado donde el jugador camina por las escenas.
class EstadoExploracion : public State
{
public:
    EstadoExploracion(StateStack* pila, GestorRecursos& recursos, Partida& partida);

    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;

private:
    void cambiarEscena(const std::string& id);
    void abrirMenuDestinos();
    void crearObjetos(const std::string& idEscena);
    void usarActividad(const std::string& idActividad);
    const sf::Font& fuente() const;

    // TEMPORAL (S1-7): borrar antes del Pull Request.
    void teclaDepuracion(sf::Keyboard::Key tecla, bool fino);

    GestorRecursos& recursos_;
    Partida&        partida_;

    // Atajos a partida_.estadisticas y partida_.tiempo (el HUD los usa así).
    Estadisticas&   estadisticas_;
    Tiempo&         tiempo_;

    MapaEscenas mapa_;
    std::unique_ptr<Escena> escena_;
    Personaje personaje_;

    Hud hud_;

    // Ventana donde se dibujó por última vez: hace falta para convertir
    // la posición del ratón (píxeles) a coordenadas lógicas del HUD.
    sf::RenderWindow* ventana_ = nullptr;

    // Todos los objetos del juego (data/objetos.txt) y los de la escena actual.
    std::vector<DatosObjeto> datosObjetos_;
    std::vector<std::unique_ptr<ObjetoActividad>> objetos_;
};