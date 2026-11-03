#pragma once
#include <vector>
#include <memory>
#include "State.hpp"

class StateStack {
private:
    std::vector<std::unique_ptr<State>> m_states;

public:
    void pushState(std::unique_ptr<State> state) {
        m_states.push_back(std::move(state));
    }

    void popState() {
        if (!m_states.empty()) m_states.pop_back();
    }

    void handleEvent(const sf::Event& event) {
        if (!m_states.empty()) m_states.back()->handleEvent(event);
    }

    void update(float dt) {
        if (!m_states.empty()) m_states.back()->update(dt);
    }

    void draw(sf::RenderWindow& window) {
        // Itera desde el fondo hasta la cima para dibujar las capas superpuestas
        for (auto& state : m_states) {
            state->draw(window);
        }
    }
    
    bool isEmpty() const { return m_states.empty(); }
};