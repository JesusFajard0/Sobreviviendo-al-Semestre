#include "world/ObjetoActividad.hpp"

#include <utility>

#include "core/CompatSFML.hpp"

namespace
{
    // El texto se crea al doble de tamaño y se reduce a la mitad
    // para que las letras salgan nítidas (igual que en MenuDestinos).
    constexpr unsigned ESCALA_TEXTO = 2;

    // Convierte un std::string en UTF-8 a texto de SFML (acentos).
    sf::String aTexto(const std::string& s)
    {
        return sf::String::fromUtf8(s.begin(), s.end());
    }
}

ObjetoActividad::ObjetoActividad(
    const sf::FloatRect& area,
    const std::string& idActividad,
    const std::string& etiqueta,
    std::function<void(const std::string&)> alUsar)
    : Interactuable(area),
      idActividad_(idActividad),
      etiqueta_(etiqueta),
      alUsar_(std::move(alUsar))
{
    forma_.setSize(tamanoRect(area));
    forma_.setPosition(posicionRect(area));
    forma_.setFillColor(sf::Color(70, 110, 160, 200));
    forma_.setOutlineColor(sf::Color::White);
    forma_.setOutlineThickness(1.f);
}

void ObjetoActividad::interactuar()
{
    if (alUsar_)
        alUsar_(idActividad_);
}

void ObjetoActividad::dibujar(sf::RenderWindow& ventana,
                              const sf::Font& fuente) const
{
    ventana.draw(forma_);

    // Nombre centrado encima del objeto.
    sf::Text texto(fuente, aTexto(etiqueta_), 10 * ESCALA_TEXTO);
    texto.setScale(sf::Vector2f(1.f / ESCALA_TEXTO, 1.f / ESCALA_TEXTO));
    texto.setFillColor(sf::Color::White);

    const sf::Vector2f posicion = posicionRect(m_areaInteraccion);
    const sf::Vector2f tamano = tamanoRect(m_areaInteraccion);
    const sf::Vector2f tamanoTexto = tamanoRect(texto.getGlobalBounds());

    texto.setPosition(sf::Vector2f(
        posicion.x + (tamano.x - tamanoTexto.x) / 2.f,
        posicion.y - tamanoTexto.y - 8.f
    ));

    ventana.draw(texto);

    // Aviso "[E]" si el jugador está encima.
    Interactuable::dibujar(ventana, fuente);
}