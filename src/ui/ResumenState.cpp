#include "ui/ResumenState.hpp"

#include <algorithm>
#include <cmath>
#include <utility>

#include "core/CompatSFML.hpp"
#include "dialogue/DialogoState.hpp"   // ajustarTexto

namespace
{
    // Los textos se crean al doble de tamano y se reducen a la mitad para
    // que salgan nitidos (igual que en MenuState).
    constexpr unsigned ESCALA_TEXTO = 2;

    constexpr float ANCHO_BOTON = 200.f;
    constexpr float ALTO_BOTON  = 30.f;
    constexpr float Y_BOTON     = 300.f;

    const sf::Color DORADO(255, 220, 80);
    const sf::Color VERDE(90, 220, 130);
    const sf::Color ROJO(235, 90, 90);
    const sf::Color GRIS(150, 150, 170);

    const char* nombreMateria(int i)
    {
        switch (static_cast<Materia>(i))
        {
            case Materia::Calculo:      return "Cálculo";
            case Materia::Fisica:       return "Física";
            case Materia::Programacion: return "Programación";
            default:                    return "?";
        }
    }

    std::string entero(float v)
    {
        return std::to_string(std::lround(v));
    }

    bool esTeclaSiguiente(sf::Keyboard::Key k)
    {
        return k == sf::Keyboard::Key::Enter ||
               k == sf::Keyboard::Key::E ||
               k == sf::Keyboard::Key::Space ||
               k == sf::Keyboard::Key::Right ||
               k == sf::Keyboard::Key::D;
    }

    bool esTeclaAnterior(sf::Keyboard::Key k)
    {
        return k == sf::Keyboard::Key::Left ||
               k == sf::Keyboard::Key::A ||
               k == sf::Keyboard::Key::Backspace;
    }
}

ResumenState::ResumenState(StateStack* pila,
                           const sf::Font& fuente,
                           const ResumenSemestre& resumen,
                           const sf::Texture* imagenFinal,
                           Accion alMenu)
    : State(pila),
      fuente_(fuente),
      resumen_(resumen),
      imagen_(imagenFinal),
      alMenu_(std::move(alMenu))
{
    // El nombre y el texto del final se parten en renglones que quepan en la
    // columna de la derecha (640 - 300 - 40 de margen).
    nombreAjustado_ = ajustar(resumen_.nombreFinal, 18, 300.f);
    textoAjustado_  = ajustar(resumen_.textoFinal, 12, 300.f);
}

// Parte cada renglon del texto (separado por \n) para que no pase de "ancho".
std::string ResumenState::ajustar(const std::string& utf8, unsigned tamano, float ancho) const
{
    sf::Text medidor = crearTexto(fuente_, "", tamano * ESCALA_TEXTO);
    const auto medir = [&medidor](const std::string& s)
    {
        medidor.setString(textoUtf8(s));
        return tamanoRect(medidor.getGlobalBounds()).x / static_cast<float>(ESCALA_TEXTO);
    };

    std::string resultado;
    std::string actual;
    for (const char c : utf8 + "\n")
    {
        if (c == '\n')
        {
            resultado += ajustarTexto(actual, medir, ancho) + "\n";
            actual.clear();
        }
        else
        {
            actual += c;
        }
    }

    if (!resultado.empty())
        resultado.pop_back();   // el \n de mas
    return resultado;
}

// ---------------------------------------------------------------
// Navegacion
// ---------------------------------------------------------------

sf::FloatRect ResumenState::rectBoton() const
{
    return crearRect(origen_.x + (tam_.x - ANCHO_BOTON) / 2.f, origen_.y + Y_BOTON,
                     ANCHO_BOTON, ALTO_BOTON);
}

void ResumenState::siguiente()
{
    if (pagina_ < Final)
    {
        ++pagina_;
        return;
    }

    // Despues del final se vuelve al menu. Solo se avisa: Juego cambia la pila
    // al final del cuadro.
    if (pedido_)
        return;

    pedido_ = true;
    if (alMenu_)
        alMenu_();
}

void ResumenState::anterior()
{
    if (pagina_ > Notas)
        --pagina_;
}

