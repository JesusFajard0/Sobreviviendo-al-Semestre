#pragma once

#include <SFML/Graphics.hpp>

#include <functional>
#include <string>

#include "world/DatosRastro.hpp"
#include "world/Interactuable.hpp"

// S3-3: pizarra u objeto decorativo que deja el examen en un salon. Se dibuja
// con formas simples (mientras no haya arte): la pizarra lleva una carita
// feliz, triste o seria; los objetos son globos, trofeo, papel arrugado o una
// nubecita. Es un Interactuable: con E entrega su texto (el chiste del
// easter egg) y quien lo creo decide como mostrarlo, igual que Npc.
class ObjetoRastro : public Interactuable
{
public:
    using AlLeer = std::function<void(const std::string&)>;

    // "cuerpo" es lo que se dibuja; el area para leerlo es un poco mas grande.
    ObjetoRastro(const sf::FloatRect& cuerpo,
                 TipoRastro tipo,
                 EstiloRastro estilo,
                 const std::string& nombre,
                 const std::string& texto,
                 AlLeer alLeer);

    void interactuar() override;

    void dibujar(sf::RenderWindow& ventana, const sf::Font& fuente) const override;

    const std::string& nombre() const { return nombre_; }
    const std::string& texto() const { return texto_; }
    TipoRastro tipo() const { return tipo_; }
    EstiloRastro estilo() const { return estilo_; }

private:
    void dibujarPizarra(sf::RenderWindow& ventana, const sf::Font& fuente) const;
    void dibujarObjeto(sf::RenderWindow& ventana, const sf::Font& fuente) const;

    TipoRastro    tipo_;
    EstiloRastro  estilo_;
    std::string   nombre_;
    std::string   texto_;
    AlLeer        alLeer_;
    sf::FloatRect cuerpo_;
};
