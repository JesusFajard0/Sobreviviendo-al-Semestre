#include "core/Partida.hpp"

Partida::Partida()
    : fases(tiempo)
{
}

Materia Partida::materiaActiva() const
{
    const int n = static_cast<int>(Materia::Cantidad);

    for (int i = 0; i < n; ++i)
    {
        const Materia m = static_cast<Materia>(i);
        if (tiempo.semana() <= Tiempo::semanaExamen(m))
            return m;
    }

    // Pasadas todas las semanas de examen: la última materia.
    return static_cast<Materia>(n - 1);
}

int Partida::faseSemestre() const
{
    return static_cast<int>(materiaActiva()) + 1;
}