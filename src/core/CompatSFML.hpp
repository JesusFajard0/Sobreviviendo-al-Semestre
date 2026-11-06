
#pragma once

#include <SFML/Graphics.hpp>

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