void ResumenState::handleEvent(const sf::Event& event)
{
    if (pedido_)
        return;

    if (const auto* tecla = event.getIf<sf::Event::KeyPressed>())
    {
        if (esTeclaSiguiente(tecla->code))
            siguiente();
        else if (esTeclaAnterior(tecla->code))
            anterior();
    }
    else if (const auto* clic = event.getIf<sf::Event::MouseButtonPressed>())
    {
        if (clic->button != sf::Mouse::Button::Left || ventana_ == nullptr)
            return;

        if (rectBoton().contains(ventana_->mapPixelToCoords(clic->position)))
            siguiente();
    }
}

void ResumenState::update(float)
{
}

// ---------------------------------------------------------------
// Dibujo
// ---------------------------------------------------------------

sf::Text ResumenState::hacerTexto(const std::string& utf8, unsigned tamano, const sf::Color& color) const
{
    sf::Text t = crearTexto(fuente_, utf8, tamano * ESCALA_TEXTO);
    t.setScale(sf::Vector2f(1.f / ESCALA_TEXTO, 1.f / ESCALA_TEXTO));
    t.setFillColor(color);
    return t;
}

// Centra el texto (ya reducido a la mitad) dentro de la caja.
void ResumenState::centrar(sf::Text& texto, const sf::FloatRect& caja) const
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

void ResumenState::draw(sf::RenderWindow& window)
{
    ventana_ = &window;

    // Se dibuja con la vista actual del juego (640 x 360 logicos). El fondo es
    // opaco: tapa la exploracion que hay debajo, incluido el HUD.
    const sf::View vista = window.getView();
    tam_    = vista.getSize();
    origen_ = sf::Vector2f(vista.getCenter().x - tam_.x / 2.f,
                           vista.getCenter().y - tam_.y / 2.f);

    sf::RectangleShape fondo(tam_);
    fondo.setPosition(origen_);
    fondo.setFillColor(sf::Color(18, 22, 44));
    window.draw(fondo);

    switch (pagina_)
    {
    case Notas:
        dibujarCabecera(window, "Notas finales");
        dibujarNotas(window);
        break;
    case Estadisticas:
        dibujarCabecera(window, "Así fue tu semestre");
        dibujarEstadisticas(window);
        break;
    default:
        dibujarCabecera(window, "Tu final");
        dibujarFinal(window);
        break;
    }

    // Boton unico: "Siguiente" y, en la ultima pantalla, "Volver al menú".
    const sf::FloatRect caja = rectBoton();
    sf::RectangleShape boton(tamanoRect(caja));
    boton.setPosition(posicionRect(caja));
    boton.setFillColor(sf::Color(70, 110, 200));
    boton.setOutlineColor(sf::Color::White);
    boton.setOutlineThickness(2.f);
    window.draw(boton);

    sf::Text etiqueta = hacerTexto(pagina_ == Final ? "Volver al menú" : "Siguiente", 14, sf::Color::White);
    centrar(etiqueta, caja);
    window.draw(etiqueta);

    const std::string ayuda = (pagina_ == Notas)
        ? "ENTER o clic: siguiente"
        : "ENTER o clic: " + std::string(pagina_ == Final ? "volver al menú" : "siguiente") +
          "     A / flecha izquierda: anterior";
    sf::Text pie = hacerTexto(ayuda, 10, GRIS);
    centrar(pie, crearRect(origen_.x, origen_.y + tam_.y - 24.f, tam_.x, 18.f));
    window.draw(pie);
}

// Franja con el titulo, el numero de pantalla y el subtitulo de esta pantalla.
void ResumenState::dibujarCabecera(sf::RenderWindow& window, const std::string& subtitulo)
{
    sf::RectangleShape franja(sf::Vector2f(tam_.x, 38.f));
    franja.setPosition(origen_);
    franja.setFillColor(sf::Color(30, 38, 80));
    window.draw(franja);

    sf::Text cabecera = hacerTexto("RESUMEN DEL SEMESTRE", 18, sf::Color::White);
    centrar(cabecera, crearRect(origen_.x, origen_.y, tam_.x, 38.f));
    window.draw(cabecera);

    sf::Text numero = hacerTexto(std::to_string(pagina_ + 1) + " / " + std::to_string(CantidadPaginas), 11, GRIS);
    numero.setPosition(sf::Vector2f(origen_.x + tam_.x - 56.f, origen_.y + 12.f));
    window.draw(numero);

    sf::Text sub = hacerTexto(subtitulo, 16, DORADO);
    centrar(sub, crearRect(origen_.x, origen_.y + 46.f, tam_.x, 24.f));
    window.draw(sub);
}

