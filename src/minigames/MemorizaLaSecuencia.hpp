
#pragma once

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <memory>
#include <random>
#include <string>
#include <utility>
#include <vector>

#include "core/CompatSFML.hpp"
#include "minigames/Minijuego.hpp"

// S2-8: minijuego generico de estudio (tipo Simon).
// Arte y audio independientes; resolucion logica 640 x 360.
class MemorizaLaSecuencia : public Minijuego
{
public:
    MemorizaLaSecuencia(StateStack* pila,
                       const sf::Font& fuente,
                       AlTerminar alTerminar)
        : Minijuego(pila, "estudio_generico_2",
                    std::move(alTerminar)),
          fuente_(fuente),
          azar_(std::random_device{}())
    {
        if (!fondo_.loadFromFile("assets/minijuegos/memoria_fondo.png"))
            std::cerr << "[Memoria] No se cargo el fondo.\n";
        else
            fondoDisponible_ = true;

        const std::string ruta = "assets/audio/minijuegos/memoria/";

        const std::array<std::string, 8> archivos = {
            "arriba.wav",
            "derecha.wav",
            "abajo.wav",
            "izquierda.wav",
            "error_memoria.wav",
            "ronda_memoria.wav",
            "victoria_memoria.wav",
            "derrota_memoria.wav"
        };

        for (std::size_t i = 0; i < archivos.size(); ++i)
        {
            if (buffers_[i].loadFromFile(ruta + archivos[i]))
            {
#if SFML_VERSION_MAJOR >= 3
                sonidos_[i] = std::make_unique<sf::Sound>(buffers_[i]);
#else
                sonidos_[i] = std::make_unique<sf::Sound>();
                sonidos_[i]->setBuffer(buffers_[i]);
#endif
                sonidos_[i]->setVolume(i < 4 ? 62.f : 72.f);
            }
        }

        musicaDisponible_ = musica_.openFromFile(
            ruta + "memoria_ambiente.ogg"
        );

        if (musicaDisponible_)
        {
#if SFML_VERSION_MAJOR >= 3
            musica_.setLooping(true);
#else
            musica_.setLoop(true);
#endif
            musica_.setVolume(35.f);
        }
    }

    void handleEvent(const sf::Event& evento) override
    {
#if SFML_VERSION_MAJOR >= 3
        if (const auto* tecla =
                evento.getIf<sf::Event::KeyPressed>())
        {
            if (tecla->code == sf::Keyboard::Key::Escape)
            {
                salir();
                return;
            }

            if (tecla->code == sf::Keyboard::Key::Enter)
            {
                confirmar();
                return;
            }

            int direccion = -1;

            switch (tecla->code)
            {
            case sf::Keyboard::Key::Up:
            case sf::Keyboard::Key::W:
                direccion = 0;
                break;

            case sf::Keyboard::Key::Right:
            case sf::Keyboard::Key::D:
                direccion = 1;
                break;

            case sf::Keyboard::Key::Down:
            case sf::Keyboard::Key::S:
                direccion = 2;
                break;

            case sf::Keyboard::Key::Left:
            case sf::Keyboard::Key::A:
                direccion = 3;
                break;

            default:
                break;
            }

            if (direccion >= 0)
                responder(direccion);
        }

        if (const auto* raton =
                evento.getIf<sf::Event::MouseButtonPressed>())
        {
            if (raton->button == sf::Mouse::Button::Left)
                clic(raton->position);
        }
#else
        if (evento.type == sf::Event::KeyPressed)
        {
            if (evento.key.code == sf::Keyboard::Escape)
            {
                salir();
                return;
            }

            if (evento.key.code == sf::Keyboard::Enter)
            {
                confirmar();
                return;
            }

            int direccion = -1;

            switch (evento.key.code)
            {
            case sf::Keyboard::Up:
            case sf::Keyboard::W:
                direccion = 0;
                break;

            case sf::Keyboard::Right:
            case sf::Keyboard::D:
                direccion = 1;
                break;

            case sf::Keyboard::Down:
            case sf::Keyboard::S:
                direccion = 2;
                break;

            case sf::Keyboard::Left:
            case sf::Keyboard::A:
                direccion = 3;
                break;

            default:
                break;
            }

            if (direccion >= 0)
                responder(direccion);
        }

        if (evento.type == sf::Event::MouseButtonPressed &&
            evento.mouseButton.button == sf::Mouse::Left)
        {
            clic({
                evento.mouseButton.x,
                evento.mouseButton.y
            });
        }
#endif
    }

