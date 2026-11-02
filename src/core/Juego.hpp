#pragma once

#include <SFML/Graphics.hpp>

#include <vector>

#include "core/GestorRecursos.hpp"
#include "world/Personaje.hpp"

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

    // Crea obstáculos provisionales para probar las colisiones.
    void crearObstaculosPrueba();

    // La ventana se declara primero para que se destruya al final.
    sf::RenderWindow ventana_;
    GestorRecursos recursos_;
    sf::View vista_;
    sf::Clock reloj_;

    // Personaje controlado por el jugador.
    Personaje personaje_;

    // Obstáculos temporales de la escena de prueba.
    std::vector<sf::RectangleShape> obstaculos_;
};