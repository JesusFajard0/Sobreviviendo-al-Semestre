#include "ui/Hud.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <iterator>

#include "core/CompatSFML.hpp"

namespace
{
    // --- Vista fija (la misma lógica que usa Juego) ---
    constexpr float ANCHO_VISTA = 640.f;
    constexpr float ALTO_VISTA  = 360.f;

    // --- Diseño provisional ---
    constexpr float    MARGEN          = 8.f;
    constexpr unsigned TAM_TEXTO       = 10;
    constexpr float    ALTO_LINEA      = 14.f;

    constexpr float BOTON_X            = 8.f;
    constexpr float BOTON_Y            = 8.f;
    constexpr float BOTON_ANCHO        = 108.f;
    constexpr float BOTON_ALTO         = 22.f;
    constexpr float BOTON_DESFASE_TEXTO = 4.f;
    constexpr float SEPARACION_PANEL   = 4.f;

    constexpr float RADIO_PUNTO        = 4.f;
    constexpr float VELOCIDAD_PARPADEO = 5.f;    // radianes por segundo
    constexpr float DOS_PI             = 6.2831853f;
    constexpr float ALPHA_MINIMO       = 90.f;   // el punto oscila entre esto y 255

    constexpr long long DINERO_MAXIMO_MOSTRADO = 9999999;

    const sf::Color COLOR_FONDO_BOTON(15, 15, 22, 215);
    const sf::Color COLOR_BORDE_BOTON(90, 90, 110);
    const sf::Color COLOR_TEXTO(235, 235, 240);
    const sf::Color COLOR_ALERTA(225, 60, 60);

    // --- Textos con tildes (los nombre() de Tiempo no las traen) ---
    const char* const DIAS[]    = {"Lunes", "Martes", "Miércoles",
                                   "Jueves", "Viernes"};
    const char* const BLOQUES[] = {"Mañana", "Tarde", "Noche", "Madrugada"};

    static_assert(std::size(DIAS) == static_cast<std::size_t>(Dia::Cantidad),
                  "DIAS debe tener un texto por cada Dia");
    static_assert(std::size(BLOQUES) == static_cast<std::size_t>(Bloque::Cantidad),
                  "BLOQUES debe tener un texto por cada Bloque");

    // true si alguna estadística general o el dinero está crítico.
    // El conocimiento no es un Stat, así que nunca genera alerta.
    bool hayAlerta(const Estadisticas& stats)
    {
        for (int i = 0; i < static_cast<int>(Stat::Cantidad); ++i)
        {
            if (stats.esCritica(static_cast<Stat>(i)))
                return true;
        }
        return false;
    }

    // Materia cuyo conocimiento se muestra: la del próximo examen.
    // Supone que las materias están en el enum en orden de examen.
    Materia materiaActual(const Tiempo& tiempo)
    {
        for (int i = 0; i < static_cast<int>(Materia::Cantidad); ++i)
        {
            const Materia m = static_cast<Materia>(i);
            if (tiempo.semana() <= Tiempo::semanaExamen(m))
                return m;
        }
        return Materia::Programacion;
    }

    void alinearDerecha(sf::Text& texto, float y)
    {
        texto.setPosition(sf::Vector2f(
            ANCHO_VISTA - MARGEN - anchoTexto(texto), y));
    }
}

bool Hud::iniciar(const GestorRecursos& recursos)
{
    vista_ = sf::View(crearRect(0.f, 0.f, ANCHO_VISTA, ALTO_VISTA));

    boton_.setSize(sf::Vector2f(BOTON_ANCHO, BOTON_ALTO));
    boton_.setPosition(sf::Vector2f(BOTON_X, BOTON_Y));
    boton_.setFillColor(COLOR_FONDO_BOTON);
    boton_.setOutlineColor(COLOR_BORDE_BOTON);
    boton_.setOutlineThickness(1.f);

    punto_.setRadius(RADIO_PUNTO);
    punto_.setOrigin(sf::Vector2f(RADIO_PUNTO, RADIO_PUNTO));
    punto_.setPosition(sf::Vector2f(BOTON_X + BOTON_ANCHO, BOTON_Y));
    punto_.setFillColor(COLOR_ALERTA);

    const sf::Font* fuente = recursos.obtenerFuente("principal");

    panel_.iniciar(fuente, sf::Vector2f(
        BOTON_X, BOTON_Y + BOTON_ALTO + SEPARACION_PANEL));

    if (fuente == nullptr)
    {
        std::cerr << "[Hud] La fuente 'principal' no esta cargada: "
                  << "el HUD se dibuja sin texto.\n";
        return false;
    }

    textoBoton_.emplace(crearTexto(*fuente, "Estadísticas [Tab]", TAM_TEXTO));
    textoBoton_->setFillColor(COLOR_TEXTO);
    textoBoton_->setPosition(sf::Vector2f(
        BOTON_X + (BOTON_ANCHO - anchoTexto(*textoBoton_)) / 2.f,
        BOTON_Y + BOTON_DESFASE_TEXTO));

    textoTiempo_.emplace(crearTexto(*fuente, "", TAM_TEXTO));
    textoTiempo_->setFillColor(COLOR_TEXTO);

    textoDinero_.emplace(crearTexto(*fuente, "", TAM_TEXTO));
    textoDinero_->setFillColor(COLOR_TEXTO);

    return true;
}

