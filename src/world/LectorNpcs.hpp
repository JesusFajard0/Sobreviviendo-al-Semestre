#pragma once

#include <string>
#include <vector>

#include "world/DatosNpc.hpp"

// Lee data/npcs.txt: los NPCs quietos de cada escena.
class LectorNpcs
{
public:
    // Devuelve false (y escribe en std::cerr) si no se pudo abrir el archivo
    // o si alguna linea es invalida. Las lineas validas se cargan igual.
    static bool cargarArchivo(const std::string& ruta, std::vector<DatosNpc>& npcs);
};
