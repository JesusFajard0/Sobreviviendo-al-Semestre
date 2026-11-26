#include "world/ObjetoRastro.hpp"

#include <algorithm>
#include <utility>

#include "core/CompatSFML.hpp"

namespace
{
    constexpr unsigned ESCALA_TEXTO = 2;

    // Cuanto crece el area para poder leerlo sin pisarlo. La pizarra va en la
    // pared de arriba, asi que se puede leer desde mas lejos.
    float alcance(TipoRastro tipo)
    {
        return tipo == TipoRastro::Pizarra ? 24.f : 14.f;
    }

    sf::FloatRect areaDeLectura(const sf::FloatRect& cuerpo, TipoRastro tipo)
    {
        const float a = alcance(tipo);
        const sf::Vector2f p = posicionRect(cuerpo);
        const sf::Vector2f t = tamanoRect(cuerpo);
        return crearRect(p.x - a, p.y - a, t.x + 2.f * a, t.y + 2.f * a);
    }

    // Formas sueltas para armar los dibujos.
    void rectangulo(sf::RenderWindow& v, float x, float y, float w, float h, const sf::Color& color)
    {
        sf::RectangleShape r(sf::Vector2f(w, h));
        r.setPosition(sf::Vector2f(x, y));
        r.setFillColor(color);
        v.draw(r);
    }

    void circulo(sf::RenderWindow& v, float cx, float cy, float radio, const sf::Color& color)
    {
        sf::CircleShape c(radio);
        c.setPosition(sf::Vector2f(cx - radio, cy - radio));
        c.setFillColor(color);
        v.draw(c);
    }

    void aro(sf::RenderWindow& v, float cx, float cy, float radio, float grosor, const sf::Color& color)
    {
        sf::CircleShape c(radio);
        c.setPosition(sf::Vector2f(cx - radio, cy - radio));
        c.setFillColor(sf::Color::Transparent);
        c.setOutlineColor(color);
        c.setOutlineThickness(grosor);
        v.draw(c);
    }

    // Carita de tiza: ojos y boca hecha con puntitos (sonrisa, ceño o linea).
    void carita(sf::RenderWindow& v, float cx, float cy, float radio, EstiloRastro estilo)
    {
        const sf::Color tiza(240, 240, 230);

        aro(v, cx, cy, radio, 1.5f, tiza);

        const float ojo = std::max(1.3f, radio * 0.13f);
        circulo(v, cx - radio * 0.38f, cy - radio * 0.25f, ojo, tiza);
        circulo(v, cx + radio * 0.38f, cy - radio * 0.25f, ojo, tiza);

        for (int k = 0; k <= 8; ++k)
        {
            const float u = -1.f + 0.25f * static_cast<float>(k);   // -1 a 1
            const float curva = (1.f - u * u) * radio * 0.22f;

            float y = cy + radio * 0.38f;                           // Serio: recta
            if (estilo == EstiloRastro::Feliz)
                y = cy + radio * 0.15f + curva;                     // el centro mas bajo
            else if (estilo == EstiloRastro::Triste)
                y = cy + radio * 0.55f - curva;                     // el centro mas alto

            rectangulo(v, cx + u * radio * 0.5f - 0.8f, y - 0.8f, 1.6f, 1.6f, tiza);
        }

        if (estilo == EstiloRastro::Triste)   // una lagrima
            rectangulo(v, cx - radio * 0.38f - 0.8f, cy - radio * 0.05f, 1.6f, radio * 0.35f,
                       sf::Color(120, 170, 255));
    }
}

ObjetoRastro::ObjetoRastro(const sf::FloatRect& cuerpo,
                           TipoRastro tipo,
                           EstiloRastro estilo,
                           const std::string& nombre,
                           const std::string& texto,
                           AlLeer alLeer)
    : Interactuable(areaDeLectura(cuerpo, tipo)),
      tipo_(tipo),
      estilo_(estilo),
      nombre_(nombre),
      texto_(texto),
      alLeer_(std::move(alLeer)),
      cuerpo_(cuerpo)
{
}

void ObjetoRastro::interactuar()
{
    if (alLeer_)
        alLeer_(texto_);
}

void ObjetoRastro::dibujar(sf::RenderWindow& ventana, const sf::Font& fuente) const
{
    if (tipo_ == TipoRastro::Pizarra)
        dibujarPizarra(ventana, fuente);
    else
        dibujarObjeto(ventana, fuente);

    // Aviso "[E]" si el jugador esta cerca.
    Interactuable::dibujar(ventana, fuente);
}

