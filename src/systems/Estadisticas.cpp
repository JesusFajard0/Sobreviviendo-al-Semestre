#include "systems/Estadisticas.hpp"

#include <algorithm>
#include <limits>

namespace
{
    constexpr float SIN_LIMITE = std::numeric_limits<float>::infinity();

    struct Config
    {
        float minimo;
        float maximo;
        float inicial;
        float umbralCritico;
        bool  criticoAlto;   // true: es crítica al subir del umbral; false: al bajar
    };

    // Valores de partida: están para probar y se pueden balancear después.
    const std::array<Config, static_cast<int>(Stat::Cantidad)> TABLA = {{
        /* SaludMental */ {0.f, 100.f,      70.f, 20.f, false},
        /* Energia     */ {0.f, 100.f,      80.f, 15.f, false},
        /* Estres      */ {0.f, 100.f,      20.f, 85.f, true },
        /* Dopamina    */ {0.f, 100.f,      50.f, 15.f, false},
        /* Dinero      */ {0.f, SIN_LIMITE, 100.f, 10.f, false}
    }};

    const Config& config(Stat s) { return TABLA[static_cast<int>(s)]; }
}

Estadisticas::Estadisticas()
{
    reiniciar();
}

void Estadisticas::reiniciar()
{
    for (int i = 0; i < N_STATS; ++i)
        valores_[i] = TABLA[i].inicial;

    conocimientos_.fill(0.f);
}

// ---------------------------------------------------------------
// Estadísticas generales
// ---------------------------------------------------------------

float Estadisticas::obtener(Stat s) const
{
    return valores_[static_cast<int>(s)];
}

void Estadisticas::establecer(Stat s, float valor)
{
    const Config& c = config(s);
    valores_[static_cast<int>(s)] = std::clamp(valor, c.minimo, c.maximo);
}

float Estadisticas::modificar(Stat s, float delta)
{
    const float antes = obtener(s);
    establecer(s, antes + delta);
    return obtener(s) - antes;
}

float Estadisticas::minimo(Stat s) const { return config(s).minimo; }
float Estadisticas::maximo(Stat s) const { return config(s).maximo; }

bool Estadisticas::esCritica(Stat s) const
{
    const Config& c = config(s);
    const float v = obtener(s);
    return c.criticoAlto ? (v >= c.umbralCritico) : (v <= c.umbralCritico);
}

// ---------------------------------------------------------------
// Conocimiento por materia
// ---------------------------------------------------------------

float Estadisticas::conocimiento(Materia m) const
{
    return conocimientos_[static_cast<int>(m)];
}

float Estadisticas::modificarConocimiento(Materia m, float delta)
{
    float& valor = conocimientos_[static_cast<int>(m)];
    const float antes = valor;
    valor = std::clamp(antes + delta, 0.f, 100.f);
    return valor - antes;
}

// ---------------------------------------------------------------
// Nombres (para HUD y resumen final)
// ---------------------------------------------------------------

const char* Estadisticas::nombre(Stat s)
{
    switch (s)
    {
        case Stat::SaludMental: return "Salud mental";
        case Stat::Energia:     return "Energia";
        case Stat::Estres:      return "Estres";
        case Stat::Dopamina:    return "Dopamina";
        case Stat::Dinero:      return "Dinero";
        default:                return "?";
    }
}

const char* Estadisticas::nombre(Materia m)
{
    switch (m)
    {
        case Materia::Calculo:      return "Calculo";
        case Materia::Fisica:       return "Fisica";
        case Materia::Programacion: return "Programacion";
        default:                    return "?";
    }
}