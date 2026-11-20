#include "minigames/SorteadorMinijuegos.hpp"
#include <algorithm>


#include <unordered_set>

SorteadorMinijuegos::SorteadorMinijuegos()
    : generador_(std::random_device{}())
{
}

std::string SorteadorMinijuegos::sortear(
    const std::string& categoria, const std::vector<std::string>& ids)
{
    // Filtrar IDs vacios y duplicados para evitar sesgos de probabilidad.
    std::vector<std::string> candidatos;
    std::unordered_set<std::string> vistos;
    for (const std::string& id : ids)
        if (!id.empty() && vistos.insert(id).second)
            candidatos.push_back(id);

    if (candidatos.empty())
        return {};

    const auto anterior = ultimoPorCategoria_.find(categoria);
    if (candidatos.size() > 1 && anterior != ultimoPorCategoria_.end())
    {
        std::vector<std::string> alternativas;
        for (const auto& id : candidatos)
            if (id != anterior->second)
                alternativas.push_back(id);
        // Si no coincide ningun ID con el anterior, alternativas = candidatos.
        if (!alternativas.empty())
            candidatos = std::move(alternativas);
    }

    std::uniform_int_distribution<std::size_t> distribucion(0, candidatos.size()-1);
    const std::string elegido = candidatos[distribucion(generador_)];
    ultimoPorCategoria_[categoria] = elegido;
    return elegido;
}
