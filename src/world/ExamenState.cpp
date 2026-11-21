#include "world/ExamenState.hpp"

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

    constexpr std::size_t ANCHO_LINEA = 62;
    constexpr float VELOCIDAD_FUNDIDO = 600.f;   // alfa por segundo

    const sf::Color COLOR_TITULO(255, 230, 90);
    const sf::Color COLOR_COMENTARIO(255, 255, 255);
    const sf::Color COLOR_DESGLOSE(190, 200, 220);
    const sf::Color COLOR_INSTRUCCION(180, 180, 180);
    const sf::Color COLOR_APROBADO(90, 220, 130);
    const sf::Color COLOR_REPROBADO(235, 90, 90);

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

    // Nombres para mostrar (con tilde).
    const char* nombreMateria(Materia m)
    {
        switch (m)
        {
        case Materia::Calculo:      return "Cálculo";
        case Materia::Fisica:       return "Física";
        case Materia::Programacion: return "Programación";
        default:                    return "?";
        }
    }

    // Comentario segun la nota.
    const char* comentarioPara(int nota, bool aprobado)
    {
        if (!aprobado) return "Esta vez no alcanzó, pero el semestre continúa.";
        if (nota >= 18) return "¡Examen brillante! Se nota todo lo que estudiaste.";
        if (nota >= 14) return "¡Buen examen! El esfuerzo valió la pena.";
        return "Aprobaste, aunque raspando.";
    }

    std::string aEntero(float valor)
    {
        return std::to_string(std::lround(valor));
    }
}

ExamenState::ExamenState(StateStack* stack, GestorRecursos& recursos,
                         const ResultadoExamen& resultado)
    : State(stack),
      m_recursos(recursos),
      m_resultado(resultado)
{
    m_colorResultado = m_resultado.aprobado ? COLOR_APROBADO : COLOR_REPROBADO;

    m_fondoOscuro.setSize(sf::Vector2f(ANCHO_VISTA, ALTO_VISTA));

    m_tarjeta.setSize(sf::Vector2f(TARJETA_W, TARJETA_H));
    m_tarjeta.setPosition(sf::Vector2f(TARJETA_X, TARJETA_Y));
    m_tarjeta.setOutlineThickness(3.f);

    configurarTextos();
    aplicarAlfa();
}

void ExamenState::configurarTextos()
{
    // Si la fuente no cargo, la pantalla se ve sin texto pero no se cae.
    const sf::Font* fuente = m_recursos.obtenerFuente("principal");
    if (fuente == nullptr)
        fuente = m_recursos.obtenerFuente("arial");
    if (fuente == nullptr)
        return;

    float y = TARJETA_Y + 14.f;

    // --- Titulo ---
    m_titulo.emplace(*fuente,
        textoUtf8(std::string("Examen de ") + nombreMateria(m_resultado.materia)), 20);
    m_titulo->setPosition(sf::Vector2f(TEXTO_X, y));
    y += 34.f;

    // --- Nota grande y veredicto ---
    m_nota.emplace(*fuente,
        textoUtf8(std::to_string(m_resultado.nota) + " / " + std::to_string(Notas::NOTA_MAXIMA)),
        46);
    m_nota->setPosition(sf::Vector2f(TEXTO_X, y));

    m_veredicto.emplace(*fuente,
        textoUtf8(m_resultado.aprobado ? "APROBADO" : "REPROBADO"), 26);
    m_veredicto->setPosition(sf::Vector2f(TEXTO_X + 200.f, y + 14.f));
    y += 70.f;

    // --- Comentario ---
    const std::string comentario =
        envolver(comentarioPara(m_resultado.nota, m_resultado.aprobado), ANCHO_LINEA);
    m_comentario.emplace(*fuente, textoUtf8(comentario), 16);
    m_comentario->setPosition(sf::Vector2f(TEXTO_X, y));
    const int lineas = 1 + static_cast<int>(std::count(comentario.begin(), comentario.end(), '\n'));
    y += static_cast<float>(lineas) * 20.f + 14.f;

    // --- De donde salio la nota ---
    const std::string desglose =
        "-- CÓMO SALIÓ LA NOTA --\n"
        "Conocimiento: " + aEntero(m_resultado.conocimiento) + " / 100\n"
        "Cómo llegaste (energía y estrés): " + aEntero(m_resultado.estado) + " / 100\n"
        "Suerte: " + aEntero(m_resultado.azar) + " / 100";
    m_desglose.emplace(*fuente, textoUtf8(desglose), 15);
    m_desglose->setPosition(sf::Vector2f(TEXTO_X, y));

    // --- Instruccion (abajo de la tarjeta) ---
    m_instruccion.emplace(*fuente, textoUtf8("Presiona ENTER para continuar"), 14);
    m_instruccion->setPosition(sf::Vector2f(TEXTO_X, TARJETA_Y + TARJETA_H - 28.f));
}

void ExamenState::aplicarAlfa()
{
    m_fondoOscuro.setFillColor(conAlfa(sf::Color(0, 0, 0, 200), m_alfa));
    m_tarjeta.setFillColor(conAlfa(sf::Color(30, 30, 45, 245), m_alfa));
    m_tarjeta.setOutlineColor(conAlfa(m_colorResultado, m_alfa));

    if (m_titulo)      m_titulo->setFillColor(conAlfa(COLOR_TITULO, m_alfa));
    if (m_nota)        m_nota->setFillColor(conAlfa(m_colorResultado, m_alfa));
    if (m_veredicto)   m_veredicto->setFillColor(conAlfa(m_colorResultado, m_alfa));
    if (m_comentario)  m_comentario->setFillColor(conAlfa(COLOR_COMENTARIO, m_alfa));
    if (m_desglose)    m_desglose->setFillColor(conAlfa(COLOR_DESGLOSE, m_alfa));
    if (m_instruccion) m_instruccion->setFillColor(conAlfa(COLOR_INSTRUCCION, m_alfa));
}

void ExamenState::handleEvent(const sf::Event& event)
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

void ExamenState::update(float dt)
{
    if (m_alfa < 255.f)
    {
        m_alfa = std::min(255.f, m_alfa + dt * VELOCIDAD_FUNDIDO);
        aplicarAlfa();
    }
}

void ExamenState::draw(sf::RenderWindow& window)
{
    window.draw(m_fondoOscuro);
    window.draw(m_tarjeta);

    if (m_titulo)      window.draw(*m_titulo);
    if (m_nota)        window.draw(*m_nota);
    if (m_veredicto)   window.draw(*m_veredicto);
    if (m_comentario)  window.draw(*m_comentario);
    if (m_desglose)    window.draw(*m_desglose);
    if (m_instruccion) window.draw(*m_instruccion);
}
