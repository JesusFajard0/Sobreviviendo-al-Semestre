#pragma once

#include <string>
#include <vector>

#include "world/DatosRastro.hpp"

// Lee data/rastros.txt: el rastro de cada salon y los avisos de cambio de fase.
class LectorRastros
{
public:
    // Devuelve false (y escribe en std::cerr) si no se pudo abrir el archivo
    // o si alguna linea es invalida. Las lineas validas se cargan igual.
    static bool cargarArchivo(const std::string& ruta,
                              std::vector<DatosRastro>& rastros,
                              std::vector<MensajeFase>& fases);
};
