#include "core/Juego.hpp"

#include <iostream>
#include <optional>

namespace
{
    // Tamaño de la ventana y de la vista lógica.
    constexpr unsigned ANCHO_VENTANA = 1280;
    constexpr unsigned ALTO_VENTANA = 720;

    constexpr float ANCHO_VISTA = 640.f;
    constexpr float ALTO_VISTA = 360.f;

    // Tiempo máximo entre cuadros.
    constexpr float DT_MAXIMO = 0.1f;
}

// ---------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------

Juego::Juego()
    : personaje_(sf::Vector2f(308.f, 160.f))
{
#if SFML_VERSION_MAJOR >= 3
    ventana_.create(
        sf::VideoMode({ANCHO_VENTANA, ALTO_VENTANA}),
        "Sobreviviendo al Semestre"
    );
#else
    ventana_.create(
        sf::VideoMode(ANCHO_VENTANA, ALTO_VENTANA),
        "Sobreviviendo al Semestre"
    );
#endif

    ventana_.setFramerateLimit(60);

    vista_.setSize(sf::Vector2f(ANCHO_VISTA, ALTO_VISTA));

    vista_.setCenter(sf::Vector2f(
        ANCHO_VISTA / 2.f,
        ALTO_VISTA / 2.f
    ));

    ventana_.setView(vista_);

    // Crear obstáculos provisionales.
    crearObstaculosPrueba();
}

// ---------------------------------------------------------------
// Bucle principal
// ---------------------------------------------------------------

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

// ---------------------------------------------------------------
// Procesamiento de eventos
// ---------------------------------------------------------------

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

// ---------------------------------------------------------------
// Actualización
// ---------------------------------------------------------------

void Juego::actualizar(float dt)
{
    sf::Vector2f direccion(0.f, 0.f);

#if SFML_VERSION_MAJOR >= 3

    // Movimiento horizontal.
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
    {
        direccion.x -= 1.f;
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
    {
        direccion.x += 1.f;
    }

    // Movimiento vertical.
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))
    {
        direccion.y -= 1.f;
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))
    {
        direccion.y += 1.f;
    }

#else

    // Movimiento horizontal.
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
    {
        direccion.x -= 1.f;
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
    {
        direccion.x += 1.f;
    }

    // Movimiento vertical.
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
    {
        direccion.y -= 1.f;
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
    {
        direccion.y += 1.f;
    }

#endif

    // El personaje se encarga de su movimiento y sus colisiones.
    personaje_.actualizar(dt, direccion, obstaculos_);
}

// ---------------------------------------------------------------
// Dibujo
// ---------------------------------------------------------------

void Juego::dibujar()
{
    ventana_.clear(sf::Color(30, 30, 40));

    // Dibujar los obstáculos.
    for (const sf::RectangleShape& obstaculo : obstaculos_)
    {
        ventana_.draw(obstaculo);
    }

    // Dibujar el personaje.
    personaje_.dibujar(ventana_);

    ventana_.display();
}

// ---------------------------------------------------------------
// Creación de obstáculos temporales
// ---------------------------------------------------------------

void Juego::crearObstaculosPrueba()
{
    // Pared horizontal.
    sf::RectangleShape paredHorizontal(sf::Vector2f(180.f, 16.f));

    paredHorizontal.setPosition(sf::Vector2f(220.f, 105.f));
    paredHorizontal.setFillColor(sf::Color(120, 120, 130));

    obstaculos_.push_back(paredHorizontal);

    // Pared vertical.
    sf::RectangleShape paredVertical(sf::Vector2f(16.f, 150.f));

    paredVertical.setPosition(sf::Vector2f(390.f, 105.f));
    paredVertical.setFillColor(sf::Color(120, 120, 130));

    obstaculos_.push_back(paredVertical);

    // Obstáculo rectangular.
    sf::RectangleShape bloque(sf::Vector2f(100.f, 18.f));

    bloque.setPosition(sf::Vector2f(180.f, 245.f));
    bloque.setFillColor(sf::Color(120, 120, 130));

    obstaculos_.push_back(bloque);
}