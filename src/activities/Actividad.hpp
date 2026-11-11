#pragma once

// Datos de una actividad que el jugador puede hacer (se leen de
// data/actividades.txt) y el resultado de intentar hacerla.

#include <array>
#include <cstdint>
#include <string>

#include "systems/Estadisticas.hpp"
#include "systems/Tiempo.hpp"

struct DatosActividad
{
    std::string  id;       // identificador sin espacios (p. ej. "dormir")
    std::string  nombre;   // nombre para mostrar (p. ej. "Dormir")

    // Máscaras de bits: un bit por Bloque y un bit por Dia en que se puede hacer.
    std::uint8_t bloques;
    std::uint8_t dias;

    bool gastaBloque;      // true: consume un bloque de tiempo; false: acción rápida

    float deltaEnergia;
    float deltaConocimiento;   // se suma a la materia activa
    float deltaSaludMental;
    float deltaEstres;
    float deltaDopamina;
    float deltaDinero;

    int limiteDiario;      // veces máximas por día (-1 = sin límite)
};

enum class EstadoActividad
{
    Disponible,         // se puede hacer ahora (la consulta puedeHacer)
    Hecha,              // se hizo (resultado de ejecutar)
    NoExiste,
    SinTiempo,          // fin de semana o semestre terminado
    FueraDeHorario,
    LimiteAlcanzado,
    SinDinero
};

struct ResultadoActividad
{
    EstadoActividad estado = EstadoActividad::NoExiste;

    // Si la actividad gastó un bloque, lo que pasó con el tiempo
    // (FinDeSemana, FinSemestre...): quien llama debe reaccionar.
    EventoTiempo evento = EventoTiempo::Ninguno;

    // Cambios realmente aplicados (para los textos flotantes "+10 energía").
    std::array<float, static_cast<int>(Stat::Cantidad)> cambios{};
    float cambioConocimiento = 0.f;

    bool hecha() const { return estado == EstadoActividad::Hecha; }
};