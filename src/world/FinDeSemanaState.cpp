#include "world/FinDeSemanaState.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <sstream>
#include <string>
#include <utility>

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
    constexpr float VELOCIDAD_FUNDIDO = 600.f;

    const sf::Color COLOR_TITULO(255, 230, 90);
    const sf::Color COLOR_SEMANA(170, 170, 190);
    const sf::Color COLOR_DESCRIPCION(255, 255, 255);
    const sf::Color COLOR_EFECTOS(200, 225, 200);
    const sf::Color COLOR_INSTRUCCION(180, 180, 180);

    sf::Color conAlfa(sf::Color c, float alfa)
    {
        const float factor = std::clamp(alfa, 0.f, 255.f) / 255.f;
        return sf::Color(c.r, c.g, c.b, static_cast<std::uint8_t>(c.a * factor));
    }

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
                                   const ResultadoFinDeSemana& resultado,
                                   std::vector<const sf::Texture*> imagenes)
    : State(stack),
      m_recursos(recursos),
      m_resultado(resultado),
      m_imagenes(std::move(imagenes))
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

    // Barras/luces laterales suaves para el ambiente de Rumba
    m_lucesRumba[0].setSize(sf::Vector2f(140.f, ALTO_VISTA));
    m_lucesRumba[0].setPosition(sf::Vector2f(0.f, 0.f));

    m_lucesRumba[1].setSize(sf::Vector2f(140.f, ALTO_VISTA));
    m_lucesRumba[1].setPosition(sf::Vector2f(ANCHO_VISTA - 140.f, 0.f));

    if (!m_imagenes.empty())
    {
        m_imagenA.setSize(sf::Vector2f(ANCHO_VISTA, ALTO_VISTA));
        m_imagenB.setSize(sf::Vector2f(ANCHO_VISTA, ALTO_VISTA));

        m_imagenA.setOrigin(sf::Vector2f(ANCHO_VISTA / 2.f, ALTO_VISTA / 2.f));
        m_imagenB.setOrigin(sf::Vector2f(ANCHO_VISTA / 2.f, ALTO_VISTA / 2.f));

        const float n = static_cast<float>(m_imagenes.size());
        if (m_resultado.tipo == TipoFinDeSemana::Rumba)
        {
            m_bucle    = true;
            m_periodo  = 1.05f;    // Más tranquilo y cómodo de ver
            m_duracion = 4.2f;
        }
        else
        {
            m_bucle    = false;
            m_periodo  = 1.5f;
            m_duracion = n * m_periodo + 0.8f;
        }

        m_mostrandoTarjeta = false;
        actualizarImagenes(0.f);
        actualizarLucesRumba();
    }

    configurarTextos();
    aplicarAlfa();
}

void FinDeSemanaState::aplicarTransformacion(sf::RectangleShape& shape, float avanceLocal, int indice)
{
    (void)avanceLocal;

    // Zoom y paneo suave continuo
    const float escalaBase = 1.02f + 0.03f * std::sin(m_tiempo * 0.7f + static_cast<float>(indice));
    const float offsetX = std::cos(m_tiempo * 0.45f + static_cast<float>(indice)) * 3.5f;
    const float offsetY = std::sin(m_tiempo * 0.55f + static_cast<float>(indice)) * 2.5f;

    // Rebote suave y discreto solo en Rumba (sin sacudidas bruscas)
    float reboteY = 0.f;
    if (m_resultado.tipo == TipoFinDeSemana::Rumba)
    {
        reboteY = std::sin(m_tiempo * 3.5f) * -1.8f;
    }

    shape.setScale(sf::Vector2f(escalaBase, escalaBase));
    shape.setPosition(sf::Vector2f(ANCHO_VISTA / 2.f + offsetX, ALTO_VISTA / 2.f + offsetY + reboteY));
}

void FinDeSemanaState::actualizarLucesRumba()
{
    if (m_resultado.tipo != TipoFinDeSemana::Rumba)
        return;

    const float pulso1 = 0.5f + 0.5f * std::sin(m_tiempo * 2.6f);
    const float pulso2 = 0.5f + 0.5f * std::cos(m_tiempo * 2.6f);

    const auto a1 = static_cast<std::uint8_t>(18.f + pulso1 * 26.f);
    const auto a2 = static_cast<std::uint8_t>(18.f + pulso2 * 26.f);

    m_lucesRumba[0].setFillColor(sf::Color(235, 65, 145, a1));
    m_lucesRumba[1].setFillColor(sf::Color(70, 185, 255, a2));
}