    void update(float dt) override
    {
        dt = std::clamp(dt, 0.f, 0.10f);
        pulso_ += dt;

        for (float& luz : luces_)
            luz = std::max(0.f, luz - dt * 2.4f);

        errorVisual_ = std::max(0.f, errorVisual_ - dt * 2.f);

        if (fase_ == Fase::Mostrar)
        {
            reloj_ += dt;

            const float transcurrido = reloj_ - 0.35f;

            const int indice = static_cast<int>(
                std::floor(transcurrido / 0.66f)
            );

            if (transcurrido >= 0.f &&
                indice >= 0 &&
                indice < static_cast<int>(secuencia_.size()) &&
                indice != ultimoMostrado_)
            {
                ultimoMostrado_ = indice;
                iluminar(secuencia_[indice]);
            }

            const float fin =
                0.35f +
                static_cast<float>(secuencia_.size()) * 0.66f +
                0.15f;

            if (reloj_ >= fin)
            {
                fase_ = Fase::Responder;
                indiceRespuesta_ = 0;

                tiempoRespuesta_ =
                    5.f +
                    static_cast<float>(secuencia_.size()) * 1.4f;
            }
        }
        else if (fase_ == Fase::Responder)
        {
            tiempoRespuesta_ -= dt;

            if (tiempoRespuesta_ <= 0.f)
                equivocacion();
        }
        else if (fase_ == Fase::Espera)
        {
            reloj_ -= dt;

            if (reloj_ <= 0.f)
            {
                if (!repetir_)
                    secuencia_.push_back(nuevaDireccion());

                comenzarDemostracion();
            }
        }
    }

    void draw(sf::RenderWindow& ventana) override
    {
        ventana_ = &ventana;
        vistaJuego_ = ventana.getView();

        // Fondo ilustrado.
        if (fondoDisponible_)
        {
#if SFML_VERSION_MAJOR >= 3
            sf::Sprite fondo(fondo_);
#else
            sf::Sprite fondo;
            fondo.setTexture(fondo_);
#endif
            ventana.draw(fondo);
        }
        else
        {
            caja(
                ventana, 0.f, 0.f, 640.f, 360.f,
                sf::Color(24, 19, 52)
            );
        }

        // Placa de contraste.
        caja(
            ventana, 151.f, 77.f, 338.f, 207.f,
            sf::Color(13, 18, 51, 182)
        );

        caja(
            ventana, 157.f, 81.f, 326.f, 2.f,
            sf::Color(224, 172, 85, 180)
        );

        texto(
            ventana,
            "MEMORIZA LA SECUENCIA",
            20, 320.f, 88.f,
            sf::Color(255, 225, 169),
            true
        );

        texto(
            ventana,
            "RONDA " +
            std::to_string(
                std::max(
                    1,
                    static_cast<int>(secuencia_.size()) - 1
                )
            ) + "/7",
            13, 190.f, 112.f,
            sf::Color(183, 230, 245),
            true
        );

        texto(
            ventana,
            "VIDAS " + std::to_string(vidas_),
            13, 452.f, 112.f,
            sf::Color(255, 174, 186),
            true
        );

        std::string mensaje;

        if (fase_ == Fase::Mostrar)
            mensaje = "OBSERVA LAS LUCES";
        else if (fase_ == Fase::Responder)
            mensaje = "REPITE EL ORDEN";
        else if (fase_ == Fase::Espera)
            mensaje = repetir_
                ? "INTENTALO DE NUEVO"
                : "EXCELENTE!";
        else
            mensaje = "MEMORIA Y CONCENTRACION";

        texto(
            ventana, mensaje, 12,
            320.f, 120.f,
            sf::Color(224, 229, 247),
            true
        );

        // Dibujar los cuatro botones luminosos.
        for (int i = 0; i < 4; ++i)
            dibujarBoton(ventana, i);

        if (fase_ == Fase::Responder)
        {
            texto(
                ventana,
                "PASO " +
                std::to_string(indiceRespuesta_ + 1) +
                "/" +
                std::to_string(secuencia_.size()),
                12, 320.f, 285.f,
                sf::Color(192, 240, 208),
                true
            );

            // Barra de tiempo para responder.
            caja(
                ventana, 230.f, 303.f, 180.f, 5.f,
                sf::Color(58, 46, 81)
            );

            caja(
                ventana,
                230.f, 303.f,
                180.f * std::clamp(
                    tiempoRespuesta_ /
                    (5.f + secuencia_.size() * 1.4f),
                    0.f, 1.f
                ),
                5.f,
                sf::Color(110, 224, 197)
            );
        }
        else
        {
            texto(
                ventana,
                "WASD / FLECHAS / CLIC",
                12, 320.f, 286.f,
                sf::Color(190, 199, 231),
                true
            );
        }

        if (errorVisual_ > 0.f)
        {
            caja(
                ventana,
                157.f, 81.f, 326.f, 3.f,
                sf::Color(247, 91, 113)
            );
        }

        // Pantalla de instrucciones.
        if (fase_ == Fase::Instrucciones)
        {
            panel(
                ventana,
                "MEMORIZA LA SECUENCIA",
                "Mira las luces y repite el orden.\n"
                "Cada ronda agrega una flecha.\n"
                "Tienes dos vidas; aprobar = 70 puntos.",
                "ENTER O CLIC PARA EMPEZAR"
            );
        }
        // Pantalla de resultados.
        else if (fase_ == Fase::Resultados)
        {
            const bool aprobado =
                puntuacion_ >= NOTA_MINIMA;

            panel(
                ventana,
                aprobado
                    ? "ESTUDIO EFICIENTE!"
                    : "ESTUDIO INEFICIENTE",
                "Longitud maxima superada: " +
                std::to_string(longitudSuperada_) +
                "/8\n"
                "Puntuacion: " +
                std::to_string(puntuacion_) +
                "/100\n" +
                (aprobado
                    ? "APROBASTE: aprendiste la materia."
                    : "NO APROBASTE: no ganaste conocimiento."),
                "ENTER O CLIC PARA CONTINUAR"
            );
        }
    }

private:
    static constexpr int LONGITUD_MAXIMA = 8;
    static constexpr int NOTA_MINIMA = 70;

