#pragma once

#include <string>
#include <vector>

#include "systems/Estadisticas.hpp"
#include "minigames/SorteadorMinijuegos.hpp"

// Construye los minijuegos disponibles para estudiar
// segun la materia activa.
inline std::vector<std::string> candidatosEstudio(Materia materia)
{
    switch (materia)
    {
    case Materia::Calculo:
        return {
            "estudio_calculo",
            "estudio_generico_1",
            "estudio_generico_2"
        };

    case Materia::Fisica:
        return {
            "estudio_fisica",
            "estudio_generico_1",
            "estudio_generico_2"
        };

    case Materia::Programacion:
        return {
            "estudio_programacion",
            "estudio_generico_1",
            "estudio_generico_2"
        };

    default:
        return {};
    }
}

// Sortea un minijuego entre los disponibles
// para la materia correspondiente.
//
// Todos los sorteos de estudio comparten historial,
// evitando repetir inmediatamente el ultimo minijuego.
inline std::string sortearMinijuegoEstudio(
    SorteadorMinijuegos& sorteador,
    Materia materia)
{
    return sorteador.sortear(
        "estudio",
        candidatosEstudio(materia)
    );
}