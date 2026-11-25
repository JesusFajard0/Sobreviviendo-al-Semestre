#pragma once

#include <map>
#include <string>

#include "systems/Notas.hpp"
#include "world/DatosRastro.hpp"

// S3-3: funciones puras del rastro (sin SFML, faciles de probar).

// Dice si el rastro debe aparecer segun como va el examen de su materia.
// "estado" y "aprobado" salen de Notas (estado(m) y aprobada(m)).
bool cumpleCondicion(CondicionRastro condicion, EstadoExamen estado, bool aprobado);

// Reemplaza cada {clave} del texto por su valor. Una clave que no esta en el
// mapa se deja tal cual, para que el error se vea en pantalla.
std::string rellenarPlantilla(const std::string& texto,
                              const std::map<std::string, std::string>& valores);
