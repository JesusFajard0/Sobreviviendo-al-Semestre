#pragma once

// S3-5: los finales del juego. Las reglas y TODOS sus umbrales se leen de
// data/finales.txt: no hay numeros de reglas en el codigo. Cada final tiene un
// nombre, un texto, una imagen y sus condiciones, y cada partida debe caer en
// exactamente uno (se revisa al cargar).
//
// Convencion de errores (sin excepciones): cargarDesdeArchivo devuelve false y
// escribe en std::cerr si algo falla; las lineas validas se cargan igual.

#include <array>
#include <map>
#include <string>
#include <vector>

#include "systems/Estadisticas.hpp"
#include "systems/Notas.hpp"

// Contadores de la partida que no salen de Notas ni de Estadisticas.
struct DatosPartida
{
    int asistencias     = 0;
    int faltas          = 0;
    int fiestas         = 0;
    int platos          = 0;
    int nochesSinDormir = 0;   // en total, no seguidas
};

// Todo lo que muestran las pantallas de resumen y de final.
struct ResumenSemestre
{
    static constexpr int N = static_cast<int>(Materia::Cantidad);

    std::array<int, N>          nota{};
    std::array<EstadoExamen, N> estado{};
    std::array<bool, N>         aprobada{};   // segun Notas (nota minima de aprobar)
    int promedio  = 0;
    int aprobadas = 0;

    float saludMental = 0.f;
    float dopamina    = 0.f;
    float dinero      = 0.f;

    int asistencias     = 0;
    int faltas          = 0;
    int fiestas         = 0;
    int platos          = 0;
    int nochesSinDormir = 0;

    std::string idFinal;
    std::string nombreFinal;
    std::string textoFinal;
    std::string imagenFinal;   // ruta; puede no existir
};

class Finales
{
public:
    // Valores con los que se evaluan las reglas (la clave es el nombre que se
    // escribe en data/finales.txt).
    using Metricas = std::map<std::string, double>;

    enum class Operador { Mayor, MayorIgual, Menor, MenorIgual, Igual, Distinto };

    struct Condicion
    {
        std::string metrica;
        Operador    operador;
        double      valor;    // si "umbral" esta vacio
        std::string umbral;   // nombre de un umbral del archivo (si no, es un numero)
    };

    struct Regla
    {
        std::string            id;
        std::string            nombre;
        std::string            imagen;
        std::vector<Condicion> condiciones;   // vacio = siempre
        std::string            texto;
    };

    // Lee data/finales.txt (umbrales y finales). Si alguna linea es invalida se
    // informa en std::cerr, se cargan las demas y se devuelve false.
    bool cargarDesdeArchivo(const std::string& ruta);

    int cantidad() const { return static_cast<int>(reglas_.size()); }
    const std::vector<Regla>& reglas() const { return reglas_; }
    const std::map<std::string, double>& umbrales() const { return umbrales_; }

    // Metricas de una partida terminada. "estadisticas_buenas" vale 1 si a la
    // vez salud_mental, dopamina y noches_sin_dormir cumplen sus umbrales.
    Metricas metricas(const Notas& notas, const Estadisticas& est, const DatosPartida& datos) const;

    // La primera regla cuyas condiciones se cumplen, o nullptr si ninguna.
    const Regla* elegir(const Metricas& m) const;

    // Arma el resumen completo (notas, estadisticas y el final que le toca).
    ResumenSemestre resumir(const Notas& notas, const Estadisticas& est, const DatosPartida& datos) const;

    // Revisa que cada partida posible caiga en EXACTAMENTE un final. Devuelve
    // cuantas combinaciones no cumplen (0 = todo bien) y, si se pide, describe
    // la primera que falla.
    int revisarCobertura(std::string* ejemplo = nullptr) const;

    // Reemplaza cada {metrica} del texto por su valor (redondeado).
    static std::string rellenar(const std::string& texto, const Metricas& m);

private:
    double valorDe(const Condicion& c) const;
    bool   cumple(const Condicion& c, const Metricas& m) const;
    int    coincidencias(const Metricas& m) const;
    double umbral(const std::string& nombre) const;   // 0 si no esta definido

    std::vector<Regla>            reglas_;
    std::map<std::string, double> umbrales_;
};
