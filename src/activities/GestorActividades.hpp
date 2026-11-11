#pragma once

// Gestor de actividades: carga el catálogo desde un archivo de texto, valida
// si una actividad se puede hacer ahora y aplica sus efectos.
//
// Convención de errores (sin excepciones):
//   - cargarDesdeArchivo devuelve false y escribe en std::cerr si algo falla.
//   - ejecutar devuelve un ResultadoActividad con el motivo si no se pudo.

#include <string>
#include <vector>

#include <chainedhash.hpp>   // DeSiGNAR: SeparateChainingHashTable

#include "activities/Actividad.hpp"
#include "systems/Estadisticas.hpp"
#include "systems/Tiempo.hpp"

class GestorActividades
{
public:
    GestorActividades();

    // Lee data/actividades.txt. Si alguna línea es inválida se informa en
    // std::cerr, se cargan las demás y se devuelve false.
    bool cargarDesdeArchivo(const std::string& ruta);

    // Devuelve la actividad o nullptr si no existe.
    const DatosActividad* buscar(const std::string& id) const;

    // Dice si se podría hacer ahora, sin hacerla (para mostrar opciones).
    EstadoActividad puedeHacer(const std::string& id,
                               const Estadisticas& estadisticas,
                               const Tiempo& tiempo) const;

    // Intenta hacer la actividad: valida, aplica los efectos y, si gasta
    // bloque, avanza el tiempo. El conocimiento va a materiaActiva.
    ResultadoActividad ejecutar(const std::string& id,
                                Estadisticas& estadisticas,
                                Tiempo& tiempo,
                                Materia materiaActiva);

    // Veces que se hizo hoy. Devuelve 0 si la actividad no existe.
    int usosHoy(const std::string& id) const;

    // Pone en cero los contadores diarios (se llama solo cuando cambia el día).
    void reiniciarContadores();

    // Dinero que envían los padres cada semana. Lo llama quien reciba
    // el evento NuevaSemana.
    static constexpr float MESADA = 100.f;
    static float aplicarMesada(Estadisticas& estadisticas);

    // Texto corto para mostrar al jugador cuando no se pudo.
    static const char* mensaje(EstadoActividad estado);

private:
    // Elemento de la tabla: el id es la clave. usosHoy es "mutable" para poder
    // contarlo aunque la búsqueda devuelva un puntero constante.
    struct Entrada
    {
        DatosActividad datos;
        mutable int    usosHoy;
    };

    // Dos entradas son "iguales" si tienen el mismo id.
    struct IgualId
    {
        bool operator()(const Entrada& a, const Entrada& b) const
        {
            return a.datos.id == b.datos.id;
        }
    };

    // Función hash: se calcula solo sobre el id.
    static Designar::nat_t hashEntrada(const Entrada& e)
    {
        return Designar::super_fast_hash(e.datos.id);
    }

    using Tabla = Designar::SeparateChainingHashTable<Entrada, IgualId>;

    const Entrada* buscarEntrada(const std::string& id) const;
    EstadoActividad validar(const Entrada& entrada,
                            const Estadisticas& estadisticas,
                            const Tiempo& tiempo) const;

    Tabla                    tabla_;
    std::vector<std::string> ids_;   // para recorrer y reiniciar contadores
};