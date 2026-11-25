#pragma once

#include <SFML/Graphics.hpp>

#include <functional>
#include <vector>

#include "core/State.hpp"

// Animación del viaje en bus: muestra varias imágenes una tras otra, con
// fundido, zoom lento y vibración. ENTER, E, ESPACIO o ESC la saltan.
// Al terminar se quita sola y llama a alTerminar (opcional).
class BusState : public State
{
public:
    using AlTerminar = std::function<void()>;

    BusState(StateStack* pila,
             std::vector<const sf::Texture*> imagenes,
             AlTerminar alTerminar = {});

    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;

private:
    void terminar();   // después de llamarla no se puede usar ningún miembro

    static constexpr float POR_IMAGEN = 2.5f;   // segundos por imagen
    static constexpr float FUNDIDO    = 0.5f;   // entrada y salida de cada una
    static constexpr float ANCHO      = 640.f;  // vista lógica
    static constexpr float ALTO       = 360.f;

    std::vector<const sf::Texture*> imagenes_;
    AlTerminar alTerminar_;
    float      t_ = 0.f;
};