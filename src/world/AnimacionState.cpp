#include "world/AnimacionState.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <utility>

#include "core/CompatSFML.hpp"
#include "core/StateStack.hpp"

namespace
{
    // Vista logica del juego.
    constexpr float ANCHO = 640.f;
    constexpr float ALTO  = 360.f;

    constexpr float FUNDIDO = 0.35f;   // segundos de entrada y de salida
    constexpr float PI      = 3.14159265f;

    // Los textos se crean al doble de tamano y se reducen a la mitad
    // para que salgan nitidos (igual que en el resto del juego).
    constexpr unsigned ESCALA_TEXTO = 2;

    float limitar01(float v) { return std::clamp(v, 0.f, 1.f); }

    sf::Color mezclar(const sf::Color& a, const sf::Color& b, float p)
    {
        p = limitar01(p);
        auto m = [p](int x, int y)
        {
            return static_cast<std::uint8_t>(static_cast<float>(x) + static_cast<float>(y - x) * p);
        };
        return sf::Color(m(a.r, b.r), m(a.g, b.g), m(a.b, b.b), m(a.a, b.a));
    }

    const sf::Color TIZA(236, 236, 230);
    const sf::Color BLANCO(255, 255, 255);
}

AnimacionState::AnimacionState(StateStack* pila,
                               const sf::Font& fuente,
                               const sf::Texture* hojaPersonaje,
                               TipoAnimacion tipo,
                               const std::string& detalle,
                               std::vector<const sf::Texture*> imagenes,
                               const std::string& textoFinal,
                               sf::Color colorFinal,
                               AlTerminar alTerminar)
    : State(pila),
      fuente_(fuente),
      hoja_(hojaPersonaje),
      tipo_(tipo),
      detalle_(detalle),
      imagenes_(std::move(imagenes)),
      textoFinal_(textoFinal),
      colorFinal_(colorFinal),
      alTerminar_(std::move(alTerminar))
{
}

// ---------------------------------------------------------------
// Tiempo y entrada
// ---------------------------------------------------------------

float AnimacionState::duracion() const
{
    switch (tipo_)
    {
    case TipoAnimacion::Clase:     return 3.0f;
    case TipoAnimacion::Examen:    return 3.2f;
    case TipoAnimacion::Discoteca: return 3.6f;
    case TipoAnimacion::Dormir:    return 3.8f;
    case TipoAnimacion::Siesta:    return 2.4f;
    case TipoAnimacion::Duchar:    return 3.2f;
    }
    return 3.f;
}

void AnimacionState::saltar()
{
    // Salta directo al fundido de salida (pero no en el primer instante,
    // para que el ENTER que abrio la actividad no la cierre).
    const float limite = duracion() - FUNDIDO;
    if (t_ > 0.6f && t_ < limite)
        t_ = limite;
}

void AnimacionState::handleEvent(const sf::Event& event)
{
    if (const auto* tecla = event.getIf<sf::Event::KeyPressed>())
    {
        const sf::Keyboard::Key k = tecla->code;
        if (k == sf::Keyboard::Key::Enter || k == sf::Keyboard::Key::E ||
            k == sf::Keyboard::Key::Space || k == sf::Keyboard::Key::Escape)
        {
            saltar();
        }
    }
}

void AnimacionState::update(float dt)
{
    t_ += dt;

    if (t_ < duracion())
        return;

    // popState destruye este objeto: se saca el callback antes y despues
    // del pop no se vuelve a usar ningun miembro.
    AlTerminar fin = std::move(alTerminar_);
    m_stack->popState();

    if (fin)
        fin();
}

// ---------------------------------------------------------------
// Utilidades de dibujo
// ---------------------------------------------------------------

sf::Color AnimacionState::alfa(const sf::Color& c) const
{
    return sf::Color(c.r, c.g, c.b, static_cast<std::uint8_t>(static_cast<float>(c.a) * k_));
}

void AnimacionState::rect(sf::RenderWindow& w, float x, float y, float ancho, float alto,
                          const sf::Color& color) const
{
    if (ancho <= 0.f || alto <= 0.f)
        return;

    sf::RectangleShape r(sf::Vector2f(ancho, alto));
    r.setPosition(sf::Vector2f(std::round(x), std::round(y)));
    r.setFillColor(alfa(color));
    w.draw(r);
}

