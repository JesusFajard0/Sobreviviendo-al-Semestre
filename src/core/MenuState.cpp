#include "core/MenuState.hpp"

#include <string>
#include <utility>

#include "core/CompatSFML.hpp"
#include "core/StateStack.hpp"

namespace
{
    // Los textos se crean al doble de tamano y se reducen a la mitad para
    // que salgan nitidos (igual que en ObjetoActividad).
    constexpr unsigned ESCALA_TEXTO = 2;

    constexpr float ANCHO_BOTON = 200.f;
    constexpr float ALTO_BOTON  = 30.f;
    constexpr float SEPARACION  = 10.f;

    const char* const ETIQUETAS[] = {"Jugar", "Cómo jugar", "Créditos", "Salir"};

    // Textos de las pantallas secundarias (editar aqui para cambiarlos).
    const char* const TEXTO_COMO_JUGAR =
        "Objetivo: sobrevivir las 8 semanas del semestre y aprobar\n"
        "los exámenes de Cálculo (semana 3), Física (semana 5) y\n"
        "Programación (semana 8).\n"
        "\n"
        "Moverte: W A S D o las flechas\n"
        "Interactuar y hablar: E\n"
        "Ver las estadísticas: Tab o el botón del HUD\n"
        "Pantalla completa: F11\n"
        "\n"
        "Cuida tu energía, estrés, salud mental, dopamina y dinero.\n"
        "Asiste a clase por la mañana y duerme por la noche.";

    const char* const TEXTO_CREDITOS =
        "Proyecto de Programación 3\n"
        "\n"
        "Jesús Fajardo\n"
        "Daniel Freitas\n"
        "Luis Ortega\n"
        "\n"
        "Hecho en C++17 con SFML y la librería DeSiGNAR.";

    bool esTeclaElegir(sf::Keyboard::Key k)
    {
        return k == sf::Keyboard::Key::Enter ||
               k == sf::Keyboard::Key::E ||
               k == sf::Keyboard::Key::Space;
    }
}

MenuState::MenuState(StateStack* pila, const sf::Font& fuente, Accion alJugar, Accion alSalir)
    : State(pila),
      fuente_(fuente),
      alJugar_(std::move(alJugar)),
      alSalir_(std::move(alSalir))
{
}

// ---------------------------------------------------------------
// Botones
// ---------------------------------------------------------------

sf::FloatRect MenuState::rectBoton(int indice) const
{
    const float x = origen_.x + (tam_.x - ANCHO_BOTON) / 2.f;
    const float y = origen_.y + tam_.y * 0.46f +
                    static_cast<float>(indice) * (ALTO_BOTON + SEPARACION);
    return crearRect(x, y, ANCHO_BOTON, ALTO_BOTON);
}

int MenuState::botonEn(const sf::Vector2f& punto) const
{
    for (int i = 0; i < CantidadOpciones; ++i)
    {
        if (rectBoton(i).contains(punto))
            return i;
    }
    return -1;
}

void MenuState::elegir(int indice)
{
    switch (indice)
    {
    case Jugar:
    {
        // popState destruye este objeto: se saca la accion antes.
        Accion iniciar = std::move(alJugar_);
        m_stack->popState();
        if (iniciar)
            iniciar();
        return;   // el objeto ya no existe
    }

    case ComoJugar:
        pantalla_ = Pantalla::ComoJugar;
        break;

    case Creditos:
        pantalla_ = Pantalla::Creditos;
        break;

    case Salir:
        if (alSalir_)
            alSalir_();
        break;

    default:
        break;
    }
}

// ---------------------------------------------------------------
// Entrada
// ---------------------------------------------------------------

