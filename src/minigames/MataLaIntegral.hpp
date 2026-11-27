
#pragma once

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iterator>
#include <memory>
#include <random>
#include <string>
#include <utility>
#include <vector>

#include "core/CompatSFML.hpp"
#include "minigames/Minijuego.hpp"

// Mata la Integral: minijuego de Calculo.
// Resolucion logica: 640 x 360.
class MataLaIntegral : public Minijuego
{
public:
    MataLaIntegral(StateStack* pila,
                   const sf::Font& fuente,
                   AlTerminar alTerminar)
        : Minijuego(pila, "estudio_calculo",
                    std::move(alTerminar)),
          fuente_(fuente),
          generador_(std::random_device{}())
    {
        tieneFondo_ = fondo_.loadFromFile(
            "assets/minijuegos/mata_integral_fondo.png"
        );

        cargarSonidos();

        // Cargar musica ambiental.
        musicaDisponible_ = musica_.openFromFile(RUTA_MUSICA);

        if (musicaDisponible_)
        {
            musica_.setLooping(true);

            // Volumen de 0 a 100.
            musica_.setVolume(20.f);
        }

        // Cinco integrales y siete distractores.
        for (int i = 0; i < 12; ++i)
            simbolos_.push_back(nuevoSimbolo(i < 5));
    }

    void handleEvent(const sf::Event& evento) override
    {
        #if SFML_VERSION_MAJOR >= 3
                if (const auto* tecla =
                        evento.getIf<sf::Event::KeyPressed>())
                {
                    if (tecla->code == sf::Keyboard::Key::Escape)
                    {
                        if (fase_ == Fase::Resultados)
                            finalizarSegunPuntuacion();
                        else
                            finalizar(EstadoMinijuego::Cancelado);
                        return;
                    }

                    if (tecla->code == sf::Keyboard::Key::Enter)
                    {
                        confirmar();
                        return;
                    }
                }

                if (const auto* raton = evento.getIf<sf::Event::MouseButtonPressed>())
                {
                    if (raton->button == sf::Mouse::Button::Left)
                        pulsar(raton->position);
                }
        #else
                if (evento.type == sf::Event::KeyPressed)
                {
                    if (evento.key.code == sf::Keyboard::Escape)
                    {
                        if (fase_ == Fase::Resultados)
                            finalizarSegunPuntuacion();
                        else
                            finalizar(EstadoMinijuego::Cancelado);
                        return;
                    }

                    if (evento.key.code == sf::Keyboard::Enter)
                    {
                        confirmar();
                        return;
                    }
                }

                if (evento.type == sf::Event::MouseButtonPressed &&
                    evento.mouseButton.button == sf::Mouse::Left)
                {
                    pulsar({
                        evento.mouseButton.x,
                        evento.mouseButton.y
                    });
                }
        #endif
    }

    void update(float dt) override
    {
        dt = std::clamp(dt, 0.f, 0.1f);
        animacion_ += dt;

        // Actualizar particulas.
        for (auto& p : particulas_)
        {
            p.x += p.vx * dt;
            p.y += p.vy * dt;
            p.vida -= dt;
        }

        particulas_.erase(
            std::remove_if(
                particulas_.begin(),
                particulas_.end(),
                [](const Particula& p)
                {
                    return p.vida <= 0.f;
                }),
            particulas_.end()
        );

        if (fase_ != Fase::Jugando)
            return;

        restante_ = std::max(0.f, restante_ - dt);

        // La velocidad aumenta durante la partida.
        const float velocidad =
            1.f + 0.80f * (1.f - restante_ / DURACION);

        for (auto& s : simbolos_)
        {
            s.x += s.vx * dt * velocidad;
            s.y += s.vy * dt * velocidad;

            // Rebotar contra los limites del tablero.
            if (s.x < 48.f || s.x > 592.f)
            {
                s.x = std::clamp(s.x, 48.f, 592.f);
                s.vx *= -1.f;
            }

            if (s.y < 100.f || s.y > 265.f)
            {
                s.y = std::clamp(s.y, 100.f, 265.f);
                s.vy *= -1.f;
            }
        }

        if (restante_ <= 0.f)
        {
            const float precision =
                (aciertos_ + errores_ > 0)
                ? static_cast<float>(aciertos_) /
                  (aciertos_ + errores_)
                : 0.f;

            // Se necesitan 24 aciertos para completar
            // el componente de progreso al 100 %.
            const float progreso =
                std::min(1.f, aciertos_ / 24.f);

            // 70 % progreso y 30 % precision.
            // La nota final se calcula entre 0 y 100.
            puntuacion_ = std::clamp(
                static_cast<int>(std::lround(
                    100.f * (
                        0.70f * progreso +
                        0.30f * precision
                    )
                )),
                0, 100
            );

            fase_ = Fase::Resultados;

            // Detener musica de fondo.
            musica_.stop();

            // Reproducir efecto de finalizacion.
            reproducir(sonidoFinal_);
        }
    }

