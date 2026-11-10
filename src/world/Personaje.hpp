#pragma once

#include <SFML/Graphics.hpp>
#include <vector>

class Personaje
{
public:
    // Constructor: recibe la posición inicial del personaje.
    explicit Personaje(
        const sf::Vector2f& posicionInicial = sf::Vector2f(308.f, 160.f)
    );

    // Actualiza el movimiento y comprueba las colisiones.
    void actualizar(
        float dt,
        const sf::Vector2f& direccion,
        const std::vector<sf::RectangleShape>& obstaculos
    );

    // Dibuja el personaje y su rectángulo de colisión.
    void dibujar(sf::RenderTarget& destino) const;

        // Rectángulo de colisión (los pies), en coordenadas del mundo.
    sf::FloatRect hitbox() const;

    // Coloca al personaje con la esquina superior izquierda del cuerpo en 'posicion'.
    void colocar(const sf::Vector2f& posicion);

        // Mantiene al personaje dentro de un área (por ejemplo, la pantalla).
    void limitarA(const sf::FloatRect& area);

private:
    // Comprueba si la hitbox colisionaría en una posición determinada.
    bool hayColisionEn(
        const sf::Vector2f& posicionHitbox,
        const std::vector<sf::RectangleShape>& obstaculos
    ) const;

    // Representación visual del personaje.
    sf::RectangleShape cuerpo_;

    // Rectángulo independiente de colisión ubicado en los pies.
    sf::RectangleShape hitbox_;

    // Velocidad de desplazamiento en unidades por segundo.
    float velocidad_;
};