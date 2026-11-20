#include "world/FinDeSemanaState.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <sstream>
#include <string>

#include "core/CompatSFML.hpp"
#include "core/StateStack.hpp"

namespace
{
    constexpr float ANCHO_VISTA = 640.f;
    constexpr float ALTO_VISTA  = 360.f;

    constexpr float TARJETA_X = 40.f;
    constexpr float TARJETA_Y = 24.f;
    constexpr float TARJETA_W = ANCHO_VISTA - 2.f * TARJETA_X;   // 560
    constexpr float TARJETA_H = ALTO_VISTA  - 2.f * TARJETA_Y;   // 312
    constexpr float MARGEN    = 20.f;
    constexpr float TEXTO_X   = TARJETA_X + MARGEN;

    constexpr std::size_t ANCHO_LINEA = 62;   // caracteres por linea
    constexpr float VELOCIDAD_FUNDIDO = 600.f; // alfa por segundo

    const sf::Color COLOR_TITULO(255, 230, 90);
    const sf::Color COLOR_SEMANA(170, 170, 190);
    const sf::Color COLOR_DESCRIPCION(255, 255, 255);
    const sf::Color COLOR_EFECTOS(200, 225, 200);
    const sf::Color COLOR_INSTRUCCION(180, 180, 180);

    // Mismo color con la transparencia multiplicada por el fundido (0 a 255).
    sf::Color conAlfa(sf::Color c, float alfa)
    {
        const float factor = std::clamp(alfa, 0.f, 255.f) / 255.f;
        return sf::Color(c.r, c.g, c.b, static_cast<std::uint8_t>(c.a * factor));
    }

    // Parte un texto largo en lineas de como maximo 'ancho' caracteres.
    std::string envolver(const std::string& texto, std::size_t ancho)
    {
        std::istringstream palabras(texto);
        std::string palabra, linea, resultado;

        while (palabras >> palabra)
        {
            if (!linea.empty() && linea.size() + 1 + palabra.size() > ancho)
            {
                resultado += linea + "\n";
                linea.clear();
            }
            linea += (linea.empty() ? "" : " ") + palabra;
        }
        return resultado + linea;
    }

