#pragma once

#include <SFML/Graphics.hpp>

#include <optional>
#include <vector>

class Personaje
{
public:
    // Constructor: recibe la posición inicial del personaje.
    explicit Personaje(
        const sf::Vector2f& posicionInicial = sf::Vector2f(308.f, 160.f)
    );

    // Actualiza el movimiento, comprueba las colisiones y anima el sprite.
    void actualizar(
        float dt,
        const sf::Vector2f& direccion,
        const std::vector<sf::RectangleShape>& obstaculos
    );

    // Dibuja el personaje (sprite, o rectángulo si no hay hoja de sprites).
    void dibujar(sf::RenderTarget& destino) const;

    // Rectángulo de colisión (los pies), en coordenadas del mundo.
    sf::FloatRect hitbox() const;

    // Devuelve la posición actual del personaje.
    sf::Vector2f posicion() const { return cuerpo_.getPosition(); }

    // Coloca al personaje con la esquina superior izquierda del cuerpo en 'posicion'.
    void colocar(const sf::Vector2f& posicion);

    // Mantiene al personaje dentro de un área (por ejemplo, la pantalla).
    void limitarA(const sf::FloatRect& area);

    // S3-6: hoja de sprites de caminar. La textura debe seguir viva mientras
    // exista el personaje (GestorRecursos la conserva).
    //
    // Formato: cuadrícula de 4 columnas x 4 filas, todos los cuadros del mismo tamaño.
    //   Filas:    0 = abajo, 1 = izquierda, 2 = derecha, 3 = arriba
    //   Columnas: 0 = quieto, 1 = paso, 2 = quieto, 3 = paso
    // El tamaño de cada cuadro se calcula solo (ancho/4 y alto/4) y se escala
    // para que mida ANCHO_LOGICO de ancho en la vista de 640x360.
    void ponerHojaSprites(const sf::Texture& textura);

    // Con el sprite puesto, la hitbox amarilla solo se ve si se pide (modo F3).
    void ponerHitboxVisible(bool visible) { hitboxVisible_ = visible; }

private:
    enum class Direccion { Abajo = 0, Izquierda = 1, Derecha = 2, Arriba = 3 };

    // Comprueba si la hitbox colisionaría en una posición determinada.
    bool hayColisionEn(
        const sf::Vector2f& posicionHitbox,
        const std::vector<sf::RectangleShape>& obstaculos
    ) const;

    void animar(float dt, const sf::Vector2f& direccion, bool seMovio);
    void aplicarCuadro();

    static constexpr int   COLUMNAS     = 4;
    static constexpr int   FILAS        = 4;
    // Modificado: Aumentado de 32.f a 48.f para escalar el sprite 1.5x en pantalla
    static constexpr float ANCHO_LOGICO = 48.f;
    static constexpr float CUADROS_POR_SEGUNDO = 8.f;

    // Representación visual provisional (se usa si no hay hoja de sprites).
    sf::RectangleShape cuerpo_;

    // Rectángulo independiente de colisión ubicado en los pies.
    sf::RectangleShape hitbox_;

    // Velocidad de desplazamiento en unidades por segundo.
    float velocidad_;

    // --- Animación ---
    std::optional<sf::Sprite> sprite_;   // SFML 3: el sprite necesita textura al crearse
    int       cuadroAncho_  = 0;         // en píxeles de la textura
    int       cuadroAlto_   = 0;
    float     escalaSprite_ = 1.f;
    Direccion direccion_    = Direccion::Abajo;
    int       cuadro_       = 0;
    float     tiempoCuadro_ = 0.f;
    bool      hitboxVisible_ = false;
};