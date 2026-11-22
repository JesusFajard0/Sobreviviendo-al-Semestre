#pragma once

#include <SFML/Graphics.hpp>

#include <optional>

#include "core/GestorRecursos.hpp"
#include "core/State.hpp"
#include "systems/Notas.hpp"

class StateStack;

// Pantalla con la nota del examen. Se apila sobre EstadoExploracion apenas se
// termina de presentar; con ENTER / ESPACIO / E se cierra.
// Se dibuja en la vista logica de 640 x 360.
class ExamenState : public State
{
public:
    ExamenState(StateStack* stack, GestorRecursos& recursos,
                const ResultadoExamen& resultado);

    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;

private:
    void configurarTextos();
    void aplicarAlfa();   // fundido de entrada

    GestorRecursos& m_recursos;
    ResultadoExamen m_resultado;

    sf::RectangleShape m_fondoOscuro;
    sf::RectangleShape m_tarjeta;

    // En SFML 3 sf::Text necesita fuente al crearse, por eso optional.
    std::optional<sf::Text> m_titulo;
    std::optional<sf::Text> m_nota;
    std::optional<sf::Text> m_veredicto;
    std::optional<sf::Text> m_comentario;
    std::optional<sf::Text> m_desglose;
    std::optional<sf::Text> m_instruccion;

    sf::Color m_colorResultado;   // verde si aprobo, rojo si reprobo
    float     m_alfa = 0.f;       // 0 a 255
};
