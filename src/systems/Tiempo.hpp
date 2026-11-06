#pragma once

#include <optional>

#include "systems/Estadisticas.hpp"   // por Materia

// Momentos del día. Son también las "fases" que usan las puertas.
enum class Fase
{
    Manana,
    Tarde,
    Noche,
    Madrugada,
    Cantidad
};

// Solo se juegan días de semana; el fin de semana es una animación.
enum class Dia
{
    Lunes,
    Martes,
    Miercoles,
    Jueves,
    Viernes,
    Cantidad
};

// Lo que pasó al avanzar el tiempo (para que el estado de juego reaccione).
enum class EventoTiempo
{
    Ninguno,
    NuevoDia,
    FinDeSemana,   // reproducir animación y luego llamar a terminarFinDeSemana()
    NuevaSemana,
    FinSemestre    // terminó el viernes de la última semana: animación final
};

class Tiempo
{
public:
    static constexpr int SEMANAS = 8;

    Tiempo();

    // Vuelve al lunes de la semana 1, por la mañana (nueva partida).
    void reiniciar();

    // Avanza un bloque. Las acciones que no gastan tiempo no lo llaman.
    EventoTiempo avanzarBloque();

    // Se llama al terminar la animación del fin de semana: pasa al lunes
    // de la siguiente semana.
    EventoTiempo terminarFinDeSemana();

    int  semana() const { return semana_; }   // 1 a SEMANAS
    Dia  dia()    const { return dia_; }
    Fase fase()   const { return fase_; }

    bool enFinDeSemana()     const { return enFinDeSemana_; }
    bool semestreTerminado() const { return terminado_; }

    // La discoteca solo existe en este momento.
    bool esViernesNoche() const;

    // --- Exámenes (caen el viernes de su semana) ---

    // Semana en que cae el examen de cada materia.
    static int semanaExamen(Materia m);

    // Materia cuyo examen cae en la semana actual (vacío si no hay).
    std::optional<Materia> examenDeLaSemana() const;

    // Materia cuyo examen es hoy (vacío si hoy no hay examen).
    std::optional<Materia> examenHoy() const;

    static const char* nombre(Dia d);
    static const char* nombre(Fase f);

private:
    int  semana_;
    Dia  dia_;
    Fase fase_;
    bool enFinDeSemana_;
    bool terminado_;
};