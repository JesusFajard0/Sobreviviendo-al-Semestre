#include "ui/BarraEstadistica.hpp"

#include <algorithm>
#include <cmath>

#include "core/CompatSFML.hpp"

namespace
{
    constexpr unsigned TAM_TEXTO       = 10;
    constexpr float    ALTO_BARRA      = 8.f;
    constexpr float    DESFASE_TEXTO_Y = 2.f;
    constexpr float    GROSOR_BORDE    = 1.f;

    const sf::Color COLOR_FONDO_BARRA(20, 20, 28);
    const sf::Color COLOR_TEXTO(235, 235, 240);
    const sf::Color COLOR_CRITICO(225, 60, 60);
}

void BarraEstadistica::iniciar(const sf::Font* fuente,
                               const std::string& etiquetaUtf8,
                               sf::Color color)
{
    fondo_.setSize(sf::Vector2f(ANCHO_BARRA, ALTO_BARRA));
    fondo_.setFillColor(COLOR_FONDO_BARRA);
    fondo_.setOutlineColor(COLOR_CRITICO);

    relleno_.setSize(sf::Vector2f(0.f, ALTO_BARRA));
    relleno_.setFillColor(color);

    if (fuente != nullptr)
    {
        etiqueta_.emplace(crearTexto(*fuente, etiquetaUtf8, TAM_TEXTO));
        valorTexto_.emplace(crearTexto(*fuente, "0", TAM_TEXTO));
        etiqueta_->setFillColor(COLOR_TEXTO);
        valorTexto_->setFillColor(COLOR_TEXTO);
    }
}

void BarraEstadistica::colocar(sf::Vector2f posicion)
{
    const float yBarra = posicion.y + (ALTO_FILA - ALTO_BARRA) / 2.f;
    const float xBarra = posicion.x + ANCHO_ETIQUETA;

    fondo_.setPosition(sf::Vector2f(xBarra, yBarra));
    relleno_.setPosition(sf::Vector2f(xBarra, yBarra));

    if (etiqueta_)
        etiqueta_->setPosition(
            sf::Vector2f(posicion.x, posicion.y + DESFASE_TEXTO_Y));

    if (valorTexto_)
        valorTexto_->setPosition(
            sf::Vector2f(xBarra + ANCHO_BARRA + 4.f,
                         posicion.y + DESFASE_TEXTO_Y));
}

void BarraEstadistica::establecer(float valor, float maximo, bool critica)
{
    const float tope = (maximo > 0.f) ? maximo : 1.f;
    const float v    = std::clamp(valor, 0.f, tope);

    relleno_.setSize(sf::Vector2f(ANCHO_BARRA * (v / tope), ALTO_BARRA));

    // El texto solo se reconstruye si el número entero cambió.
    const int entero = static_cast<int>(std::lround(v));
    if (valorTexto_ && entero != valorEntero_)
        valorTexto_->setString(textoUtf8(std::to_string(entero)));
    valorEntero_ = entero;

    fondo_.setOutlineThickness(critica ? GROSOR_BORDE : 0.f);
}

void BarraEstadistica::dibujar(sf::RenderTarget& destino) const
{
    if (etiqueta_)
        destino.draw(*etiqueta_);

    destino.draw(fondo_);
    destino.draw(relleno_);

    if (valorTexto_)
        destino.draw(*valorTexto_);
}