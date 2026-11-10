#include "GestorActividades.hpp"
#include <fstream>
#include <sstream>
#include <iostream>

bool GestorActividades::cargarActividades(const std::string& rutaArchivo) {
    std::ifstream archivo(rutaArchivo);
    if (!archivo.is_open()) {
        std::cerr << "[GestorActividades] No se pudo abrir: " << rutaArchivo << std::endl;
        return false;
    }

    std::string linea;
    while (std::getline(archivo, linea)) {
        if (linea.empty() || linea[0] == '#') continue; // Omitir comentarios

        std::stringstream ss(linea);
        std::string segmento;
        DatosActividad act;

        std::getline(ss, segmento, '|'); act.id = std::stoi(segmento);
        std::getline(ss, segmento, '|'); act.nombre = segmento;
        std::getline(ss, segmento, '|'); act.bloquePermitido = std::stoi(segmento);
        std::getline(ss, segmento, '|'); act.cambiaFase = (std::stoi(segmento) == 1);
        std::getline(ss, segmento, '|'); act.deltaEnergia = std::stof(segmento);
        std::getline(ss, segmento, '|'); act.deltaConocimiento = std::stof(segmento);
        std::getline(ss, segmento, '|'); act.deltaSaludMental = std::stof(segmento);
        std::getline(ss, segmento, '|'); act.deltaEstres = std::stof(segmento);
        std::getline(ss, segmento, '|'); act.deltaDopamina = std::stof(segmento);
        std::getline(ss, segmento, '|'); act.deltaDinero = std::stof(segmento);
        std::getline(ss, segmento, '|'); act.limiteDiario = std::stoi(segmento);

        actividadesDisponibles.push_back(act);
    }
    return true;
}

bool GestorActividades::esUniversidadAbierta(int idActividad, int bloqueActual, int diaSemana) const {
    // Actividades de la universidad: 10 (Clase), 11 (Biblioteca), 12 (Cafetería), 13 (Patio)
    if (idActividad >= 10 && idActividad <= 13) {
        if (bloqueActual >= 2) { // Noche (2) o Madrugada (3)
            std::cout << "[Universidad] Cerrada a esta hora.\n";
            return false;
        }
    }
    return true;
}

bool GestorActividades::ejecutarActividad(int idActividad, Estadisticas& estadisticas, int bloqueActual, int diaSemana) {
    const DatosActividad* act = obtenerActividad(idActividad);
    if (!act) return false;

    // Validar si la universidad está cerrada
    if (!esUniversidadAbierta(idActividad, bloqueActual, diaSemana)) {
        return false;
    }

    // Validar límite diario
    if (act->limiteDiario != -1) {
        if (contadorDiario[idActividad] >= act->limiteDiario) {
            std::cout << "[Actividad] ¡Ya alcanzaste el límite diario para esta acción!\n";
            return false;
        }
    }

    // Aplicar efectos a las estadísticas generales
    estadisticas.modificar(Stat::Energia, act->deltaEnergia);
    estadisticas.modificar(Stat::SaludMental, act->deltaSaludMental);
    estadisticas.modificar(Stat::Estres, act->deltaEstres);
    estadisticas.modificar(Stat::Dopamina, act->deltaDopamina);
    estadisticas.modificar(Stat::Dinero, act->deltaDinero);

    // Si da conocimiento, aplicarlo a la materia genérica o cálculo por defecto
    if (act->deltaConocimiento > 0) {
        estadisticas.modificarConocimiento(Materia::Calculo, act->deltaConocimiento);
    }

    // Incrementar uso diario
    contadorDiario[idActividad]++;
    return true;
}

void GestorActividades::reiniciarContadoresDiarios() {
    contadorDiario.clear();
}

void GestorActividades::verificarPenalizacionSueño(Estadisticas& estadisticas, bool durmioEnMadrugada) {
    if (!durmioEnMadrugada) {
        estadisticas.modificar(Stat::Energia, -40.0f);
        estadisticas.modificar(Stat::SaludMental, -20.0f);
        estadisticas.modificar(Stat::Estres, 30.0f);
        std::cout << "[Tiempo] ¡No dormiste en la madrugada! Estás exhausto y el estrés se disparó.\n";
    }
}

void GestorActividades::verificarMesadaFamiliar(int semanaActual, int& semanaAnterior, Estadisticas& estadisticas) {
    if (semanaActual > semanaAnterior) {
        float mesada = 100.0f;
        estadisticas.modificar(Stat::Dinero, mesada);
        std::cout << "[Economía] ¡Tus padres te enviaron la mesada semanal de $" << mesada << "!\n";
        semanaAnterior = semanaActual;
    }
}

const DatosActividad* GestorActividades::obtenerActividad(int idActividad) const {
    for (const auto& act : actividadesDisponibles) {
        if (act.id == idActividad) return &act;
    }
    return nullptr;
}