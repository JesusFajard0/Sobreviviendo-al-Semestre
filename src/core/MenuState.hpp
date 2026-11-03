#pragma once
#include "State.hpp"
#include "StateStack.hpp"
#include "GameState.hpp"
#include <memory>

class MenuState : public State {
public:
    MenuState(StateStack* stack) : State(stack) {}

    void handleEvent(const sf::Event& event) override {
        if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Enter) {
            m_stack->popState();
            m_stack->pushState(std::make_unique<GameState>(m_stack));
        }
    }

    void update(float dt) override {
        // Lógica de animación o selección del menú principal
    }

    void draw(sf::RenderWindow& window) override {
        // Dibujar el título y las opciones del menú
    }
};