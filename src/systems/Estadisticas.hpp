#pragma once

#include <array>

// Estadísticas del jugador que se administran durante el semestre.
enum class Stat
{
    SaludMental,
    Energia,
    Estres,
    Dopamina,
    Dinero,
    Cantidad   // no es una estadística: solo cuenta cuántas hay
};

// Materias, cada una con su nivel de conocimiento.
enum class Materia
{
    Calculo,
    Fisica,
    Programacion,
    Cantidad
};

class Estadisticas
{
public:
    Estadisticas();

    // Vuelve todo a los valores iniciales (nueva partida).
    void reiniciar();

    // --- Estadísticas generales ---
    float obtener(Stat s) const;
    void  establecer(Stat s, float valor);

    // Suma (o resta, si es negativo) respetando los límites.
    // Devuelve el cambio realmente aplicado.
    float modificar(Stat s, float delta);

    float minimo(Stat s) const;
    float maximo(Stat s) const;   // el dinero no tiene máximo (infinito)

    // true si la estadística está en nivel crítico
    // (p. ej. estrés muy alto o energía muy baja).
    bool esCritica(Stat s) const;

    // --- Conocimiento por materia (0 a 100) ---
    float conocimiento(Materia m) const;
    float modificarConocimiento(Materia m, float delta);

    static const char* nombre(Stat s);
    static const char* nombre(Materia m);

private:
    static constexpr int N_STATS    = static_cast<int>(Stat::Cantidad);
    static constexpr int N_MATERIAS = static_cast<int>(Materia::Cantidad);

    std::array<float, N_STATS>    valores_;
    std::array<float, N_MATERIAS> conocimientos_;
};