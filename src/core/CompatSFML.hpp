
#pragma once

#include <SFML/Graphics.hpp>
#include <string>

#if SFML_VERSION_MAJOR >= 3

inline sf::FloatRect crearRect(
    float x, float y, float ancho, float alto)
{
    return sf::FloatRect(
        sf::Vector2f(x, y),
        sf::Vector2f(ancho, alto)
    );
}

inline bool interseca(
    const sf::FloatRect& a,
    const sf::FloatRect& b)
{
    return a.findIntersection(b).has_value();
}

inline sf::Vector2f posicionRect(
    const sf::FloatRect& rect)
{
    return rect.position;
}

inline sf::Vector2f tamanoRect(
    const sf::FloatRect& rect)
{
    return rect.size;
}

#else

inline sf::FloatRect crearRect(
    float x, float y, float ancho, float alto)
{
    return sf::FloatRect(x, y, ancho, alto);
}

inline bool interseca(
    const sf::FloatRect& a,
    const sf::FloatRect& b)
{
    return a.intersects(b);
}

inline sf::Vector2f posicionRect(
    const sf::FloatRect& rect)
{
    return sf::Vector2f(rect.left, rect.top);
}

inline sf::Vector2f tamanoRect(
    const sf::FloatRect& rect)
{
    return sf::Vector2f(rect.width, rect.height);
}

#endif

// ---------------------------------------------------------------
// Texto (cambia bastante entre SFML 2 y 3)
// ---------------------------------------------------------------

// Convierte un std::string en UTF-8 a sf::String (para tildes y ñ).
inline sf::String textoUtf8(const std::string& s)
{
    return sf::String::fromUtf8(s.begin(), s.end());
}

inline sf::Text crearTexto(const sf::Font& fuente, const std::string& utf8, unsigned tamano)
{
    #if SFML_VERSION_MAJOR >= 3
        return sf::Text(fuente, textoUtf8(utf8), tamano);
    #else
        return sf::Text(textoUtf8(utf8), fuente, tamano);
    #endif
}

inline float anchoTexto(const sf::Text& texto)
{
    #if SFML_VERSION_MAJOR >= 3
        return texto.getLocalBounds().size.x;
    #else
        return texto.getLocalBounds().width;
    #endif
}
