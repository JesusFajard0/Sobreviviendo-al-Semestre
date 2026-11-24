#pragma once

#include <SFML/Graphics.hpp>

#include <functional>
#include <string>

#include "world/Interactuable.hpp"

// NPC quieto. Es un Interactuable: al pulsar E avisa con el id de su
// conversacion y quien lo creo decide que pasa (igual que Puerta y
// ObjetoActividad). Mientras no haya arte se dibuja como un rectangulo.
class Npc : public Interactuable
{
public:
    // "cuerpo" es lo que se dibuja; el area donde se puede hablar con el es
    // un poco mas grande, para no tener que pisarlo.
    Npc(const sf::FloatRect& cuerpo,
        const std::string& nombre,
        const std::string& idDialogo,
        std::function<void(const std::string&)> alHablar);

    void interactuar() override;

    void dibujar(sf::RenderWindow& ventana, const sf::Font& fuente) const override;

    const std::string& nombre() const { return nombre_; }
    const std::string& idDialogo() const { return idDialogo_; }

    // S3-6: borde inferior del cuerpo (sus pies). Decide si el personaje
    // pasa por delante o por detras del NPC.
    float yPies() const;

private:
    std::string nombre_;
    std::string idDialogo_;
    std::function<void(const std::string&)> alHablar_;
    sf::FloatRect cuerpo_;
    sf::RectangleShape forma_;
};
