#include "world/Escena.hpp"
#include <iostream>
#include "core/CompatSFML.hpp"

namespace
{
    // Tamaño de la vista lógica (el mismo que usa Juego).
    constexpr float ANCHO_VISTA = 640.f;
    constexpr float ALTO_VISTA = 360.f;
}

Escena::Escena(const DatosEscena& datos,
               const std::vector<DatosRectangulo>& rectangulos,
               std::function<void()> alUsarPuerta)
    : id_(datos.id),
      spawn_(datos.spawn),
      fondo_(sf::Vector2f(ANCHO_VISTA, ALTO_VISTA))
{
    fondo_.setFillColor(datos.color);

    for (const DatosRectangulo& r : rectangulos)
    {
        if (r.tipo == "rect_colision")
        {
            sf::RectangleShape obstaculo(sf::Vector2f(r.ancho, r.alto));
            obstaculo.setPosition(sf::Vector2f(r.x, r.y));
            obstaculo.setFillColor(sf::Color(120, 120, 130));

            obstaculos_.push_back(obstaculo);
        }
        else if (r.tipo == "rect_salida")
        {
            puerta_ = std::make_unique<Puerta>(
                crearRect(r.x, r.y, r.ancho, r.alto),
                alUsarPuerta
            );
        }
    }
}

void Escena::ponerFondo(const sf::Texture& textura)
{
    const sf::Vector2u tamano = textura.getSize();

    if (tamano.x != static_cast<unsigned>(ANCHO_VISTA) ||
        tamano.y != static_cast<unsigned>(ALTO_VISTA))
    {
        std::cerr << "[Escena] El fondo de '" << id_ << "' mide "
                  << tamano.x << "x" << tamano.y
                  << " y se estirará a 640x360.\n";
    }

    fondo_.setTexture(&textura, true);
    fondo_.setFillColor(sf::Color::White);
    tieneFondo_ = true;
}

void Escena::dibujar(sf::RenderWindow& ventana, const sf::Font& fuente, bool verColisiones) const
{
    ventana.draw(fondo_);

    // Con arte, los rectángulos de prueba solo se ven en modo depuración.
    const bool verFormas = !tieneFondo_ || verColisiones;

    if (verFormas)
    {
        for (const sf::RectangleShape& obstaculo : obstaculos_)
            ventana.draw(obstaculo);
    }

    if (puerta_)
    {
        if (verFormas)
            puerta_->dibujar(ventana, fuente);                // forma + [E]
        else
            puerta_->Interactuable::dibujar(ventana, fuente); // solo [E]
    }
}

sf::FloatRect Escena::limites() const
{
    return crearRect(0.f, 0.f, ANCHO_VISTA, ALTO_VISTA);
}