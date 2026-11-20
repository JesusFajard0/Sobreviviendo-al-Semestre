#include "world/Mueble.hpp"

#include <algorithm>

#include "core/CompatSFML.hpp"

Mueble::Mueble(const sf::Texture& textura, const std::string& nombre,
               float x, float y, float escala, float baseAlto)
    : nombre_(nombre),
      sprite_(textura),
      escala_(escala),
      ancho_(static_cast<float>(textura.getSize().x) * escala),
      alto_(static_cast<float>(textura.getSize().y) * escala),
      baseAlto_(baseAlto)
{
    sprite_.setScale(sf::Vector2f(escala, escala));
    sprite_.setPosition(sf::Vector2f(x, y));
    ponerBaseAlto(baseAlto);
}

sf::FloatRect Mueble::area() const
{
    const sf::Vector2f p = sprite_.getPosition();
    return crearRect(p.x, p.y, ancho_, alto_);
}

sf::FloatRect Mueble::colision() const
{
    const sf::Vector2f p = sprite_.getPosition();
    return crearRect(p.x, p.y + alto_ - baseAlto_, ancho_, baseAlto_);
}

float Mueble::yPies() const
{
    return sprite_.getPosition().y + alto_;
}

void Mueble::colocar(float x, float y)
{
    sprite_.setPosition(sf::Vector2f(x, y));
}

void Mueble::ponerBaseAlto(float alto)
{
    baseAlto_ = std::clamp(alto, 0.f, alto_);
}

void Mueble::dibujar(sf::RenderWindow& ventana) const
{
    ventana.draw(sprite_);
}