    void draw(sf::RenderWindow& ventana) override
    {
        ventana_ = &ventana;
        vistaJuego_ = ventana.getView();

        // Dibujar fondo PNG.
        if (tieneFondo_)
        {
            #if SFML_VERSION_MAJOR >= 3
                        sf::Sprite imagen(fondo_);
            #else
                        sf::Sprite imagen;
                        imagen.setTexture(fondo_);
            #endif
                        ventana.draw(imagen);
        }
        else
        {
            sf::RectangleShape base({640.f, 360.f});
            base.setFillColor(sf::Color(12, 38, 46));
            ventana.draw(base);
        }

        // Interfaz superior.
        texto(
            ventana, "MATA LA INTEGRAL", 20,
            320.f, 22.f,
            sf::Color(246, 221, 155), true
        );

        texto(
            ventana, "SOLO LAS INTEGRALES", 11,
            320.f, 49.f,
            sf::Color(168, 206, 197), true
        );

        // Marcadores inferiores.
        texto(
            ventana,
            "ACIERTOS: " + std::to_string(aciertos_),
            16, 163.f, 321.f,
            sf::Color(185, 234, 191), true
        );

        texto(
            ventana,
            "ERRORES: " + std::to_string(errores_),
            16, 317.f, 321.f,
            sf::Color(248, 164, 151), true
        );

        texto(
            ventana,
            "TIEMPO: " + std::to_string(
                static_cast<int>(std::ceil(restante_))
            ),
            16, 470.f, 321.f,
            sf::Color(239, 213, 158), true
        );

        if (fase_ == Fase::Jugando)
        {
            for (const auto& s : simbolos_)
                dibujarSimbolo(ventana, s);
        }

        // Particulas de aciertos y errores.
        for (const auto& p : particulas_)
        {
            sf::RectangleShape chispa({3.f, 3.f});
            chispa.setPosition({p.x, p.y});

            sf::Color color = p.color;
            color.a = static_cast<std::uint8_t>(
                255.f * std::clamp(
                    p.vida / 0.55f, 0.f, 1.f
                )
            );

            chispa.setFillColor(color);
            ventana.draw(chispa);
        }

        // Pantalla inicial.
        if (fase_ == Fase::Instrucciones)
        {
            panel(
                ventana,
                "ATRAPA LAS INTEGRALES",
                "Haz clic SOLO en el simbolo integral:  ∫\n"
                "No toques dx, pi, sumas ni otros signos.\n"
                "Tienes 25 segundos. Necesitas 70 puntos.",
                "ENTER O CLIC PARA EMPEZAR"
            );
        }
        // Pantalla de resultados.
        else if (fase_ == Fase::Resultados)
        {
            const bool aprobado =
                puntuacion_ >= PUNTUACION_MINIMA;

            panel(
                ventana,
                aprobado
                    ? "ESTUDIO EFICIENTE!"
                    : "ESTUDIO INEFICIENTE",
                "Aciertos: " + std::to_string(aciertos_) +
                "     Errores: " + std::to_string(errores_) +
                "\n\nPUNTUACION: " +
                std::to_string(puntuacion_) + " / 100" +
                "\nMinimo para aprender: " +
                std::to_string(PUNTUACION_MINIMA),
                "ENTER O CLIC PARA CONTINUAR"
            );
        }
    }

private:
    static constexpr float DURACION = 25.f;
    static constexpr int PUNTUACION_MINIMA = 70;
    // Cambia solo este nombre cuando quieras otra musica.
    static constexpr const char* RUTA_MUSICA = "assets/audio/minijuegos/musica_calculo.ogg";