void AnimacionState::circulo(sf::RenderWindow& w, float cx, float cy, float radio,
                             const sf::Color& color) const
{
    sf::CircleShape c(radio);
    c.setOrigin(sf::Vector2f(radio, radio));
    c.setPosition(sf::Vector2f(cx, cy));
    c.setFillColor(alfa(color));
    w.draw(c);
}

void AnimacionState::texto(sf::RenderWindow& w, const std::string& s, unsigned tamano,
                           float centroX, float y, const sf::Color& color, bool sombra) const
{
    const float k = 1.f / static_cast<float>(ESCALA_TEXTO);

    for (int pasada = sombra ? 0 : 1; pasada < 2; ++pasada)
    {
        const bool esSombra = (pasada == 0);

        sf::Text t = crearTexto(fuente_, s, tamano * ESCALA_TEXTO);
        t.setScale(sf::Vector2f(k, k));
        t.setFillColor(esSombra ? alfa(sf::Color(0, 0, 0, 150)) : alfa(color));

        const sf::FloatRect b = t.getLocalBounds();
        const sf::Vector2f p0 = posicionRect(b);
        const sf::Vector2f tam = tamanoRect(b);
        const float d = esSombra ? 1.f : 0.f;

        t.setPosition(sf::Vector2f(
            std::round(centroX - (tam.x * k) / 2.f - p0.x * k + d),
            std::round(y - p0.y * k + d)));
        w.draw(t);
    }
}

// Barra de progreso al pie de la pantalla.
void AnimacionState::barra(sf::RenderWindow& w) const
{
    constexpr float ANCHO_BARRA = 200.f;
    const float x = (ANCHO - ANCHO_BARRA) / 2.f;
    const float avance = limitar01(t_ / duracion());

    rect(w, x - 1.f, 341.f, ANCHO_BARRA + 2.f, 8.f, sf::Color(0, 0, 0, 150));
    rect(w, x, 342.f, ANCHO_BARRA * avance, 6.f, sf::Color(120, 200, 120));
}

// Dibuja un cuadro de la hoja del personaje (4 columnas x 4 filas:
// filas = abajo, izquierda, derecha, arriba). Si no hay hoja, un rectangulo.
// (centroX, yPies) es el punto de apoyo: centro de los pies.
void AnimacionState::personaje(sf::RenderWindow& w, int fila, int cuadro, float centroX,
                               float yPies, float escala, bool espejo, float grados) const
{
    if (hoja_ == nullptr)
    {
        const float ancho = 24.f * escala;
        const float alto  = 36.f * escala;
        rect(w, centroX - ancho / 2.f, yPies - alto, ancho, alto, sf::Color(59, 130, 246));
        return;
    }

    const sf::Vector2u tam = hoja_->getSize();
    const int cw = static_cast<int>(tam.x) / 4;
    const int ch = static_cast<int>(tam.y) / 4;
    if (cw <= 0 || ch <= 0)
        return;

    sf::Sprite s(*hoja_);
    s.setTextureRect(sf::IntRect(sf::Vector2i((cuadro % 4) * cw, (fila % 4) * ch),
                                 sf::Vector2i(cw, ch)));

    const float esc = (32.f / static_cast<float>(cw)) * escala;   // 32 de ancho = tamano del juego
    s.setOrigin(sf::Vector2f(static_cast<float>(cw) / 2.f, static_cast<float>(ch)));
    s.setScale(sf::Vector2f(espejo ? -esc : esc, esc));
    s.setPosition(sf::Vector2f(std::round(centroX), std::round(yPies)));
    if (grados != 0.f)
        s.setRotation(sf::degrees(grados));
    s.setColor(sf::Color(255, 255, 255, static_cast<std::uint8_t>(255.f * k_)));
    w.draw(s);
}

// ---------------------------------------------------------------
// Dibujo
// ---------------------------------------------------------------

