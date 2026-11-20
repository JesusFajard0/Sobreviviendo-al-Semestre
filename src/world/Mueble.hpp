#pragma once

#include <SFML/Graphics.hpp>

#include <cstddef>
#include <string>

// Sprite de un mueble colocado en una escena. Solo su base (franja inferior)
// bloquea el paso; el resto se puede rodear por detrás.
class Mueble
{
public:
    Mueble(const sf::Texture& textura, const std::string& nombre,
           float x, float y, float escala, float baseAlto);

    const std::string& nombre() const { return nombre_; }
    float escala() const { return escala_; }
    float baseAlto() const { return baseAlto_; }

    // Rectángulo que ocupa el sprite, ya con la escala aplicada.
    sf::FloatRect area() const;

    // Franja inferior que bloquea el paso.
    sf::FloatRect colision() const;

    // Borde inferior del sprite: decide si el personaje pasa delante o detrás.
    float yPies() const;

    void colocar(float x, float y);
    void ponerBaseAlto(float alto);

    void dibujar(sf::RenderWindow& ventana) const;

    // Posición de su colisión en la lista de obstáculos de la Escena.
    std::size_t indiceColision = 0;

private:
    std::string nombre_;
    sf::Sprite sprite_;
    float escala_;
    float ancho_;      // del sprite ya escalado
    float alto_;
    float baseAlto_;
};