    enum class Fase
    {
        Instrucciones,
        Jugando,
        Resultados
    };

    struct Simbolo
    {
        std::string etiqueta;
        float x = 0.f;
        float y = 0.f;
        float vx = 0.f;
        float vy = 0.f;
        float onda = 0.f;
        float radio = 20.f;
        bool integral = false;
        sf::Color color = sf::Color::White;
    };

    struct Particula
    {
        float x, y;
        float vx, vy;
        float vida;
        sf::Color color;
    };

    const sf::Font& fuente_;
    std::mt19937 generador_;

    sf::Texture fondo_;
    bool tieneFondo_ = false;

    sf::RenderWindow* ventana_ = nullptr;
    sf::View vistaJuego_;

    Fase fase_ = Fase::Instrucciones;
    float restante_ = DURACION;
    float animacion_ = 0.f;

    int aciertos_ = 0;
    int errores_ = 0;
    int puntuacion_ = 0;

    std::vector<Simbolo> simbolos_;
    std::vector<Particula> particulas_;

    // Buffers y reproductores de audio.
    sf::SoundBuffer bufferInicio_;
    sf::SoundBuffer bufferAcierto_;
    sf::SoundBuffer bufferError_;
    sf::SoundBuffer bufferFinal_;

    std::unique_ptr<sf::Sound> sonidoInicio_;
    std::unique_ptr<sf::Sound> sonidoAcierto_;
    std::unique_ptr<sf::Sound> sonidoError_;
    std::unique_ptr<sf::Sound> sonidoFinal_;

    // Musica ambiental del minijuego.
    sf::Music musica_;
    bool musicaDisponible_ = false;

    float azar(float a, float b)
    {
        std::uniform_real_distribution<float> dist(a, b);
        return dist(generador_);
    }

    Simbolo nuevoSimbolo(bool esIntegral)
    {
        static const std::vector<std::string> distractores = {
            "dx", "∂", "π", "Σ", "√",
            "∞", "Δ", "dy", "+"
        };

        static const sf::Color colores[] = {
            sf::Color(241, 217, 157),
            sf::Color(167, 222, 208),
            sf::Color(227, 178, 194),
            sf::Color(190, 195, 244)
        };

        Simbolo s;
        s.integral = esIntegral;

        if (esIntegral)
        {
            s.etiqueta = "∫";
        }
        else
        {
            const int indice = static_cast<int>(
                azar(
                    0.f,
                    static_cast<float>(
                        distractores.size()
                    ) - 0.001f
                )
            );
            s.etiqueta = distractores[indice];
        }

        s.x = azar(64.f, 575.f);
        s.y = azar(110.f, 250.f);

        // Mayor velocidad horizontal.
        s.vx = azar(54.f, 105.f) * (azar(0.f, 1.f) < 0.5f ? -1.f : 1.f);

        // Mayor velocidad vertical.
        s.vy = azar(36.f, 75.f) * (azar(0.f, 1.f) < 0.5f ? -1.f : 1.f);

        s.onda = azar(0.f, 6.28318f);
        s.radio = esIntegral ? 23.f : 19.f;

        s.color = colores[static_cast<int>(azar(0.f, 3.999f))];

        // Evitar simbolos demasiado superpuestos.
        for (int intento = 0; intento < 24; ++intento)
        {
            bool cerca = false;

            for (const auto& otro : simbolos_)
            {
                const float dx = s.x - otro.x;
                const float dy = s.y - otro.y;

                if (dx * dx + dy * dy < 47.f * 47.f)
                {
                    cerca = true;
                    break;
                }
            }

            if (!cerca)
                break;

            s.x = azar(64.f, 575.f);
            s.y = azar(110.f, 250.f);
        }

        return s;
    }

