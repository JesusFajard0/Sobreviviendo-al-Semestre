#include "world/Personaje.hpp"
#include "core/CompatSFML.hpp"

#include <cmath>
#include <algorithm>

// ---------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------

Personaje::Personaje(const sf::Vector2f& posicionInicial)
    : cuerpo_(sf::Vector2f(48.f, 72.f)),
      hitbox_(sf::Vector2f(28.f, 16.f)),
      velocidad_(120.f)
{
    // Configuración visual del personaje (solo se ve si no hay sprite).
    cuerpo_.setFillColor(sf::Color(59, 130, 246));
    cuerpo_.setPosition(posicionInicial);

    // Configuración del rectángulo de colisión.
    hitbox_.setFillColor(sf::Color::Transparent);
    hitbox_.setOutlineColor(sf::Color::Yellow);
    hitbox_.setOutlineThickness(1.f);

    // La hitbox se centra horizontalmente en los pies.
    hitbox_.setPosition(sf::Vector2f(
        posicionInicial.x + 10.f,
        posicionInicial.y + 54.f
    ));
}

// ---------------------------------------------------------------
// Hoja de sprites
// ---------------------------------------------------------------

void Personaje::ponerHojaSprites(const sf::Texture& textura)
{
    const sf::Vector2u tam = textura.getSize();

    cuadroAncho_ = static_cast<int>(tam.x) / COLUMNAS;
    cuadroAlto_  = static_cast<int>(tam.y) / FILAS;

    if (cuadroAncho_ <= 0 || cuadroAlto_ <= 0)
    {
        sprite_.reset();   // textura inválida: se sigue usando el rectángulo
        return;
    }

    escalaSprite_ = ANCHO_LOGICO / static_cast<float>(cuadroAncho_);

    sprite_.emplace(textura);
    sprite_->setScale(sf::Vector2f(escalaSprite_, escalaSprite_));

    cuadro_       = 0;
    tiempoCuadro_ = 0.f;
    direccion_    = Direccion::Abajo;
    aplicarCuadro();
}

void Personaje::aplicarCuadro()
{
    if (!sprite_)
        return;

    // Mapeo corregido según la disposición de columnas de tu imagen:
    // Columna 0: Frente (Abajo)
    // Columna 1: Perfil Derecho
    // Columna 2: Espalda (Arriba)
    // Columna 3: Perfil Izquierdo
    int columnaDireccion = 0;

    switch (direccion_)
    {
        case Direccion::Abajo:     columnaDireccion = 0; break;
        case Direccion::Derecha:   columnaDireccion = 1; break;
        case Direccion::Arriba:    columnaDireccion = 2; break;
        case Direccion::Izquierda: columnaDireccion = 3; break;
    }

    sprite_->setTextureRect(sf::IntRect(
        sf::Vector2i(columnaDireccion * cuadroAncho_, cuadro_ * cuadroAlto_),
        sf::Vector2i(cuadroAncho_, cuadroAlto_)
    ));
}

void Personaje::animar(float dt, const sf::Vector2f& direccion, bool seMovio)
{
    if (!sprite_)
        return;

    // La dirección solo cambia cuando hay entrada: al soltar las teclas
    // se queda mirando hacia donde iba. En diagonal gana el perfil.
    if (direccion.x != 0.f)
        direccion_ = (direccion.x < 0.f) ? Direccion::Izquierda : Direccion::Derecha;
    else if (direccion.y != 0.f)
        direccion_ = (direccion.y < 0.f) ? Direccion::Arriba : Direccion::Abajo;

    if (seMovio)
    {
        tiempoCuadro_ += dt;
        const float duracion = 1.f / CUADROS_POR_SEGUNDO;

        while (tiempoCuadro_ >= duracion)
        {
            tiempoCuadro_ -= duracion;
            cuadro_ = (cuadro_ + 1) % FILAS;
        }
    }
    else
    {
        // Quieto (o empujando una pared): cuadro de reposo.
        cuadro_       = 0;
        tiempoCuadro_ = 0.f;
    }

    aplicarCuadro();
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

    const sf::Vector2f posicionAntes = cuerpo_.getPosition();

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

    // -----------------------------------------------------------
    // 5. Animación (solo camina si de verdad se movió)
    // -----------------------------------------------------------

    const sf::Vector2f posicionDespues = cuerpo_.getPosition();
    const bool seMovio = (posicionDespues != posicionAntes);

    animar(dt, direccion, seMovio);
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
    if (sprite_)
    {
        // El sprite se apoya en los pies: su borde inferior coincide con el
        // borde inferior del cuerpo y queda centrado en horizontal.
        const sf::Vector2f pos = cuerpo_.getPosition();
        const sf::Vector2f tam = cuerpo_.getSize();

        const float ancho = static_cast<float>(cuadroAncho_) * escalaSprite_;
        const float alto  = static_cast<float>(cuadroAlto_)  * escalaSprite_;

        sf::Sprite dibujo = *sprite_;
        dibujo.setPosition(sf::Vector2f(
            std::round(pos.x + tam.x / 2.f - ancho / 2.f),
            std::round(pos.y + tam.y - alto)));

        destino.draw(dibujo);

        if (hitboxVisible_)
            destino.draw(hitbox_);
        return;
    }

    // Sin hoja de sprites: rectángulo y hitbox.
    destino.draw(cuerpo_);
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

    // Mismos desplazamientos ajustados para situar la hitbox en los pies.
    hitbox_.setPosition(sf::Vector2f(
        posicion.x + 10.f,
        posicion.y + 54.f
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