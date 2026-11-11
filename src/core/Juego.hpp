#pragma once

#include <SFML/Graphics.hpp>

#include "core/GestorRecursos.hpp"
#include "core/StateStack.hpp"
#include "systems/Estadisticas.hpp"
#include "systems/Tiempo.hpp"

// Clase principal del juego: crea la ventana y ejecuta el bucle principal.
// Orden del bucle en cada cuadro: procesarEventos -> actualizar(dt) -> dibujar.

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

    // La ventana se declara primero para que se destruya al final.
    sf::RenderWindow ventana_;
    GestorRecursos recursos_;
    sf::View vista_;
    sf::Clock reloj_;

    // Datos de la partida, compartidos por los estados de la pila.
    Estadisticas estadisticas_;
    Tiempo       tiempo_;

    // Pila de estados (exploración, menús...). Va al final para que
    // se destruya antes que la ventana y los recursos que usa.
    StateStack pila_;
};