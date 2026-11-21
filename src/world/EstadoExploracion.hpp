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
#include "systems/FinDeSemana.hpp"
#include "ui/Hud.hpp"
#include "world/DatosObjeto.hpp"
#include "world/Escena.hpp"
#include "world/MapaEscenas.hpp"
#include "world/ObjetoActividad.hpp"
#include "world/DatosNpc.hpp"
#include "world/Npc.hpp"
#include "dialogue/GestorDialogos.hpp"
#include "world/Personaje.hpp"
#include "world/DatosMueble.hpp"
#include "world/Mueble.hpp"

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
    
    // Herramienta de zonas (F3): mover y redimensionar puertas y actividades.
    Interactuable* zonaEn(const sf::Vector2f& punto);
    void imprimirZona(const Interactuable& zona);
    void imprimirTodasLasZonas();
    
    // Muebles (sprites con colisión en la base).
    void crearMuebles(const std::string& idEscena);
    Mueble* muebleEn(const sf::Vector2f& punto);
    void imprimirMueble(const Mueble& mueble);

    // S2-7: NPCs quietos y dialogos.
    void crearNpcs(const std::string& idEscena);
    void hablarCon(const std::string& idDialogo);

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

    
    // Herramienta de colisiones (con F3 activo): arrastra el ratón para
    // dibujar un rectángulo; Retroceso borra el último.
    bool arrastrando_ = false;
    sf::Vector2f inicioArrastre_{0.f, 0.f};
    int colisionesNuevas_ = 0;
    
    // Zona (puerta o actividad) que se está moviendo o redimensionando.
    Interactuable* zonaActiva_ = nullptr;
    bool zonaRedimensionando_ = false;
    sf::Vector2f desfaseZona_{0.f, 0.f};
    
    // Muebles (data/muebles.txt): todos y los de la escena actual.
    std::vector<DatosMueble> datosMuebles_;
    std::vector<std::unique_ptr<Mueble>> muebles_;

    // Herramienta de muebles (F3 + Ctrl): mover el mueble o ajustar su base.
    Mueble* muebleActivo_ = nullptr;
    bool muebleAjustandoBase_ = false;

        // S2-3 / S2-4
    Asistencia asistencia_;
    Colapso    colapso_;

    // S2-5: fin de semana automatico, orden del apartamento y modificador de la semana.
    FinDeSemana finDeSemana_;
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

    // S2-7: dialogos (data/dialogos.txt), todos los NPCs (data/npcs.txt)
    // y los de la escena actual.
    GestorDialogos dialogos_;
    std::vector<DatosNpc> datosNpcs_;
    std::vector<std::unique_ptr<Npc>> npcs_;
};