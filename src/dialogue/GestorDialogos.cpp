#include "dialogue/GestorDialogos.hpp"

#include <cctype>
#include <fstream>
#include <iostream>
#include <utility>

namespace
{
    constexpr int N_COLUMNAS = 4;   // dialogo | id | hablante | texto

    std::string recortar(const std::string& s)
    {
        std::size_t ini = 0;
        std::size_t fin = s.size();
        while (ini < fin && std::isspace(static_cast<unsigned char>(s[ini])))
            ++ini;
        while (fin > ini && std::isspace(static_cast<unsigned char>(s[fin - 1])))
            --fin;
        return s.substr(ini, fin - ini);
    }

    // Divide por el separador conservando los campos vacios.
    std::vector<std::string> dividir(const std::string& texto, char separador)
    {
        std::vector<std::string> partes;
        std::size_t inicio = 0;
        while (true)
        {
            const std::size_t pos = texto.find(separador, inicio);
            if (pos == std::string::npos)
            {
                partes.push_back(recortar(texto.substr(inicio)));
                break;
            }
            partes.push_back(recortar(texto.substr(inicio, pos - inicio)));
            inicio = pos + 1;
        }
        return partes;
    }
}

GestorDialogos::GestorDialogos()
    : tabla_(IgualId(), &hashEntrada)
{
}

const GestorDialogos::Entrada* GestorDialogos::buscarEntrada(const std::string& id) const
{
    const Entrada sonda{id, {}};
    return tabla_.search(sonda);
}

const std::vector<LineaDialogo>* GestorDialogos::buscar(const std::string& id) const
{
    const Entrada* e = buscarEntrada(id);
    return e != nullptr ? &e->lineas : nullptr;
}

bool GestorDialogos::cargarDesdeArchivo(const std::string& ruta)
{
    std::ifstream archivo(ruta);
    if (!archivo.is_open())
    {
        std::cerr << "[GestorDialogos] No se pudo abrir '" << ruta << "'\n";
        return false;
    }

    // Primero se agrupan las frases por id (en orden de aparicion) y al final
    // se insertan en la tabla: asi una conversacion puede ir en varias lineas.
    std::vector<Entrada> grupos;

    bool todoBien = true;
    int numeroLinea = 0;
    std::string linea;

    while (std::getline(archivo, linea))
    {
        ++numeroLinea;

        linea = recortar(linea);
        if (linea.empty() || linea[0] == '#')
            continue;

        const std::vector<std::string> c = dividir(linea, '|');

        if (static_cast<int>(c.size()) != N_COLUMNAS || c[0] != "dialogo")
        {
            std::cerr << "[GestorDialogos] Linea " << numeroLinea
                      << ": se esperaba 'dialogo | idDialogo | hablante | texto'\n";
            todoBien = false;
            continue;
        }

        if (c[1].empty() || c[3].empty())
        {
            std::cerr << "[GestorDialogos] Linea " << numeroLinea
                      << ": el id y el texto no pueden estar vacios\n";
            todoBien = false;
            continue;
        }

        Entrada* grupo = nullptr;
        for (Entrada& g : grupos)
        {
            if (g.id == c[1])
            {
                grupo = &g;
                break;
            }
        }

        if (grupo == nullptr)
        {
            grupos.push_back(Entrada{c[1], {}});
            grupo = &grupos.back();
        }

        grupo->lineas.push_back(LineaDialogo{c[2], c[3]});
    }

    for (Entrada& g : grupos)
    {
        ids_.push_back(g.id);
        tabla_.insert(std::move(g));
    }

    return todoBien;
}
