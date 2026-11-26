#pragma once

#include <string>

#include "systems/Estadisticas.hpp"   // por Materia

// S3-3: el rastro es la decoracion que deja el examen en su salon (un
// "easter egg"): si te fue bien hay celebracion; si te fue mal, tristeza.
enum class TipoRastro
{
    Pizarra,   // pizarra con una carita dibujada; con E se lee el texto
    Objeto,    // objeto decorativo (globos, trofeo...); con E se lee el texto
    Ambiente,  // todo el salon: tono de color y particulas (confeti o lluvia)
    Mensaje    // sale solo, una vez, al entrar al salon
};

// Como se dibuja cada tipo. Cada tipo acepta solo algunos estilos:
//   Pizarra:  Feliz, Triste, Serio
//   Objeto:   Globos, Trofeo, Papel, Nube
//   Ambiente: Fiesta, Tristeza
//   Mensaje:  Ninguno
enum class EstiloRastro
{
    Ninguno,
    Feliz, Triste, Serio,
    Globos, Trofeo, Papel, Nube,
    Fiesta, Tristeza
};

// Cuando aparece el rastro, segun lo que paso en el examen de su materia.
enum class CondicionRastro
{
    Aprobado,
    Reprobado,
    NoPresento,
    Presentado,   // aprobado o reprobado
    Cualquiera    // el examen ya paso (se presento o no)
};

// Una linea "rastro | ..." de data/rastros.txt.
struct DatosRastro
{
    std::string     escena;
    Materia         materia;
    CondicionRastro condicion;
    TipoRastro      tipo;
    EstiloRastro    estilo;

    float x;
    float y;
    float ancho;
    float alto;

    std::string nombre;   // titulo que se dibuja (pizarra) o nombre del objeto
    std::string texto;    // lo que se lee con E, o el mensaje de entrada
};

// Una linea "fase | N | texto": aviso al empezar la fase N.
struct MensajeFase
{
    int         fase;
    std::string texto;
};
