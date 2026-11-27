#pragma once

// Estado compartido de una partida: lo que el HUD, las puertas y las
// actividades necesitan ver y modificar. Lo crea Juego y se pasa por
// referencia a quien lo use.

#include "activities/GestorActividades.hpp"
#include "systems/Estadisticas.hpp"
#include "systems/Fases.hpp"
#include "systems/Notas.hpp"
#include "systems/Tiempo.hpp"

struct Partida
{
    Partida();

    Estadisticas      estadisticas;
    Notas             notas;        // S2-6: notas de los tres examenes
    Tiempo            tiempo;
    GestorActividades actividades;

    // Horarios de las escenas (data/fases.txt). Va después de "tiempo"
    // porque guarda una referencia a él.
    Fases             fases;

    // Materia del próximo examen: Cálculo hasta la semana 3, Física en las
    // semanas 4 y 5, y Programación de la 6 a la 8.
    Materia materiaActiva() const;

    // Fase del semestre que usan las puertas (faseMinima en data/escenas.txt):
    // 1 = Cálculo, 2 = Física, 3 = Programación.
    int faseSemestre() const;

    // S3-5: nueva partida. Deja las estadisticas, las notas, el tiempo y los
    // contadores de actividades como al empezar. (Los datos cargados de los
    // archivos de data/ se conservan.)
    void reiniciar();

    // S3-5: lo que el jugador pidio al terminar el semestre. Juego lo atiende al
    // final de cada cuadro (fuera de cualquier estado), porque implica vaciar la
    // pila.
    enum class Peticion { Ninguna, VolverAlMenu };
    Peticion peticion = Peticion::Ninguna;
};