// Pantalla 1: las tres notas sobre 20 y el promedio.
void ResumenState::dibujarNotas(sf::RenderWindow& window)
{
    const float anchoFila = 420.f;
    const float x = origen_.x + (tam_.x - anchoFila) / 2.f;

    for (int i = 0; i < ResumenSemestre::N; ++i)
    {
        const float y = origen_.y + 82.f + static_cast<float>(i) * 44.f;

        std::string estado;
        sf::Color color = GRIS;
        std::string valor = std::to_string(resumen_.nota[i]) + " / " + std::to_string(Notas::NOTA_MAXIMA);

        if (resumen_.estado[i] == EstadoExamen::NoPresento)
        {
            estado = "NO PRESENTÓ";
        }
        else if (resumen_.estado[i] == EstadoExamen::Pendiente)
        {
            estado = "SIN EXAMEN";
            valor  = "-";
        }
        else if (resumen_.aprobada[i])
        {
            estado = "APROBADO";
            color  = VERDE;
        }
        else
        {
            estado = "REPROBADO";
            color  = ROJO;
        }

        sf::RectangleShape tarjeta(sf::Vector2f(anchoFila, 36.f));
        tarjeta.setPosition(sf::Vector2f(x, y));
        tarjeta.setFillColor(sf::Color(30, 36, 66));
        window.draw(tarjeta);

        sf::RectangleShape franja(sf::Vector2f(5.f, 36.f));
        franja.setPosition(sf::Vector2f(x, y));
        franja.setFillColor(color);
        window.draw(franja);

        sf::Text materia = hacerTexto(nombreMateria(i), 14, sf::Color::White);
        materia.setPosition(sf::Vector2f(x + 16.f, y + 9.f));
        window.draw(materia);

        sf::Text nota = hacerTexto(valor, 16, sf::Color::White);
        nota.setPosition(sf::Vector2f(x + 200.f, y + 7.f));
        window.draw(nota);

        sf::Text est = hacerTexto(estado, 11, color);
        est.setPosition(sf::Vector2f(x + 300.f, y + 12.f));
        window.draw(est);
    }

    sf::Text promedio = hacerTexto("Promedio:  " + std::to_string(resumen_.promedio) + " / " +
                                   std::to_string(Notas::NOTA_MAXIMA), 20, DORADO);
    centrar(promedio, crearRect(origen_.x, origen_.y + 224.f, tam_.x, 30.f));
    window.draw(promedio);

    sf::Text aprobadas = hacerTexto("Aprobaste " + std::to_string(resumen_.aprobadas) + " de " +
                                    std::to_string(ResumenSemestre::N) + " materias", 12, sf::Color(220, 220, 235));
    centrar(aprobadas, crearRect(origen_.x, origen_.y + 258.f, tam_.x, 20.f));
    window.draw(aprobadas);
}

// Una fila de la tabla de estadisticas: etiqueta a la izquierda y valor a la derecha.
void ResumenState::dibujarFila(sf::RenderWindow& window, float x, float y, float ancho,
                               const std::string& etiqueta, const std::string& valor)
{
    sf::RectangleShape fila(sf::Vector2f(ancho, 24.f));
    fila.setPosition(sf::Vector2f(x, y));
    fila.setFillColor(sf::Color(30, 36, 66));
    window.draw(fila);

    sf::Text et = hacerTexto(etiqueta, 12, sf::Color(220, 220, 235));
    et.setPosition(sf::Vector2f(x + 10.f, y + 5.f));
    window.draw(et);

    sf::Text val = hacerTexto(valor, 14, DORADO);
    const float ancho_val = tamanoRect(val.getGlobalBounds()).x;
    val.setPosition(sf::Vector2f(x + ancho - ancho_val - 10.f, y + 4.f));
    window.draw(val);
}

