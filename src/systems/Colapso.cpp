#include "systems/Colapso.hpp"

namespace
{
    // Valores de partida para probar; se pueden balancear (o pasar a data/).
    constexpr float ENERGIA_TRAS_COLAPSO = 25.f;
    constexpr float ESTRES_TRAS_COLAPSO  = 60.f;
    constexpr float SALUD_TRAS_COLAPSO   = 25.f;
    constexpr int   BLOQUES_PERDIDOS     = 1;

    constexpr float NOCHE_EN_VELA_ENERGIA = -10.f;
    constexpr float NOCHE_EN_VELA_ESTRES  =  10.f;
    constexpr int   NOCHES_PARA_DANO_MENTAL = 2;   // desde la 2a seguida
    constexpr float DANO_MENTAL_SALUD     = -10.f;
    constexpr float DANO_MENTAL_DOPAMINA  = -10.f;
}

void Colapso::reiniciar()
{
    durmioEstaNoche_ = false;
    nochesSinDormir_ = 0;
}

ResultadoColapso Colapso::evaluar(Estadisticas& est)
{
    ResultadoColapso r;

    if (est.obtener(Stat::Energia) <= est.minimo(Stat::Energia))
    {
        est.establecer(Stat::Energia, ENERGIA_TRAS_COLAPSO);
        est.modificar(Stat::Estres, 10.f);
        est.modificar(Stat::Dinero, -20.f);    // gasto de emergencia
        r.motivo  = MotivoColapso::Energia;
        r.mensaje = "Te desmayaste de cansancio";
    }
    else if (est.obtener(Stat::Estres) >= est.maximo(Stat::Estres))
    {
        est.establecer(Stat::Estres, ESTRES_TRAS_COLAPSO);
        est.modificar(Stat::SaludMental, -15.f);
        r.motivo  = MotivoColapso::Estres;
        r.mensaje = "Colapso nervioso";
    }
    else if (est.obtener(Stat::SaludMental) <= est.minimo(Stat::SaludMental))
    {
        est.establecer(Stat::SaludMental, SALUD_TRAS_COLAPSO);
        est.modificar(Stat::Energia, -20.f);
        r.motivo  = MotivoColapso::SaludMental;
        r.mensaje = "Necesitas parar un momento";
    }
    else
    {
        return r;   // todo en orden
    }

    r.huboColapso     = true;
    r.bloquesPerdidos = BLOQUES_PERDIDOS;
    return r;
}

ResultadoAmanecer Colapso::alAmanecer(Estadisticas& est)
{
    ResultadoAmanecer r;

    if (durmioEstaNoche_)
    {
        nochesSinDormir_ = 0;
        durmioEstaNoche_ = false;
        return r;
    }

    ++nochesSinDormir_;
    est.modificar(Stat::Energia, NOCHE_EN_VELA_ENERGIA);
    est.modificar(Stat::Estres,  NOCHE_EN_VELA_ESTRES);

    if (nochesSinDormir_ >= NOCHES_PARA_DANO_MENTAL)
    {
        est.modificar(Stat::SaludMental, DANO_MENTAL_SALUD);
        est.modificar(Stat::Dopamina,    DANO_MENTAL_DOPAMINA);
        r.mensaje = "Llevas varias noches sin dormir";
    }
    else
    {
        r.mensaje = "Pasaste la noche en vela";
    }

    r.durmio         = false;
    r.nochesSeguidas = nochesSinDormir_;
    return r;
}
