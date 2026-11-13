#include "world/ClaseState.hpp"

#include <algorithm>
#include <cstdint>
#include <utility>

#include "core/StateStack.hpp"

ClaseState::ClaseState(StateStack* pila,
                       const sf::Font& fuente,
                       const std::string& materia,
                       AlTerminar alTerminar)
    : State(pila),
      texto_(fuente, "En clase de " + materia + "...", 18),
      alTerminar_(std::move(alTerminar))
{
    // Centrado en la vista logica.
    const sf::FloatRect b = texto_.getLocalBounds();
    texto_.setOrigin({b.position.x + b.size.x / 2.f,
                      b.position.y + b.size.y / 2.f});
    texto_.setPosition({ANCHO / 2.f, ALTO / 2.f - 10.f});
}

void ClaseState::handleEvent(const sf::Event&)
{
    // Sin entrada: el jugador no puede hacer nada mientras esta en clase.
}

void ClaseState::update(float dt)
{
    transcurrido_ += dt;

    if (transcurrido_ < DURACION)
        return;

    // popState destruye este objeto: se saca el callback antes y despues
    // del pop no se vuelve a usar ningun miembro.
    AlTerminar fin = std::move(alTerminar_);
    m_stack->popState();

    if (fin)
        fin();
}

void ClaseState::draw(sf::RenderWindow& window)
{
    // Opacidad: sube al empezar y baja al terminar.
    const float entrada = std::min(1.f, transcurrido_ / FUNDIDO);
    const float salida  = std::min(1.f, (DURACION - transcurrido_) / FUNDIDO);
    const float k       = std::max(0.f, std::min(entrada, salida));
    const auto  alfa    = static_cast<std::uint8_t>(230.f * k);

    const sf::View vistaPrevia = window.getView();
    window.setView(sf::View(sf::FloatRect({0.f, 0.f}, {ANCHO, ALTO})));

    sf::RectangleShape fondo({ANCHO, ALTO});
    fondo.setFillColor(sf::Color(0, 0, 0, alfa));
    window.draw(fondo);

    texto_.setFillColor(sf::Color(255, 255, 255, alfa));
    window.draw(texto_);

    // Barra de progreso provisional.
    constexpr float ANCHO_BARRA = 200.f;
    const float x = (ANCHO - ANCHO_BARRA) / 2.f;
    const float y = ALTO / 2.f + 20.f;

    sf::RectangleShape marco({ANCHO_BARRA, 8.f});
    marco.setPosition({x, y});
    marco.setFillColor(sf::Color(60, 60, 60, alfa));
    window.draw(marco);

    sf::RectangleShape relleno({ANCHO_BARRA * std::min(1.f, transcurrido_ / DURACION), 8.f});
    relleno.setPosition({x, y});
    relleno.setFillColor(sf::Color(120, 200, 120, alfa));
    window.draw(relleno);

    window.setView(vistaPrevia);
}
