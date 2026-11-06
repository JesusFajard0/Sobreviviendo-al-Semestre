#ifndef INTERACTUABLE_HPP
#define INTERACTUABLE_HPP

#include <SFML/Graphics.hpp>
#include <string>

class Interactuable {
protected:
    sf::FloatRect m_areaInteraccion; // El área donde el jugador puede pulsar 'E'
    bool m_mostrarAviso;             // Si el jugador está cerca, se pone en true

public:
    Interactuable(sf::FloatRect area);
    virtual ~Interactuable() = default;

    // Método virtual puro, cada hijo decide qué pasa al interactuar
    virtual void interactuar() = 0; 

    // Verifica si el hitbox) está dentro del área
    bool puedeInteractuar(const sf::FloatRect& hitboxJugador);

    // Para dibujar el aviso "[E]" o un sprite
    virtual void dibujar(sf::RenderWindow& ventana, const sf::Font& fuente) const;
};

#endif