#pragma once

#include <SFML/Graphics.hpp>

#include <optional>

#include "core/GestorRecursos.hpp"
#include "core/State.hpp"
#include "systems/FinDeSemana.hpp"

class StateStack;

// Pantalla que resume el fin de semana (lo que "hizo" el personaje).
// Se apila sobre EstadoExploracion; con ENTER / ESPACIO / E se cierra.
// Se dibuja en la vista logica de 640 x 360.
class FinDeSemanaState : public State
{
public:
    FinDeSemanaState(StateStack* stack, GestorRecursos& recursos,
                     const ResultadoFinDeSemana& resultado);

    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;

private:
    void configurarTextos();
    void aplicarAlfa();   // fundido de entrada

    GestorRecursos&      m_recursos;
    ResultadoFinDeSemana m_resultado;

    sf::RectangleShape m_fondoOscuro;
    sf::RectangleShape m_tarjeta;

    // En SFML 3 sf::Text necesita fuente al crearse, por eso optional.
    std::optional<sf::Text> m_titulo;
    std::optional<sf::Text> m_semana;
    std::optional<sf::Text> m_descripcion;
    std::optional<sf::Text> m_efectos;
    std::optional<sf::Text> m_instruccion;

    sf::Color m_colorBorde;
    float     m_alfa = 0.f;   // 0 a 255
};
