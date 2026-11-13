#pragma once

#include <SFML/Graphics.hpp>

#include <functional>
#include <string>

#include "core/State.hpp"

// S2-3: animacion provisional de "asistir a clase".
// Se apila sobre EstadoExploracion: oscurece la pantalla, muestra un texto
// y una barra de progreso, y al terminar se quita solo y llama a alTerminar
// (ahi se aplican los efectos y se avanza el bloque).
class ClaseState : public State
{
public:
    using AlTerminar = std::function<void()>;

    ClaseState(StateStack* pila,
               const sf::Font& fuente,
               const std::string& materia,
               AlTerminar alTerminar = {});

    void handleEvent(const sf::Event& event) override;   // bloquea la entrada
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;

private:
    static constexpr float DURACION = 2.0f;   // segundos
    static constexpr float FUNDIDO  = 0.3f;   // entrada y salida
    static constexpr float ANCHO    = 640.f;  // vista logica
    static constexpr float ALTO     = 360.f;

    sf::Text   texto_;
    AlTerminar alTerminar_;
    float      transcurrido_ = 0.f;
};
