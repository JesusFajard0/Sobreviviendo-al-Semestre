#include "Interactuable.hpp"

Interactuable::Interactuable(sf::FloatRect area) 
    : m_areaInteraccion(area), m_mostrarAviso(false) {}

bool Interactuable::puedeInteractuar(const sf::FloatRect& hitboxJugador) {
    m_mostrarAviso = m_areaInteraccion.intersects(hitboxJugador);
    return m_mostrarAviso;
}

void Interactuable::dibujar(sf::RenderWindow& ventana, const sf::Font& fuente) const {
    if (m_mostrarAviso) {
        sf::Text textoE;
        textoE.setFont(fuente);
        textoE.setString("[E]");
        textoE.setCharacterSize(20);
        textoE.setFillColor(sf::Color::Yellow);
        
        // Posicionar el aviso justo encima del área interactuable
        textoE.setPosition(m_areaInteraccion.left + (m_areaInteraccion.width / 2.f) - 10.f, 
                           m_areaInteraccion.top - 25.f);
        
        ventana.draw(textoE);
    }
}