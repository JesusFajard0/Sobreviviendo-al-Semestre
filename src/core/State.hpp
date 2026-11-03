#pragma once
#include <SFML/Graphics.hpp>

class StateStack; // Declaración adelantada

class State {
protected:
    StateStack* m_stack;

public:
    State(StateStack* stack) : m_stack(stack) {}
    virtual ~State() = default;

    virtual void handleEvent(const sf::Event& event) = 0;
    virtual void update(float dt) = 0;
    virtual void draw(sf::RenderWindow& window) = 0;
};