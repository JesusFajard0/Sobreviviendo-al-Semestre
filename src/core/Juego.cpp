#include "core/Juego.hpp"

#include <iostream>
#include <memory>
#include <optional>

#include "world/EstadoExploracion.hpp"

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

Juego::Juego()
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

    if (!recursos_.cargarFuente("principal",
                                "assets/fuentes/principal.ttf"))
    {
        std::cerr << "[Juego] No se pudo cargar la fuente principal.\n";
    }

    if (!partida_.actividades.cargarDesdeArchivo("data/actividades.txt"))
    {
        std::cerr << "[Juego] Hubo problemas al cargar data/actividades.txt.\n";
    }

    pila_.pushState(
        std::make_unique<EstadoExploracion>(&pila_, recursos_, partida_)
    );
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
        else
            pila_.handleEvent(*evento);
    }

#else

    sf::Event evento;

    while (ventana_.pollEvent(evento))
    {
        if (evento.type == sf::Event::Closed)
            ventana_.close();
        else
            pila_.handleEvent(evento);
    }

#endif
}

void Juego::actualizar(float dt)
{
    pila_.update(dt);
}

void Juego::dibujar()
{
    ventana_.clear(sf::Color(30, 30, 40));

    pila_.draw(ventana_);

    ventana_.display();
}