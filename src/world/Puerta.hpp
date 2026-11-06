#ifndef PUERTA_HPP
#define PUERTA_HPP

#include "Interactuable.hpp"
#include <iostream>

class Puerta : public Interactuable {
private:
    std::string m_destinoId; // A qué lugar lleva

public:
    Puerta(sf::FloatRect area, const std::string& destinoId);

    void interactuar() override;
};

#endif