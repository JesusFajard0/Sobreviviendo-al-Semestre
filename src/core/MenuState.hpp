#pragma once

#include <SFML/Graphics.hpp>

#include <functional>

#include "core/State.hpp"

// S3-8: menu de inicio. Es el primer estado de la pila: muestra el titulo del
// juego y los botones Jugar, Como jugar, Creditos y Salir. Se maneja con el
// teclado (W/S o flechas y ENTER) y con el raton (pasar encima y clic).
//
// No sabe como se crea una partida ni como se cierra la ventana: Juego le
// pasa esas dos acciones, igual que MenuDestinos recibe "alElegir".
class MenuState : public State
{
public:
    using Accion = std::function<void()>;

    MenuState(StateStack* pila, const sf::Font& fuente, Accion alJugar, Accion alSalir);

    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;

private:
    enum Opcion { Jugar = 0, ComoJugar, Creditos, Salir, CantidadOpciones };
    enum class Pantalla { Principal, ComoJugar, Creditos };

    sf::FloatRect rectBoton(int indice) const;
    int  botonEn(const sf::Vector2f& punto) const;   // -1 si no hay ninguno
    void elegir(int indice);                         // despues de Jugar no se puede usar this

    sf::Text hacerTexto(const std::string& utf8, unsigned tamano, const sf::Color& color) const;
    void centrar(sf::Text& texto, const sf::FloatRect& caja) const;
    void dibujarPrincipal(sf::RenderWindow& window);
    void dibujarPanel(sf::RenderWindow& window, const std::string& titulo, const std::string& cuerpo);

    const sf::Font& fuente_;
    Accion alJugar_;
    Accion alSalir_;

    Pantalla pantalla_ = Pantalla::Principal;
    int      seleccion_ = Jugar;

    // Vista con la que se dibujo por ultima vez: sirve para ubicar los botones
    // y para convertir la posicion del raton (pixeles) a coordenadas logicas.
    sf::RenderWindow* ventana_ = nullptr;
    sf::Vector2f      origen_{0.f, 0.f};
    sf::Vector2f      tam_{640.f, 360.f};
};
