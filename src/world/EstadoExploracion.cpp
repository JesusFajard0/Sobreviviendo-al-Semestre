#include "world/EstadoExploracion.hpp"

#include <functional>
#include <iostream>
#include <utility>

#include "core/StateStack.hpp"
#include "world/MenuDestinos.hpp"

EstadoExploracion::EstadoExploracion(StateStack* pila, GestorRecursos& recursos, Estadisticas& estadisticas, Tiempo& tiempo)
    : State(pila),
      recursos_(recursos),
      estadisticas_(estadisticas),
      tiempo_(tiempo),
      personaje_(),
      fase_(1)
{
    if (!mapa_.cargar("data/escenas.txt"))
    {
        std::cerr << "[EstadoExploracion] No se pudo cargar el mapa.\n";
        return;
    }

    cambiarEscena(mapa_.raiz());
}

const sf::Font& EstadoExploracion::fuente() const
{
    // Si la fuente no cargó, se usa una vacía para no fallar.
    static const sf::Font vacia;

    const sf::Font* f = recursos_.obtenerFuente("principal");
    return (f != nullptr) ? *f : vacia;
}

void EstadoExploracion::cambiarEscena(const std::string& id)
{
    const DatosEscena* datos = mapa_.datos(id);

    if (datos == nullptr)
    {
        std::cerr << "[EstadoExploracion] Escena inexistente: "
                  << id << '\n';
        return;
    }

    escena_ = std::make_unique<Escena>(
        *datos,
        mapa_.rectangulos(id),
        [this]() { abrirMenuDestinos(); }
    );

    personaje_.colocar(escena_->spawn());
}

void EstadoExploracion::abrirMenuDestinos()
{
    if (!escena_)
        return;

    const std::string idActual = escena_->id();

    auto alElegir = [this, idActual](const std::string& idDestino)
    {
        switch (mapa_.puedeIr(idActual, idDestino, fase_))
        {
        case ResultadoCambio::Ok:
            cambiarEscena(idDestino);
            break;

        case ResultadoCambio::Animacion:
            std::cout << "[EstadoExploracion] Discoteca: "
                      << "animacion pendiente.\n";
            break;

        case ResultadoCambio::Bloqueada:
        case ResultadoCambio::NoExiste:
            break;
        }
    };

    m_stack->pushState(std::make_unique<MenuDestinos>(
        m_stack,
        fuente(),
        mapa_.destinos(idActual, fase_),
        alElegir
    ));
}

void EstadoExploracion::handleEvent(const sf::Event& event)
{
    const auto* tecla = event.getIf<sf::Event::KeyPressed>();

    if (tecla == nullptr ||
        tecla->code != sf::Keyboard::Key::E ||
        !escena_)
    {
        return;
    }

    Puerta* puerta = escena_->puerta();

    if (puerta != nullptr &&
        puerta->puedeInteractuar(personaje_.hitbox()))
    {
        puerta->interactuar();
    }
}

void EstadoExploracion::update(float dt)
{
    if (!escena_)
        return;

    using Tecla = sf::Keyboard::Key;
    sf::Vector2f direccion(0.f, 0.f);

    if (sf::Keyboard::isKeyPressed(Tecla::A) ||
        sf::Keyboard::isKeyPressed(Tecla::Left))
        direccion.x -= 1.f;

    if (sf::Keyboard::isKeyPressed(Tecla::D) ||
        sf::Keyboard::isKeyPressed(Tecla::Right))
        direccion.x += 1.f;

    if (sf::Keyboard::isKeyPressed(Tecla::W) ||
        sf::Keyboard::isKeyPressed(Tecla::Up))
        direccion.y -= 1.f;

    if (sf::Keyboard::isKeyPressed(Tecla::S) ||
        sf::Keyboard::isKeyPressed(Tecla::Down))
        direccion.y += 1.f;

    personaje_.actualizar(dt, direccion, escena_->obstaculos());

    personaje_.limitarA(escena_->limites());   // <-- línea nueva

    // Actualiza si se muestra el aviso "[E]".
    if (Puerta* puerta = escena_->puerta())
        puerta->puedeInteractuar(personaje_.hitbox());
}

void EstadoExploracion::draw(sf::RenderWindow& window)
{
    if (!escena_)
        return;

    escena_->dibujar(window, fuente());
    personaje_.dibujar(window);
}