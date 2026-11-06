
#include "Interactuable.hpp"
#include "core/CompatSFML.hpp"

Interactuable::Interactuable(sf::FloatRect area)
    : m_areaInteraccion(area), m_mostrarAviso(false)
{
}

bool Interactuable::puedeInteractuar(
    const sf::FloatRect& hitboxJugador)
{
    m_mostrarAviso = interseca(
        m_areaInteraccion,
        hitboxJugador
    );

    return m_mostrarAviso;
}

void Interactuable::dibujar(
    sf::RenderWindow& ventana,
    const sf::Font& fuente
) const
{
    if (m_mostrarAviso)
    {
        // Se crea al doble de tamaño y se reduce a la mitad
        // para que se vea nítido en la ventana ampliada.
        sf::Text textoE(fuente, "[E]", 40);
        textoE.setScale(sf::Vector2f(0.5f, 0.5f));

        textoE.setFillColor(sf::Color::Yellow);

        const sf::Vector2f posicion =
            posicionRect(m_areaInteraccion);

        const sf::Vector2f tamano =
            tamanoRect(m_areaInteraccion);

        textoE.setPosition(
            sf::Vector2f(
                posicion.x + tamano.x / 2.f - 10.f,
                posicion.y - 25.f
            )
        );

        ventana.draw(textoE);
    }
}
