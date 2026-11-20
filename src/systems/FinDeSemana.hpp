#pragma once

#include <array>
#include <random>
#include <string>

#include "systems/Estadisticas.hpp"
#include "systems/Tiempo.hpp"

// S2-5: fin de semana automatico, ponderado por estadisticas.
//
// El fin de semana no lo elige el jugador: el juego sortea entre rumba, aseo
// y estudio. Los pesos dependen de como llega el jugador (dopamina, orden del
// apartamento, conocimiento, estres, si fue a la discoteca el viernes y si
// hay examen la semana siguiente). Los efectos de cada opcion viven en
// data/fin_de_semana.txt.
//
// Esta clase NO toca el tiempo ni la mesada: solo lee/modifica Estadisticas
// por su interfaz publica. Quien recibe el EventoTiempo decide cuando llamar.

enum class TipoFinDeSemana
{
    Rumba,
    Aseo,
    Estudio,
    Cantidad
};

struct DatosFinDeSemana
{
    std::string id;       // "rumba", "aseo" o "estudio"
    std::string nombre;
    float deltaEnergia      = 0.f;
    float deltaConocimiento = 0.f;   // va a la materia de la semana siguiente
    float deltaSaludMental  = 0.f;
    float deltaEstres       = 0.f;
    float deltaDopamina     = 0.f;
    float deltaDinero       = 0.f;
    float deltaOrden        = 0.f;   // orden del apartamento
    float pesoBase          = 1.f;
    std::string texto;               // lo que se muestra en la caja de texto
};

struct ResultadoFinDeSemana
{
    TipoFinDeSemana tipo = TipoFinDeSemana::Aseo;
    std::string     nombre;
    std::string     texto;
    // Cambios realmente aplicados (en el orden del enum Stat), para la caja de texto.
    std::array<float, static_cast<int>(Stat::Cantidad)> cambios{};
    float cambioConocimiento = 0.f;
    float cambioOrden        = 0.f;

    // Los llena quien muestra la pantalla (no el sorteo):
    float mesada      = 0.f;   // mesada recibida el lunes
    int   semanaNueva = 0;     // semana que empieza (0 = no se muestra)
};

class FinDeSemana
{
public:
    FinDeSemana();

    // Lee data/fin_de_semana.txt. Si hay lineas invalidas informa en std::cerr,
    // carga las demas y devuelve false. Faltan opciones => false.
    bool cargarDesdeArchivo(const std::string& ruta);

    // Nueva partida.
    void reiniciar();

    // Para pruebas reproducibles.
    void fijarSemilla(unsigned semilla) { azar_.seed(semilla); }

    // --- Lo que el juego avisa durante la semana ---

    // Llamar despues de cada actividad HECHA (ResultadoActividad::hecha()).
    // Lleva el orden del apartamento y recuerda si hubo discoteca.
    void registrarActividad(const std::string& idActividad);

    // Llamar cuando Tiempo avise NuevoDia o FinDeSemana (termino un dia).
    // Si no lavaste los platos hoy, baja el orden del apartamento.
    void cerrarDia();

    // --- Fin de semana ---

    // Llamar cuando ya se hizo tiempo.terminarFinDeSemana(): asi la semana y la
    // materia activa ya son las de la semana que empieza, y estudiar en el fin
    // de semana cuenta para la materia nueva si hubo cambio de fase.
    //   semanaQueViene   = tiempo.semana()   (ya incrementada)
    //   materiaSiguiente = partida.materiaActiva()
    ResultadoFinDeSemana resolver(Estadisticas& est, int semanaQueViene,
                                  Materia materiaSiguiente);

    float ordenCasa() const { return ordenCasa_; }

    // Pesos del sorteo (publico para poder probarlos y mostrarlos al depurar).
    std::array<float, static_cast<int>(TipoFinDeSemana::Cantidad)>
    pesos(const Estadisticas& est, int semanaQueViene, Materia materiaSiguiente) const;

    static const char* nombre(TipoFinDeSemana t);

private:
    std::array<DatosFinDeSemana, static_cast<int>(TipoFinDeSemana::Cantidad)> datos_;
    std::array<bool, static_cast<int>(TipoFinDeSemana::Cantidad)>            cargado_{};

    float ordenCasa_    = 80.f;   // 0 a 100
    bool  lavoHoy_      = false;
    bool  fueDiscoteca_ = false;  // desde el ultimo fin de semana

    std::mt19937 azar_;
};
