#pragma once

#include <random>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

class SorteadorMinijuegos
{
public:
    SorteadorMinijuegos();

    // Sorteo uniforme, utilizado por los minijuegos de computadora.
    std::string sortear(
        const std::string& categoria,
        const std::vector<std::string>& ids);

    // Sorteo ponderado.
    // Un peso mayor aumenta la probabilidad de salir.
    // Se omiten IDs vacios y pesos no positivos.
    // Si existe una alternativa, no repite el ultimo ID.
    std::string sortearPonderado(
        const std::string& categoria,
        const std::vector<std::pair<std::string, int>>& candidatos);

    // Reinicia el historial de sorteos.
    void reiniciar()
    {
        ultimoPorCategoria_.clear();
    }

private:
    std::mt19937 generador_;

    std::unordered_map<std::string, std::string>
        ultimoPorCategoria_;
};