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
#include "systems/Asistencia.hpp"
#include "systems/Colapso.hpp"
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

    // Caja de texto modal: pausa el juego hasta que se pulse ENTER o E.
    // Se coloca encima del personaje sin salirse de la pantalla.
    void mostrarMensaje(const std::string& texto);

    // Fin de semana y fin de semestre: se revisan en cada cuadro, sin importar
    // si los provocó una actividad o una tecla de depuración.
    void revisarEventosTiempo();

    // S2-4: si alguna estadistica llego al extremo, hay colapso (pierde un bloque).
    void revisarColapso();

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

    // F3: muestra colisiones y puerta encima del arte.
    bool verColisiones_ = false;

    // S2-3 / S2-4
    Asistencia asistencia_;
    Colapso    colapso_;

    // --- Caja de texto modal ---
    bool mostrandoMensaje_ = false;
    sf::RectangleShape fondoMensaje_;
    sf::Text textoMensaje_;

    // Para avisar del fin del semestre una sola vez.
    bool finSemestreMostrado_ = false;

    // Para mostrar la guía del estudiante al iniciar la partida
    bool guiaInicialMostrada_ = false;

    // Banderas de control para exámenes y asistencias
    bool avisoExamenMostradoHoy = false;
    bool avisoOlvidoExamenMostradoHoy = false;
    bool faltaClaseVerificadaHoy = false;
// Para controlar la paginación interactiva del manual del estudiante
    int paginaManual_ = 0;
    // Ventana donde se dibujó por última vez: hace falta para convertir
    // la posición del ratón (píxeles) a coordenadas lógicas del HUD.
    sf::RenderWindow* ventana_ = nullptr;

    // Todos los objetos del juego (data/objetos.txt) y los de la escena actual.
    std::vector<DatosObjeto> datosObjetos_;
    std::vector<std::unique_ptr<ObjetoActividad>> objetos_;
};