    // "+15", "-5", "0"
    std::string conSigno(float valor)
    {
        const long entero = std::lround(valor);
        return std::string(entero > 0 ? "+" : "") + std::to_string(entero);
    }

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

FinDeSemanaState::FinDeSemanaState(StateStack* stack, GestorRecursos& recursos,
                                   const ResultadoFinDeSemana& resultado)
    : State(stack),
      m_recursos(recursos),
      m_resultado(resultado)
{
    switch (m_resultado.tipo)
    {
    case TipoFinDeSemana::Rumba:   m_colorBorde = sf::Color(230, 80, 120);  break;
    case TipoFinDeSemana::Aseo:    m_colorBorde = sf::Color(80, 200, 120);  break;
    case TipoFinDeSemana::Estudio: m_colorBorde = sf::Color(80, 150, 240);  break;
    default:                       m_colorBorde = sf::Color::White;         break;
    }

    m_fondoOscuro.setSize(sf::Vector2f(ANCHO_VISTA, ALTO_VISTA));

    m_tarjeta.setSize(sf::Vector2f(TARJETA_W, TARJETA_H));
    m_tarjeta.setPosition(sf::Vector2f(TARJETA_X, TARJETA_Y));
    m_tarjeta.setOutlineThickness(3.f);

    configurarTextos();
    aplicarAlfa();
}

void FinDeSemanaState::configurarTextos()
{
    // Si la fuente no cargo, la pantalla se ve sin texto pero no se cae.
    const sf::Font* fuente = m_recursos.obtenerFuente("principal");
    if (fuente == nullptr)
        fuente = m_recursos.obtenerFuente("arial");
    if (fuente == nullptr)
        return;

    float y = TARJETA_Y + 14.f;

    // --- Titulo: que paso este fin de semana ---
    const std::string nombre = m_resultado.nombre.empty() ? "Fin de semana" : m_resultado.nombre;
    m_titulo.emplace(*fuente, textoUtf8("Fin de semana: " + nombre), 20);
    m_titulo->setPosition(sf::Vector2f(TEXTO_X, y));
    y += 28.f;

    // --- Semana que termino / semana que empieza ---
    if (m_resultado.semanaNueva > 0)
    {
        m_semana.emplace(*fuente,
            textoUtf8("Terminó la semana " + std::to_string(m_resultado.semanaNueva - 1) +
                      ". Empieza la semana " + std::to_string(m_resultado.semanaNueva) + "."),
            14);
        m_semana->setPosition(sf::Vector2f(TEXTO_X, y));
        y += 26.f;
    }

    // --- Lo que hizo el personaje ---
    const std::string descripcion = envolver(m_resultado.texto, ANCHO_LINEA);
    m_descripcion.emplace(*fuente, textoUtf8(descripcion), 16);
    m_descripcion->setPosition(sf::Vector2f(TEXTO_X, y));
    const int lineasDescripcion =
        1 + static_cast<int>(std::count(descripcion.begin(), descripcion.end(), '\n'));
    y += static_cast<float>(lineasDescripcion) * 20.f + 14.f;

    // --- Efectos ---
    std::string cambios;
    for (int i = 0; i < static_cast<int>(Stat::Cantidad); ++i)
    {
        if (m_resultado.cambios[i] != 0.f)
            cambios += std::string(nombreVisible(static_cast<Stat>(i))) + " " +
                       conSigno(m_resultado.cambios[i]) + "    ";
    }
    if (m_resultado.cambioConocimiento != 0.f)
        cambios += "Conocimiento " + conSigno(m_resultado.cambioConocimiento) + "    ";
    if (m_resultado.cambioOrden != 0.f)
        cambios += "Orden de la casa " + conSigno(m_resultado.cambioOrden);

    std::string efectos = "-- EFECTOS --\n";
    if (!cambios.empty())
        efectos += envolver(cambios, ANCHO_LINEA) + "\n";
    efectos += "Mesada de tus padres: " + conSigno(m_resultado.mesada);

    m_efectos.emplace(*fuente, textoUtf8(efectos), 15);
    m_efectos->setPosition(sf::Vector2f(TEXTO_X, y));

    // --- Instruccion (abajo de la tarjeta) ---
    const std::string cierre = (m_resultado.semanaNueva > 0)
        ? "Presiona ENTER para empezar la semana " + std::to_string(m_resultado.semanaNueva)
        : std::string("Presiona ENTER para continuar");
    m_instruccion.emplace(*fuente, textoUtf8(cierre), 14);
    m_instruccion->setPosition(sf::Vector2f(TEXTO_X, TARJETA_Y + TARJETA_H - 28.f));
}

void FinDeSemanaState::aplicarAlfa()
{
    m_fondoOscuro.setFillColor(conAlfa(sf::Color(0, 0, 0, 200), m_alfa));
    m_tarjeta.setFillColor(conAlfa(sf::Color(30, 30, 45, 245), m_alfa));
    m_tarjeta.setOutlineColor(conAlfa(m_colorBorde, m_alfa));

    if (m_titulo)       m_titulo->setFillColor(conAlfa(COLOR_TITULO, m_alfa));
    if (m_semana)       m_semana->setFillColor(conAlfa(COLOR_SEMANA, m_alfa));
    if (m_descripcion)  m_descripcion->setFillColor(conAlfa(COLOR_DESCRIPCION, m_alfa));
    if (m_efectos)      m_efectos->setFillColor(conAlfa(COLOR_EFECTOS, m_alfa));
    if (m_instruccion)  m_instruccion->setFillColor(conAlfa(COLOR_INSTRUCCION, m_alfa));
}

void FinDeSemanaState::handleEvent(const sf::Event& event)
{
    const auto* tecla = event.getIf<sf::Event::KeyPressed>();
    if (tecla == nullptr)
        return;

    if (tecla->code == sf::Keyboard::Key::Enter ||
        tecla->code == sf::Keyboard::Key::Space ||
        tecla->code == sf::Keyboard::Key::E)
    {
        if (m_stack != nullptr)
            m_stack->popState();
    }
}

void FinDeSemanaState::update(float dt)
{
    if (m_alfa < 255.f)
    {
        m_alfa = std::min(255.f, m_alfa + dt * VELOCIDAD_FUNDIDO);
        aplicarAlfa();
    }
}

void FinDeSemanaState::draw(sf::RenderWindow& window)
{
    window.draw(m_fondoOscuro);
    window.draw(m_tarjeta);

    if (m_titulo)      window.draw(*m_titulo);
    if (m_semana)      window.draw(*m_semana);
    if (m_descripcion) window.draw(*m_descripcion);
    if (m_efectos)     window.draw(*m_efectos);
    if (m_instruccion) window.draw(*m_instruccion);
}
