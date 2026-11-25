#include "minigames/SorteadorMinijuegos.hpp"

#include <unordered_map>

SorteadorMinijuegos::SorteadorMinijuegos()
    : generador_(std::random_device{}())
{
}

std::string SorteadorMinijuegos::sortear(
    const std::string& categoria,
    const std::vector<std::string>& ids)
{
    // Convertir el sorteo normal en uno con pesos iguales.
    std::vector<std::pair<std::string, int>> candidatos;

    candidatos.reserve(ids.size());

    for (const auto& id : ids)
    {
        candidatos.emplace_back(id, 1);
    }

    return sortearPonderado(categoria, candidatos);
}

std::string SorteadorMinijuegos::sortearPonderado(
    const std::string& categoria,
    const std::vector<std::pair<std::string, int>>& entradas)
{
    // Unificar IDs repetidos sumando sus pesos.
    std::vector<std::string> ids;
    std::vector<double> pesos;

    std::unordered_map<std::string, std::size_t> posiciones;

    for (const auto& entrada : entradas)
    {
        if (entrada.first.empty() || entrada.second <= 0)
        {
            continue;
        }

        const auto encontrado = posiciones.find(entrada.first);

        if (encontrado == posiciones.end())
        {
            posiciones.emplace(entrada.first, ids.size());

            ids.push_back(entrada.first);

            pesos.push_back(
                static_cast<double>(entrada.second)
            );
        }
        else
        {
            pesos[encontrado->second] += entrada.second;
        }
    }

    // Si no hay minijuegos validos, devolver ID vacio.
    if (ids.empty())
    {
        return {};
    }

    // Evitar repetir el ultimo minijuego de esta categoria
    // cuando exista mas de una alternativa.
    const auto anterior = ultimoPorCategoria_.find(categoria);

    if (ids.size() > 1 &&
        anterior != ultimoPorCategoria_.end())
    {
        const auto repetido =
            posiciones.find(anterior->second);

        if (repetido != posiciones.end())
        {
            // Peso cero significa que no puede salir.
            pesos[repetido->second] = 0.0;
        }
    }

    // Seleccionar segun los pesos disponibles.
    std::discrete_distribution<std::size_t> distribucion(
        pesos.begin(),
        pesos.end()
    );

    const std::string elegido =
        ids[distribucion(generador_)];

    // Recordar el resultado de esta categoria.
    ultimoPorCategoria_[categoria] = elegido;

    return elegido;
}