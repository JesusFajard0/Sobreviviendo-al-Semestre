#pragma once

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "minigames/Minijuego.hpp"

// Fabrica de minijuegos indexada por identificador.
//
// Las categorias permiten organizar los minijuegos.
// Tanto "computadora" como "estudio" pueden usar
// el sistema de sorteo antirrepeticion.
//
// Los minijuegos de estudio se seleccionan
// segun la materia activa.
class RegistroMinijuegos
{
public:
    using Fabrica = std::function<std::unique_ptr<Minijuego>(
        StateStack*, Minijuego::AlTerminar)>;

    // Devuelve false si el ID es vacio, esta repetido o falta la fabrica.
    bool registrar(const std::string& id, const std::string& categoria,
                   Fabrica fabrica);

    std::unique_ptr<Minijuego> crear(const std::string& id, StateStack* pila,
                                      Minijuego::AlTerminar alTerminar) const;

    bool contiene(const std::string& id) const;
    std::vector<std::string> idsDeCategoria(const std::string& categoria) const;

private:
    struct Entrada { std::string categoria; Fabrica fabrica; };
    std::unordered_map<std::string, Entrada> entradas_;
    std::vector<std::string> orden_; // Mantiene un orden estable de registro.
};
