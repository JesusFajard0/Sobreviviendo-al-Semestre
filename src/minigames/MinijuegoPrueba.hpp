#pragma once

#include <SFML/Graphics.hpp>

#include <string>
#include <utility>

#include "core/CompatSFML.hpp"
#include "minigames/Minijuego.hpp"

// Minijuego temporal para comprobar la integracion.
// ENTER: completar.
// ESC: cancelar.
class MinijuegoPrueba : public Minijuego
{
public:
    MinijuegoPrueba(
        StateStack* pila,
        const sf::Font& fuente,
        const std::string& id,
        AlTerminar alTerminar)
        : Minijuego(pila, id, std::move(alTerminar)),
          texto_(crearTexto(
              fuente,
              "MINIJUEGO DE PRUEBA\n\n"
              "ID: " + id + "\n\n"
              "ENTER - Completar\n"
              "ESC - Cancelar",
              18))
    {
        texto_.setPosition({120.f, 110.f});
        texto_.setFillColor(sf::Color::White);
    }

    void handleEvent(const sf::Event& event) override
    {
#if SFML_VERSION_MAJOR >= 3
        const auto* tecla =
            event.getIf<sf::Event::KeyPressed>();

        if (tecla == nullptr)
            return;

        if (tecla->code == sf::Keyboard::Key::Enter)
        {
            finalizar(EstadoMinijuego::Completado, 100);
            return;
        }

        if (tecla->code == sf::Keyboard::Key::Escape)
        {
            finalizar(EstadoMinijuego::Cancelado);
            return;
        }
#else
        if (event.type != sf::Event::KeyPressed)
            return;

        if (event.key.code == sf::Keyboard::Enter)
        {
            finalizar(EstadoMinijuego::Completado, 100);
            return;
        }

        if (event.key.code == sf::Keyboard::Escape)
        {
            finalizar(EstadoMinijuego::Cancelado);
            return;
        }
#endif
    }

    void update(float dt) override
    {
        (void)dt;
    }

    void draw(sf::RenderWindow& window) override
    {
        const sf::View vistaAnterior = window.getView();

        window.setView(
            sf::View(crearRect(0.f, 0.f, 640.f, 360.f))
        );

        sf::RectangleShape fondo({640.f, 360.f});
        fondo.setFillColor(sf::Color(18, 24, 42, 250));

        window.draw(fondo);
        window.draw(texto_);

        window.setView(vistaAnterior);
    }

private:
    sf::Text texto_;
};