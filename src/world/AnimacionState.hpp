#pragma once

#include <SFML/Graphics.hpp>

#include <functional>
#include <string>
#include <vector>

#include "core/State.hpp"

// S3-4: animaciones de actividades (clase, examen, discoteca, dormir, siesta y duchar).
//
// Se apila sobre EstadoExploracion, cubre la pantalla con un fundido, reproduce
// la animacion y se quita sola. ENTER, E, ESPACIO o ESC la saltan (despues de
// medio segundo). Todo se dibuja con formas simples y, si existe, con la hoja
// de sprites del personaje (assets/sprites/personaje.png), asi que cuando
// llegue el arte definitivo solo hay que cambiar el PNG.
enum class TipoAnimacion
{
    Clase,
    Examen,
    Discoteca,
    Dormir,    // de noche: anochece, se duerme y amanece
    Siesta,    // version corta y de dia
    Duchar
};

class AnimacionState : public State
{
public:
    using AlTerminar = std::function<void()>;

    // hojaPersonaje puede ser nullptr (se dibuja un rectangulo azul).
    // detalle: nombre de la materia (clase y examen); en las demas se ignora.
    // imagenes: dibujos de la escena (assets/animaciones/*.png); si la lista
    // esta vacia se usa el dibujo provisional hecho con codigo.
    // textoFinal: texto grande que sale abajo al final (la nota del examen).
    AnimacionState(StateStack* pila,
                   const sf::Font& fuente,
                   const sf::Texture* hojaPersonaje,
                   TipoAnimacion tipo,
                   const std::string& detalle = "",
                   std::vector<const sf::Texture*> imagenes = {},
                   const std::string& textoFinal = "",
                   sf::Color colorFinal = sf::Color::White,
                   AlTerminar alTerminar = {});

    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;

private:
    float duracion() const;
    float inicioFinal() const { return duracion() * 0.55f; }
    void  saltar();
    void  dibujarTextoFinal(sf::RenderWindow& w);

    // Versiones con imagenes (una por animacion).
    void dibujarConImagenes(sf::RenderWindow& w);
    void imagen(sf::RenderWindow& w, const sf::Texture& tex, float opacidad, float zoom = 1.f) const;
    void rotulo(sf::RenderWindow& w, const std::string& s) const;

    // Una funcion por animacion.
    void dibujarClase(sf::RenderWindow& w);
    void dibujarExamen(sf::RenderWindow& w);
    void dibujarDiscoteca(sf::RenderWindow& w);
    void dibujarDormir(sf::RenderWindow& w, bool noche);
    void dibujarDuchar(sf::RenderWindow& w);

    // Utilidades de dibujo (todas aplican el fundido k_).
    sf::Color alfa(const sf::Color& c) const;
    void rect(sf::RenderWindow& w, float x, float y, float ancho, float alto,
              const sf::Color& color) const;
    void circulo(sf::RenderWindow& w, float cx, float cy, float radio,
                 const sf::Color& color) const;
    void texto(sf::RenderWindow& w, const std::string& s, unsigned tamano,
               float centroX, float y, const sf::Color& color, bool sombra = true) const;
    void barra(sf::RenderWindow& w) const;
    void personaje(sf::RenderWindow& w, int fila, int cuadro, float centroX,
                   float yPies, float escala, bool espejo = false,
                   float grados = 0.f) const;

    const sf::Font&    fuente_;
    const sf::Texture* hoja_;
    TipoAnimacion      tipo_;
    std::string        detalle_;
    std::vector<const sf::Texture*> imagenes_;
    std::string        textoFinal_;
    sf::Color          colorFinal_;
    AlTerminar         alTerminar_;

    float t_ = 0.f;   // segundos transcurridos
    float k_ = 0.f;   // opacidad del fundido (0 a 1)
};