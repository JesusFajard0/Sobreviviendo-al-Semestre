#pragma once

#include <string>

// Un NPC colocado en una escena (se lee de data/npcs.txt).
struct DatosNpc
{
    std::string escena;    // id de la escena donde esta
    std::string nombre;    // se muestra debajo del NPC
    std::string dialogo;   // id de la conversacion (data/dialogos.txt)

    float x;
    float y;
    float ancho;
    float alto;
};
