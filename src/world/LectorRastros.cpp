#include "world/LectorRastros.hpp"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <iostream>

namespace
{
    // rastro | escena | materia | condicion | tipo | x | y | ancho | alto | estilo | nombre | texto
    constexpr int COLUMNAS_RASTRO = 12;
    constexpr int COLUMNAS_FASE   = 3;    // fase | numero | texto

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

    std::string minusculas(std::string s)
    {
        std::transform(s.begin(), s.end(), s.begin(),
                       [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        return s;
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

    // En el archivo el salto de linea se escribe \n (dos caracteres).
    std::string conSaltos(const std::string& s)
    {
        std::string r;
        for (std::size_t i = 0; i < s.size(); ++i)
        {
            if (s[i] == '\\' && i + 1 < s.size() && s[i + 1] == 'n')
            {
                r += '\n';
                ++i;
            }
            else
            {
                r += s[i];
            }
        }
        return r;
    }

    bool leerFloat(const std::string& texto, float& valor)
    {
        if (texto.empty())
            return false;
        char* fin = nullptr;
        const double v = std::strtod(texto.c_str(), &fin);
        if (fin == texto.c_str() || *fin != '\0')
            return false;
        valor = static_cast<float>(v);
        return true;
    }

    bool leerEntero(const std::string& texto, int& valor)
    {
        if (texto.empty())
            return false;
        char* fin = nullptr;
        const long v = std::strtol(texto.c_str(), &fin, 10);
        if (fin == texto.c_str() || *fin != '\0')
            return false;
        valor = static_cast<int>(v);
        return true;
    }

    bool leerMateria(const std::string& texto, Materia& m)
    {
        const std::string t = minusculas(texto);
        if (t == "calculo")      { m = Materia::Calculo;      return true; }
        if (t == "fisica")       { m = Materia::Fisica;       return true; }
        if (t == "programacion") { m = Materia::Programacion; return true; }
        return false;
    }

    bool leerCondicion(const std::string& texto, CondicionRastro& c)
    {
        const std::string t = minusculas(texto);
        if (t == "aprobado")    { c = CondicionRastro::Aprobado;   return true; }
        if (t == "reprobado")   { c = CondicionRastro::Reprobado;  return true; }
        if (t == "no_presento") { c = CondicionRastro::NoPresento; return true; }
        if (t == "presentado")  { c = CondicionRastro::Presentado; return true; }
        if (t == "cualquiera")  { c = CondicionRastro::Cualquiera; return true; }
        return false;
    }

    bool leerTipo(const std::string& texto, TipoRastro& tipo)
    {
        const std::string t = minusculas(texto);
        if (t == "pizarra")  { tipo = TipoRastro::Pizarra;  return true; }
        if (t == "objeto")   { tipo = TipoRastro::Objeto;   return true; }
        if (t == "ambiente") { tipo = TipoRastro::Ambiente; return true; }
        if (t == "mensaje")  { tipo = TipoRastro::Mensaje;  return true; }
        return false;
    }

    // El estilo debe ser uno de los que acepta ese tipo.
    bool leerEstilo(const std::string& texto, TipoRastro tipo, EstiloRastro& estilo)
    {
        const std::string t = minusculas(texto);

        switch (tipo)
        {
            case TipoRastro::Pizarra:
                if (t == "feliz")  { estilo = EstiloRastro::Feliz;  return true; }
                if (t == "triste") { estilo = EstiloRastro::Triste; return true; }
                if (t == "serio")  { estilo = EstiloRastro::Serio;  return true; }
                return false;

            case TipoRastro::Objeto:
                if (t == "globos") { estilo = EstiloRastro::Globos; return true; }
                if (t == "trofeo") { estilo = EstiloRastro::Trofeo; return true; }
                if (t == "papel")  { estilo = EstiloRastro::Papel;  return true; }
                if (t == "nube")   { estilo = EstiloRastro::Nube;   return true; }
                return false;

            case TipoRastro::Ambiente:
                if (t == "fiesta")   { estilo = EstiloRastro::Fiesta;   return true; }
                if (t == "tristeza") { estilo = EstiloRastro::Tristeza; return true; }
                return false;

            case TipoRastro::Mensaje:
                estilo = EstiloRastro::Ninguno;   // el estilo no se usa (poner -)
                return true;
        }
        return false;
    }
}

bool LectorRastros::cargarArchivo(const std::string& ruta,
                                  std::vector<DatosRastro>& rastros,
                                  std::vector<MensajeFase>& fases)
{
    std::ifstream archivo(ruta);
    if (!archivo.is_open())
    {
        std::cerr << "[LectorRastros] No se pudo abrir '" << ruta << "'\n";
        return false;
    }

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

        // ---- fase | numero | texto ----
        if (c[0] == "fase")
        {
            MensajeFase m{};
            if (static_cast<int>(c.size()) != COLUMNAS_FASE ||
                !leerEntero(c[1], m.fase) || m.fase < 2 || c[2].empty())
            {
                std::cerr << "[LectorRastros] Linea " << numeroLinea
                          << ": se esperaba 'fase | numero(2 o mas) | texto'\n";
                todoBien = false;
                continue;
            }
            m.texto = conSaltos(c[2]);
            fases.push_back(m);
            continue;
        }

        // ---- rastro | ... ----
        if (c[0] != "rastro" || static_cast<int>(c.size()) != COLUMNAS_RASTRO)
        {
            std::cerr << "[LectorRastros] Linea " << numeroLinea
                      << ": se esperaba 'rastro' con " << COLUMNAS_RASTRO
                      << " columnas o 'fase' con " << COLUMNAS_FASE << "\n";
            todoBien = false;
            continue;
        }

        DatosRastro d{};
        d.escena = c[1];
        d.nombre = conSaltos(c[10]);
        d.texto  = conSaltos(c[11]);

        const bool bien =
            !d.escena.empty() &&
            leerMateria(c[2], d.materia) &&
            leerCondicion(c[3], d.condicion) &&
            leerTipo(c[4], d.tipo) &&
            leerEstilo(c[9], d.tipo, d.estilo) &&
            leerFloat(c[5], d.x) &&
            leerFloat(c[6], d.y) &&
            leerFloat(c[7], d.ancho) &&
            leerFloat(c[8], d.alto);

        // Que debe traer cada tipo: la pizarra y el objeto se dibujan (tamano
        // y nombre) y se leen (texto); el mensaje solo necesita texto; el
        // ambiente cubre todo el salon y no necesita nada mas.
        bool completo = false;
        if (bien)
        {
            switch (d.tipo)
            {
                case TipoRastro::Pizarra:
                case TipoRastro::Objeto:
                    completo = d.ancho > 0.f && d.alto > 0.f &&
                               !d.nombre.empty() && !d.texto.empty();
                    break;
                case TipoRastro::Mensaje:
                    completo = !d.texto.empty();
                    break;
                case TipoRastro::Ambiente:
                    completo = true;
                    break;
            }
        }

        if (!completo)
        {
            std::cerr << "[LectorRastros] Linea " << numeroLinea << ": valores invalidos\n";
            todoBien = false;
            continue;
        }

        rastros.push_back(d);
    }

    return todoBien;
}
