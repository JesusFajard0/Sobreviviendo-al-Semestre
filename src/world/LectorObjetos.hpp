#pragma once

#include <string>
#include <vector>

#include "world/DatosObjeto.hpp"

// Lee data/objetos.txt: los objetos de actividad de cada escena.
class LectorObjetos
{
public:
    // Devuelve false (y escribe en std::cerr) si no se pudo abrir el archivo
    // o si alguna línea es inválida. Las líneas válidas se cargan igual.
    static bool cargarArchivo(
        const std::string& ruta,
        std::vector<DatosObjeto>& objetos
    );
};