#include "ObjetoActividad.hpp"

ObjetoActividad::ObjetoActividad(sf::FloatRect area, const std::string& nombreActividad)
    : Interactuable(area), m_nombreActividad(nombreActividad) {}

void ObjetoActividad::interactuar() {
    // Aquí más adelante se conectaran las estadísticas reales. 
    // Por los momentos, imprimimos en consola para probar que funciona.
    std::cout << "Haciendo actividad: " << m_nombreActividad << ". ¡Estadísticas actualizadas!" << std::endl;
}