    enum class Fase
    {
        Instrucciones,
        Mostrar,
        Responder,
        Espera,
        Resultados
    };

    const sf::Font& fuente_;
    std::mt19937 azar_;

    sf::Texture fondo_;
    bool fondoDisponible_ = false;

    sf::Music musica_;
    bool musicaDisponible_ = false;

    std::array<sf::SoundBuffer, 8> buffers_;
    std::array<std::unique_ptr<sf::Sound>, 8> sonidos_;

    sf::RenderWindow* ventana_ = nullptr;
    sf::View vistaJuego_;

    Fase fase_ = Fase::Instrucciones;

    std::vector<int> secuencia_;

    std::array<float, 4> luces_{
        {0.f, 0.f, 0.f, 0.f}
    };

    float reloj_ = 0.f;
    float pulso_ = 0.f;
    float tiempoRespuesta_ = 0.f;
    float errorVisual_ = 0.f;

    int ultimoMostrado_ = -1;
    int indiceRespuesta_ = 0;
    int vidas_ = 2;
    int longitudSuperada_ = 0;
    int puntuacion_ = 0;

    bool repetir_ = false;

    // --------------------------------------------------------
    // Generacion aleatoria y sonido.
    // --------------------------------------------------------

    int nuevaDireccion()
    {
        return std::uniform_int_distribution<int>(
            0, 3
        )(azar_);
    }

    void reproducir(int indice)
    {
        if (sonidos_[indice])
            sonidos_[indice]->play();
    }

    void iluminar(int direccion)
    {
        luces_[direccion] = 1.f;
        reproducir(direccion);
    }

    // --------------------------------------------------------
    // Ciclo de juego.
    // --------------------------------------------------------

    void iniciar()
    {
        secuencia_.clear();

        secuencia_.push_back(nuevaDireccion());
        secuencia_.push_back(nuevaDireccion());

        if (musicaDisponible_)
            musica_.play();

        comenzarDemostracion();
    }

    void comenzarDemostracion()
    {
        fase_ = Fase::Mostrar;
        reloj_ = 0.f;
        ultimoMostrado_ = -1;
        indiceRespuesta_ = 0;

        luces_.fill(0.f);
    }

    void confirmar()
    {
        if (fase_ == Fase::Instrucciones)
        {
            iniciar();
        }
        else if (fase_ == Fase::Resultados)
        {
            const auto estado =
                puntuacion_ >= NOTA_MINIMA
                    ? EstadoMinijuego::Completado
                    : EstadoMinijuego::Fallido;

            finalizar(estado, puntuacion_);
        }
    }

