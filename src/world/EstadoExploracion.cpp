#include "world/EstadoExploracion.hpp"

#include <functional>
#include <iostream>
#include <utility>

#include "core/CompatSFML.hpp"
#include "core/StateStack.hpp"
#include "world/LectorObjetos.hpp"
#include "world/MenuDestinos.hpp"

EstadoExploracion::EstadoExploracion(StateStack* pila,
                                     GestorRecursos& recursos,
                                     Partida& partida)
    : State(pila),
      recursos_(recursos),
      partida_(partida),
      estadisticas_(partida.estadisticas),
      tiempo_(partida.tiempo),
      personaje_(),
      textoMensaje_(fuente(), "", 16) // Inicializar texto para SFML 3
{
    hud_.iniciar(recursos_);

    // Configuración visual de la caja de texto
    fondoMensaje_.setFillColor(sf::Color(20, 20, 30, 230)); // Azul muy oscuro, semi-transparente
    fondoMensaje_.setOutlineColor(sf::Color::White);
    fondoMensaje_.setOutlineThickness(2.f);

    if (!mapa_.cargar("data/escenas.txt"))
    {
        std::cerr << "[EstadoExploracion] No se pudo cargar el mapa.\n";
        return;
    }

    if (!LectorObjetos::cargarArchivo("data/objetos.txt", datosObjetos_))
    {
        std::cerr << "[EstadoExploracion] Hubo problemas al cargar data/objetos.txt.\n";
    }

    for (const DatosObjeto& d : datosObjetos_)
    {
        if (mapa_.datos(d.escena) == nullptr)
        {
            std::cerr << "[EstadoExploracion] Objeto en escena inexistente: "
                      << d.escena << '\n';
        }
    }

    cambiarEscena(mapa_.raiz());
}

const sf::Font& EstadoExploracion::fuente() const
{
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

    crearObjetos(id);
}

void EstadoExploracion::crearObjetos(const std::string& idEscena)
{
    objetos_.clear();

    for (const DatosObjeto& d : datosObjetos_)
    {
        if (d.escena != idEscena)
            continue;

        const DatosActividad* actividad = partida_.actividades.buscar(d.actividad);

        if (actividad == nullptr)
        {
            std::cerr << "[EstadoExploracion] Objeto con actividad desconocida: "
                      << d.actividad << '\n';
            continue;
        }

        objetos_.push_back(std::make_unique<ObjetoActividad>(
            crearRect(d.x, d.y, d.ancho, d.alto),
            d.actividad,
            actividad->nombre,
            [this](const std::string& id) { usarActividad(id); }
        ));
    }
}

void EstadoExploracion::usarActividad(const std::string& idActividad)
{
    const ResultadoActividad r = partida_.actividades.ejecutar(
        idActividad,
        partida_.estadisticas,
        partida_.tiempo,
        partida_.materiaActiva()
    );

    std::string contenidoPantalla;

    if (!r.hecha())
    {
        contenidoPantalla = std::string("[ Aviso ]\n\n") + std::string(GestorActividades::mensaje(r.estado)) + "\n\n(Presiona ENTER)";
        std::cout << "[Actividad] " << GestorActividades::mensaje(r.estado) << '\n';
    }
    else
    {
        const DatosActividad* act = partida_.actividades.buscar(idActividad);
        std::string nombreActividad = act ? act->nombre : idActividad;

        contenidoPantalla = std::string("== ") + nombreActividad + " ==\n\n";

        // Resultados
        for (int i = 0; i < static_cast<int>(Stat::Cantidad); ++i)
        {
            float delta = r.cambios[i];
            if (delta != 0.f)
            {
                std::string signo = (delta > 0) ? "+" : "";
                contenidoPantalla += std::string(Estadisticas::nombre(static_cast<Stat>(i))) + ": " + signo + std::to_string(static_cast<int>(delta)) + "\n";
            }
        }

        if (r.cambioConocimiento != 0.f)
        {
            std::string signo = (r.cambioConocimiento > 0) ? "+" : "";
            contenidoPantalla += std::string("Conocimiento: ") + signo + std::to_string(static_cast<int>(r.cambioConocimiento)) + "\n";
        }

        contenidoPantalla += "\n(Presiona ENTER)";

        // Log Consola
        std::cout << "[Actividad] " << idActividad << ':' << std::showpos;
        for (int i = 0; i < static_cast<int>(Stat::Cantidad); ++i)
        {
            if (r.cambios[i] != 0.f)
                std::cout << ' ' << Estadisticas::nombre(static_cast<Stat>(i)) << ' ' << r.cambios[i];
        }
        if (r.cambioConocimiento != 0.f) std::cout << " Conocimiento " << r.cambioConocimiento;
        std::cout << std::noshowpos << " | "
                  << Tiempo::nombre(partida_.tiempo.dia()) << ' '
                  << Tiempo::nombre(partida_.tiempo.bloque())
                  << " (semana " << partida_.tiempo.semana() << ")\n";

        switch (r.evento)
        {
        case EventoTiempo::NuevoDia:
            std::cout << "[Tiempo] Nuevo dia.\n";
            break;
        case EventoTiempo::FinDeSemana:
            std::cout << "[Tiempo] Fin de semana (animacion pendiente).\n";
            partida_.tiempo.terminarFinDeSemana();
            GestorActividades::aplicarMesada(partida_.estadisticas);
            std::cout << "[Tiempo] Llega la mesada. Empieza la semana " << partida_.tiempo.semana() << ".\n";
            break;
        case EventoTiempo::FinSemestre:
            std::cout << "[Tiempo] Fin del semestre (animacion final pendiente).\n";
            break;
        default:
            break;
        }
    }

    // Configurar y centrar el cuadro de texto
    textoMensaje_.setString(contenidoPantalla);
    sf::FloatRect bounds = textoMensaje_.getLocalBounds();
    
    // Darle margen al fondo
    fondoMensaje_.setSize(sf::Vector2f(bounds.size.x + 40.f, bounds.size.y + 40.f));    
    // Posicionarlo justo arriba del personaje
    sf::Vector2f centro = personaje_.posicion();
    
    // Corrección para SFML 3: Envolver en sf::Vector2f()
    fondoMensaje_.setOrigin(sf::Vector2f(fondoMensaje_.getSize().x / 2.f, fondoMensaje_.getSize().y / 2.f));
    fondoMensaje_.setPosition(sf::Vector2f(centro.x, centro.y - 80.f)); 
    
    // Corrección para SFML 3: Envolver en sf::Vector2f()
    textoMensaje_.setOrigin(sf::Vector2f(bounds.position.x + bounds.size.x / 2.f, bounds.position.y + bounds.size.y / 2.f));    textoMensaje_.setPosition(fondoMensaje_.getPosition());

    // Activar la pausa y mostrar el mensaje
    mostrandoMensaje_ = true;
}

