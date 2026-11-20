#pragma once

#include <string>
#include <vector>

#include "world/DatosMueble.hpp"

// Lee data/muebles.txt: los muebles de cada escena.
class LectorMuebles
{
public:
    // Devuelve false (y escribe en std::cerr) si no se pudo abrir el archivo
    // o si alguna línea es inválida. Las líneas válidas se cargan igual.
    static bool cargarArchivo(
        const std::string& ruta,
        std::vector<DatosMueble>& muebles
    );
};