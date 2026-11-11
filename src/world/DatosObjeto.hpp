#pragma once

#include <string>

// Un objeto de actividad colocado en una escena (se lee de data/objetos.txt).
struct DatosObjeto
{
    std::string escena;      // id de la escena donde está
    std::string actividad;   // id de la actividad (data/actividades.txt)

    float x;
    float y;
    float ancho;
    float alto;
};