void AnimacionState::draw(sf::RenderWindow& window)
{
    const float d = duracion();
    k_ = limitar01(std::min(t_ / FUNDIDO, (d - t_) / FUNDIDO));

    if (!imagenes_.empty())
    {
        dibujarConImagenes(window);
        dibujarTextoFinal(window);
        return;
    }

    switch (tipo_)
    {
    case TipoAnimacion::Clase:     dibujarClase(window);          break;
    case TipoAnimacion::Examen:    dibujarExamen(window);         break;
    case TipoAnimacion::Discoteca: dibujarDiscoteca(window);      break;
    case TipoAnimacion::Dormir:    dibujarDormir(window, true);   break;
    case TipoAnimacion::Siesta:    dibujarDormir(window, false);  break;
    case TipoAnimacion::Duchar:    dibujarDuchar(window);         break;
    }

    dibujarTextoFinal(window);
}

// Texto grande que sale abajo, sobre la animacion, en la parte final
// (en el examen: la nota, de una, sin esperar la otra pantalla).
void AnimacionState::dibujarTextoFinal(sf::RenderWindow& w)
{
    if (textoFinal_.empty())
        return;

    const float p = limitar01((t_ - inicioFinal()) / 0.3f);
    if (p <= 0.f)
        return;

    rect(w, 0.f, 286.f, ANCHO, 54.f, sf::Color(0, 0, 0, static_cast<std::uint8_t>(190.f * p)));

    // Aparece con rebote: empieza chico y se pasa un poco del tamano final.
    const float rebote = 0.6f + 0.4f * p + 0.15f * std::sin(p * PI);
    texto(w, textoFinal_, static_cast<unsigned>(24.f * rebote), 320.f, 296.f, colorFinal_);
}

// ---------------------------------------------------------------
// Animaciones con imagenes
// ---------------------------------------------------------------

// Dibuja una imagen cubriendo toda la vista (640x360), centrada.
void AnimacionState::imagen(sf::RenderWindow& w, const sf::Texture& tex, float opacidad,
                            float zoom) const
{
    const sf::Vector2u t = tex.getSize();
    if (t.x == 0 || t.y == 0)
        return;

    const float esc = std::max(ANCHO / static_cast<float>(t.x),
                               ALTO  / static_cast<float>(t.y)) * zoom;

    sf::Sprite s(tex);
    s.setOrigin(sf::Vector2f(static_cast<float>(t.x) / 2.f, static_cast<float>(t.y) / 2.f));
    s.setScale(sf::Vector2f(esc, esc));
    s.setPosition(sf::Vector2f(ANCHO / 2.f, ALTO / 2.f));
    s.setColor(sf::Color(255, 255, 255, static_cast<std::uint8_t>(255.f * k_ * limitar01(opacidad))));
    w.draw(s);
}

// Texto de abajo sobre una franja oscura, para que se lea sobre cualquier dibujo.
void AnimacionState::rotulo(sf::RenderWindow& w, const std::string& s) const
{
    rect(w, 0.f, 306.f, ANCHO, 54.f, sf::Color(0, 0, 0, 130));
    texto(w, s, 14, 320.f, 314.f, BLANCO);
}

