
#pragma once

#include "DatosEscena.hpp"

#include <string>
#include <vector>



class LectorEscenas
{
public:
    static bool cargarArchivo(
        const std::string& ruta,
        std::vector<DatosEscena>& escenas,
        std::vector<DatosRectangulo>& rectangulos
    );
};
