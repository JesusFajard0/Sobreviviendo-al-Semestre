#include "world/EstadoExploracion.hpp"

#include <algorithm>
#include <cmath>
#include <functional>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include "core/CompatSFML.hpp"
#include "core/StateStack.hpp"
#include "world/LectorObjetos.hpp"
#include "world/ClaseState.hpp"
#include "world/MenuDestinos.hpp"

namespace
{
    // Tamaño de la vista lógica (el mismo que usa Juego).
    constexpr float ANCHO_VISTA = 640.f;
    constexpr float ALTO_VISTA  = 360.f;

    // Caja de texto modal.
    constexpr unsigned TAM_TEXTO_MENSAJE = 14;
    constexpr float    RELLENO_CAJA      = 20.f;   // espacio entre texto y borde

    // Número redondeado con signo: "+15", "-5", "0".
    std::string conSigno(float valor)
    {
        const long entero = std::lround(valor);
        return std::string(entero > 0 ? "+" : "") + std::to_string(entero);
    }

    // Nombres para mostrar (con tilde).
    const char* nombreVisible(Stat s)
    {
        switch (s)
        {
        case Stat::SaludMental: return "Salud mental";
        case Stat::Energia:     return "Energía";
        case Stat::Estres:      return "Estrés";
        case Stat::Dopamina:    return "Dopamina";
        case Stat::Dinero:      return "Dinero";
        default:                return "?";
        }
    }
}

