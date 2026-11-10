#include "world/Personaje.hpp"
#include "core/CompatSFML.hpp"

#include <cmath>
#include <algorithm>

// ---------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------

Personaje::Personaje(const sf::Vector2f& posicionInicial)
    : cuerpo_(sf::Vector2f(24.f, 36.f)),
      hitbox_(sf::Vector2f(14.f, 8.f)),
      velocidad_(120.f)
{
    // Configuración visual del personaje.
    cuerpo_.setFillColor(sf::Color(59, 130, 246));
    cuerpo_.setPosition(posicionInicial);

    // Configuración del rectángulo de colisión.
    hitbox_.setFillColor(sf::Color::Transparent);
    hitbox_.setOutlineColor(sf::Color::Yellow);
    hitbox_.setOutlineThickness(1.f);

    // La hitbox se centra horizontalmente en los pies.
    hitbox_.setPosition(sf::Vector2f(
        posicionInicial.x + 5.f,
        posicionInicial.y + 27.f
    ));
}

// ---------------------------------------------------------------
// Actualización del movimiento
// ---------------------------------------------------------------

void Personaje::actualizar(
    float dt,
    const sf::Vector2f& direccion,
    const std::vector<sf::RectangleShape>& obstaculos
)
{
    if (dt <= 0.f)
        return;

    // -----------------------------------------------------------
    // 1. Normalizar la dirección
    // -----------------------------------------------------------

    sf::Vector2f direccionNormalizada = direccion;

    float longitud = std::sqrt(
        direccionNormalizada.x * direccionNormalizada.x +
        direccionNormalizada.y * direccionNormalizada.y
    );

    if (longitud > 0.f)
    {
        direccionNormalizada.x /= longitud;
        direccionNormalizada.y /= longitud;
    }

    // -----------------------------------------------------------
    // 2. Calcular el desplazamiento
    // -----------------------------------------------------------

    float desplazamientoX =
        direccionNormalizada.x * velocidad_ * dt;

    float desplazamientoY =
        direccionNormalizada.y * velocidad_ * dt;

    // -----------------------------------------------------------
    // 3. Intentar mover en el eje X
    // -----------------------------------------------------------

    sf::Vector2f nuevaPosicionX = hitbox_.getPosition();

    nuevaPosicionX.x += desplazamientoX;

    if (!hayColisionEn(nuevaPosicionX, obstaculos))
    {
        cuerpo_.move(sf::Vector2f(desplazamientoX, 0.f));
        hitbox_.move(sf::Vector2f(desplazamientoX, 0.f));
    }

    // -----------------------------------------------------------
    // 4. Intentar mover en el eje Y
    // -----------------------------------------------------------

    sf::Vector2f nuevaPosicionY = hitbox_.getPosition();

    nuevaPosicionY.y += desplazamientoY;

    if (!hayColisionEn(nuevaPosicionY, obstaculos))
    {
        cuerpo_.move(sf::Vector2f(0.f, desplazamientoY));
        hitbox_.move(sf::Vector2f(0.f, desplazamientoY));
    }
}

// ---------------------------------------------------------------
// Detección de colisiones
// ---------------------------------------------------------------

bool Personaje::hayColisionEn(
    const sf::Vector2f& posicionHitbox,
    const std::vector<sf::RectangleShape>& obstaculos
) const
{
    // Dimensiones de la hitbox del personaje.
    sf::Vector2f tamanoHitbox = hitbox_.getSize();

    // Límites de la hitbox en la posición que queremos comprobar.
    float izquierdaPersonaje = posicionHitbox.x;
    float derechaPersonaje =
        posicionHitbox.x + tamanoHitbox.x;

    float arribaPersonaje = posicionHitbox.y;
    float abajoPersonaje =
        posicionHitbox.y + tamanoHitbox.y;

    // Revisar todos los obstáculos.
    for (const sf::RectangleShape& obstaculo : obstaculos)
    {
        sf::Vector2f posicionObstaculo = obstaculo.getPosition();
        sf::Vector2f tamanoObstaculo = obstaculo.getSize();

        float izquierdaObstaculo = posicionObstaculo.x;
        float derechaObstaculo =
            posicionObstaculo.x + tamanoObstaculo.x;

        float arribaObstaculo = posicionObstaculo.y;
        float abajoObstaculo =
            posicionObstaculo.y + tamanoObstaculo.y;

        // Comprobar si existe intersección entre ambos rectángulos.
        bool colisionHorizontal =
            izquierdaPersonaje < derechaObstaculo &&
            derechaPersonaje > izquierdaObstaculo;

        bool colisionVertical =
            arribaPersonaje < abajoObstaculo &&
            abajoPersonaje > arribaObstaculo;

        if (colisionHorizontal && colisionVertical)
            return true;
    }

    return false;
}

// ---------------------------------------------------------------
// Dibujo
// ---------------------------------------------------------------

void Personaje::dibujar(sf::RenderTarget& destino) const
{
    destino.draw(cuerpo_);

    // Se dibuja la hitbox para facilitar las pruebas.
    destino.draw(hitbox_);
}

sf::FloatRect Personaje::hitbox() const
{
    const sf::Vector2f pos = hitbox_.getPosition();
    const sf::Vector2f tam = hitbox_.getSize();

    return crearRect(pos.x, pos.y, tam.x, tam.y);
}

void Personaje::colocar(const sf::Vector2f& posicion)
{
    cuerpo_.setPosition(posicion);

    // Mismos desplazamientos que usa el constructor para los pies.
    hitbox_.setPosition(sf::Vector2f(
        posicion.x + 5.f,
        posicion.y + 27.f
    ));
}

void Personaje::limitarA(const sf::FloatRect& area)
{
    const sf::Vector2f origen = posicionRect(area);
    const sf::Vector2f tamano = tamanoRect(area);
    const sf::Vector2f tamanoCuerpo = cuerpo_.getSize();

    sf::Vector2f pos = cuerpo_.getPosition();

    pos.x = std::clamp(pos.x, origen.x,
                       origen.x + tamano.x - tamanoCuerpo.x);

    pos.y = std::clamp(pos.y, origen.y,
                       origen.y + tamano.y - tamanoCuerpo.y);

    // colocar() mueve también la hitbox de los pies.
    colocar(pos);
}