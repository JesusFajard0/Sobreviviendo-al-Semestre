#include "world/BusState.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <utility>

#include "core/StateStack.hpp"

BusState::BusState(StateStack* pila,
                   std::vector<const sf::Texture*> imagenes,
                   AlTerminar alTerminar)
    : State(pila),
      imagenes_(std::move(imagenes)),
      alTerminar_(std::move(alTerminar))
{
}

void BusState::terminar()
{
    // popState destruye este objeto: se saca el callback antes.
    AlTerminar fin = std::move(alTerminar_);
    m_stack->popState();

    if (fin)
        fin();
}

void BusState::handleEvent(const sf::Event& event)
{
    if (const auto* tecla = event.getIf<sf::Event::KeyPressed>())
    {
        if (tecla->code == sf::Keyboard::Key::Enter ||
            tecla->code == sf::Keyboard::Key::E ||
            tecla->code == sf::Keyboard::Key::Space ||
            tecla->code == sf::Keyboard::Key::Escape)
        {
            terminar();   // el objeto ya no existe
        }
    }
}

void BusState::update(float dt)
{
    t_ += dt;

    if (imagenes_.empty() ||
        t_ >= POR_IMAGEN * static_cast<float>(imagenes_.size()))
    {
        terminar();
    }
}

void BusState::draw(sf::RenderWindow& window)
{
    if (imagenes_.empty())
        return;

    const int n = static_cast<int>(imagenes_.size());
    const int i = std::min(static_cast<int>(t_ / POR_IMAGEN), n - 1);
    const float local = t_ - static_cast<float>(i) * POR_IMAGEN;

    // Fondo negro (tapa la exploración que hay debajo).
    sf::RectangleShape negro(sf::Vector2f(ANCHO, ALTO));
    negro.setFillColor(sf::Color::Black);
    window.draw(negro);

    // Cada imagen sale de negro y vuelve a negro.
    const float k = std::clamp(std::min(local / FUNDIDO, (POR_IMAGEN - local) / FUNDIDO),
                               0.f, 1.f);

    const sf::Texture& textura = *imagenes_[i];
    const sf::Vector2u tam = textura.getSize();
    if (tam.x == 0 || tam.y == 0)
        return;

    sf::Sprite sprite(textura);
    sprite.setOrigin(sf::Vector2f(static_cast<float>(tam.x) / 2.f,
                                  static_cast<float>(tam.y) / 2.f));

    // Cubre toda la pantalla y hace un zoom lento (así la vibración no
    // deja ver bordes).
    const float cubrir = std::max(ANCHO / static_cast<float>(tam.x),
                                  ALTO  / static_cast<float>(tam.y));
    const float zoom = 1.06f + 0.05f * (local / POR_IMAGEN);
    sprite.setScale(sf::Vector2f(cubrir * zoom, cubrir * zoom));

    // Vibración del bus.
    const float vibracion = std::sin(t_ * 28.f) * 0.7f + std::sin(t_ * 9.f) * 0.5f;
    sprite.setPosition(sf::Vector2f(ANCHO / 2.f, ALTO / 2.f + vibracion));

    sprite.setColor(sf::Color(255, 255, 255, static_cast<std::uint8_t>(255.f * k)));
    window.draw(sprite);
}