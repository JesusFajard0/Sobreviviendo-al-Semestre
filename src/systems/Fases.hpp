#pragma once

#include <cstdint>
#include <string>

#include <chainedhash.hpp>   // DeSiGNAR: SeparateChainingHashTable

#include "systems/Tiempo.hpp"

class Fases
{
public:
    explicit Fases(const Tiempo& tiempo);

    bool cargarDesdeArchivo(const std::string& ruta);

    bool escenaPermitida(const std::string& escena) const;

    bool permitidaEn(const std::string& escena, Bloque bloque, Dia dia) const;

    void limpiar();

private:
    struct Regla
    {
        std::string  escena;
        std::uint8_t bloques;
        std::uint8_t dias;
    };

    struct IgualEscena
    {
        bool operator()(const Regla& a, const Regla& b) const
        {
            return a.escena == b.escena;
        }
    };

    static Designar::nat_t hashRegla(const Regla& r)
    {
        return Designar::super_fast_hash(r.escena);
    }

    using Tabla = Designar::SeparateChainingHashTable<Regla, IgualEscena>;

    const Tiempo& tiempo_;
    Tabla         reglas_;
};