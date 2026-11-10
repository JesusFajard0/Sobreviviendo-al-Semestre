#pragma once

#include <string>

struct DatosActividad {
    int id;
    std::string nombre;
    int bloquePermitido;     // 0=Mañana, 1=Tarde, 2=Noche, 3=Madrugada, 4=Viernes Noche, -1=Cualquiera
    bool cambiaFase;         // true si consume el bloque de tiempo, false si es acción rápida
    float deltaEnergia;
    float deltaConocimiento;
    float deltaSaludMental;
    float deltaEstres;
    float deltaDopamina;
    float deltaDinero;
    int limiteDiario;        // Veces máximas que se puede hacer al día (-1 si ilimitado)
};