#pragma once

#include <optional>
#include <string>

#include <SFML/Graphics.hpp>

#include "core/GestorRecursos.hpp"
#include "systems/Estadisticas.hpp"
#include "systems/Tiempo.hpp"
#include "ui/PanelEstadisticas.hpp"

// Interfaz mínima fija en pantalla: tiempo y dinero arriba a la derecha,
// botón "Estadísticas" arriba a la izquierda con punto de alerta, y el
// panel de barras desplegable. Solo LEE los datos del juego.
class Hud
{
public:
    // Devuelve false si la fuente no cargó (el HUD sigue funcionando,
    // pero sin texto).
    bool iniciar(const GestorRecursos& recursos);

    // Tab alterna el panel.
    void teclaPresionada(sf::Keyboard::Key tecla);

    // posLogica: posición del clic en coordenadas de la vista del HUD
    // (usar mapPixelToCoords con vista()).
    void ratonPresionado(sf::Vector2f posLogica);

    void actualizar(float dt, const Estadisticas& stats, const Tiempo& tiempo);

    // Dibuja con una vista fija de 640x360 y restaura la vista anterior.
    void dibujar(sf::RenderTarget& destino) const;

    const sf::View& vista() const { return vista_; }
    bool panelAbierto() const { return panelAbierto_; }

private:
    void alternarPanel();
    void actualizarTextoTiempo(const Tiempo& tiempo);
    void actualizarTextoDinero(const Estadisticas& stats);

    sf::View           vista_;
    sf::RectangleShape boton_;
    sf::CircleShape    punto_;
    PanelEstadisticas  panel_;

    std::optional<sf::Text> textoBoton_;
    std::optional<sf::Text> textoTiempo_;
    std::optional<sf::Text> textoDinero_;

    // Últimos textos puestos, para no reconstruirlos sin necesidad.
    std::string ultimoTiempo_;
    std::string ultimoDinero_;

    bool  panelAbierto_ = false;
    bool  hayAlerta_    = false;
    float fase_         = 0.f;   // fase del parpadeo del punto
};