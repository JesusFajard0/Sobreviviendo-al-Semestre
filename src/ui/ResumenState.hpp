#pragma once

#include <SFML/Graphics.hpp>

#include <functional>
#include <string>

#include "core/State.hpp"
#include "systems/Finales.hpp"

// S3-5: lo que pasa despues del examen final. Tres pantallas seguidas:
//   1. las tres notas sobre 20 y el promedio
//   2. las estadisticas: clases asistidas y faltas, fiestas, platos lavados,
//      noches sin dormir y salud mental, dopamina y dinero finales
//   3. el final que le toco (nombre, imagen y texto); de aqui se vuelve al menu
//
// No cambia la pila por su cuenta: al terminar solo avisa por "alMenu" (que
// pide a Juego volver al menu principal). Asi ni este estado ni
// EstadoExploracion se destruyen mientras se esta ejecutando uno de sus metodos.
class ResumenState : public State
{
public:
    using Accion = std::function<void()>;

    // "imagenFinal" puede ser nullptr (no hay imagen): se dibuja un recuadro
    // provisional. La textura la guarda GestorRecursos y debe durar mas que
    // este estado.
    ResumenState(StateStack* pila,
                 const sf::Font& fuente,
                 const ResumenSemestre& resumen,
                 const sf::Texture* imagenFinal,
                 Accion alMenu);

    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;

private:
    enum Pagina { Notas = 0, Estadisticas, Final, CantidadPaginas };

    sf::FloatRect rectBoton() const;
    void siguiente();    // en la ultima pantalla pide volver al menu
    void anterior();

    sf::Text hacerTexto(const std::string& utf8, unsigned tamano, const sf::Color& color) const;
    void centrar(sf::Text& texto, const sf::FloatRect& caja) const;
    std::string ajustar(const std::string& utf8, unsigned tamano, float ancho) const;

    void dibujarCabecera(sf::RenderWindow& window, const std::string& subtitulo);
    void dibujarNotas(sf::RenderWindow& window);
    void dibujarEstadisticas(sf::RenderWindow& window);
    void dibujarFinal(sf::RenderWindow& window);
    void dibujarFila(sf::RenderWindow& window, float x, float y, float ancho,
                     const std::string& etiqueta, const std::string& valor);

    const sf::Font&    fuente_;
    ResumenSemestre    resumen_;
    const sf::Texture* imagen_;
    Accion             alMenu_;

    std::string nombreAjustado_;   // el nombre y el texto del final, ya partidos en renglones
    std::string textoAjustado_;

    int  pagina_ = Notas;
    bool pedido_ = false;          // ya se pidio volver al menu: se ignora todo

    sf::RenderWindow* ventana_ = nullptr;
    sf::Vector2f      origen_{0.f, 0.f};
    sf::Vector2f      tam_{640.f, 360.f};
};
