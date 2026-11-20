#pragma once

#include <random>
#include <string>
#include <unordered_map>
#include <vector>

class SorteadorMinijuegos
{
public:
    SorteadorMinijuegos();

    // Devuelve ID vacio si no hay candidatos. Si hay 2 o mas IDs
    // distintos, excluye el ultimo sorteado para esta categoria.
    std::string sortear(const std::string& categoria,
                        const std::vector<std::string>& ids);

    // Util para una nueva partida, si decidimos reiniciar el historial.
    void reiniciar() { ultimoPorCategoria_.clear(); }

private:
    std::mt19937 generador_;
    std::unordered_map<std::string, std::string> ultimoPorCategoria_;
};