EstadoExploracion::EstadoExploracion(StateStack* pila,
                                     GestorRecursos& recursos,
                                     Partida& partida)
    : State(pila),
      recursos_(recursos),
      partida_(partida),
      estadisticas_(partida.estadisticas),
      tiempo_(partida.tiempo),
      personaje_(),
      textoMensaje_(fuente(), "", TAM_TEXTO_MENSAJE)   // en SFML 3 el texto necesita fuente al crearse
{
    hud_.iniciar(recursos_);

    // Configuración visual de la caja de texto (opaca para que se lea bien).
    fondoMensaje_.setFillColor(sf::Color(20, 20, 30, 255));
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

    // Avisa si algún objeto apunta a una escena que no existe.
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

    crearObjetos(id);
}
void EstadoExploracion::crearObjetos(const std::string& idEscena)
{
    objetos_.clear();

    std::optional<Materia> examenHoy = tiempo_.examenHoy();
    bool esMañanaDeExamen = examenHoy.has_value() && (tiempo_.bloque() == Bloque::Manana);

    for (const DatosObjeto& d : datosObjetos_)
    {
        if (d.escena != idEscena)
            continue;

        // Si es la mañana del examen, ocultamos el botón de "asistir_clase" del salón correspondiente
        if (esMañanaDeExamen && d.actividad == "asistir_clase")
        {
            bool esSalonCorrespondiente = 
                (idEscena == "salon_calculo" && examenHoy.value() == Materia::Calculo) ||
                (idEscena == "salon_fisica" && examenHoy.value() == Materia::Fisica) ||
                (idEscena == "laboratorio" && examenHoy.value() == Materia::Programacion);

            if (esSalonCorrespondiente)
            {
                continue; // Salta esta iteración para no dibujar el botón de clase
            }
        }

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
    } // <--- El bucle for termina aquí afuera

    // --- INSTANCIACIÓN DINÁMICA DE EXÁMENES (Solo en la mañana del examen) ---
    if (esMañanaDeExamen)
    {
        bool mostrarExamen = false;
        if (idEscena == "salon_calculo" && examenHoy.value() == Materia::Calculo)
            mostrarExamen = true;
        else if (idEscena == "salon_fisica" && examenHoy.value() == Materia::Fisica)
            mostrarExamen = true;
        else if (idEscena == "laboratorio" && examenHoy.value() == Materia::Programacion)
            mostrarExamen = true;

        if (mostrarExamen)
        {
            const DatosActividad* actExamen = partida_.actividades.buscar("presentar_examen");
            if (actExamen != nullptr)
            {
                objetos_.push_back(std::make_unique<ObjetoActividad>(
                    crearRect(250.f, 90.f, 140.f, 50.f),
                    "presentar_examen",
                    actExamen->nombre,
                    [this](const std::string& id) { usarActividad(id); }
                ));
            }
        }
    }
}
// ---------------------------------------------------------------
// Actividades y mensajes
// ---------------------------------------------------------------
void EstadoExploracion::usarActividad(const std::string& idActividad)
{
    // --- LECTURA DEL MANUAL DEL ESTUDIANTE (Inicia la Pág. 1, continúa en handleEvent) ---
    if (idActividad == "leer_manual")
    {
        paginaManual_ = 1;
        mostrarMensaje(
            "== MANUAL DEL ESTUDIANTE (1/4) ==\n\n"
            "• Asiste siempre a clases en la mañana (evita faltar).\n"
            "• Los viernes de las semanas 3, 5 y 8 son los exámenes parciales.\n"
            "• Biblioteca y Cafetería: Dan mejores beneficios de conocimiento y\n"
            "  bienestar, aunque sacrificas un poco más de energía o dinero.\n\n"
            "(Presiona ENTER para ver la página 2)"
        );
        return;
    }
    // -----------------------------------------------------------------

    // --- VALIDACIÓN DE CURSO TERMINADO ---
    if (idActividad == "asistir_clase" && escena_)
    {
        std::string idEscena = escena_->id();
        int semanaActual = partida_.tiempo.semana();
        bool cursoTerminado = false;
        std::string nombreMateria = "";

        // Cálculo termina después de la semana 3
        if (idEscena == "salon_calculo" && semanaActual > 3)
        {
            cursoTerminado = true;
            nombreMateria = "Cálculo";
        }
        // Física termina después de la semana 5
        else if (idEscena == "salon_fisica" && semanaActual > 5)
        {
            cursoTerminado = true;
            nombreMateria = "Física";
        }
        // Programación termina después de la semana 8
        else if (idEscena == "laboratorio" && semanaActual > 8)
        {
            cursoTerminado = true;
            nombreMateria = "Programación";
        }

        if (cursoTerminado)
        {
            mostrarMensaje(
                "== CURSO TERMINADO ==\n\n"
                "El periodo de clases de " + nombreMateria + " ya concluyó.\n"
                "Este curso ha finalizado.\n\n"
                "(Presiona ENTER)"
            );
            return;
        }
    }
    // -------------------------------------

    const ResultadoActividad r = partida_.actividades.ejecutar(
        idActividad,
        partida_.estadisticas,
        partida_.tiempo,
        partida_.materiaActiva()
    );

    if (!r.hecha())
    {
        std::string mensajeError = GestorActividades::mensaje(r.estado);

        // Mensaje personalizado si intenta asistir a clase fuera de la mañana
        if (idActividad == "asistir_clase" && partida_.tiempo.bloque() != Bloque::Manana)
        {
            mensajeError = "Solo puedes asistir a clases en la mañana.";
        }

        std::cout << "[Actividad] " << mensajeError << '\n';

        mostrarMensaje(
            std::string("[ Aviso ]\n\n") +
            mensajeError +
            "\n\n(Presiona ENTER)"
        );
        return;
    }

    const DatosActividad* actividad = partida_.actividades.buscar(idActividad);
    const std::string nombre = (actividad != nullptr) ? actividad->nombre : idActividad;

    std::string contenido = "== " + nombre + " ==\n\n";

    for (int i = 0; i < static_cast<int>(Stat::Cantidad); ++i)
    {
        if (r.cambios[i] != 0.f)
        {
            contenido += std::string(nombreVisible(static_cast<Stat>(i))) + ": " +
                         conSigno(r.cambios[i]) + "\n";
        }
    }

    if (r.cambioConocimiento != 0.f)
        contenido += "Conocimiento: " + conSigno(r.cambioConocimiento) + "\n";

    // --- S2-3: contador de asistencias ---
    const bool fueAClase = (idActividad == "asistir_clase");
    if (fueAClase)
    {
        const Materia materia = partida_.materiaActiva();
        asistencia_.registrarAsistencia(materia);
        contenido += "Asistencias en la materia: " +
                     std::to_string(asistencia_.asistencias(materia)) + "\n";
    }

    // --- S2-4: noches sin dormir ---
    if (idActividad == "dormir")   // ADAPTAR: id real de la actividad de dormir
        colapso_.marcarSueno();

    if (r.evento == EventoTiempo::NuevoDia || r.evento == EventoTiempo::FinDeSemana)
    {
        const ResultadoAmanecer noche = colapso_.alAmanecer(estadisticas_);
        if (!noche.durmio)
            contenido += std::string("\n") + noche.mensaje + "\n";
    }

    // La discoteca todavía no tiene escena: por ahora es solo este cuadro.
    if (idActividad == "discoteca")
        contenido += "\n[ Aquí irá la animación de la discoteca ]\n";

    contenido += "\n(Presiona ENTER)";

    // Registro en consola (útil para depurar).
    std::cout << "[Actividad] " << idActividad << ':' << std::showpos;
    for (int i = 0; i < static_cast<int>(Stat::Cantidad); ++i)
    {
        if (r.cambios[i] != 0.f)
            std::cout << ' ' << Estadisticas::nombre(static_cast<Stat>(i))
                      << ' ' << r.cambios[i];
    }
    if (r.cambioConocimiento != 0.f)
        std::cout << " Conocimiento " << r.cambioConocimiento;
    std::cout << std::noshowpos << " | "
              << Tiempo::nombre(partida_.tiempo.dia()) << ' '
              << Tiempo::nombre(partida_.tiempo.bloque())
              << " (semana " << partida_.tiempo.semana() << ")\n";

    if (r.evento == EventoTiempo::NuevoDia)
        std::cout << "[Tiempo] Nuevo dia.\n";

    mostrarMensaje(contenido);

    // S2-3: animacion provisional de clase. Se apila encima y, al terminar,
    // se ve la caja con los resultados.
    if (fueAClase)
        m_stack->pushState(std::make_unique<ClaseState>(
            m_stack, fuente(), Estadisticas::nombre(partida_.materiaActiva())));
}
void EstadoExploracion::mostrarMensaje(const std::string& texto)
{
    textoMensaje_.setString(textoUtf8(texto));

    const sf::FloatRect limites = textoMensaje_.getLocalBounds();
    const sf::Vector2f posTexto = posicionRect(limites);
    const sf::Vector2f tamTexto = tamanoRect(limites);

    const sf::Vector2f tamCaja(
        tamTexto.x + 2.f * RELLENO_CAJA,
        tamTexto.y + 2.f * RELLENO_CAJA
    );

    // Siempre en el centro de la pantalla.
    const sf::Vector2f centro(ANCHO_VISTA / 2.f, ALTO_VISTA / 2.f);

    fondoMensaje_.setSize(tamCaja);
    fondoMensaje_.setOrigin(sf::Vector2f(tamCaja.x / 2.f, tamCaja.y / 2.f));
    fondoMensaje_.setPosition(centro);

    textoMensaje_.setOrigin(sf::Vector2f(
        posTexto.x + tamTexto.x / 2.f,
        posTexto.y + tamTexto.y / 2.f
    ));
    textoMensaje_.setPosition(centro);

    mostrandoMensaje_ = true;
}

