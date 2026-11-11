#pragma once

#include <array>

#include <SFML/Graphics.hpp>

#include "systems/Estadisticas.hpp"
#include "ui/BarraEstadistica.hpp"

// Panel desplegable con 5 barras: Energía, Estrés, Salud mental,
// Dopamina y el Conocimiento de la materia actual.
class PanelEstadisticas
{
public:
    static constexpr float MARGEN = 6.f;
    static constexpr float ANCHO  = 2.f * MARGEN + BarraEstadistica::ANCHO_TOTAL;
    static constexpr float ALTO   = 2.f * MARGEN + 5.f * BarraEstadistica::ALTO_FILA;

    // posicion = esquina superior izquierda del panel.
    void iniciar(const sf::Font* fuente, sf::Vector2f posicion);

    // Solo LEE las estadísticas; no las modifica.
    void actualizar(const Estadisticas& stats, Materia materiaActual);

    void dibujar(sf::RenderTarget& destino) const;

private:
    sf::RectangleShape                  fondo_;
    std::array<BarraEstadistica, 5>     barras_;
};