    void salir()
    {
        if (fase_ == Fase::Resultados)
            confirmar();
        else
            finalizar(EstadoMinijuego::Cancelado);
    }

    void terminar()
    {
        puntuacion_ = static_cast<int>(
            std::lround(
                100.f * longitudSuperada_ /
                LONGITUD_MAXIMA
            )
        );

        fase_ = Fase::Resultados;

        musica_.stop();

        reproducir(
            puntuacion_ >= NOTA_MINIMA ? 6 : 7
        );
    }

    void equivocacion()
    {
        if (fase_ != Fase::Responder)
            return;

        --vidas_;

        errorVisual_ = 0.5f;
        reproducir(4);

        if (vidas_ <= 0)
        {
            terminar();
            return;
        }

        // Repetir la misma secuencia tras un error.
        repetir_ = true;
        reloj_ = 1.0f;
        fase_ = Fase::Espera;
    }

    void responder(int direccion)
    {
        if (fase_ != Fase::Responder)
            return;

        if (secuencia_[indiceRespuesta_] != direccion)
        {
            equivocacion();
            return;
        }

        iluminar(direccion);

        ++indiceRespuesta_;

        // Se completo la secuencia de esta ronda.
        if (indiceRespuesta_ ==
            static_cast<int>(secuencia_.size()))
        {
            longitudSuperada_ =
                static_cast<int>(secuencia_.size());

            if (longitudSuperada_ >= LONGITUD_MAXIMA)
            {
                terminar();
                return;
            }

            repetir_ = false;
            reloj_ = 1.0f;
            fase_ = Fase::Espera;

            reproducir(5);
        }
    }

    // --------------------------------------------------------
    // Mouse y distribucion de los botones.
    // --------------------------------------------------------

    void clic(sf::Vector2i pixel)
    {
        if (!ventana_)
            return;

        const sf::Vector2f punto =
            ventana_->mapPixelToCoords(
                pixel,
                vistaJuego_
            );

        if (fase_ == Fase::Instrucciones ||
            fase_ == Fase::Resultados)
        {
            if (punto.x >= 204.f &&
                punto.x <= 436.f &&
                punto.y >= 239.f &&
                punto.y <= 271.f)
            {
                confirmar();
            }
            return;
        }

        if (fase_ != Fase::Responder)
            return;

        for (int i = 0; i < 4; ++i)
        {
            auto p = posicionBoton(i);

            if (punto.x >= p.x &&
                punto.x <= p.x + 100.f &&
                punto.y >= p.y &&
                punto.y <= p.y + 58.f)
            {
                responder(i);
                return;
            }
        }
    }

    static sf::Vector2f posicionBoton(int i)
    {
        // Arriba, derecha, abajo, izquierda (2x2).
        switch (i)
        {
        case 0:
            return {208.f, 142.f};

        case 1:
            return {332.f, 142.f};

        case 2:
            return {332.f, 213.f};

        default:
            return {208.f, 213.f};
        }
    }

    // --------------------------------------------------------
    // Dibujado de la interfaz.
    // --------------------------------------------------------

    static void caja(
        sf::RenderWindow& ventana,
        float x, float y,
        float w, float h,
        sf::Color color)
    {
        sf::RectangleShape r({w, h});
        r.setPosition({x, y});
        r.setFillColor(color);

        ventana.draw(r);
    }

    void texto(
        sf::RenderWindow& ventana,
        const std::string& contenido,
        unsigned tam,
        float x, float y,
        sf::Color color,
        bool centrar = false)
    {
        sf::Text t = crearTexto(
            fuente_,
            contenido,
            tam
        );

        t.setFillColor(color);

        if (centrar)
            x -= anchoTexto(t) / 2.f;

        t.setPosition({x, y});

        ventana.draw(t);
    }

    static sf::Color colorBoton(int i)
    {
        static const std::array<sf::Color, 4> colores = {{
            sf::Color(77, 205, 239),   // Arriba: cyan.
            sf::Color(251, 164, 96),   // Derecha: ambar.
            sf::Color(154, 226, 127),  // Abajo: verde.
            sf::Color(223, 133, 218)   // Izquierda: violeta.
        }};

        return colores[i];
    }

