#include "world/IntroState.hpp"

#include <algorithm>
#include <filesystem>
#include <sstream>
#include <string>
#include <utility>

#include "core/CompatSFML.hpp"
#include "core/StateStack.hpp"

namespace
{
    constexpr float ANCHO_VISTA = 640.f;
    constexpr float ALTO_VISTA  = 360.f;

    constexpr float CAJA_X = 28.f;
    constexpr float CAJA_Y = 232.f;
    constexpr float CAJA_W = ANCHO_VISTA - 2.f * CAJA_X;
    constexpr float CAJA_H = 112.f;

    constexpr float MARGEN_X = 42.f;
    constexpr float MARGEN_Y = 242.f;
    constexpr std::size_t ANCHO_LINEA = 68;

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
}

IntroState::IntroState(StateStack* stack, GestorRecursos& recursos,
                       std::vector<DiapositivaIntro> diapositivas)
    : State(stack),
      m_recursos(recursos),
      m_diapositivas(std::move(diapositivas))
{
    m_fondo.setSize(sf::Vector2f(ANCHO_VISTA, ALTO_VISTA));
    m_fondo.setFillColor(sf::Color(15, 15, 22, 255));

    m_imagen.setSize(sf::Vector2f(ANCHO_VISTA, ALTO_VISTA));
    m_imagen.setPosition(sf::Vector2f(0.f, 0.f));

    m_cajaTexto.setSize(sf::Vector2f(CAJA_W, CAJA_H));
    m_cajaTexto.setPosition(sf::Vector2f(CAJA_X, CAJA_Y));
    m_cajaTexto.setFillColor(sf::Color(18, 20, 32, 225));
    m_cajaTexto.setOutlineColor(sf::Color(235, 215, 110));
    m_cajaTexto.setOutlineThickness(2.f);

    if (m_diapositivas.empty())
    {
        if (m_stack != nullptr)
            m_stack->popState();
        return;
    }

    cargarDiapositiva(0);
}

void IntroState::cargarDiapositiva(std::size_t indice)
{
    if (indice >= m_diapositivas.size())
        return;

    m_indiceActual = indice;
    m_tieneImagen = false;

    const std::string& ruta = m_diapositivas[indice].rutaImagen;
    const std::string clave = "intro_" + std::to_string(indice + 1);

    if (!ruta.empty())
    {
        if (m_recursos.obtenerTextura(clave) == nullptr && std::filesystem::exists(ruta))
            m_recursos.cargarTextura(clave, ruta);

        if (sf::Texture* tex = m_recursos.obtenerTextura(clave))
        {
            tex->setSmooth(true);
            m_imagen.setTexture(tex, true);
            m_imagen.setFillColor(sf::Color::White);
            m_tieneImagen = true;
        }
    }

    const sf::Font* fuente = m_recursos.obtenerFuente("principal");
    if (fuente == nullptr)
        fuente = m_recursos.obtenerFuente("arial");
    if (fuente == nullptr)
        return;

    const std::string envuelto = envolver(m_diapositivas[indice].texto, ANCHO_LINEA);
    m_texto.emplace(*fuente, textoUtf8(envuelto), 15);
    m_texto->setFillColor(sf::Color::White);
    m_texto->setPosition(sf::Vector2f(MARGEN_X, MARGEN_Y));

    const bool esUltima = (indice + 1 >= m_diapositivas.size());
    const std::string pie = esUltima
        ? "Presiona ENTER para comenzar (" + std::to_string(indice + 1) + "/" + std::to_string(m_diapositivas.size()) + ")"
        : "Presiona ENTER para continuar (" + std::to_string(indice + 1) + "/" + std::to_string(m_diapositivas.size()) + ")";

    m_instruccion.emplace(*fuente, textoUtf8(pie), 13);
    m_instruccion->setFillColor(sf::Color(255, 225, 110));
    m_instruccion->setPosition(sf::Vector2f(MARGEN_X, CAJA_Y + CAJA_H - 24.f));
}

void IntroState::avanzar()
{
    if (m_indiceActual + 1 < m_diapositivas.size())
    {
        cargarDiapositiva(m_indiceActual + 1);
        return;
    }

    if (m_stack != nullptr)
        m_stack->popState();
}

void IntroState::handleEvent(const sf::Event& event)
{
    const auto* tecla = event.getIf<sf::Event::KeyPressed>();
    if (tecla == nullptr)
        return;

    if (tecla->code == sf::Keyboard::Key::Enter ||
        tecla->code == sf::Keyboard::Key::Space ||
        tecla->code == sf::Keyboard::Key::E)
    {
        avanzar();
    }
}

void IntroState::update(float dt)
{
    (void)dt;
}

void IntroState::draw(sf::RenderWindow& window)
{
    window.draw(m_fondo);

    if (m_tieneImagen)
        window.draw(m_imagen);

    window.draw(m_cajaTexto);

    if (m_texto)
        window.draw(*m_texto);
    if (m_instruccion)
        window.draw(*m_instruccion);
}