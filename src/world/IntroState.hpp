#pragma once

#include <SFML/Graphics.hpp>

#include <optional>
#include <string>
#include <vector>

#include "core/GestorRecursos.hpp"
#include "core/State.hpp"

class StateStack;

struct DiapositivaIntro
{
    std::string rutaImagen;
    std::string texto;
};

// Secuencia introductoria con viñetas que avanzan con ENTER / ESPACIO / E.
class IntroState : public State
{
public:
    IntroState(StateStack* stack, GestorRecursos& recursos,
               std::vector<DiapositivaIntro> diapositivas);

    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;

private:
    void cargarDiapositiva(std::size_t indice);
    void avanzar();

    GestorRecursos& m_recursos;
    std::vector<DiapositivaIntro> m_diapositivas;
    std::size_t m_indiceActual = 0;

    sf::RectangleShape m_fondo;
    sf::RectangleShape m_imagen;
    bool               m_tieneImagen = false;

    sf::RectangleShape m_cajaTexto;
    std::optional<sf::Text> m_texto;
    std::optional<sf::Text> m_instruccion;
};