void EstadoExploracion::revisarEventosTiempo()
{
    // Si hay una caja abierta se espera a que el jugador la cierre.
    if (mostrandoMensaje_)
        return;

    // --- GUÍA DEL ESTUDIANTE (Al iniciar la partida) ---
    if (!guiaInicialMostrada_ && tiempo_.semana() == 1 && tiempo_.dia() == Dia::Lunes && tiempo_.bloque() == Bloque::Manana)
    {
        guiaInicialMostrada_ = true;
        mostrarMensaje(
            "== GUÍA DEL ESTUDIANTE ==\n\n"
            "• Ve a clases siempre en la mañana (evita faltar).\n"
            "• Los viernes de las semanas 3, 5 y 8 son los exámenes parciales.\n"
            "• Revisa el Manual en el apartamento para ver los límites de cada actividad.\n"
            "• Cuida tu energía y tu estrés durmiendo o descansando.\n\n"
            "(Presiona ENTER para comenzar)"
        );
        return;
    }
    // ----------------------------------------------------

    // --- 0. FIN DE SEMANA (Debe ir PRIMERO para que no salten avisos de clases/exámenes en el descanso) ---
    if (tiempo_.enFinDeSemana())
    {
        const int semanaQueTermina = tiempo_.semana();

        tiempo_.terminarFinDeSemana();
        partida_.actividades.reiniciarContadores(); 
        const float mesada = GestorActividades::aplicarMesada(estadisticas_);

        std::cout << "[Tiempo] Fin de semana: empieza la semana "
                  << tiempo_.semana() << ".\n";

        mostrarMensaje(
            "== Fin de semana ==\n\n"
            "Terminó la semana " + std::to_string(semanaQueTermina) + ".\n"
            "Tus padres te envían la mesada: " + conSigno(mesada) + "\n\n"
            "Empieza la semana " + std::to_string(tiempo_.semana()) + ".\n\n"
            "(Presiona ENTER)"
        );
        return;
    }

    if (tiempo_.semestreTerminado() && !finSemestreMostrado_)
    {
        finSemestreMostrado_ = true;

        std::cout << "[Tiempo] Fin del semestre.\n";

        mostrarMensaje(
            "== Fin del semestre ==\n\n"
            "Terminaste las " + std::to_string(Tiempo::SEMANAS) + " semanas.\n"
            "(Aquí irán la animación final\n"
            "y el resumen de finales.)\n\n"
            "(Presiona ENTER)"
        );
        return;
    }

    // Detector de cambio de bloque: resetea las banderas solo cuando entramos a un nuevo bloque/mañana
    static Bloque bloqueAnterior = static_cast<Bloque>(-1);
    Bloque bloqueActual = tiempo_.bloque();

    if (bloqueActual != bloqueAnterior)
    {
        if (bloqueActual == Bloque::Manana)
        {
            avisoExamenMostradoHoy = false;
            avisoOlvidoExamenMostradoHoy = false;
            faltaClaseVerificadaHoy = false;
        }
        bloqueAnterior = bloqueActual;
    }

    // --- 1. AVISO DE EXAMEN POR LA MAÑANA ---
    std::optional<Materia> examenMat = tiempo_.examenHoy();
    if (examenMat.has_value() && tiempo_.bloque() == Bloque::Manana && !avisoExamenMostradoHoy)
    {
        avisoExamenMostradoHoy = true;
        std::string nombreMat = "";
        if (examenMat.value() == Materia::Calculo) nombreMat = "Cálculo";
        else if (examenMat.value() == Materia::Fisica) nombreMat = "Física";
        else if (examenMat.value() == Materia::Programacion) nombreMat = "Programación";

        mostrarMensaje(
            "== ¡DÍA DE EXAMEN! ==\n\n"
            "Hoy tienes el parcial de " + nombreMat + ".\n"
            "Dirígete al salón correspondiente.\n\n"
            "(Presiona ENTER)"
        );
        return;
    }

    // --- 2. AVISO DE EXAMEN OLVIDADO (Si pasa a la tarde/noche del viernes sin hacerlo) ---
    if (examenMat.has_value() && tiempo_.bloque() != Bloque::Manana && !avisoOlvidoExamenMostradoHoy)
    {
        if (partida_.actividades.usosHoy("presentar_examen") == 0)
        {
            avisoOlvidoExamenMostradoHoy = true;
            mostrarMensaje(
                "== EXAMEN NO PRESENTADO ==\n\n"
                "Se pasó el horario del parcial.\n"
                "¡Se te olvidó el examen o no fuiste a tiempo!\n\n"
                "(Presiona ENTER)"
            );
            return;
        }
    }

    // --- 3. AVISO DE "FALTASTE A CLASE" AL PASAR A LA TARDE ---
    if (tiempo_.bloque() == Bloque::Tarde && !faltaClaseVerificadaHoy)
    {
        faltaClaseVerificadaHoy = true; 

        if (partida_.actividades.usosHoy("asistir_clase") == 0 && !examenMat.has_value())
        {
            const Materia materiaFalta = partida_.materiaActiva();
            asistencia_.registrarFalta(materiaFalta);   // S2-3

            mostrarMensaje(
                std::string("== AVISO ==\n\n"
                "Pasó la mañana y no fuiste a la universidad.\n"
                "¡Faltaste a clase!\n") +
                "Faltas en la materia: " +
                std::to_string(asistencia_.faltas(materiaFalta)) + "\n\n"
                "(Presiona ENTER)"
            );
            return;
        }
    }

    // Si la escena actual cerró por la hora
    if (escena_ && escena_->id() != mapa_.raiz())
    {
        const std::string idActual = escena_->id();

        if (!partida_.fases.permitidaEn(idActual, tiempo_.bloque(), tiempo_.dia()))
        {
            const DatosEscena* datos = mapa_.datos(idActual);
            const std::string nombre = (datos != nullptr) ? datos->nombre : idActual;

            cambiarEscena(mapa_.raiz());

            std::cout << "[Tiempo] " << nombre << " cerro: vuelves al apartamento.\n";

            mostrarMensaje(
                "== Cerrado ==\n\n" +
                nombre + " cerró por hoy.\n"
                "Vuelves al apartamento.\n\n"
                "(Presiona ENTER)"
            );
        }
    }
}

