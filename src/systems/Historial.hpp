#pragma once

// S3-5: contadores de toda la partida que el resumen del semestre necesita y
// que no vivian en ningun otro sitio (las asistencias y faltas estan en
// Asistencia y las noches sin dormir en Colapso).
//
// Solo cuenta; quien decide cuando algo cuenta es EstadoExploracion:
//   - fiesta:  una ida a la discoteca o un fin de semana de rumba
//   - platos:  la actividad "lavar_platos" o un fin de semana de aseo
class Historial
{
public:
    void reiniciar()
    {
        fiestas_ = 0;
        platos_  = 0;
    }

    void registrarFiesta() { ++fiestas_; }
    void registrarPlatos() { ++platos_; }

    int fiestas() const { return fiestas_; }
    int platos()  const { return platos_; }

private:
    int fiestas_ = 0;
    int platos_  = 0;
};