    void dibujarBoton(
        sf::RenderWindow& ventana,
        int i)
    {
        const sf::Vector2f p = posicionBoton(i);

        const float brillo = luces_[i];
        const sf::Color c = colorBoton(i);

        const int extra =
            static_cast<int>(130.f * brillo);

        // Resplandor exterior.
        caja(
            ventana,
            p.x - 4.f, p.y - 4.f,
            108.f, 66.f,
            sf::Color(
                c.r, c.g, c.b,
                static_cast<std::uint8_t>(
                    32 + 145 * brillo
                )
            )
        );

        // Sombra.
        caja(
            ventana,
            p.x + 3.f, p.y + 5.f,
            100.f, 58.f,
            sf::Color(6, 8, 31)
        );

        // Cuerpo del boton.
        caja(
            ventana,
            p.x, p.y,
            100.f, 58.f,
            sf::Color(
                static_cast<std::uint8_t>(
                    std::min(255, 24 + extra)
                ),
                static_cast<std::uint8_t>(
                    std::min(255, 29 + extra)
                ),
                static_cast<std::uint8_t>(
                    std::min(255, 64 + extra)
                )
            )
        );

        // Bordes coloreados.
        caja(
            ventana,
            p.x, p.y,
            100.f, 3.f, c
        );

        caja(
            ventana,
            p.x, p.y + 55.f,
            100.f, 3.f, c
        );

        caja(
            ventana,
            p.x, p.y,
            3.f, 58.f, c
        );

        caja(
            ventana,
            p.x + 97.f, p.y,
            3.f, 58.f, c
        );

        // Flechas construidas con geometria.
        // No dependen de que la fuente tenga simbolos.
        sf::ConvexShape punta(3);

        const float cx = p.x + 50.f;
        const float cy = p.y + 27.f;

        sf::Vector2f a, b, d;

        if (i == 0)
        {
            // Arriba.
            a = {cx, cy - 15.f};
            b = {cx - 13.f, cy + 7.f};
            d = {cx + 13.f, cy + 7.f};
        }
        else if (i == 1)
        {
            // Derecha.
            a = {cx + 16.f, cy};
            b = {cx - 7.f, cy - 13.f};
            d = {cx - 7.f, cy + 13.f};
        }
        else if (i == 2)
        {
            // Abajo.
            a = {cx, cy + 15.f};
            b = {cx - 13.f, cy - 7.f};
            d = {cx + 13.f, cy - 7.f};
        }
        else
        {
            // Izquierda.
            a = {cx - 16.f, cy};
            b = {cx + 7.f, cy - 13.f};
            d = {cx + 7.f, cy + 13.f};
        }

        punta.setPoint(0, a);
        punta.setPoint(1, b);
        punta.setPoint(2, d);

        punta.setFillColor(
            brillo > 0.f
                ? sf::Color::White
                : c
        );

        ventana.draw(punta);

        const char* teclas[4] = {
            "W", "D", "S", "A"
        };

        texto(
            ventana,
            teclas[i],
            11,
            p.x + 86.f,
            p.y + 41.f,
            sf::Color(230, 225, 245)
        );
    }

    void panel(
        sf::RenderWindow& ventana,
        const std::string& titulo,
        const std::string& contenido,
        const std::string& accion)
    {
        // Sombra.
        caja(
            ventana,
            131.f, 82.f,
            378.f, 200.f,
            sf::Color(7, 9, 30, 245)
        );

        // Borde dorado.
        caja(
            ventana,
            135.f, 86.f,
            370.f, 192.f,
            sf::Color(47, 35, 74)
        );

        // Fondo interior.
        caja(
            ventana,
            138.f, 89.f,
            364.f, 186.f,
            sf::Color(18, 26, 56)
        );

        caja(
            ventana,
            138.f, 89.f,
            364.f, 2.f,
            sf::Color(241, 179, 96)
        );

        texto(
            ventana,
            titulo, 21,
            320.f, 105.f,
            sf::Color(255, 219, 156),
            true
        );

        texto(
            ventana,
            contenido, 15,
            320.f, 156.f,
            sf::Color(228, 230, 246),
            true
        );

        caja(
            ventana,
            204.f, 239.f,
            232.f, 32.f,
            sf::Color(56, 137, 141)
        );

        caja(
            ventana,
            207.f, 242.f,
            226.f, 26.f,
            sf::Color(73, 180, 175)
        );

        texto(
            ventana,
            accion, 13,
            320.f, 245.f,
            sf::Color(249, 251, 255),
            true
        );
    }
};
