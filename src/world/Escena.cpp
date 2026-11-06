#include "world/Escena.hpp"

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

void Escena::dibujar(sf::RenderWindow& ventana,
                     const sf::Font& fuente) const
{
    ventana.draw(fondo_);

    for (const sf::RectangleShape& obstaculo : obstaculos_)
        ventana.draw(obstaculo);

    if (puerta_)
        puerta_->dibujar(ventana, fuente);
}