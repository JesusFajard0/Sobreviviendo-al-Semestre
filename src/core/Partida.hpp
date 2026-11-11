#pragma once

// Estado compartido de una partida: lo que el HUD, las puertas y las
// actividades necesitan ver y modificar. Lo crea Juego y se pasa por
// referencia a quien lo use.

#include "activities/GestorActividades.hpp"
#include "systems/Estadisticas.hpp"
#include "systems/Tiempo.hpp"

struct Partida
{
    Estadisticas      estadisticas;
    Tiempo            tiempo;
    GestorActividades actividades;

    // Materia del próximo examen: Cálculo hasta la semana 3, Física en las
    // semanas 4 y 5, y Programación de la 6 a la 8.
    Materia materiaActiva() const;

    // Fase del semestre que usan las puertas (faseMinima en data/escenas.txt):
    // 1 = Cálculo, 2 = Física, 3 = Programación.
    int faseSemestre() const;
};  