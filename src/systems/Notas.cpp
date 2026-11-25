#include "systems/Notas.hpp"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>

Notas::Notas()
    : azar_(std::random_device{}())
{
    reiniciar();
}

void Notas::reiniciar()
{
    estados_.fill(EstadoExamen::Pendiente);
    notas_.fill(0);
}

// ---------------------------------------------------------------
// Carga: lineas "clave valor" (# = comentario)
// ---------------------------------------------------------------

bool Notas::cargarDesdeArchivo(const std::string& ruta)
{
    std::ifstream archivo(ruta);
    if (!archivo.is_open())
    {
        std::cerr << "[Notas] No se pudo abrir '" << ruta << "' (se usan los valores por defecto)\n";
        return false;
    }

    bool todoBien = true;
    int numeroLinea = 0;
    std::string linea;

    while (std::getline(archivo, linea))
    {
        ++numeroLinea;

        const std::size_t comentario = linea.find('#');
        if (comentario != std::string::npos)
            linea.erase(comentario);

        std::istringstream ss(linea);
        std::string clave;
        if (!(ss >> clave))
            continue;

        float valor = 0.f;
        std::string sobrante;
        if (!(ss >> valor) || (ss >> sobrante))
        {
            std::cerr << "[Notas] Linea " << numeroLinea << ": se esperaba 'clave valor'\n";
            todoBien = false;
            continue;
        }

        if (clave == "peso_conocimiento" && valor >= 0.f)      pesoConocimiento_ = valor;
        else if (clave == "peso_estado" && valor >= 0.f)       pesoEstado_       = valor;
        else if (clave == "peso_azar" && valor >= 0.f)         pesoAzar_         = valor;
        else if (clave == "nota_aprobatoria" && valor >= 0.f && valor <= NOTA_MAXIMA)
            notaAprobatoria_ = static_cast<int>(std::lround(valor));
        else
        {
            std::cerr << "[Notas] Linea " << numeroLinea << ": clave o valor invalido '"
                      << clave << "'\n";
            todoBien = false;
        }
    }

    if (pesoConocimiento_ + pesoEstado_ + pesoAzar_ <= 0.f)
    {
        std::cerr << "[Notas] Los pesos suman 0: se usan los valores por defecto\n";
        pesoConocimiento_ = 75.f;
        pesoEstado_       = 10.f;
        pesoAzar_         = 15.f;
        todoBien = false;
    }

    return todoBien;
}

// ---------------------------------------------------------------
// Calculo de la nota
// ---------------------------------------------------------------

ResultadoExamen Notas::calcular(const Estadisticas& est, Materia m)
{
    ResultadoExamen r;
    r.materia  = m;
    r.presento = true;

    r.conocimiento = est.conocimiento(m);

    // Llegar descansado y tranquilo ayuda: promedio de energia y calma (100 - estres).
    r.estado = std::clamp(
        (est.obtener(Stat::Energia) + (100.f - est.obtener(Stat::Estres))) / 2.f,
        0.f, 100.f);

    r.azar = std::uniform_real_distribution<float>(0.f, 100.f)(azar_);

    const float suma = pesoConocimiento_ + pesoEstado_ + pesoAzar_;
    r.puntaje = (pesoConocimiento_ * r.conocimiento +
                 pesoEstado_       * r.estado +
                 pesoAzar_         * r.azar) / suma;

    r.nota = std::clamp(
        static_cast<int>(std::lround(r.puntaje * NOTA_MAXIMA / 100.f)),
        0, NOTA_MAXIMA);

    r.aprobado = r.nota >= notaAprobatoria_;
    return r;
}

// ---------------------------------------------------------------
// Registro
// ---------------------------------------------------------------

void Notas::registrar(const ResultadoExamen& r)
{
    const int i = static_cast<int>(r.materia);
    estados_[i] = EstadoExamen::Presentado;
    notas_[i]   = r.nota;
}

void Notas::registrarNoPresento(Materia m)
{
    const int i = static_cast<int>(m);
    if (estados_[i] != EstadoExamen::Pendiente)
        return;

    estados_[i] = EstadoExamen::NoPresento;
    notas_[i]   = 0;
}

int Notas::materiasAprobadas() const
{
    int n = 0;
    for (int i = 0; i < N_MATERIAS; ++i)
        if (aprobada(static_cast<Materia>(i)))
            ++n;
    return n;
}

int Notas::promedio() const
{
    int suma = 0;
    for (int i = 0; i < N_MATERIAS; ++i)
        suma += notas_[i];

    return static_cast<int>(std::lround(static_cast<double>(suma) / N_MATERIAS));
}
