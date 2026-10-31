#pragma once

// Clase principal del juego: crea la ventana y ejecuta el bucle principal.
// Orden del bucle en cada cuadro: procesarEventos -> actualizar(dt) -> dibujar.

#include <SFML/Graphics.hpp>

#include "core/GestorRecursos.hpp"

class Juego
{
public:
    Juego();

    // Bucle principal; termina cuando se cierra la ventana.
    void ejecutar();

private:
    void procesarEventos();
    void actualizar(float dt);
    void dibujar();


    // La ventana se declara primero para que se destruya al final,
    // después de liberar los recursos.
    sf::RenderWindow ventana_;
    GestorRecursos recursos_;
    sf::View vista_;
    sf::Clock reloj_;

    // Marcador de prueba (semana 0); se reemplazará por el jugador real.
    sf::RectangleShape jugador_;
};