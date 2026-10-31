#include "core/Juego.hpp"

#include <iostream>
#include <optional>

namespace
{
    // Tamaño de la ventana y de la vista lógica (el mundo se dibuja a 640x360
    // y se escala x2 a la ventana).
    constexpr unsigned ANCHO_VENTANA = 1280;
    constexpr unsigned ALTO_VENTANA  = 720;
    constexpr float    ANCHO_VISTA   = 640.f;
    constexpr float    ALTO_VISTA    = 360.f;

    // Si un cuadro tarda demasiado (ventana arrastrada, pausa del depurador),
    // se limita dt para que nada "salte" en la simulación.
    constexpr float DT_MAXIMO = 0.1f;
}

Juego::Juego()
    : jugador_(sf::Vector2f(24.f, 36.f))
{
#if SFML_VERSION_MAJOR >= 3
    ventana_.create(sf::VideoMode({ANCHO_VENTANA, ALTO_VENTANA}), "Sobreviviendo al Semestre");
#else
    ventana_.create(sf::VideoMode(ANCHO_VENTANA, ALTO_VENTANA), "Sobreviviendo al Semestre");
#endif
    ventana_.setFramerateLimit(60);

    vista_.setSize(sf::Vector2f(ANCHO_VISTA, ALTO_VISTA));
    vista_.setCenter(sf::Vector2f(ANCHO_VISTA / 2.f, ALTO_VISTA / 2.f));
    ventana_.setView(vista_);

    jugador_.setFillColor(sf::Color(59, 130, 246));
    jugador_.setPosition(sf::Vector2f(308.f, 160.f));

}

void Juego::ejecutar()
{
    reloj_.restart();
    while (ventana_.isOpen())
    {
        float dt = reloj_.restart().asSeconds();
        if (dt > DT_MAXIMO)
            dt = DT_MAXIMO;

        procesarEventos();
        actualizar(dt);
        dibujar();
    }
}

void Juego::procesarEventos()
{
#if SFML_VERSION_MAJOR >= 3
    while (const std::optional evento = ventana_.pollEvent())
    {
        if (evento->is<sf::Event::Closed>())
            ventana_.close();
    }
#else
    sf::Event evento;
    while (ventana_.pollEvent(evento))
    {
        if (evento.type == sf::Event::Closed)
            ventana_.close();
    }
#endif
}

void Juego::actualizar(float dt)
{
    const float velocidad = 120.f;
    sf::Vector2f mov(0.f, 0.f);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))  mov.x -= velocidad * dt;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) mov.x += velocidad * dt;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))    mov.y -= velocidad * dt;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))  mov.y += velocidad * dt;
    jugador_.move(mov);
}

void Juego::dibujar()
{
    ventana_.clear(sf::Color(30, 30, 40));
    ventana_.draw(jugador_);
    ventana_.display();
}