// ---------------------------------------------------------------
// Menú de destinos
// ---------------------------------------------------------------

void EstadoExploracion::abrirMenuDestinos()
{
    if (!escena_)
        return;

    const std::string idActual = escena_->id();

    // Horarios: lo que está cerrado ahora aparece bloqueado, y la discoteca
    // solo aparece cuando abre (viernes de noche o de madrugada).
    std::vector<Destino> visibles;

    for (Destino d : mapa_.destinos(idActual, partida_.faseSemestre()))
    {
        const bool abierto = partida_.fases.escenaPermitida(d.id);

        if (d.esAnimacion)
        {
            if (!abierto)
                continue;
        }
        else if (!d.esPadre && !abierto)
        {
            d.bloqueado = true;
        }

        visibles.push_back(d);
    }

    auto alElegir = [this, idActual](const std::string& idDestino)
    {
        switch (mapa_.puedeIr(idActual, idDestino, partida_.faseSemestre()))
        {
        case ResultadoCambio::Ok:
            cambiarEscena(idDestino);
            break;

        case ResultadoCambio::Animacion:
            // El id de la escena de animación coincide con el de su actividad
            // (p. ej. "discoteca"): se hace como cualquier otra actividad.
            usarActividad(idDestino);
            break;

        case ResultadoCambio::Bloqueada:
        case ResultadoCambio::NoExiste:
            break;
        }
    };

    m_stack->pushState(std::make_unique<MenuDestinos>(
        m_stack,
        fuente(),
        std::move(visibles),
        alElegir
    ));
}