void AnimacionState::dibujarConImagenes(sf::RenderWindow& w)
{
    rect(w, 0.f, 0.f, ANCHO, ALTO, sf::Color(0, 0, 0));

    const int n = static_cast<int>(imagenes_.size());
    const float p = limitar01(t_ / duracion());

    // Mientras sale el texto final (la nota) no se tapa con el rotulo.
    const bool hayFinal = !textoFinal_.empty() && t_ >= inicioFinal();

    switch (tipo_)
    {
    case TipoAnimacion::Clase:
    {
        // Transicion suave entre poses, zoom progresivo con micro-respiracion
        // y un pulso sutil de luz en el aula para que se sienta viva.
        constexpr float POR_POSE = 1.5f;
        constexpr float TRAMO    = 0.35f;

        const float pos  = t_ / POR_POSE;
        const int   i    = static_cast<int>(pos) % n;
        const int   sig  = (i + 1) % n;
        const float frac = pos - std::floor(pos);
        const float fund = (n > 1)
            ? limitar01((frac - (1.f - TRAMO / POR_POSE)) / (TRAMO / POR_POSE))
            : 0.f;

        const float respiro = 0.008f * std::sin(t_ * 3.2f);
        const float zoom    = 1.f + 0.05f * p + respiro;

        imagen(w, *imagenes_[i], 1.f, zoom);
        if (fund > 0.f)
            imagen(w, *imagenes_[sig], fund, zoom);

        const float brilloAula = 0.5f + 0.5f * std::sin(t_ * 2.4f);
        rect(w, 0.f, 0.f, ANCHO, 185.f,
             sf::Color(255, 245, 200, static_cast<std::uint8_t>(14.f * brilloAula)));

        const int puntos = (static_cast<int>(t_ * 2.5f) % 3) + 1;
        const std::string suspensivos(static_cast<std::size_t>(puntos), '.');
        rotulo(w, "En clase de " + detalle_ + suspensivos);
        break;
    }

    case TipoAnimacion::Examen:
    {
        // Alterna entre escribir y pensar.
        const int i = static_cast<int>(t_ / 0.9f) % n;
        imagen(w, *imagenes_[i], 1.f);
        if (!hayFinal)
            rotulo(w, "Presentando el examen...");
        break;
    }

    case TipoAnimacion::Discoteca:
    {
        static const sf::Color PALETA[4] = {
            sf::Color(226, 60, 140), sf::Color(60, 200, 236),
            sf::Color(248, 214, 60), sf::Color(130, 70, 210)
        };

        // Cambia de pose al ritmo y la imagen "late" con cada golpe.
        const float ritmo = t_ * 3.6f;
        const int   golpe = static_cast<int>(ritmo);
        const float pulso = 1.f + 0.03f * std::fabs(std::sin(ritmo * PI));

        imagen(w, *imagenes_[golpe % n], 1.f, pulso);

        const sf::Color c = PALETA[golpe % 4];
        const float destello = 1.f - (ritmo - static_cast<float>(golpe));
        rect(w, 0.f, 0.f, ANCHO, ALTO,
             sf::Color(c.r, c.g, c.b, static_cast<std::uint8_t>(45.f * destello)));

        rotulo(w, "¡Noche de rumba!");
        break;
    }

    case TipoAnimacion::Dormir:
    case TipoAnimacion::Siesta:
    {
        const bool noche = (tipo_ == TipoAnimacion::Dormir);

        // De noche: la primera imagen se funde con la segunda (amanecer).
        const float amanecer = (noche && n >= 2) ? limitar01((p - 0.55f) / 0.2f) : 0.f;

        imagen(w, *imagenes_[0], 1.f);
        if (amanecer > 0.f)
            imagen(w, *imagenes_[1], amanecer);

        // Zzz que suben desde la cabeza mientras duerme.
        if (amanecer < 0.6f)
        {
            for (int i = 0; i < 3; ++i)
            {
                const float fase = std::fmod(t_ * 0.7f + static_cast<float>(i) * 0.33f, 1.f);
                const unsigned tam = 12u + static_cast<unsigned>(fase * 12.f);
                const float x = (noche ? 128.f : 300.f) + fase * 36.f + static_cast<float>(i) * 8.f;
                const float y = (noche ? 200.f : 140.f) - fase * 80.f;
                const float visible = (1.f - fase) * (1.f - amanecer / 0.6f);
                texto(w, "Z", tam, x, y,
                      sf::Color(255, 255, 255, static_cast<std::uint8_t>(255.f * visible)), true);
            }
        }

        std::string r = "Echando una siesta...";
        if (noche)
            r = (amanecer > 0.4f) ? "Amanece..." : "Durmiendo...";
        rotulo(w, r);
        break;
    }

    case TipoAnimacion::Duchar:
    {
        constexpr float POR_POSE = 1.6f;
        constexpr float TRAMO    = 0.35f;

        const float pos  = t_ / POR_POSE;
        const int   i    = static_cast<int>(pos) % n;
        const int   sig  = (i + 1) % n;
        const float frac = pos - std::floor(pos);
        const float fund = (n > 1)
            ? limitar01((frac - (1.f - TRAMO / POR_POSE)) / (TRAMO / POR_POSE))
            : 0.f;

        const float respiro = 0.006f * std::sin(t_ * 2.8f);
        const float zoom    = 1.f + 0.04f * p + respiro;

        imagen(w, *imagenes_[i], 1.f, zoom);
        if (fund > 0.f)
            imagen(w, *imagenes_[sig], fund, zoom);

        const int puntos = (static_cast<int>(t_ * 2.5f) % 3) + 1;
        const std::string suspensivos(static_cast<std::size_t>(puntos), '.');
        rotulo(w, "Duchándose" + suspensivos);
        break;
    }
    }

    barra(w);
}