    float yDibujada(const Simbolo& s) const
    {
        return s.y +
               3.f * std::sin(
                   animacion_ * 2.3f + s.onda
               );
    }

    void dibujarSimbolo(sf::RenderWindow& ventana, const Simbolo& s)
    {
        const float y = yDibujada(s);

        // Circulo del simbolo.
        sf::CircleShape halo(s.radio);

        halo.setOrigin({s.radio, s.radio});
        halo.setPosition({s.x, y});

        halo.setFillColor(sf::Color(29, 62, 68, 180));

        halo.setOutlineThickness(1.2f);

        halo.setOutlineColor(
            sf::Color(
                s.color.r,
                s.color.g,
                s.color.b,
                95
            )
        );

        ventana.draw(halo);

        // Crear texto y calcular su centro real.
        sf::Text simbolo = crearTexto(
            fuente_,
            s.etiqueta,
            s.integral ? 26 : 22
        );

        simbolo.setFillColor(s.color);

        const sf::FloatRect limites =
            simbolo.getLocalBounds();

        const sf::Vector2f posicion =
            posicionRect(limites);

        const sf::Vector2f tamano =
            tamanoRect(limites);

        // Centrado horizontal y vertical.
        simbolo.setOrigin({
            posicion.x + tamano.x / 2.f,
            posicion.y + tamano.y / 2.f
        });

        simbolo.setPosition({s.x, y});

        ventana.draw(simbolo);
    }

    void texto(
        sf::RenderWindow& ventana,
        const std::string& cadena,
        unsigned tam,
        float x,
        float y,
        sf::Color color,
        bool centro)
    {
        sf::Text t = crearTexto(fuente_, cadena, tam);
        t.setFillColor(color);

        if (centro)
            x -= anchoTexto(t) / 2.f;

        t.setPosition({x, y});
        ventana.draw(t);
    }

    void panel(
        sf::RenderWindow& ventana,
        const std::string& titulo,
        const std::string& cuerpo,
        const std::string& boton)
    {
        sf::RectangleShape sombra({464.f, 232.f});
        sombra.setPosition({91.f, 80.f});
        sombra.setFillColor(
            sf::Color(3, 15, 23, 238)
        );
        sombra.setOutlineThickness(2.f);
        sombra.setOutlineColor(
            sf::Color(180, 150, 93)
        );

        ventana.draw(sombra);

        sf::RectangleShape borde({450.f, 218.f});
        borde.setPosition({98.f, 87.f});
        borde.setFillColor(
            sf::Color(19, 45, 54)
        );
        borde.setOutlineThickness(1.f);
        borde.setOutlineColor(
            sf::Color(61, 111, 113)
        );

        ventana.draw(borde);

        texto(
            ventana, titulo, 25,
            320.f, 103.f,
            sf::Color(250, 221, 158), true
        );

        texto(
            ventana, cuerpo, 17,
            320.f, 160.f,
            sf::Color(219, 235, 219), true
        );

        sf::RectangleShape botonRect({315.f, 34.f});
        botonRect.setPosition({162.f, 253.f});
        botonRect.setFillColor(
            sf::Color(41, 115, 112)
        );
        botonRect.setOutlineThickness(1.f);
        botonRect.setOutlineColor(
            sf::Color(190, 222, 179)
        );

        ventana.draw(botonRect);

        texto(
            ventana, boton, 17,
            320.f, 260.f,
            sf::Color::White, true
        );
    }

    void empezar()
    {
        fase_ = Fase::Jugando;

        // Iniciar musica ambiental.
        if (musicaDisponible_)
            musica_.play();

        reproducir(sonidoInicio_);
    }

