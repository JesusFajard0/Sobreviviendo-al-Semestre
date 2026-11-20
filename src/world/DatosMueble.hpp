#pragma once

#include <string>

// Un mueble colocado en una escena (se lee de data/muebles.txt).
struct DatosMueble
{
    std::string escena;   // id de la escena
    std::string sprite;   // nombre del PNG en assets/sprites (sin .png)

    float x;
    float y;
    float escala;
    float baseAlto;
};