// ---------------------------------------------------------------
// Clase: pizarra que se va llenando y el estudiante tomando apuntes
// ---------------------------------------------------------------

void AnimacionState::dibujarClase(sf::RenderWindow& w)
{
    rect(w, 0.f, 0.f, ANCHO, ALTO, sf::Color(190, 205, 225));      // pared
    rect(w, 0.f, 248.f, ANCHO, 112.f, sf::Color(150, 118, 88));    // piso
    rect(w, 0.f, 244.f, ANCHO, 4.f, sf::Color(110, 84, 62));       // zocalo

    // Pizarra.
    rect(w, 150.f, 36.f, 340.f, 140.f, sf::Color(92, 62, 36));
    rect(w, 158.f, 44.f, 324.f, 124.f, sf::Color(38, 88, 70));
    rect(w, 150.f, 176.f, 340.f, 6.f, sf::Color(120, 90, 60));
    texto(w, detalle_, 12, 320.f, 50.f, TIZA, false);

    // Renglones que aparecen de a uno + tiza activa en el renglon actual.
    for (int i = 0; i < 5; ++i)
    {
        const float inicioRenglon = 0.5f + static_cast<float>(i) * 0.4f;
        const float progreso = limitar01((t_ - inicioRenglon) / 0.5f);
        const float maximo   = (i % 2 == 0) ? 260.f : 200.f;
        const float yRenglon = 80.f + static_cast<float>(i) * 17.f;

        rect(w, 176.f, yRenglon, maximo * progreso, 3.f, TIZA);

        if (progreso > 0.f && progreso < 1.f)
        {
            const float saltoTiza = std::sin(t_ * 28.f) * 1.5f;
            rect(w, 176.f + maximo * progreso, yRenglon - 1.f + saltoTiza, 4.f, 3.f, BLANCO);
        }
    }

    // Estudiante de espaldas, mirando la pizarra, con un pequeno vaiven.
    const float vaiven = std::round(std::sin(t_ * 4.f));
    personaje(w, 3, 0, 320.f, 284.f + vaiven, 2.f);

    // Pupitre por delante: tapa de la cintura para abajo.
    // (baja: solo tapa los zapatos, el cuerpo se ve completo)
    rect(w, 200.f, 276.f, 240.f, 10.f, sf::Color(135, 98, 62));
    rect(w, 200.f, 286.f, 240.f, 3.f, sf::Color(95, 68, 42));
    rect(w, 212.f, 289.f, 8.f, 26.f, sf::Color(95, 68, 42));
    rect(w, 420.f, 289.f, 8.f, 26.f, sf::Color(95, 68, 42));

    const int puntos = (static_cast<int>(t_ * 2.5f) % 3) + 1;
    const std::string suspensivos(static_cast<std::size_t>(puntos), '.');
    texto(w, "En clase de " + detalle_ + suspensivos, 14, 320.f, 316.f, BLANCO);
    barra(w);
}

// ---------------------------------------------------------------
// Examen: la hoja se llena de respuestas, el reloj corre
// ---------------------------------------------------------------

