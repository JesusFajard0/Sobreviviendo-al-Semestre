#pragma once

#include "Actividad.hpp"
#include "../systems/Estadisticas.hpp"
#include <vector>
#include <string>
#include <unordered_map>

class GestorActividades {
private:
    std::vector<DatosActividad> actividadesDisponibles;
    std::unordered_map<int, int> contadorDiario; // <id_actividad, veces_realizadas_hoy>

public:
    bool cargarActividades(const std::string& rutaArchivo);
    bool ejecutarActividad(int idActividad, Estadisticas& estadisticas, int bloqueActual, int diaSemana);
    void reiniciarContadoresDiarios();
    
    // Validaciones especiales
    bool esUniversidadAbierta(int idActividad, int bloqueActual, int diaSemana) const;
    void verificarPenalizacionSueño(Estadisticas& estadisticas, bool durmioEnMadrugada);
    void verificarMesadaFamiliar(int semanaActual, int& semanaAnterior, Estadisticas& estadisticas);

    const DatosActividad* obtenerActividad(int idActividad) const;
};