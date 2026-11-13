#pragma once

#include "systems/Estadisticas.hpp"

// S2-4: colapso por estadisticas criticas y noches sin dormir.
// Solo LEE/MODIFICA Estadisticas por su interfaz publica; no la cambia.
// No toca el tiempo: si hay que perder bloques, lo indica en el resultado
// y quien llama lo avanza con su manejo normal de EventoTiempo.

enum class MotivoColapso
{
    Ninguno,
    Energia,       // energia en el minimo
    Estres,        // estres en el maximo
    SaludMental    // salud mental en el minimo
};

struct ResultadoColapso
{
    bool           huboColapso    = false;
    MotivoColapso  motivo         = MotivoColapso::Ninguno;
    const char*    mensaje        = "";
    int            bloquesPerdidos = 0;   // cuantos bloques debe avanzar quien llama
};

struct ResultadoAmanecer
{
    bool        durmio          = true;
    int         nochesSeguidas  = 0;      // noches seguidas sin dormir (0 si durmio)
    const char* mensaje         = "";
};

class Colapso
{
public:
    void reiniciar();

    // Revisa las estadisticas y, si alguna llego a su extremo, aplica el
    // colapso (deja los valores lejos del limite, asi no se repite en bucle).
    // Llamar despues de cada accion que cambie estadisticas.
    ResultadoColapso evaluar(Estadisticas& est);

    // La accion de dormir llama esto (una linea). Vale para la noche en curso.
    void marcarSueno() { durmioEstaNoche_ = true; }

    // Llamar cuando Tiempo avise que termino una noche (NuevoDia / FinDeSemana).
    // Si no se durmio, aplica la penalizacion y cuenta la noche en vela.
    ResultadoAmanecer alAmanecer(Estadisticas& est);

    int nochesSinDormir() const { return nochesSinDormir_; }

private:
    bool durmioEstaNoche_ = false;
    int  nochesSinDormir_ = 0;
};
