#pragma once

#include <SFML/Graphics.hpp>

#include <array>
#include <optional>
#include <vector>

#include "core/GestorRecursos.hpp"
#include "core/State.hpp"
#include "systems/FinDeSemana.hpp"

class StateStack;

// Pantalla que resume el fin de semana con animación fluida (zoom, paneo y fundidos).
class FinDeSemanaState : public State
{
public:
    FinDeSemanaState(StateStack* stack, GestorRecursos& recursos,
                     const ResultadoFinDeSemana& resultado,
                     std::vector<const sf::Texture*> imagenes = {});

    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;

private:
    void configurarTextos();
    void actualizarImagenes(float dt);
    void aplicarTransformacion(sf::RectangleShape& shape, float avanceLocal, int indice);
    void actualizarLucesRumba();
    void aplicarAlfa();

    GestorRecursos&      m_recursos;
    ResultadoFinDeSemana m_resultado;

    std::vector<const sf::Texture*> m_imagenes;
    sf::RectangleShape m_imagenA;   // cuadro actual
    sf::RectangleShape m_imagenB;   // cuadro siguiente (para el fundido)

    // Luces suaves para darle ambiente a Rumba
    std::array<sf::RectangleShape, 2> m_lucesRumba;

    sf::RectangleShape m_fondoOscuro;
    sf::RectangleShape m_tarjeta;

    std::optional<sf::Text> m_titulo;
    std::optional<sf::Text> m_semana;
    std::optional<sf::Text> m_descripcion;
    std::optional<sf::Text> m_efectos;
    std::optional<sf::Text> m_instruccion;

    sf::Color m_colorBorde;
    float     m_tiempo           = 0.f;
    float     m_periodo          = 1.4f;     // segundos por cuadro
    bool      m_bucle            = false;
    float     m_duracion         = 0.f;
    bool      m_mostrandoTarjeta = true;
    float     m_alfa             = 0.f;      // 0 a 255
};