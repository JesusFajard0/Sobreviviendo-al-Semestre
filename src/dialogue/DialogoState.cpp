#include "dialogue/DialogoState.hpp"

#include <utility>

#include "core/CompatSFML.hpp"
#include "core/StateStack.hpp"

namespace
{
    // Los textos se crean al doble de tamano y se reducen a la mitad para
    // que salgan nitidos (igual que en ObjetoActividad).
    constexpr unsigned ESCALA_TEXTO = 2;
    constexpr unsigned TAM_FRASE    = 14;
    constexpr unsigned TAM_PEQUENO  = 10;

    constexpr float MARGEN_CAJA  = 20.f;
    constexpr float ALTO_CAJA    = 100.f;
    constexpr float RELLENO      = 12.f;

    void prepararTexto(sf::Text& t, const sf::Color& color)
    {
        t.setScale(sf::Vector2f(1.f / ESCALA_TEXTO, 1.f / ESCALA_TEXTO));
        t.setFillColor(color);
    }

    bool esTeclaAvanzar(sf::Keyboard::Key k)
    {
        return k == sf::Keyboard::Key::E ||
               k == sf::Keyboard::Key::Enter ||
               k == sf::Keyboard::Key::Space;
    }
}

std::string ajustarTexto(const std::string& texto,
                         const std::function<float(const std::string&)>& medir,
                         float anchoMaximo)
{
    std::string resultado;
    std::string renglon;
    std::string palabra;

    auto cerrarPalabra = [&]()
    {
        if (palabra.empty())
            return;

        if (renglon.empty())
        {
            renglon = palabra;
        }
        else if (medir(renglon + " " + palabra) <= anchoMaximo)
        {
            renglon += " " + palabra;
        }
        else
        {
            resultado += renglon + "\n";
            renglon = palabra;
        }
        palabra.clear();
    };

    for (const char c : texto)
    {
        if (c == ' ')
            cerrarPalabra();
        else
            palabra += c;
    }
    cerrarPalabra();

    return resultado + renglon;
}

DialogoState::DialogoState(StateStack* pila,
                           const sf::Font& fuente,
                           std::vector<LineaDialogo> lineas,
                           AlTerminar alTerminar)
    : State(pila),
      lineas_(std::move(lineas)),
      alTerminar_(std::move(alTerminar)),
      textoHablante_(crearTexto(fuente, "", TAM_FRASE * ESCALA_TEXTO)),
      textoFrase_(crearTexto(fuente, "", TAM_FRASE * ESCALA_TEXTO)),
      textoAyuda_(crearTexto(fuente, "", TAM_PEQUENO * ESCALA_TEXTO))
{
    prepararTexto(textoHablante_, sf::Color(255, 220, 80));
    prepararTexto(textoFrase_,    sf::Color::White);
    prepararTexto(textoAyuda_,    sf::Color(170, 170, 170));

    // Se parte cada frase en renglones que quepan dentro de la caja.
    sf::Text medidor = crearTexto(fuente, "", TAM_FRASE * ESCALA_TEXTO);
    const auto medir = [&medidor](const std::string& s)
    {
        medidor.setString(textoUtf8(s));
        return tamanoRect(medidor.getGlobalBounds()).x / static_cast<float>(ESCALA_TEXTO);
    };

    constexpr float ANCHO_VISTA = 640.f;   // la vista logica del juego
    const float anchoUtil = ANCHO_VISTA - 2.f * MARGEN_CAJA - 2.f * RELLENO;

    for (const LineaDialogo& l : lineas_)
        ajustadas_.push_back(ajustarTexto(l.texto, medir, anchoUtil));
}

void DialogoState::cerrar()
{
    // popState destruye este objeto: se saca el callback antes.
    AlTerminar fin = std::move(alTerminar_);
    m_stack->popState();

    if (fin)
        fin();
}

void DialogoState::avanzar()
{
    ++actual_;
    if (actual_ >= lineas_.size())
        cerrar();   // ultima instruccion
}

void DialogoState::handleEvent(const sf::Event& event)
{
    if (const auto* tecla = event.getIf<sf::Event::KeyPressed>())
    {
        if (tecla->code == sf::Keyboard::Key::Escape)
        {
            cerrar();
            return;   // el objeto ya no existe
        }

        // Con la tecla mantenida SFML repite el evento: solo cuenta una
        // pulsacion por cada vez que se suelta.
        if (esTeclaAvanzar(tecla->code) && puedeAvanzar_)
        {
            puedeAvanzar_ = false;
            avanzar();
        }
    }
    else if (const auto* suelta = event.getIf<sf::Event::KeyReleased>())
    {
        if (esTeclaAvanzar(suelta->code))
            puedeAvanzar_ = true;
    }
}

void DialogoState::update(float)
{
    // Una conversacion vacia no tiene nada que mostrar.
    if (lineas_.empty())
        cerrar();
}

void DialogoState::draw(sf::RenderWindow& window)
{
    if (actual_ >= lineas_.size())
        return;

    // Se dibuja con la vista actual, pegado a su borde inferior.
    const sf::View vista = window.getView();
    const sf::Vector2f tam    = vista.getSize();
    const sf::Vector2f centro = vista.getCenter();
    const sf::Vector2f origen(centro.x - tam.x / 2.f, centro.y - tam.y / 2.f);

    const float x = origen.x + MARGEN_CAJA;
    const float y = origen.y + tam.y - ALTO_CAJA - MARGEN_CAJA;
    const float ancho = tam.x - 2.f * MARGEN_CAJA;

    sf::RectangleShape caja(sf::Vector2f(ancho, ALTO_CAJA));
    caja.setPosition(sf::Vector2f(x, y));
    caja.setFillColor(sf::Color(20, 20, 30, 240));
    caja.setOutlineColor(sf::Color::White);
    caja.setOutlineThickness(2.f);
    window.draw(caja);

    const LineaDialogo& linea = lineas_[actual_];

    textoHablante_.setString(textoUtf8(linea.hablante));
    textoHablante_.setPosition(sf::Vector2f(x + RELLENO, y + 6.f));
    window.draw(textoHablante_);

    textoFrase_.setString(textoUtf8(ajustadas_[actual_]));
    textoFrase_.setPosition(sf::Vector2f(x + RELLENO, y + 28.f));
    window.draw(textoFrase_);

    const bool ultima = (actual_ + 1 == lineas_.size());
    textoAyuda_.setString(textoUtf8(
        std::to_string(actual_ + 1) + "/" + std::to_string(lineas_.size()) +
        (ultima ? "   E: cerrar" : "   E: seguir")));
    textoAyuda_.setPosition(sf::Vector2f(x + ancho - 130.f, y + ALTO_CAJA - 18.f));
    window.draw(textoAyuda_);
}