void AnimacionState::dibujarExamen(sf::RenderWindow& w)
{
    const float dur = duracion();

    rect(w, 0.f, 0.f, ANCHO, ALTO, sf::Color(125, 94, 66));        // escritorio

    // Hoja con su sombra.
    rect(w, 214.f, 32.f, 220.f, 264.f, sf::Color(0, 0, 0, 70));
    rect(w, 210.f, 28.f, 220.f, 264.f, sf::Color(246, 246, 238));
    texto(w, "EXAMEN", 14, 320.f, 36.f, sf::Color(40, 40, 60), false);
    texto(w, detalle_, 10, 320.f, 56.f, sf::Color(100, 100, 120), false);

    // Respuestas que se van escribiendo, renglon por renglon.
    constexpr int   RENGLONES = 10;
    constexpr float INICIO    = 0.4f;
    constexpr float POR_RENGLON = 0.22f;

    for (int i = 0; i < RENGLONES; ++i)
    {
        const float progreso = limitar01((t_ - INICIO - static_cast<float>(i) * POR_RENGLON) / POR_RENGLON);
        const float maximo = (i == RENGLONES - 1) ? 110.f : 176.f;
        rect(w, 230.f, 84.f + static_cast<float>(i) * 16.f, maximo * progreso, 4.f, sf::Color(70, 70, 100));
    }

    // Lapiz siguiendo el renglon actual.
    const float escrito = (t_ - INICIO) / POR_RENGLON;
    if (escrito >= 0.f && escrito < static_cast<float>(RENGLONES))
    {
        const int   i        = static_cast<int>(escrito);
        const float progreso = escrito - static_cast<float>(i);
        const float maximo   = (i == RENGLONES - 1) ? 110.f : 176.f;
        const sf::Vector2f punta(230.f + maximo * progreso, 86.f + static_cast<float>(i) * 16.f);

        sf::RectangleShape cuerpo(sf::Vector2f(34.f, 6.f));
        cuerpo.setOrigin(sf::Vector2f(0.f, 3.f));
        cuerpo.setPosition(punta);
        cuerpo.setRotation(sf::degrees(-35.f));
        cuerpo.setFillColor(alfa(sf::Color(240, 200, 60)));
        w.draw(cuerpo);

        sf::RectangleShape mina(sf::Vector2f(6.f, 6.f));
        mina.setOrigin(sf::Vector2f(0.f, 3.f));
        mina.setPosition(punta);
        mina.setRotation(sf::degrees(-35.f));
        mina.setFillColor(alfa(sf::Color(60, 40, 30)));
        w.draw(mina);
    }

    // Nota pendiente, parpadeando.
    const bool parpadeo = (static_cast<int>(t_ * 3.f) % 2 == 0);
    texto(w, std::string("Nota: ") + (parpadeo ? "?" : " "), 12, 320.f, 256.f,
          sf::Color(170, 40, 40), false);

    // Reloj arriba a la derecha: las manecillas avanzan mientras dura el examen.
    circulo(w, 564.f, 64.f, 27.f, sf::Color(40, 40, 60));
    circulo(w, 564.f, 64.f, 24.f, sf::Color(245, 245, 240));

    const float vuelta = t_ / dur;
    const float angulos[2] = { vuelta * 540.f - 90.f, vuelta * 45.f - 90.f };
    const float largos[2]  = { 18.f, 12.f };
    for (int i = 0; i < 2; ++i)
    {
        sf::RectangleShape mano(sf::Vector2f(largos[i], 2.f));
        mano.setOrigin(sf::Vector2f(0.f, 1.f));
        mano.setPosition(sf::Vector2f(564.f, 64.f));
        mano.setRotation(sf::degrees(angulos[i]));
        mano.setFillColor(alfa(sf::Color(40, 40, 60)));
        w.draw(mano);
    }

    texto(w, "Presentando el examen...", 14, 320.f, 312.f, BLANCO);
    barra(w);
}

// ---------------------------------------------------------------
// Discoteca: pista de colores, luces y el personaje bailando
// ---------------------------------------------------------------

