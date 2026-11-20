#pragma once

// S2-7: carga los dialogos desde data/dialogos.txt y los guarda en una tabla
// hash de DeSiGNAR (clave: idDialogo), igual que GestorActividades.
//
// Convencion de errores (sin excepciones): cargarDesdeArchivo devuelve false
// y escribe en std::cerr si algo falla; buscar devuelve nullptr si no existe.

#include <string>
#include <vector>

#include <chainedhash.hpp>   // DeSiGNAR: SeparateChainingHashTable

// Una frase de una conversacion.
struct LineaDialogo
{
    std::string hablante;   // puede estar vacio (narracion)
    std::string texto;
};

class GestorDialogos
{
public:
    GestorDialogos();

    // Lee data/dialogos.txt. Si alguna linea es invalida se informa en
    // std::cerr, se cargan las demas y se devuelve false.
    bool cargarDesdeArchivo(const std::string& ruta);

    // Frases de la conversacion en orden, o nullptr si no existe.
    const std::vector<LineaDialogo>* buscar(const std::string& id) const;

    bool existe(const std::string& id) const { return buscar(id) != nullptr; }

    int cantidad() const { return static_cast<int>(ids_.size()); }

private:
    struct Entrada
    {
        std::string               id;
        std::vector<LineaDialogo> lineas;
    };

    struct IgualId
    {
        bool operator()(const Entrada& a, const Entrada& b) const
        {
            return a.id == b.id;
        }
    };

    static Designar::nat_t hashEntrada(const Entrada& e)
    {
        return Designar::super_fast_hash(e.id);
    }

    using Tabla = Designar::SeparateChainingHashTable<Entrada, IgualId>;

    const Entrada* buscarEntrada(const std::string& id) const;

    Tabla                    tabla_;
    std::vector<std::string> ids_;
};
