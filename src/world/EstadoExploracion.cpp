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
    hud_.iniciar(recursos_);

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
    // Clic izquierdo: puede ser sobre el botón del HUD.
    if (const auto* raton = event.getIf<sf::Event::MouseButtonPressed>())
    {
        if (raton->button == sf::Mouse::Button::Left && ventana_ != nullptr)
        {
            hud_.ratonPresionado(
                ventana_->mapPixelToCoords(raton->position, hud_.vista()));
        }
        return;
    }

    const auto* tecla = event.getIf<sf::Event::KeyPressed>();

    if (tecla == nullptr)
        return;

    hud_.teclaPresionada(tecla->code);   // Tab abre/cierra el panel
    
    teclaDepuracion(tecla->code, tecla->shift);   // TEMPORAL (S1-7)

    if (tecla->code != sf::Keyboard::Key::E || !escena_)
        return;

    Puerta* puerta = escena_->puerta();

    if (puerta != nullptr &&
        puerta->puedeInteractuar(personaje_.hitbox()))
    {
        puerta->interactuar();
    }
}

void EstadoExploracion::update(float dt)
{
    hud_.actualizar(dt, estadisticas_, tiempo_);

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
    ventana_ = &window;

    if (!escena_)
        return;

    escena_->dibujar(window, fuente());
    personaje_.dibujar(window);

    hud_.dibujar(window);   // encima de todo, con su vista fija
}

// TEMPORAL (S1-7): teclas de depuración. Borrar antes del Pull Request.
void EstadoExploracion::teclaDepuracion(sf::Keyboard::Key tecla, bool fino)
{
    using Tecla = sf::Keyboard::Key;
    const float paso = fino ? 1.f : 10.f;

    switch (tecla)
    {
    case Tecla::Z: estadisticas_.modificar(Stat::Energia,    -paso); break;
    case Tecla::X: estadisticas_.modificar(Stat::Energia,     paso); break;
    case Tecla::C: estadisticas_.modificar(Stat::Estres,     -paso); break;
    case Tecla::V: estadisticas_.modificar(Stat::Estres,      paso); break;
    case Tecla::B: estadisticas_.modificar(Stat::SaludMental, -paso); break;
    case Tecla::N: estadisticas_.modificar(Stat::SaludMental,  paso); break;
    case Tecla::F: estadisticas_.modificar(Stat::Dopamina,   -paso); break;
    case Tecla::G: estadisticas_.modificar(Stat::Dopamina,    paso); break;
    case Tecla::H: estadisticas_.modificar(Stat::Dinero,     -paso); break;
    case Tecla::J: estadisticas_.modificar(Stat::Dinero,      paso); break;

    case Tecla::K:   // conocimiento de las tres materias
        for (int i = 0; i < static_cast<int>(Materia::Cantidad); ++i)
            estadisticas_.modificarConocimiento(static_cast<Materia>(i), paso);
        break;

    case Tecla::T: tiempo_.avanzarBloque();       break;
    case Tecla::Y: tiempo_.terminarFinDeSemana(); break;

    default: break;
    }
}
