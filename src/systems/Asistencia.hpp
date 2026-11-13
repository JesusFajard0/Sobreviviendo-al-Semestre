#pragma once

#include <array>

#include "systems/Estadisticas.hpp"   // por Materia

// S2-3: contadores de asistencias y faltas por materia.
// Solo cuenta; quien decide cuando se asiste o se falta es EstadoExploracion
// (la actividad "asistir_clase" y el aviso de "Faltaste a clase").
class Asistencia
{
public:
    Asistencia() { reiniciar(); }

    void reiniciar()
    {
        asistencias_.fill(0);
        faltas_.fill(0);
    }

    void registrarAsistencia(Materia m) { ++asistencias_[static_cast<int>(m)]; }
    void registrarFalta(Materia m)      { ++faltas_[static_cast<int>(m)]; }

    int asistencias(Materia m) const { return asistencias_[static_cast<int>(m)]; }
    int faltas(Materia m)      const { return faltas_[static_cast<int>(m)]; }

    int totalAsistencias() const { int s = 0; for (int v : asistencias_) s += v; return s; }
    int totalFaltas()      const { int s = 0; for (int v : faltas_)      s += v; return s; }

private:
    static constexpr int N = static_cast<int>(Materia::Cantidad);

    std::array<int, N> asistencias_;
    std::array<int, N> faltas_;
};
