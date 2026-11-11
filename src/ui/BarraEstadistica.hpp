#pragma once

#include <optional>
#include <string>

#include <SFML/Graphics.hpp>

// Una fila del panel: etiqueta, barra de 0 a máximo y valor numérico.
// Las posiciones se fijan una vez con colocar(); los textos solo
// se actualizan cuando el valor cambia.
class BarraEstadistica
{
public:
    // Medidas de la fila (en coordenadas lógicas de 640x360).
    static constexpr float ALTO_FILA       = 18.f;
    static constexpr float ANCHO_ETIQUETA  = 64.f;
    static constexpr float ANCHO_BARRA     = 70.f;
    static constexpr float ANCHO_VALOR     = 26.f;
    static constexpr float ANCHO_TOTAL     = ANCHO_ETIQUETA + ANCHO_BARRA + ANCHO_VALOR;

    // fuente puede ser nullptr: entonces la barra se dibuja sin texto.
    void iniciar(const sf::Font* fuente, const std::string& etiquetaUtf8,
                 sf::Color color);

    // Esquina superior izquierda de la fila.
    void colocar(sf::Vector2f posicion);

    // Valores fuera de rango se recortan a [0, maximo].
    void establecer(float valor, float maximo, bool critica);

    void dibujar(sf::RenderTarget& destino) const;

private:
    std::optional<sf::Text> etiqueta_;
    std::optional<sf::Text> valorTexto_;
    sf::RectangleShape      fondo_;
    sf::RectangleShape      relleno_;
    int                     valorEntero_ = -1;
};