void AnimacionState::dibujarDiscoteca(sf::RenderWindow& w)
{
    static const sf::Color PALETA[4] = {
        sf::Color(226, 60, 140), sf::Color(60, 200, 236),
        sf::Color(248, 214, 60), sf::Color(130, 70, 210)
    };

    rect(w, 0.f, 0.f, ANCHO, ALTO, sf::Color(18, 8, 38));

    // Pista: las baldosas cambian de color al ritmo.
    const int paso = static_cast<int>(t_ * 4.f);
    for (int j = 0; j < 4; ++j)
    {
        for (int i = 0; i < 20; ++i)
        {
            const sf::Color c = PALETA[(i + j + paso) % 4];
            rect(w, static_cast<float>(i) * 32.f, 236.f + static_cast<float>(j) * 31.f,
                 31.f, 30.f, sf::Color(c.r, c.g, c.b, 170));
        }
    }

    // Haces de luz que se mueven.
    for (int i = 0; i < 3; ++i)
    {
        const float x0 = 160.f + static_cast<float>(i) * 160.f;
        const float objetivo = x0 + std::sin(t_ * 2.2f + static_cast<float>(i) * 2.1f) * 110.f;
        const sf::Color c = PALETA[(i + paso / 2) % 4];

        sf::ConvexShape haz;
        haz.setPointCount(3);
        haz.setPoint(0, sf::Vector2f(x0, 0.f));
        haz.setPoint(1, sf::Vector2f(objetivo - 34.f, 240.f));
        haz.setPoint(2, sf::Vector2f(objetivo + 34.f, 240.f));
        haz.setFillColor(alfa(sf::Color(c.r, c.g, c.b, 60)));
        w.draw(haz);
    }

    // Bola disco con destellos.
    rect(w, 319.f, 0.f, 2.f, 20.f, sf::Color(150, 150, 170));
    circulo(w, 320.f, 36.f, 16.f, sf::Color(205, 205, 222));
    for (int i = 0; i < 6; ++i)
    {
        const float a = t_ * 2.f + static_cast<float>(i) * 1.05f;
        rect(w, 320.f + std::cos(a) * 9.f - 1.f, 36.f + std::sin(a) * 9.f - 1.f, 3.f, 3.f, BLANCO);
    }
    for (int i = 0; i < 8; ++i)
    {
        const float a = t_ * 0.9f + static_cast<float>(i) * 0.785f;
        const float brillo = 0.5f + 0.5f * std::sin(t_ * 7.f + static_cast<float>(i) * 1.9f);
        rect(w, 320.f + std::cos(a) * 62.f, 36.f + std::sin(a) * 40.f, 3.f, 3.f,
             sf::Color(255, 255, 255, static_cast<std::uint8_t>(255.f * brillo)));
    }

    // El personaje baila de frente: pasitos, balanceo de lado a lado y saltitos.
    // Nunca cambia de direccion, asi no parece que gira.
    const float ritmo    = t_ * 7.f;
    const int   cuadro   = (static_cast<int>(t_ * 4.f) % 2 == 0) ? 1 : 3;   // paso izq / paso der
    const float salto    = std::fabs(std::sin(ritmo)) * 7.f;
    const float balanceo = std::sin(ritmo * 0.5f) * 10.f;
    personaje(w, 0, cuadro, 320.f + balanceo, 300.f - salto, 2.5f);

    texto(w, "¡Noche de rumba!", 16, 320.f, 318.f, sf::Color(255, 230, 90));
}

// ---------------------------------------------------------------
// Dormir (de noche, con amanecer) y siesta (de dia, corta)
// ---------------------------------------------------------------

