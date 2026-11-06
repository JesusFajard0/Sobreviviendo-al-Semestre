#include "systems/Tiempo.hpp"

Tiempo::Tiempo()
{
    reiniciar();
}

void Tiempo::reiniciar()
{
    semana_        = 1;
    dia_           = Dia::Lunes;
    fase_          = Fase::Manana;
    enFinDeSemana_ = false;
    terminado_     = false;
}

EventoTiempo Tiempo::avanzarBloque()
{
    // Con el semestre terminado o en pleno fin de semana no se avanza.
    if (terminado_ || enFinDeSemana_)
        return EventoTiempo::Ninguno;

    // Dentro del mismo día: mañana -> tarde -> noche -> madrugada.
    if (fase_ != Fase::Madrugada)
    {
        fase_ = static_cast<Fase>(static_cast<int>(fase_) + 1);
        return EventoTiempo::Ninguno;
    }

    // Terminó la madrugada: empieza otro día.
    fase_ = Fase::Manana;

    if (dia_ != Dia::Viernes)
    {
        dia_ = static_cast<Dia>(static_cast<int>(dia_) + 1);
        return EventoTiempo::NuevoDia;
    }

    // Terminó el viernes: en la última semana se acaba el semestre,
    // si no, llega el fin de semana.
    if (semana_ >= SEMANAS)
    {
        terminado_ = true;
        return EventoTiempo::FinSemestre;
    }

    enFinDeSemana_ = true;
    return EventoTiempo::FinDeSemana;
}

EventoTiempo Tiempo::terminarFinDeSemana()
{
    if (!enFinDeSemana_)
        return EventoTiempo::Ninguno;

    enFinDeSemana_ = false;
    ++semana_;
    dia_  = Dia::Lunes;
    fase_ = Fase::Manana;
    return EventoTiempo::NuevaSemana;
}

bool Tiempo::esViernesNoche() const
{
    return dia_ == Dia::Viernes && fase_ == Fase::Noche;
}

// ---------------------------------------------------------------
// Exámenes
// ---------------------------------------------------------------

int Tiempo::semanaExamen(Materia m)
{
    switch (m)
    {
        case Materia::Calculo:      return 3;
        case Materia::Fisica:       return 5;
        case Materia::Programacion: return 8;
        default:                    return -1;
    }
}

std::optional<Materia> Tiempo::examenDeLaSemana() const
{
    for (int i = 0; i < static_cast<int>(Materia::Cantidad); ++i)
    {
        const Materia m = static_cast<Materia>(i);
        if (semanaExamen(m) == semana_)
            return m;
    }
    return std::nullopt;
}

std::optional<Materia> Tiempo::examenHoy() const
{
    if (dia_ != Dia::Viernes)
        return std::nullopt;

    return examenDeLaSemana();
}

// ---------------------------------------------------------------
// Nombres (para HUD)
// ---------------------------------------------------------------

const char* Tiempo::nombre(Dia d)
{
    switch (d)
    {
        case Dia::Lunes:     return "Lunes";
        case Dia::Martes:    return "Martes";
        case Dia::Miercoles: return "Miercoles";
        case Dia::Jueves:    return "Jueves";
        case Dia::Viernes:   return "Viernes";
        default:             return "?";
    }
}

const char* Tiempo::nombre(Fase f)
{
    switch (f)
    {
        case Fase::Manana:    return "Manana";
        case Fase::Tarde:     return "Tarde";
        case Fase::Noche:     return "Noche";
        case Fase::Madrugada: return "Madrugada";
        default:              return "?";
    }
}