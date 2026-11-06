#ifndef OBJETO_ACTIVIDAD_HPP
#define OBJETO_ACTIVIDAD_HPP

#include "Interactuable.hpp"
#include <iostream>
#include <string>

class ObjetoActividad : public Interactuable {
private:
    std::string m_nombreActividad;

public:
    ObjetoActividad(sf::FloatRect area, const std::string& nombreActividad);
    void interactuar() override;
};

#endif