void FinDeSemanaState::actualizarImagenes(float dt)
{
    (void)dt;
    if (m_imagenes.empty())
        return;

    const int n = static_cast<int>(m_imagenes.size());
    const int paso = static_cast<int>(m_tiempo / m_periodo);
    const int actual    = m_bucle ? paso % n       : std::min(paso, n - 1);
    const int siguiente = m_bucle ? (actual + 1) % n : std::min(actual + 1, n - 1);

    const float u = std::fmod(m_tiempo, m_periodo) / m_periodo;

    float mezcla = 0.f;
    if (actual != siguiente)
    {
        mezcla = std::clamp((u - 0.6f) / 0.4f, 0.f, 1.f);
    }
    mezcla = mezcla * mezcla * (3.f - 2.f * mezcla);

    m_imagenA.setTexture(m_imagenes[actual], true);
    m_imagenA.setFillColor(sf::Color::White);
    aplicarTransformacion(m_imagenA, u, actual);

    m_imagenB.setTexture(m_imagenes[siguiente], true);
    m_imagenB.setFillColor(sf::Color(255, 255, 255, static_cast<std::uint8_t>(255.f * mezcla)));
    aplicarTransformacion(m_imagenB, u, siguiente);
}

void FinDeSemanaState::configurarTextos()
{
    const sf::Font* fuente = m_recursos.obtenerFuente("principal");
    if (fuente == nullptr)
        fuente = m_recursos.obtenerFuente("arial");
    if (fuente == nullptr)
        return;

    float y = TARJETA_Y + 14.f;

    const std::string nombre = m_resultado.nombre.empty() ? "Fin de semana" : m_resultado.nombre;
    m_titulo.emplace(*fuente, textoUtf8("Fin de semana: " + nombre), 20);
    m_titulo->setPosition(sf::Vector2f(TEXTO_X, y));
    y += 28.f;

    if (m_resultado.semanaNueva > 0)
    {
        m_semana.emplace(*fuente,
            textoUtf8("Terminó la semana " + std::to_string(m_resultado.semanaNueva - 1) +
                      ". Empieza la semana " + std::to_string(m_resultado.semanaNueva) + "."),
            14);
        m_semana->setPosition(sf::Vector2f(TEXTO_X, y));
        y += 26.f;
    }

    const std::string descripcion = envolver(m_resultado.texto, ANCHO_LINEA);
    m_descripcion.emplace(*fuente, textoUtf8(descripcion), 16);
    m_descripcion->setPosition(sf::Vector2f(TEXTO_X, y));
    const int lineasDescripcion =
        1 + static_cast<int>(std::count(descripcion.begin(), descripcion.end(), '\n'));
    y += static_cast<float>(lineasDescripcion) * 20.f + 14.f;

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

    const std::string cierre = (m_resultado.semanaNueva > 0)
        ? "Presiona ENTER para empezar la semana " + std::to_string(m_resultado.semanaNueva)
        : std::string("Presiona ENTER para continuar");
    m_instruccion.emplace(*fuente, textoUtf8(cierre), 14);
    m_instruccion->setPosition(sf::Vector2f(TEXTO_X, TARJETA_Y + TARJETA_H - 28.f));
}

void FinDeSemanaState::aplicarAlfa()
{
    m_fondoOscuro.setFillColor(conAlfa(sf::Color(0, 0, 0, 150), m_alfa));
    m_tarjeta.setFillColor(conAlfa(sf::Color(30, 30, 45, 225), m_alfa));
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
        if (!m_mostrandoTarjeta)
        {
            m_mostrandoTarjeta = true;
            return;
        }
        if (m_stack != nullptr)
            m_stack->popState();
    }
}

void FinDeSemanaState::update(float dt)
{
    m_tiempo += dt;
    actualizarImagenes(dt);
    actualizarLucesRumba();

    if (!m_mostrandoTarjeta)
    {
        if (m_tiempo >= m_duracion)
            m_mostrandoTarjeta = true;
        return;
    }

    if (m_alfa < 255.f)
    {
        m_alfa = std::min(255.f, m_alfa + dt * VELOCIDAD_FUNDIDO);
        aplicarAlfa();
    }
}

void FinDeSemanaState::draw(sf::RenderWindow& window)
{
    if (!m_imagenes.empty())
    {
        window.draw(m_imagenA);
        window.draw(m_imagenB);
    }

    if (m_resultado.tipo == TipoFinDeSemana::Rumba)
    {
        window.draw(m_lucesRumba[0]);
        window.draw(m_lucesRumba[1]);
    }

    if (!m_mostrandoTarjeta)
        return;

    window.draw(m_fondoOscuro);
    window.draw(m_tarjeta);

    if (m_titulo)      window.draw(*m_titulo);
    if (m_semana)      window.draw(*m_semana);
    if (m_descripcion) window.draw(*m_descripcion);
    if (m_efectos)     window.draw(*m_efectos);
    if (m_instruccion) window.draw(*m_instruccion);
}