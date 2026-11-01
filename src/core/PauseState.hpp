#pragma once
#include "State.hpp"
#include "StateStack.hpp"

class PauseState : public State {
public:
    PauseState(StateStack* stack) : State(stack) {}

    void handleEvent(const sf::Event& event) override {
        if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
            m_stack->popState();
        }
    }

    void update(float dt) override {
        // La pausa no actualiza el juego de fondo
    }

    void draw(sf::RenderWindow& window) override {
        // Dibujar superposición o interfaz de pausa
    }
};