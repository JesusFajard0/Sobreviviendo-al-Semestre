#pragma once

#include <SFML/Graphics.hpp>

#include "world/DatosRastro.hpp"

// Posicion (relativa a la esquina del salon) de una particula del ambiente
// en el instante "tiempo". Es una funcion pura: no usa SFML ni azar, asi que
// siempre da lo mismo y se puede probar. Siempre cae dentro de ancho x alto.
struct PosParticula
{
    float x;
    float y;
};

PosParticula posicionParticula(EstiloRastro estilo, int indice, float tiempo,
                               float ancho, float alto);

// S3-3: ambiente de un salon segun el examen. Fiesta: tono calido y confeti
// que cae. Tristeza: tono azul grisaceo y lluvia. Se dibuja encima de la
// escena y debajo del HUD.
class AmbienteRastro
{
public:
    static constexpr int PARTICULAS = 36;

    explicit AmbienteRastro(EstiloRastro estilo) : estilo_(estilo) {}

    void actualizar(float dt) { tiempo_ += dt; }
    void dibujar(sf::RenderWindow& ventana) const;

    EstiloRastro estilo() const { return estilo_; }

private:
    EstiloRastro estilo_;
    float        tiempo_ = 0.f;
};
