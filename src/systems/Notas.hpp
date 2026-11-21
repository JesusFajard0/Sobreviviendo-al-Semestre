#pragma once

#include <array>
#include <random>
#include <string>

#include "systems/Estadisticas.hpp"

// S2-6: notas de los examenes.
//
// Hay un examen por materia (Calculo, Fisica, Programacion). La nota es un
// numero entero sobre 20 y se aprueba con 10 o mas. Sale de un puntaje de 0 a
// 100 con tres partes (los pesos viven en data/notas.txt):
//   - conocimiento de la materia
//   - estado del jugador al llegar al examen (energia y estres)
//   - azar
// La asistencia a clase NO entra en la nota: ya cuenta dentro del conocimiento.
//
// Esta clase solo calcula y guarda; no toca el tiempo ni las estadisticas.
// Vive en Partida para que el rastro de los salones y el resumen final puedan
// leer las notas.

enum class EstadoExamen
{
    Pendiente,    // todavia no llega el examen
    Presentado,   // se presento: hay nota
    NoPresento    // no se presento: nota 0
};

struct ResultadoExamen
{
    Materia materia      = Materia::Calculo;
    bool    presento     = true;
    int     nota         = 0;      // 0 a 20
    bool    aprobado     = false;

    // Para mostrar de donde salio la nota (cada parte de 0 a 100).
    float   conocimiento = 0.f;
    float   estado       = 0.f;
    float   azar         = 0.f;
    float   puntaje      = 0.f;    // ponderado, 0 a 100
};

class Notas
{
public:
    static constexpr int NOTA_MAXIMA = 20;

    Notas();

    // Lee data/notas.txt (pesos y nota minima). Si hay lineas invalidas informa
    // en std::cerr, usa los valores por defecto en esas y devuelve false.
    bool cargarDesdeArchivo(const std::string& ruta);

    // Nueva partida: borra todas las notas.
    void reiniciar();

    // Para pruebas reproducibles.
    void fijarSemilla(unsigned semilla) { azar_.seed(semilla); }

    // Calcula la nota con las estadisticas ACTUALES, sin guardarla. Llamar justo
    // antes de aplicar los efectos del examen (que gastan energia y suben estres).
    ResultadoExamen calcular(const Estadisticas& est, Materia m);

    // Guarda la nota de un examen presentado.
    void registrar(const ResultadoExamen& r);

    // El examen paso sin presentarse: nota 0 (no pisa una nota ya guardada).
    void registrarNoPresento(Materia m);

    EstadoExamen estado(Materia m) const { return estados_[static_cast<int>(m)]; }
    bool tieneNota(Materia m) const { return estado(m) != EstadoExamen::Pendiente; }
    int  nota(Materia m) const { return notas_[static_cast<int>(m)]; }
    bool aprobada(Materia m) const { return tieneNota(m) && nota(m) >= notaAprobatoria_; }

    int materiasAprobadas() const;

    // Promedio simple de las tres notas, redondeado (las pendientes cuentan 0).
    int promedio() const;

    int notaAprobatoria() const { return notaAprobatoria_; }

private:
    static constexpr int N_MATERIAS = static_cast<int>(Materia::Cantidad);

    std::array<EstadoExamen, N_MATERIAS> estados_{};
    std::array<int, N_MATERIAS>          notas_{};

    // Valores por defecto (se pisan con data/notas.txt).
    float pesoConocimiento_ = 75.f;
    float pesoEstado_       = 10.f;
    float pesoAzar_         = 15.f;
    int   notaAprobatoria_  = 10;

    std::mt19937 azar_;
};