void Hud::teclaPresionada(sf::Keyboard::Key tecla)
{
    if (tecla == sf::Keyboard::Key::Tab)
        alternarPanel();
}

void Hud::ratonPresionado(sf::Vector2f posLogica)
{
    if (boton_.getGlobalBounds().contains(posLogica))
        alternarPanel();
}

void Hud::alternarPanel()
{
    panelAbierto_ = !panelAbierto_;
}

void Hud::actualizar(float dt, const Estadisticas& stats, const Tiempo& tiempo)
{
    panel_.actualizar(stats, materiaActual(tiempo));
    actualizarTextoTiempo(tiempo);
    actualizarTextoDinero(stats);

    hayAlerta_ = hayAlerta(stats);

    if (hayAlerta_)
    {
        fase_ += dt * VELOCIDAD_PARPADEO;
        if (fase_ > DOS_PI)
            fase_ -= DOS_PI;

        const float opacidad = 0.5f + 0.5f * std::sin(fase_);
        const float alpha    = ALPHA_MINIMO + (255.f - ALPHA_MINIMO) * opacidad;

        sf::Color color = COLOR_ALERTA;
        color.a = static_cast<std::uint8_t>(alpha);
        punto_.setFillColor(color);
    }
}

void Hud::actualizarTextoTiempo(const Tiempo& tiempo)
{
    if (!textoTiempo_)
        return;

    std::string s;

    if (tiempo.semestreTerminado())
    {
        s = "Fin del semestre";
    }
    else
    {
        s = "Semana " + std::to_string(tiempo.semana());

        if (tiempo.enFinDeSemana())
        {
            s += " · Fin de semana";
        }
        else
        {
            s += std::string(" · ") + DIAS[static_cast<int>(tiempo.dia())]
               + " · " + BLOQUES[static_cast<int>(tiempo.bloque())];
        }
    }

    if (s == ultimoTiempo_)
        return;

    ultimoTiempo_ = s;
    textoTiempo_->setString(textoUtf8(s));
    alinearDerecha(*textoTiempo_, MARGEN);
}

void Hud::actualizarTextoDinero(const Estadisticas& stats)
{
    if (!textoDinero_)
        return;

    const float dinero = stats.obtener(Stat::Dinero);

    // Mismo criterio que el punto de alerta: lo decide Estadisticas.
    textoDinero_->setFillColor(
        stats.esCritica(Stat::Dinero) ? COLOR_ALERTA : COLOR_TEXTO);

    const long long entero = std::llround(dinero);

    std::string s = "Dinero: ";
    if (entero > DINERO_MAXIMO_MOSTRADO)
        s += std::to_string(DINERO_MAXIMO_MOSTRADO) + "+";
    else
        s += std::to_string(entero);

    if (s == ultimoDinero_)
        return;

    ultimoDinero_ = s;
    textoDinero_->setString(textoUtf8(s));
    alinearDerecha(*textoDinero_, MARGEN + ALTO_LINEA);
}

void Hud::dibujar(sf::RenderTarget& destino) const
{
    const sf::View anterior = destino.getView();
    sf::View vistaHud = vista_;
    vistaHud.setViewport(anterior.getViewport());   // mismo letterbox que el juego
    destino.setView(vistaHud);

    destino.draw(boton_);
    if (textoBoton_)
        destino.draw(*textoBoton_);

    if (hayAlerta_)
        destino.draw(punto_);

    if (textoTiempo_)
        destino.draw(*textoTiempo_);
    if (textoDinero_)
        destino.draw(*textoDinero_);

    if (panelAbierto_)
        panel_.dibujar(destino);

    destino.setView(anterior);
}