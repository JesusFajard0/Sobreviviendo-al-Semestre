#include "Puerta.hpp"

Puerta::Puerta(sf::FloatRect area, const std::string& destinoId)
    : Interactuable(area), m_destinoId(destinoId) {}

void Puerta::interactuar() {
    // Aquí irá la lógica para cambiar de escena o mapa
    std::cout << "Interactuando con la puerta. Viajando a: " << m_destinoId << std::endl;
}