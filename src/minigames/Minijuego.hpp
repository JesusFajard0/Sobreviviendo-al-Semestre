#pragma once

#include <functional>
#include <string>
#include <utility>

#include "core/State.hpp"
#include "core/StateStack.hpp"

struct ResultadoMinijuego
{
    bool  exito   = false;
    float puntaje = 0.f;
};

class Minijuego : public State
{
public:
    using AlTerminar = std::function<void(const ResultadoMinijuego&)>;

    explicit Minijuego(StateStack* pila) : State(pila) {}

    virtual std::string id() const = 0;

    void establecerAlTerminar(AlTerminar f) { alTerminar_ = std::move(f); }

protected:
    // Las clases hijas la llaman al acabar. Quita el minijuego de la pila:
    // despues de llamarla NO se debe usar ningun miembro (el objeto ya no existe).
    void terminar(bool exito, float puntaje = 0.f)
    {
        AlTerminar fin = std::move(alTerminar_);
        StateStack* pila = m_stack;
        const ResultadoMinijuego r{exito, puntaje};

        pila->popState();
        if (fin)
            fin(r);
    }

private:
    AlTerminar alTerminar_;
};