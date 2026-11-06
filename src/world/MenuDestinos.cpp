#include "world/MenuDestinos.hpp"

#include <cstddef>
#include <utility>

#include "core/StateStack.hpp"

namespace
{
    constexpr float ANCHO_VISTA = 640.f;
    constexpr float ALTO_VISTA = 360.f;
    constexpr float ANCHO_PANEL = 320.f;
    constexpr float ALTO_FILA = 26.f;

    // Convierte un std::string en UTF-8 a texto de SFML (acentos y ¿).
    sf::String aTexto(const std::string& s)
    {
        return sf::String::fromUtf8(s.begin(), s.end());
    }

        // La vista (640x360) se amplía x2 en la ventana (1280x720).
    // El texto se crea al doble de tamaño y se reduce a la mitad
    // para que las letras salgan nítidas.
    constexpr unsigned ESCALA_TEXTO = 2;

    sf::Text crearTexto(const sf::Font& fuente,
                        const std::string& s,
                        unsigned tamano)
    {
        sf::Text texto(fuente, aTexto(s), tamano * ESCALA_TEXTO);
        texto.setScale(sf::Vector2f(1.f / ESCALA_TEXTO,
                                    1.f / ESCALA_TEXTO));
        return texto;
    }
}

MenuDestinos::MenuDestinos(
    StateStack* pila,
    const sf::Font& fuente,
    std::vector<Destino> destinos,
    std::function<void(const std::string&)> alElegir)
    : State(pila),
      fuente_(fuente),
      destinos_(std::move(destinos)),
      alElegir_(std::move(alElegir)),
      seleccion_(0)
{
}

void MenuDestinos::handleEvent(const sf::Event& event)
{
    const auto* tecla = event.getIf<sf::Event::KeyPressed>();

    if (tecla == nullptr)
        return;

    using Tecla = sf::Keyboard::Key;
    const int total = static_cast<int>(destinos_.size());

    switch (tecla->code)
    {
    case Tecla::W:
    case Tecla::Up:
        if (total > 0)
            seleccion_ = (seleccion_ + total - 1) % total;
        break;

    case Tecla::S:
    case Tecla::Down:
        if (total > 0)
            seleccion_ = (seleccion_ + 1) % total;
        break;

    case Tecla::Enter:
        elegir();
        break;

    case Tecla::Escape:
        m_stack->popState();   // destruye este menú
        break;

    default:
        break;
    }
}

void MenuDestinos::elegir()
{
    if (seleccion_ < 0 ||
        seleccion_ >= static_cast<int>(destinos_.size()))
    {
        return;
    }

    const Destino& destino = destinos_[seleccion_];

    if (destino.bloqueado)
        return;

    // Copias locales: popState() destruye este objeto,
    // así que después no se debe usar nada de 'this'.
    const std::string id = destino.id;
    const auto alElegir = alElegir_;

    m_stack->popState();
    alElegir(id);
}

void MenuDestinos::update(float)
{
}

void MenuDestinos::draw(sf::RenderWindow& window)
{
    // Oscurece lo que hay detrás.
    sf::RectangleShape velo(sf::Vector2f(ANCHO_VISTA, ALTO_VISTA));
    velo.setFillColor(sf::Color(0, 0, 0, 140));
    window.draw(velo);

    // Panel central.
    const float alto =
        70.f + ALTO_FILA * static_cast<float>(destinos_.size());

    sf::RectangleShape panel(sf::Vector2f(ANCHO_PANEL, alto));
    panel.setPosition(sf::Vector2f(
        (ANCHO_VISTA - ANCHO_PANEL) / 2.f,
        (ALTO_VISTA - alto) / 2.f
    ));
    panel.setFillColor(sf::Color(30, 30, 50, 240));
    panel.setOutlineColor(sf::Color::White);
    panel.setOutlineThickness(2.f);
    window.draw(panel);

    const sf::Vector2f origen = panel.getPosition();

    // Título.
    sf::Text titulo = crearTexto(fuente_, "¿A dónde quieres ir?", 18);
    titulo.setFillColor(sf::Color::White);
    titulo.setPosition(sf::Vector2f(origen.x + 16.f, origen.y + 10.f));
    window.draw(titulo);

    // Opciones.
    for (std::size_t i = 0; i < destinos_.size(); ++i)
    {
        const Destino& d = destinos_[i];
        const bool elegido = (static_cast<int>(i) == seleccion_);

        std::string linea = (elegido ? "> " : "   ") + d.nombre;

        if (d.bloqueado)
            linea += " [bloqueado]";

        sf::Text texto = crearTexto(fuente_, linea, 16);

        if (d.bloqueado)
            texto.setFillColor(sf::Color(120, 120, 120));
        else if (elegido)
            texto.setFillColor(sf::Color::Yellow);
        else
            texto.setFillColor(sf::Color::White);

        texto.setPosition(sf::Vector2f(
            origen.x + 16.f,
            origen.y + 44.f + ALTO_FILA * static_cast<float>(i)
        ));

        window.draw(texto);
    }

    // Ayuda.
    sf::Text ayuda = crearTexto(fuente_, "W/S mover  -  Enter elegir  -  Esc cerrar", 12);
    ayuda.setFillColor(sf::Color(170, 170, 170));
    ayuda.setPosition(sf::Vector2f(origen.x + 16.f, origen.y + alto - 22.f));
    window.draw(ayuda);
}