// ---------------------------------------------------------------
// Eventos, actualización y dibujo
// ---------------------------------------------------------------

void EstadoExploracion::handleEvent(const sf::Event& event)
{
    // Con una caja abierta solo se escucha ENTER o E para cerrarla o avanzar página.
    if (mostrandoMensaje_)
    {
        const auto* tecla = event.getIf<sf::Event::KeyPressed>();

        if (tecla != nullptr &&
            (tecla->code == sf::Keyboard::Key::Enter ||
             tecla->code == sf::Keyboard::Key::E))
        {
            // --- NAVEGACIÓN CONTINUA DEL MANUAL ---
            if (paginaManual_ > 0)
            {
                paginaManual_++;
                if (paginaManual_ == 2)
                {
                    mostrarMensaje(
                        "== MANUAL DEL ESTUDIANTE (2/4) ==\n\n"
                        "• Ocio y Fiestas:\n"
                        "  Los viernes por la noche y madrugada hay discoteca.\n"
                        "  Puedes ir para mejorar tu dopamina y bajar el estrés,\n"
                        "  pero ten en cuenta que gastas bastante dinero.\n\n"
                        "(Presiona ENTER para ver la página 3)"
                    );
                }
                else if (paginaManual_ == 3)
                {
                    mostrarMensaje(
                        "== MANUAL DEL ESTUDIANTE (3/4) ==\n\n"
                        "• Actividades con Límite Diario (Acciones rápidas - No gastan bloque):\n"
                        "  - Comer en casa (máx. 3/día) | Ducharse (máx. 2/día)\n"
                        "  - Lavar platos (máx. 1/día)   | Hacer ejercicio (máx. 1/día)\n"
                        "  - Siesta (máx. 1/día)        | Comer en cafetería (máx. 2/día)\n\n"
                        "(Presiona ENTER para ver la página 4)"
                    );
                }
                else if (paginaManual_ == 4)
                {
                    mostrarMensaje(
                        "== MANUAL DEL ESTUDIANTE (4/4) ==\n\n"
                        "• Actividades que Avanzan el Bloque de Tiempo:\n"
                        "  - Estudiar en casa (máx. 3/día)\n"
                        "  - Estudiar en biblioteca (máx. 2/día)\n"
                        "  - Usar la computadora (máx. 2/día)\n"
                        "  - Dormir (noche/madrugada) y Asistir a clases (mañana)\n\n"
                        "(Presiona ENTER para cerrar el manual)"
                    );
                }
                else
                {
                    paginaManual_ = 0;
                    mostrandoMensaje_ = false;
                }
            }
            else
            {
                mostrandoMensaje_ = false;
            }
        }
        return;
    }

    // Clic izquierdo: puede ser sobre el botón del HUD.
    if (const auto* raton = event.getIf<sf::Event::MouseButtonPressed>())
    {
        if (raton->button == sf::Mouse::Button::Left && ventana_ != nullptr)
        {
                    sf::View vistaHud = hud_.vista();
            vistaHud.setViewport(ventana_->getView().getViewport());
            hud_.ratonPresionado(ventana_->mapPixelToCoords(raton->position, vistaHud));
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

    // Fin de semana, fin de semestre y escenas que cierran (los provoque una
    // actividad o una tecla de depuración).
    revisarEventosTiempo();

    // S2-4: colapso por estadisticas extremas (espera a que se cierre la caja).
    revisarColapso();

    // Con la caja abierta el personaje se congela.
    if (mostrandoMensaje_ || !escena_)
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
    personaje_.limitarA(escena_->limites());

    // Actualiza si se muestra el aviso "[E]".
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

    hud_.dibujar(window);

    // La caja de texto va por encima de todo (también del HUD), con un velo
    // oscuro detrás para que se lea bien.
    if (mostrandoMensaje_)
    {
        sf::RectangleShape velo(sf::Vector2f(ANCHO_VISTA, ALTO_VISTA));
        velo.setFillColor(sf::Color(0, 0, 0, 150));
        window.draw(velo);

        window.draw(fondoMensaje_);
        window.draw(textoMensaje_);
    }
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

    case Tecla::T: 
    {
        EventoTiempo ev = tiempo_.avanzarBloque();
        if (ev == EventoTiempo::NuevoDia || ev == EventoTiempo::NuevaSemana)
        {
            partida_.actividades.reiniciarContadores();
        }
        break;
    }
    case Tecla::Y: tiempo_.terminarFinDeSemana(); break;

    default: break;
    }
}

// ---------------------------------------------------------------
// S2-4: colapso
// ---------------------------------------------------------------
void EstadoExploracion::revisarColapso()
{
    // No se pisa una caja que el jugador todavia no cerro.
    if (mostrandoMensaje_)
        return;

    const ResultadoColapso r = colapso_.evaluar(estadisticas_);
    if (!r.huboColapso)
        return;

    std::string texto = std::string("== COLAPSO ==\n\n") + r.mensaje + ".\n";

    // Pierde tiempo igual que la tecla T: al cambiar de dia se reinician los contadores.
    for (int i = 0; i < r.bloquesPerdidos; ++i)
    {
        const EventoTiempo ev = tiempo_.avanzarBloque();

        if (ev == EventoTiempo::NuevoDia || ev == EventoTiempo::NuevaSemana)
            partida_.actividades.reiniciarContadores();

        if (ev == EventoTiempo::NuevoDia || ev == EventoTiempo::FinDeSemana)
        {
            const ResultadoAmanecer noche = colapso_.alAmanecer(estadisticas_);
            if (!noche.durmio)
                texto += std::string("\n") + noche.mensaje + "\n";
        }
    }

    texto += "\nPierdes un bloque de tiempo.\n\n(Presiona ENTER)";

    std::cout << "[Colapso] " << r.mensaje << '\n';
    mostrarMensaje(texto);
}
