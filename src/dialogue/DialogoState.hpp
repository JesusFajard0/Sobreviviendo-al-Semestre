#pragma once

#include <SFML/Graphics.hpp>

#include <cstddef>
#include <functional>
#include <string>
#include <vector>

#include "core/State.hpp"
#include "dialogue/GestorDialogos.hpp"

// Parte un texto en renglones que no pasen de anchoMaximo, cortando por
// espacios. "medir" devuelve el ancho de un texto. Una palabra mas ancha que
// el maximo se deja sola en su renglon.
std::string ajustarTexto(const std::string& texto,
                         const std::function<float(const std::string&)>& medir,
                         float anchoMaximo);

// S2-7: caja de dialogo. Se apila sobre EstadoExploracion y muestra una frase
// por vez. E, ENTER o ESPACIO pasan a la siguiente; ESC cierra la conversacion.
// Al terminar se quita sola y llama a alTerminar (opcional).
class DialogoState : public State
{
public:
    using AlTerminar = std::function<void()>;

    DialogoState(StateStack* pila,
                 const sf::Font& fuente,
                 std::vector<LineaDialogo> lineas,
                 AlTerminar alTerminar = {});

    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;

private:
    void avanzar();
    void cerrar();   // despues de llamarla no se puede usar ningun miembro

    std::vector<LineaDialogo> lineas_;
    std::vector<std::string>  ajustadas_;   // cada frase ya partida en renglones
    std::size_t               actual_ = 0;
    bool                      puedeAvanzar_ = false;   // hay que soltar la tecla entre frases
    AlTerminar                alTerminar_;

    sf::Text textoHablante_;
    sf::Text textoFrase_;
    sf::Text textoAyuda_;
};