    // Determina si el estudio fue eficiente o ineficiente.
    void finalizarSegunPuntuacion()
    {
        if (puntuacion_ >= PUNTUACION_MINIMA)
        {
            finalizar(
                EstadoMinijuego::Completado,
                puntuacion_
            );
        }
        else
        {
            finalizar(
                EstadoMinijuego::Fallido,
                puntuacion_
            );
        }
    }

    void confirmar()
    {
        if (fase_ == Fase::Instrucciones)
        {
            empezar();
        }
        else if (fase_ == Fase::Resultados)
        {
            finalizarSegunPuntuacion();
        }
    }

    void pulsar(sf::Vector2i posicionPixel)
    {
        if (!ventana_)
            return;

        const sf::Vector2f p =
            ventana_->mapPixelToCoords(
                posicionPixel,
                vistaJuego_
            );

        if (fase_ == Fase::Instrucciones ||
            fase_ == Fase::Resultados)
        {
            if (p.x >= 162.f && p.x <= 477.f &&
                p.y >= 253.f && p.y <= 287.f)
            {
                confirmar();
            }
            return;
        }

        // Limites del tablero.
        if (p.x < 28.f || p.x > 612.f ||
            p.y < 82.f || p.y > 292.f)
        {
            return;
        }

        // Detectar el simbolo seleccionado.
        for (auto it = simbolos_.rbegin();
             it != simbolos_.rend(); ++it)
        {
            const float dx = p.x - it->x;
            const float dy = p.y - yDibujada(*it);

            if (dx * dx + dy * dy >
                it->radio * it->radio)
            {
                continue;
            }

            const bool correcto = it->integral;

            if (correcto)
            {
                ++aciertos_;
                reproducir(sonidoAcierto_);
            }
            else
            {
                ++errores_;
                reproducir(sonidoError_);
            }

            chispas(it->x, yDibujada(*it), correcto);

            // Localizar el simbolo en el vector.
            const auto indice =
                static_cast<std::size_t>(
                    std::distance(
                        it, simbolos_.rend()
                    ) - 1
                );

            int integrales = 0;

            for (const auto& s : simbolos_)
            {
                if (s.integral)
                    ++integrales;
            }

            // Reemplazar el simbolo eliminado.
            const bool siguienteIntegral =
                (integrales <= 3) ||
                (azar(0.f, 1.f) < 0.42f);

            simbolos_[indice] =
                nuevoSimbolo(siguienteIntegral);

            return;
        }
    }

    void chispas(
        float x, float y, bool acierto)
    {
        const sf::Color color = acierto
            ? sf::Color(161, 246, 190)
            : sf::Color(245, 122, 130);

        for (int i = 0; i < 13; ++i)
        {
            const float angulo =
                azar(0.f, 6.28318f);

            const float rapidez =
                azar(22.f, 90.f);

            particulas_.push_back({
                x,
                y,
                std::cos(angulo) * rapidez,
                std::sin(angulo) * rapidez,
                azar(0.30f, 0.55f),
                color
            });
        }
    }

    void cargarSonidos()
    {
        sonidoInicio_ = crearSonido(
            bufferInicio_,
            "assets/audio/minijuegos/inicio.wav"
        );

        sonidoAcierto_ = crearSonido(
            bufferAcierto_,
            "assets/audio/minijuegos/acierto.wav"
        );

        sonidoError_ = crearSonido(
            bufferError_,
            "assets/audio/minijuegos/error.wav"
        );

        sonidoFinal_ = crearSonido(
            bufferFinal_,
            "assets/audio/minijuegos/final.wav"
        );
    }

    static std::unique_ptr<sf::Sound> crearSonido(
        sf::SoundBuffer& buffer,
        const std::string& ruta)
    {
        if (!buffer.loadFromFile(ruta))
            return nullptr;

#if SFML_VERSION_MAJOR >= 3
        return std::make_unique<sf::Sound>(buffer);
#else
        auto sonido = std::make_unique<sf::Sound>();
        sonido->setBuffer(buffer);
        return sonido;
#endif
    }

    static void reproducir(
        const std::unique_ptr<sf::Sound>& sonido)
    {
        if (sonido)
            sonido->play();
    }
};