void EstadoExploracion::abrirMenuDestinos()
{
    if (!escena_)
        return;

    const std::string idActual = escena_->id();

    auto alElegir = [this, idActual](const std::string& idDestino)
    {
        switch (mapa_.puedeIr(idActual, idDestino, partida_.faseSemestre()))
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
        mapa_.destinos(idActual, partida_.faseSemestre()),
        alElegir
    ));
}

void EstadoExploracion::handleEvent(const sf::Event& event)
{
    // Si hay un mensaje bloqueando la pantalla, solo escuchamos la tecla ENTER o E para cerrarlo
    if (mostrandoMensaje_)
    {
        const auto* tecla = event.getIf<sf::Event::KeyPressed>();
        if (tecla != nullptr && (tecla->code == sf::Keyboard::Key::Enter || tecla->code == sf::Keyboard::Key::E))
        {
            mostrandoMensaje_ = false; // Cerramos el mensaje
        }
        return; // No procesamos ni clics ni otros movimientos
    }

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

    // La puerta tiene prioridad sobre los objetos.
    Puerta* puerta = escena_->puerta();

    if (puerta != nullptr &&
        puerta->puedeInteractuar(personaje_.hitbox()))
    {
        puerta->interactuar();
        return;
    }

    for (const std::unique_ptr<ObjetoActividad>& objeto : objetos_)
    {
        if (objeto->puedeInteractuar(personaje_.hitbox()))
        {
            objeto->interactuar();
            return;
        }
    }
}

void EstadoExploracion::update(float dt)
{
    hud_.actualizar(dt, estadisticas_, tiempo_);

    // Si el cuadro de texto está abierto, el personaje se congela
    if (mostrandoMensaje_ || !escena_)
        return;

    using Tecla = sf::Keyboard::Key;
    sf::Vector2f direccion(0.f, 0.f);

    if (sf::Keyboard::isKeyPressed(Tecla::A) || sf::Keyboard::isKeyPressed(Tecla::Left))
        direccion.x -= 1.f;
    if (sf::Keyboard::isKeyPressed(Tecla::D) || sf::Keyboard::isKeyPressed(Tecla::Right))
        direccion.x += 1.f;
    if (sf::Keyboard::isKeyPressed(Tecla::W) || sf::Keyboard::isKeyPressed(Tecla::Up))
        direccion.y -= 1.f;
    if (sf::Keyboard::isKeyPressed(Tecla::S) || sf::Keyboard::isKeyPressed(Tecla::Down))
        direccion.y += 1.f;

    personaje_.actualizar(dt, direccion, escena_->obstaculos());
    personaje_.limitarA(escena_->limites());

    if (Puerta* puerta = escena_->puerta())
        puerta->puedeInteractuar(personaje_.hitbox());

    for (const std::unique_ptr<ObjetoActividad>& objeto : objetos_)
        objeto->puedeInteractuar(personaje_.hitbox());
}

void EstadoExploracion::draw(sf::RenderWindow& window)
{
    ventana_ = &window;

    if (!escena_)
        return;

    escena_->dibujar(window, fuente());

    for (const std::unique_ptr<ObjetoActividad>& objeto : objetos_)
        objeto->dibujar(window, fuente());

    personaje_.dibujar(window);

    // Dibujamos la caja de texto por encima de la escena si está activa
    if (mostrandoMensaje_)
    {
        window.draw(fondoMensaje_);
        window.draw(textoMensaje_);
    }

    hud_.dibujar(window);   // HUD siempre de último
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