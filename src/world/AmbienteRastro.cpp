#include "world/AmbienteRastro.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace
{
    float fraccion(float v)
    {
        return v - std::floor(v);
    }
}

PosParticula posicionParticula(EstiloRastro estilo, int indice, float tiempo,
                               float ancho, float alto)
{
    const float i = static_cast<float>(indice);

    const float base = fraccion(i * 0.6180339f);          // reparto horizontal
    const float fase = fraccion(i * 0.7548777f + 0.13f);  // altura de arranque
    const float vel  = (estilo == EstiloRastro::Tristeza ? 90.f : 35.f) +
                       40.f * fraccion(i * 0.5698403f);

    float x = base * ancho;
    if (estilo == EstiloRastro::Fiesta)
        x += 6.f * std::sin(tiempo * 2.f + i);            // el confeti se mece

    const float y = std::fmod(fase * alto + tiempo * vel, alto);

    return PosParticula{std::clamp(x, 0.f, ancho), y};
}

void AmbienteRastro::dibujar(sf::RenderWindow& ventana) const
{
    // Se dibuja con la vista actual del juego, cubriendola toda.
    const sf::View vista = ventana.getView();
    const sf::Vector2f tam    = vista.getSize();
    const sf::Vector2f centro = vista.getCenter();
    const sf::Vector2f origen(centro.x - tam.x / 2.f, centro.y - tam.y / 2.f);

    const bool fiesta = (estilo_ == EstiloRastro::Fiesta);

    // Tono de todo el salon.
    sf::RectangleShape tono(tam);
    tono.setPosition(origen);
    tono.setFillColor(fiesta ? sf::Color(255, 210, 120, 28)    // dorado suave
                             : sf::Color(30, 40, 90, 70));     // azul apagado
    ventana.draw(tono);

    // Confeti de colores o lluvia.
    static const sf::Color COLORES[] = {
        sf::Color(235, 70, 70),  sf::Color(250, 210, 60), sf::Color(80, 190, 90),
        sf::Color(70, 140, 235), sf::Color(220, 90, 200), sf::Color(250, 140, 50)
    };

    for (int i = 0; i < PARTICULAS; ++i)
    {
        const PosParticula p = posicionParticula(estilo_, i, tiempo_, tam.x, tam.y);

        sf::RectangleShape particula(fiesta ? sf::Vector2f(3.f, 4.f) : sf::Vector2f(1.f, 7.f));
        particula.setPosition(sf::Vector2f(origen.x + p.x, origen.y + p.y));
        particula.setFillColor(fiesta ? COLORES[i % 6] : sf::Color(160, 180, 230, 170));
        ventana.draw(particula);
    }
}