void MenuState::handleEvent(const sf::Event& event)
{
    // En "Como jugar" y "Creditos" cualquier tecla de aceptar, ESC o un clic
    // izquierdo vuelven al menu.
    if (pantalla_ != Pantalla::Principal)
    {
        if (const auto* tecla = event.getIf<sf::Event::KeyPressed>())
        {
            if (tecla->code == sf::Keyboard::Key::Escape || esTeclaElegir(tecla->code))
                pantalla_ = Pantalla::Principal;
        }
        else if (const auto* clic = event.getIf<sf::Event::MouseButtonPressed>())
        {
            if (clic->button == sf::Mouse::Button::Left)
                pantalla_ = Pantalla::Principal;
        }
        return;
    }

    if (const auto* tecla = event.getIf<sf::Event::KeyPressed>())
    {
        switch (tecla->code)
        {
        case sf::Keyboard::Key::Up:
        case sf::Keyboard::Key::W:
            seleccion_ = (seleccion_ + CantidadOpciones - 1) % CantidadOpciones;
            break;

        case sf::Keyboard::Key::Down:
        case sf::Keyboard::Key::S:
            seleccion_ = (seleccion_ + 1) % CantidadOpciones;
            break;

        default:
            if (esTeclaElegir(tecla->code))
                elegir(seleccion_);
            break;
        }
    }
    else if (const auto* mover = event.getIf<sf::Event::MouseMoved>())
    {
        if (ventana_ == nullptr)
            return;

        const int sobre = botonEn(ventana_->mapPixelToCoords(mover->position));
        if (sobre >= 0)
            seleccion_ = sobre;
    }
    else if (const auto* clic = event.getIf<sf::Event::MouseButtonPressed>())
    {
        if (clic->button != sf::Mouse::Button::Left || ventana_ == nullptr)
            return;

        const int sobre = botonEn(ventana_->mapPixelToCoords(clic->position));
        if (sobre >= 0)
        {
            seleccion_ = sobre;
            elegir(sobre);   // despues de Jugar no se usa nada mas
        }
    }
}

void MenuState::update(float)
{
}

// ---------------------------------------------------------------
// Dibujo
// ---------------------------------------------------------------

sf::Text MenuState::hacerTexto(const std::string& utf8, unsigned tamano, const sf::Color& color) const
{
    sf::Text t = crearTexto(fuente_, utf8, tamano * ESCALA_TEXTO);
    t.setScale(sf::Vector2f(1.f / ESCALA_TEXTO, 1.f / ESCALA_TEXTO));
    t.setFillColor(color);
    return t;
}

// Centra el texto (ya reducido a la mitad) dentro de la caja.
void MenuState::centrar(sf::Text& texto, const sf::FloatRect& caja) const
{
    const float k = 1.f / static_cast<float>(ESCALA_TEXTO);
    const sf::Vector2f lp = posicionRect(texto.getLocalBounds());
    const sf::Vector2f ls = tamanoRect(texto.getLocalBounds());
    const sf::Vector2f cp = posicionRect(caja);
    const sf::Vector2f cs = tamanoRect(caja);

    texto.setPosition(sf::Vector2f(
        cp.x + (cs.x - ls.x * k) / 2.f - lp.x * k,
        cp.y + (cs.y - ls.y * k) / 2.f - lp.y * k));
}

void MenuState::draw(sf::RenderWindow& window)
{
    ventana_ = &window;

    // Se dibuja con la vista actual del juego (640 x 360 logicos).
    const sf::View vista = window.getView();
    tam_    = vista.getSize();
    origen_ = sf::Vector2f(vista.getCenter().x - tam_.x / 2.f,
                           vista.getCenter().y - tam_.y / 2.f);

    sf::RectangleShape fondo(tam_);
    fondo.setPosition(origen_);
    fondo.setFillColor(sf::Color(18, 22, 44));
    window.draw(fondo);

    // Franja decorativa detras del titulo.
    sf::RectangleShape franja(sf::Vector2f(tam_.x, tam_.y * 0.30f));
    franja.setPosition(sf::Vector2f(origen_.x, origen_.y + tam_.y * 0.08f));
    franja.setFillColor(sf::Color(30, 38, 80));
    window.draw(franja);

    switch (pantalla_)
    {
    case Pantalla::Principal:
        dibujarPrincipal(window);
        break;
    case Pantalla::ComoJugar:
        dibujarPanel(window, "Cómo jugar", TEXTO_COMO_JUGAR);
        break;
    case Pantalla::Creditos:
        dibujarPanel(window, "Créditos", TEXTO_CREDITOS);
        break;
    }
}

