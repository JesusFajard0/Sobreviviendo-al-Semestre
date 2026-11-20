#include <algorithm>
#include <cassert>
#include <iostream>
#include <string>
#include <vector>

#include "minigames/SorteadorMinijuegos.hpp"
#include "minigames/SeleccionEstudio.hpp"

int main()
{
    SorteadorMinijuegos sorteador;

    // PRUEBA 1: Computadora
    assert(sorteador.sortear("computadora", {}).empty());
    assert(sorteador.sortear("computadora", {"unico"}) == "unico");

    const std::vector<std::string> juegosPC = {
        "pc_1", "pc_2", "pc_3"
    };

    std::string anteriorPC;

    for (int i = 0; i < 1000; ++i)
    {
        std::string elegido =
            sorteador.sortear("computadora", juegosPC);

        assert(std::find(juegosPC.begin(), juegosPC.end(), elegido)
               != juegosPC.end());

        if (!anteriorPC.empty())
            assert(elegido != anteriorPC);

        anteriorPC = elegido;
    }

    // PRUEBA 2: Estudio por materia
    const std::vector<Materia> materias = {
        Materia::Calculo,
        Materia::Fisica,
        Materia::Programacion
    };

    const std::vector<std::string> especificos = {
        "estudio_calculo",
        "estudio_fisica",
        "estudio_programacion"
    };

    std::string anteriorEstudio;

    for (std::size_t m = 0; m < materias.size(); ++m)
    {
        const auto candidatos = candidatosEstudio(materias[m]);

        assert(candidatos.size() == 3);
        assert(candidatos[0] == especificos[m]);
        assert(candidatos[1] == "estudio_generico_1");
        assert(candidatos[2] == "estudio_generico_2");

        for (int i = 0; i < 1000; ++i)
        {
            const auto elegido =
                sortearMinijuegoEstudio(sorteador, materias[m]);

            assert(std::find(candidatos.begin(),
                             candidatos.end(),
                             elegido) != candidatos.end());

            assert(elegido != anteriorEstudio);

            anteriorEstudio = elegido;
        }
    }

    // PRUEBA 3: Materia invalida
    assert(candidatosEstudio(Materia::Cantidad).empty());

    assert(sortearMinijuegoEstudio(
        sorteador, Materia::Cantidad).empty());

    std::cout
        << "S2-8: sorteos de computadora y estudio superados\n";
}