void AnimacionState::dibujarDormir(sf::RenderWindow& w, bool noche)
{
    const float p        = limitar01(t_ / duracion());
    const float amanecer = noche ? limitar01((p - 0.65f) / 0.35f) : 0.f;

    // Fondo y piso.
    const sf::Color fondo = noche ? mezclar(sf::Color(12, 16, 44), sf::Color(240, 172, 112), amanecer)
                                  : sf::Color(150, 200, 240);
    const sf::Color piso  = noche ? mezclar(sf::Color(40, 32, 34), sf::Color(150, 110, 84), amanecer)
                                  : sf::Color(160, 124, 92);
    rect(w, 0.f, 0.f, ANCHO, ALTO, fondo);
    rect(w, 0.f, 250.f, ANCHO, 110.f, piso);

    // Ventana.
    const sf::Color cielo = noche ? mezclar(sf::Color(8, 10, 40), sf::Color(250, 190, 130), amanecer)
                                  : sf::Color(130, 190, 250);
    rect(w, 438.f, 40.f, 124.f, 134.f, sf::Color(92, 62, 36));
    rect(w, 446.f, 48.f, 108.f, 118.f, cielo);

    if (noche)
    {
        static const float EX[6] = { 456.f, 470.f, 524.f, 536.f, 540.f, 486.f };
        static const float EY[6] = {  60.f, 100.f,  72.f, 118.f,  58.f, 134.f };
        for (int i = 0; i < 6; ++i)
        {
            const float brillo = (0.5f + 0.5f * std::sin(t_ * 3.f + static_cast<float>(i) * 1.7f)) * (1.f - amanecer);
            rect(w, EX[i], EY[i], 2.f, 2.f, sf::Color(255, 255, 220, static_cast<std::uint8_t>(255.f * brillo)));
        }

        if (p < 0.7f)
            circulo(w, 472.f, 80.f + p * 50.f, 13.f, sf::Color(240, 240, 205));      // luna baja
        else
            circulo(w, 472.f, 152.f - amanecer * 46.f, 12.f, sf::Color(255, 214, 100)); // sale el sol
    }
    else
    {
        circulo(w, 520.f, 84.f, 13.f, sf::Color(255, 224, 110));
        rect(w, 454.f, 112.f, 40.f, 10.f, sf::Color(255, 255, 255, 230));             // nubes
        rect(w, 462.f, 104.f, 24.f, 10.f, sf::Color(255, 255, 255, 230));
    }

    // Marcos de la ventana por delante.
    rect(w, 498.f, 48.f, 4.f, 118.f, sf::Color(92, 62, 36));
    rect(w, 446.f, 104.f, 108.f, 4.f, sf::Color(92, 62, 36));

    // Cama.
    rect(w, 126.f, 176.f, 12.f, 80.f, sf::Color(100, 68, 40));                // cabecera
    rect(w, 404.f, 196.f, 10.f, 60.f, sf::Color(100, 68, 40));                // pies
    rect(w, 130.f, 214.f, 280.f, 40.f, sf::Color(110, 76, 46));               // base
    rect(w, 138.f, 200.f, 264.f, 20.f, sf::Color(225, 228, 238));             // colchon
    rect(w, 146.f, 190.f, 46.f, 14.f, sf::Color(246, 246, 250));              // almohada

    // El estudiante (el mismo sprite del juego) acostado boca arriba: la cabeza
    // queda sobre la almohada. Sin la hoja, se dibuja una cabeza simple.
    if (hoja_ != nullptr)
    {
        personaje(w, 0, 0, 246.f, 205.f, 2.f, false, -90.f);
    }
    else
    {
        rect(w, 160.f, 176.f, 22.f, 18.f, sf::Color(244, 205, 165));
        rect(w, 160.f, 173.f, 22.f, 8.f, sf::Color(48, 34, 30));
        rect(w, 164.f, 186.f, 4.f, 2.f, sf::Color(48, 34, 30));
        rect(w, 172.f, 186.f, 4.f, 2.f, sf::Color(48, 34, 30));
    }

    // Cobija que sube y baja al respirar.
    const float respiro = std::sin(t_ * 3.f) * 1.5f;
    rect(w, 186.f, 194.f - respiro, 216.f, 28.f + respiro, sf::Color(62, 100, 220));
    rect(w, 186.f, 194.f - respiro, 216.f, 3.f, sf::Color(100, 140, 245));
    rect(w, 186.f, 214.f, 216.f, 2.f, sf::Color(38, 66, 168));

    // Zzz que suben y se desvanecen.
    const sf::Color colorZ = noche ? sf::Color(255, 255, 255) : sf::Color(70, 70, 110);
    for (int i = 0; i < 3; ++i)
    {
        const float fase = std::fmod(t_ * 0.8f + static_cast<float>(i) * 0.33f, 1.f);
        const unsigned tam = 12u + static_cast<unsigned>(fase * 10.f);
        texto(w, "Z", tam, 206.f + fase * 40.f + static_cast<float>(i) * 10.f, 176.f - fase * 70.f,
              sf::Color(colorZ.r, colorZ.g, colorZ.b, static_cast<std::uint8_t>(255.f * (1.f - fase))),
              false);
    }

    // Rotulo.
    std::string rotulo = "Echando una siesta...";
    if (noche)
        rotulo = (amanecer > 0.4f) ? "Amanece..." : "Durmiendo...";
    texto(w, rotulo, 14, 320.f, 316.f, BLANCO);
}

// ---------------------------------------------------------------
// Duchar: respaldo sencillo si no hay imagenes en assets/animaciones/
// ---------------------------------------------------------------

void AnimacionState::dibujarDuchar(sf::RenderWindow& w)
{
    rect(w, 0.f, 0.f, ANCHO, ALTO, sf::Color(225, 220, 215));
    rect(w, 0.f, 240.f, ANCHO, 120.f, sf::Color(90, 125, 165));
    rect(w, 170.f, 160.f, 300.f, 95.f, sf::Color(238, 242, 248));
    rect(w, 176.f, 152.f, 288.f, 24.f, sf::Color(255, 255, 255, 220));

    if (hoja_ != nullptr)
        personaje(w, 0, 0, 320.f, 225.f, 2.f);

    const int puntos = (static_cast<int>(t_ * 2.5f) % 3) + 1;
    const std::string suspensivos(static_cast<std::size_t>(puntos), '.');
    texto(w, "Duchándose" + suspensivos, 14, 320.f, 316.f, BLANCO);
    barra(w);
}