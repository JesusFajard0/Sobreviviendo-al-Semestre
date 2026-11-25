#pragma once

#include <string>
#include <utility>
#include <vector>

#include "systems/Estadisticas.hpp"
#include "minigames/SorteadorMinijuegos.hpp"

// Construye los candidatos segun la materia activa,
// con probabilidades base 50 / 25 / 25.
inline std::vector<std::pair<std::string, int>>
candidatosEstudioPonderados(Materia materia)
{
    std::string especifico;

    switch (materia)
    {
    case Materia::Calculo:
        especifico = "estudio_calculo";
        break;

    case Materia::Fisica:
        especifico = "estudio_fisica";
        break;

    case Materia::Programacion:
        especifico = "estudio_programacion";
        break;

    default:
        return {};
    }

    return {
        {especifico, 50},
        {"estudio_generico_1", 25},
        {"estudio_generico_2", 25}
    };
}

// Conservar la funcion original para compatibilidad
// con otras partes del proyecto y con las pruebas.
inline std::vector<std::string>
candidatosEstudio(Materia materia)
{
    std::vector<std::string> ids;

    for (const auto& candidato :
         candidatosEstudioPonderados(materia))
    {
        ids.push_back(candidato.first);
    }

    return ids;
}

// Seleccionar el minijuego correspondiente a una
// sesion de estudio utilizando el sorteo ponderado.
inline std::string sortearMinijuegoEstudio(
    SorteadorMinijuegos& sorteador,
    Materia materia)
{
    return sorteador.sortearPonderado(
        "estudio",
        candidatosEstudioPonderados(materia)
    );
}