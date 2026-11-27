#include "core/Juego.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

#include "core/CompatSFML.hpp"
#include "core/MenuState.hpp"
#include "world/EstadoExploracion.hpp"
#include "world/IntroState.hpp"

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
    vista_.setSize(sf::Vector2f(ANCHO_VISTA, ALTO_VISTA));
    vista_.setCenter(sf::Vector2f(ANCHO_VISTA / 2.f, ALTO_VISTA / 2.f));

    crearVentana();

    if (!recursos_.cargarFuente("principal",
                                "assets/fuentes/principal.ttf"))
    {
        std::cerr << "[Juego] No se pudo cargar la fuente principal.\n";
    }

    if (!partida_.actividades.cargarDesdeArchivo("data/actividades.txt"))
    {
        std::cerr << "[Juego] Hubo problemas al cargar data/actividades.txt.\n";
    }

    if (!partida_.fases.cargarDesdeArchivo("data/fases.txt"))
    {
        std::cerr << "[Juego] Hubo problemas al cargar data/fases.txt.\n";
    }

    // S3-8: el juego arranca en el menu de inicio; "Jugar" crea la partida.
    static const sf::Font fuenteVacia;   // por si la fuente no cargo
    const sf::Font* fuente = recursos_.obtenerFuente("principal");

    pila_.pushState(std::make_unique<MenuState>(
        &pila_,
        (fuente != nullptr) ? *fuente : fuenteVacia,
        [this]() { iniciarPartida(); },
        [this]() { ventana_.close(); }
    ));
}

void Juego::iniciarPartida()
{
    std::vector<DiapositivaIntro> intro = {
        {
            "assets/intro/intro_1.png",
            "Sobreviviendo al Semestre: administra 8 semanas de vida universitaria y equilibra tus energías para no caer en el intento."
        },
        {
            "assets/intro/intro_2.png",
            "Asiste a clases, estudia y supera las fases de Cálculo (examen la 3.ª semana), Física (examen la 5.ª semana) y Programación (última y 8.ª semana) antes de que lleguen los exámenes."
        },
        {
            "assets/intro/intro_3.png",
            "Dormir, comer, ordenar o salir de rumba (viernes o fin de semana) sube o baja tus stats: cada decisión importa."
        },
        {
            "assets/intro/intro_4.png",
            "¿Lograrás aprobar y graduarte con buenos números, o terminarás sobreviviendo de milagro?"
        }
    };

    pila_.pushState(
        std::make_unique<EstadoExploracion>(&pila_, recursos_, partida_)
    );
    pila_.pushState(
        std::make_unique<IntroState>(&pila_, recursos_, std::move(intro))
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
        {
            ventana_.close();
            continue;
        }

        if (evento->is<sf::Event::Resized>())
        {
            ajustarVista();
            continue;
        }

        if (const auto* tecla = evento->getIf<sf::Event::KeyPressed>())
        {
            if (tecla->code == sf::Keyboard::Key::F11)
            {
                pantallaCompleta_ = !pantallaCompleta_;
                crearVentana();
                continue;
            }
            if (tecla->code == sf::Keyboard::Key::F10)
            {
                modoEscala_ = (modoEscala_ == ModoEscala::Entero)
                                  ? ModoEscala::Fraccionario
                                  : ModoEscala::Entero;
                ajustarVista();
                continue;
            }
        }

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
    ventana_.clear(sf::Color::Black);

    pila_.draw(ventana_);

    ventana_.display();
}

void Juego::crearVentana()
{
    if (pantallaCompleta_)
    {
        ventana_.create(sf::VideoMode::getDesktopMode(),
                        "Sobreviviendo al Semestre",
                        sf::State::Fullscreen);
    }
    else
    {
        ventana_.create(sf::VideoMode({ANCHO_VENTANA, ALTO_VENTANA}),
                        "Sobreviviendo al Semestre",
                        sf::Style::Default,
                        sf::State::Windowed);
    }

    // create() reinicia estos ajustes, por eso se repiten aquí.
    ventana_.setFramerateLimit(60);
    ventana_.setKeyRepeatEnabled(false);

    ajustarVista();
}

void Juego::ajustarVista()
{
    const sf::Vector2u t = ventana_.getSize();
    if (t.x == 0 || t.y == 0)
        return;

    const float tx = static_cast<float>(t.x);
    const float ty = static_cast<float>(t.y);

    float escala = std::min(tx / ANCHO_VISTA, ty / ALTO_VISTA);

    if (modoEscala_ == ModoEscala::Entero && escala >= 1.f)
        escala = std::floor(escala);

    // Tamaño y posición en píxeles enteros para que el borde quede nítido.
    const float anchoPx = std::round(ANCHO_VISTA * escala);
    const float altoPx  = std::round(ALTO_VISTA * escala);
    const float x = std::floor((tx - anchoPx) / 2.f);
    const float y = std::floor((ty - altoPx) / 2.f);

    vista_.setViewport(crearRect(x / tx, y / ty, anchoPx / tx, altoPx / ty));
    ventana_.setView(vista_);
}