// Pantalla 2: estadisticas del semestre.
void ResumenState::dibujarEstadisticas(sf::RenderWindow& window)
{
    const float ancho = 250.f;
    const float xIzq  = origen_.x + 50.f;
    const float xDer  = origen_.x + tam_.x - 50.f - ancho;
    const float y0    = origen_.y + 86.f;
    const float paso  = 30.f;

    sf::Text encIzq = hacerTexto("Vida universitaria", 12, GRIS);
    encIzq.setPosition(sf::Vector2f(xIzq, origen_.y + 74.f - 1.f));
    sf::Text encDer = hacerTexto("Cómo terminaste", 12, GRIS);
    encDer.setPosition(sf::Vector2f(xDer, origen_.y + 74.f - 1.f));

    const std::pair<const char*, int> izquierda[] = {
        {"Clases asistidas", resumen_.asistencias},
        {"Faltas",           resumen_.faltas},
        {"Fiestas",          resumen_.fiestas},
        {"Platos lavados",   resumen_.platos},
        {"Noches sin dormir", resumen_.nochesSinDormir}
    };

    float y = y0 + 8.f;
    for (const auto& f : izquierda)
    {
        dibujarFila(window, xIzq, y, ancho, f.first, std::to_string(f.second));
        y += paso;
    }

    y = y0 + 8.f;
    dibujarFila(window, xDer, y,            ancho, "Salud mental", entero(resumen_.saludMental));
    dibujarFila(window, xDer, y + paso,     ancho, "Dopamina",     entero(resumen_.dopamina));
    dibujarFila(window, xDer, y + 2 * paso, ancho, "Dinero",       "$ " + entero(resumen_.dinero));

    window.draw(encIzq);
    window.draw(encDer);
}

// Pantalla 3: el final (imagen a la izquierda; nombre y texto a la derecha).
void ResumenState::dibujarFinal(sf::RenderWindow& window)
{
    const float caja = 220.f;
    const float x = origen_.x + 40.f;
    const float y = origen_.y + 76.f;

    sf::RectangleShape marco(sf::Vector2f(caja, caja));
    marco.setPosition(sf::Vector2f(x, y));
    marco.setFillColor(sf::Color(30, 36, 66));
    marco.setOutlineColor(sf::Color::White);
    marco.setOutlineThickness(2.f);
    window.draw(marco);

    if (imagen_ != nullptr)
    {
        // La imagen se ajusta a la caja sin deformarse.
        const sf::Vector2u t = imagen_->getSize();
        if (t.x > 0 && t.y > 0)
        {
            const float escala = std::min(caja / static_cast<float>(t.x),
                                          caja / static_cast<float>(t.y));
            sf::Sprite sprite(*imagen_);
            sprite.setScale(sf::Vector2f(escala, escala));
            sprite.setPosition(sf::Vector2f(
                x + (caja - static_cast<float>(t.x) * escala) / 2.f,
                y + (caja - static_cast<float>(t.y) * escala) / 2.f));
            window.draw(sprite);
        }
    }
    else
    {
        // Recuadro provisional mientras no haya imagen para este final.
        sf::Text aviso = hacerTexto("Imagen del final\n(pendiente)", 12, GRIS);
        centrar(aviso, crearRect(x, y, caja, caja));
        window.draw(aviso);
    }

    const float xTexto = origen_.x + 300.f;

    sf::Text nombre = hacerTexto(nombreAjustado_, 18, DORADO);
    nombre.setPosition(sf::Vector2f(xTexto, y));
    window.draw(nombre);

    // El texto empieza debajo del nombre (una linea de 18 mide unos 24 px).
    const int lineasNombre = 1 + static_cast<int>(std::count(nombreAjustado_.begin(), nombreAjustado_.end(), '\n'));
    sf::Text texto = hacerTexto(textoAjustado_, 12, sf::Color(230, 230, 240));
    texto.setPosition(sf::Vector2f(xTexto, y + 8.f + 24.f * static_cast<float>(lineasNombre)));
    window.draw(texto);
}
