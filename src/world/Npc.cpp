#include "world/Npc.hpp"

#include <utility>

#include "core/CompatSFML.hpp"

namespace
{
    constexpr unsigned ESCALA_TEXTO = 2;
    constexpr float    ALCANCE      = 14.f;   // cuanto crece el area de dialogo

    // Area donde se puede hablar: el cuerpo agrandado por todos lados.
    sf::FloatRect areaDeDialogo(const sf::FloatRect& cuerpo)
    {
        const sf::Vector2f p = posicionRect(cuerpo);
        const sf::Vector2f t = tamanoRect(cuerpo);
        return crearRect(p.x - ALCANCE, p.y - ALCANCE,
                         t.x + 2.f * ALCANCE, t.y + 2.f * ALCANCE);
    }
}

Npc::Npc(const sf::FloatRect& cuerpo,
         const std::string& nombre,
         const std::string& idDialogo,
         std::function<void(const std::string&)> alHablar)
    : Interactuable(areaDeDialogo(cuerpo)),
      nombre_(nombre),
      idDialogo_(idDialogo),
      alHablar_(std::move(alHablar)),
      cuerpo_(cuerpo)
{
    forma_.setSize(tamanoRect(cuerpo));
    forma_.setPosition(posicionRect(cuerpo));
    forma_.setFillColor(sf::Color(200, 140, 80, 230));
    forma_.setOutlineColor(sf::Color::White);
    forma_.setOutlineThickness(1.f);
}

void Npc::interactuar()
{
    if (alHablar_)
        alHablar_(idDialogo_);
}

void Npc::dibujar(sf::RenderWindow& ventana, const sf::Font& fuente) const
{
    ventana.draw(forma_);

    // Nombre centrado debajo del NPC (el aviso "[E]" va arriba).
    sf::Text texto = crearTexto(fuente, nombre_, 10 * ESCALA_TEXTO);
    texto.setScale(sf::Vector2f(1.f / ESCALA_TEXTO, 1.f / ESCALA_TEXTO));
    texto.setFillColor(sf::Color::White);

    const sf::Vector2f posicion = posicionRect(cuerpo_);
    const sf::Vector2f tamano   = tamanoRect(cuerpo_);
    const sf::Vector2f tamanoTexto = tamanoRect(texto.getGlobalBounds());

    texto.setPosition(sf::Vector2f(
        posicion.x + (tamano.x - tamanoTexto.x) / 2.f,
        posicion.y + tamano.y + 4.f
    ));

    ventana.draw(texto);

    // Aviso "[E]" si el jugador esta cerca.
    Interactuable::dibujar(ventana, fuente);
}