// Pizarra verde con marco de madera: la carita a la izquierda y el titulo
// (por ejemplo la nota) a su derecha.
void ObjetoRastro::dibujarPizarra(sf::RenderWindow& ventana, const sf::Font& fuente) const
{
    const sf::Vector2f p = posicionRect(cuerpo_);
    const sf::Vector2f t = tamanoRect(cuerpo_);

    sf::RectangleShape tablero(t);
    tablero.setPosition(p);
    tablero.setFillColor(sf::Color(30, 70, 50, 245));
    tablero.setOutlineColor(sf::Color(150, 110, 70));
    tablero.setOutlineThickness(3.f);
    ventana.draw(tablero);

    const float radio = t.y * 0.34f;
    const float cx = p.x + t.y * 0.6f;
    const float cy = p.y + t.y * 0.5f;
    carita(ventana, cx, cy, radio, estilo_);

    sf::Text titulo = crearTexto(fuente, nombre_, 10 * ESCALA_TEXTO);
    titulo.setScale(sf::Vector2f(1.f / ESCALA_TEXTO, 1.f / ESCALA_TEXTO));
    titulo.setFillColor(sf::Color(240, 240, 230));

    const sf::Vector2f tt = tamanoRect(titulo.getGlobalBounds());
    titulo.setPosition(sf::Vector2f(p.x + t.y * 1.1f, p.y + (t.y - tt.y) / 2.f));
    ventana.draw(titulo);
}

// Objetos de decoracion hechos con formas, con su nombre debajo.
void ObjetoRastro::dibujarObjeto(sf::RenderWindow& ventana, const sf::Font& fuente) const
{
    const sf::Vector2f p = posicionRect(cuerpo_);
    const sf::Vector2f t = tamanoRect(cuerpo_);
    const float cx = p.x + t.x / 2.f;

    switch (estilo_)
    {
    case EstiloRastro::Globos:
    {
        const float r = t.x * 0.22f;
        const float yg = p.y + r;
        rectangulo(ventana, cx - 0.5f, yg + r, 1.f, t.y - 2.f * r, sf::Color(220, 220, 220));
        circulo(ventana, cx - r * 1.1f, yg + r * 0.4f, r, sf::Color(235, 70, 70));
        circulo(ventana, cx + r * 1.1f, yg + r * 0.4f, r, sf::Color(70, 140, 235));
        circulo(ventana, cx, yg - r * 0.2f, r, sf::Color(250, 210, 60));
        break;
    }

    case EstiloRastro::Trofeo:
        rectangulo(ventana, p.x + t.x * 0.25f, p.y, t.x * 0.5f, t.y * 0.5f, sf::Color(240, 200, 60));
        rectangulo(ventana, cx - t.x * 0.06f, p.y + t.y * 0.5f, t.x * 0.12f, t.y * 0.25f, sf::Color(200, 160, 40));
        rectangulo(ventana, p.x + t.x * 0.2f, p.y + t.y * 0.75f, t.x * 0.6f, t.y * 0.25f, sf::Color(120, 80, 40));
        break;

    case EstiloRastro::Papel:
        rectangulo(ventana, p.x, p.y + t.y * 0.1f, t.x * 0.9f, t.y * 0.8f, sf::Color(200, 200, 195));
        rectangulo(ventana, p.x + t.x * 0.1f, p.y, t.x * 0.8f, t.y * 0.8f, sf::Color(235, 235, 225));
        rectangulo(ventana, p.x + t.x * 0.25f, p.y + t.y * 0.25f, t.x * 0.4f, t.y * 0.12f, sf::Color(210, 50, 50));
        break;

    case EstiloRastro::Nube:
    {
        const float r = t.y * 0.28f;
        const sf::Color gris(175, 180, 195);
        circulo(ventana, p.x + t.x * 0.28f, p.y + r * 1.1f, r, gris);
        circulo(ventana, p.x + t.x * 0.55f, p.y + r * 0.8f, r * 1.2f, gris);
        circulo(ventana, p.x + t.x * 0.78f, p.y + r * 1.1f, r, gris);
        for (int k = 0; k < 3; ++k)
            rectangulo(ventana, p.x + t.x * (0.3f + 0.2f * static_cast<float>(k)), p.y + t.y * 0.7f,
                       1.2f, t.y * 0.3f, sf::Color(120, 170, 255));
        break;
    }

    default:
        break;
    }

    sf::Text etiqueta = crearTexto(fuente, nombre_, 10 * ESCALA_TEXTO);
    etiqueta.setScale(sf::Vector2f(1.f / ESCALA_TEXTO, 1.f / ESCALA_TEXTO));
    etiqueta.setFillColor(sf::Color::White);

    const sf::Vector2f tt = tamanoRect(etiqueta.getGlobalBounds());
    etiqueta.setPosition(sf::Vector2f(cx - tt.x / 2.f, p.y + t.y + 4.f));
    ventana.draw(etiqueta);
}
