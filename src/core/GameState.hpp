#pragma once
#include "State.hpp"
#include "StateStack.hpp"
#include "PauseState.hpp"

class GameState : public State {
public:
    GameState(StateStack* stack) : State(stack) {}

    void handleEvent(const sf::Event& event) override {
        if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
            // Apila el menú de pausa; GameState queda congelado abajo
            m_stack->pushState(std::make_unique<PauseState>(m_stack));
        }
    }

    void update(float dt) override {
        // Lógica del semestre, movimiento del personaje, estadísticas
    }

    void draw(sf::RenderWindow& window) override {
        // window.draw(escenario);
        // window.draw(personaje);
    }
};