void MenuState::dibujarPrincipal(sf::RenderWindow& window)
{
    // Titulo en dos renglones, con sombra.
    const float altoTitulo = tam_.y * 0.30f;
    const float yTitulo    = origen_.y + tam_.y * 0.08f;

    const sf::FloatRect caja1 = crearRect(origen_.x, yTitulo + 6.f,  tam_.x, altoTitulo / 2.f);
    const sf::FloatRect caja2 = crearRect(origen_.x, yTitulo + altoTitulo / 2.f - 2.f, tam_.x, altoTitulo / 2.f);

    for (int pasada = 0; pasada < 2; ++pasada)
    {
        const bool sombra = (pasada == 0);
        const sf::Color color = sombra ? sf::Color(0, 0, 0, 160) : sf::Color(255, 220, 80);
        const float desplazamiento = sombra ? 2.f : 0.f;

        sf::Text l1 = hacerTexto("Sobreviviendo", 30, color);
        sf::Text l2 = hacerTexto("al Semestre",   30, color);
        centrar(l1, caja1);
        centrar(l2, caja2);
        l1.move(sf::Vector2f(desplazamiento, desplazamiento));
        l2.move(sf::Vector2f(desplazamiento, desplazamiento));
        window.draw(l1);
        window.draw(l2);
    }

    // Botones.
    for (int i = 0; i < CantidadOpciones; ++i)
    {
        const sf::FloatRect caja = rectBoton(i);
        const bool activo = (i == seleccion_);

        sf::RectangleShape boton(tamanoRect(caja));
        boton.setPosition(posicionRect(caja));
        boton.setFillColor(activo ? sf::Color(70, 110, 200) : sf::Color(40, 44, 70));
        boton.setOutlineColor(activo ? sf::Color::White : sf::Color(90, 94, 130));
        boton.setOutlineThickness(2.f);
        window.draw(boton);

        sf::Text etiqueta = hacerTexto(ETIQUETAS[i], 14, activo ? sf::Color::White : sf::Color(200, 200, 215));
        centrar(etiqueta, caja);
        window.draw(etiqueta);
    }

    // Ayuda al pie.
    sf::Text ayuda = hacerTexto("W / S o flechas: mover     ENTER o clic: elegir", 10, sf::Color(150, 150, 170));
    centrar(ayuda, crearRect(origen_.x, origen_.y + tam_.y - 28.f, tam_.x, 20.f));
    window.draw(ayuda);
}

void MenuState::dibujarPanel(sf::RenderWindow& window, const std::string& titulo, const std::string& cuerpo)
{
    const float margen = 40.f;
    const sf::FloatRect caja = crearRect(origen_.x + margen, origen_.y + 30.f,
                                         tam_.x - 2.f * margen, tam_.y - 60.f);

    sf::RectangleShape panel(tamanoRect(caja));
    panel.setPosition(posicionRect(caja));
    panel.setFillColor(sf::Color(20, 24, 40, 245));
    panel.setOutlineColor(sf::Color::White);
    panel.setOutlineThickness(2.f);
    window.draw(panel);

    const sf::Vector2f cp = posicionRect(caja);
    const sf::Vector2f cs = tamanoRect(caja);

    sf::Text tit = hacerTexto(titulo, 20, sf::Color(255, 220, 80));
    centrar(tit, crearRect(cp.x, cp.y + 8.f, cs.x, 28.f));
    window.draw(tit);

    sf::Text texto = hacerTexto(cuerpo, 12, sf::Color::White);
    texto.setPosition(sf::Vector2f(cp.x + 24.f, cp.y + 48.f));
    window.draw(texto);

    sf::Text volver = hacerTexto("ESC, ENTER o clic para volver", 10, sf::Color(150, 150, 170));
    centrar(volver, crearRect(cp.x, cp.y + cs.y - 26.f, cs.x, 20.f));
    window.draw(volver);
}
