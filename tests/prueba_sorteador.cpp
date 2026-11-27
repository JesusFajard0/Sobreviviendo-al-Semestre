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

    // PRUEBA 1: computadora sin candidatos.
    assert(sorteador.sortear("computadora", {}).empty());

    // PRUEBA 2: computadora con un solo candidato.
    assert(sorteador.sortear(
        "computadora", {"unico"}) == "unico");

    // PRUEBA 3: antirrepeticion en computadora.
    const std::vector<std::string> juegosPC = {
        "pc_1", "pc_2", "pc_3"
    };

    std::string anteriorPC;

    for (int i = 0; i < 1000; ++i)
    {
        const auto elegido =
            sorteador.sortear("computadora", juegosPC);

        assert(std::find(
            juegosPC.begin(),
            juegosPC.end(),
            elegido
        ) != juegosPC.end());

        assert(elegido != anteriorPC);

        anteriorPC = elegido;
    }

    // PRUEBA 4: seleccion por materia.
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
        const auto candidatos =
            candidatosEstudio(materias[m]);

        const auto ponderados =
            candidatosEstudioPonderados(materias[m]);

        assert(candidatos.size() == 3);

        assert(candidatos[0] == especificos[m]);
        assert(candidatos[1] == "estudio_generico_1");
        assert(candidatos[2] == "estudio_generico_2");

        // Comprobar pesos.
        assert(ponderados[0].second == 50);
        assert(ponderados[1].second == 25);
        assert(ponderados[2].second == 25);

        // Comprobar antirrepeticion.
        for (int i = 0; i < 1000; ++i)
        {
            const auto elegido =
                sortearMinijuegoEstudio(
                    sorteador,
                    materias[m]
                );

            assert(std::find(
                candidatos.begin(),
                candidatos.end(),
                elegido
            ) != candidatos.end());

            assert(elegido != anteriorEstudio);

            anteriorEstudio = elegido;
        }
    }

    // PRUEBA 5: materia invalida.
    assert(candidatosEstudio(
        Materia::Cantidad).empty());

    assert(sortearMinijuegoEstudio(
        sorteador,
        Materia::Cantidad
    ).empty());

    // PRUEBA 6: pesos invalidos.
    assert(sorteador.sortearPonderado(
        "sin_opciones",
        {{"a", 0}, {"b", -1}}
    ).empty());

    // PRUEBA 7: un candidato con peso positivo.
    assert(sorteador.sortearPonderado(
        "una_opcion",
        {{"a", 0}, {"b", 10}}
    ) == "b");

    // PRUEBA 8: verificar estadisticamente los pesos.
    // Reiniciamos el historial para medir 50/25/25
    // antes de aplicar la antirrepeticion.
    int vecesEspecifico = 0;

    const int repeticiones = 10000;

    for (int i = 0; i < repeticiones; ++i)
    {
        sorteador.reiniciar();

        if (sortearMinijuegoEstudio(
            sorteador,
            Materia::Calculo
        ) == "estudio_calculo")
        {
            ++vecesEspecifico;
        }
    }

    // Esperamos aproximadamente 5000 apariciones.
    // El margen permite variaciones aleatorias.
    assert(vecesEspecifico > 4600);
    assert(vecesEspecifico < 5400);

    std::cout
        << "S2-8: sorteo ponderado y "
        << "antirrepeticion superados\n";

    return 0;
}