#pragma once

// Fases: decide qué escenas están abiertas según el momento del juego.
//
// Las reglas se leen de un archivo de texto (data/fases.txt) y se guardan
// por nombre de escena en una tabla hash de DeSiGNAR.
//
// Convención:
//   - Una escena sin regla está siempre abierta.
//   - Durante el fin de semana y al terminar el semestre nada está abierto.
//   - cargarDesdeArchivo devuelve false y escribe en std::cerr si falla.

#include <cstdint>
#include <string>

#include <chainedhash.hpp>   // DeSiGNAR: SeparateChainingHashTable

#include "systems/Tiempo.hpp"

class Fases
{
public:
    // Guarda una referencia: el Tiempo debe existir mientras exista Fases.
    explicit Fases(const Tiempo& tiempo);

    // Lee las reglas. Si alguna línea es inválida se informa en std::cerr,
    // se cargan las demás y se devuelve false.
    bool cargarDesdeArchivo(const std::string& ruta);

    // ¿Se puede entrar a esta escena ahora mismo?
    bool escenaPermitida(const std::string& escena) const;

    // ¿Estaría abierta en esa fase y ese día? (no depende del Tiempo actual)
    bool permitidaEn(const std::string& escena, Fase fase, Dia dia) const;

    // Borra todas las reglas cargadas.
    void limpiar();

private:
    // Elemento de la tabla: el nombre de la escena es la clave.
    // fases y dias son máscaras de bits (un bit por Fase / por Dia).
    struct Regla
    {
        std::string  escena;
        std::uint8_t fases;
        std::uint8_t dias;
    };

    // Dos reglas son "iguales" si tienen la misma escena.
    struct IgualEscena
    {
        bool operator()(const Regla& a, const Regla& b) const
        {
            return a.escena == b.escena;
        }
    };

    // Función hash: se calcula solo sobre el nombre de la escena.
    static Designar::nat_t hashRegla(const Regla& r)
    {
        return Designar::super_fast_hash(r.escena);
    }

    using Tabla = Designar::SeparateChainingHashTable<Regla, IgualEscena>;

    const Tiempo& tiempo_;
    Tabla         reglas_;
};