#pragma once

#include <functional>
#include <string>
#include <utility>

#include "core/State.hpp"
#include "core/StateStack.hpp"

// Resultado de una sesion. Se distingue cancelar de perder para no
// consumir una actividad cuando el jugador simplemente sale del minijuego.
enum class EstadoMinijuego { Completado, Fallido, Cancelado };

struct ResultadoMinijuego
{
    std::string id;
    EstadoMinijuego estado = EstadoMinijuego::Cancelado;
    int puntuacion = 0;
};

// Los minijuegos son estados de la pila, igual que ClaseState.
// Una implementacion concreta define handleEvent/update/draw.
class Minijuego : public State
{
public:
    using AlTerminar = std::function<void(const ResultadoMinijuego&)>;

    Minijuego(StateStack* pila, std::string id, AlTerminar alTerminar)
        : State(pila), id_(std::move(id)), alTerminar_(std::move(alTerminar)) {}

    const std::string& id() const { return id_; }

protected:
    // IMPORTANTE: popState() destruye *this. No acceder a miembros despues.
    void finalizar(EstadoMinijuego estado, int puntuacion = 0)
    {
        ResultadoMinijuego resultado{id_, estado, puntuacion};
        AlTerminar callback = std::move(alTerminar_);
        m_stack->popState();
        if (callback) callback(resultado);
    }

private:
    std::string id_;
    AlTerminar alTerminar_;
};
