#include "minigames/RegistroMinijuegos.hpp"

#include <utility>

bool RegistroMinijuegos::registrar(const std::string& id,
                                  const std::string& categoria, Fabrica fabrica)
{
    if (id.empty() || categoria.empty() || !fabrica || contiene(id))
        return false;
    entradas_.emplace(id, Entrada{categoria, std::move(fabrica)});
    orden_.push_back(id);
    return true;
}

std::unique_ptr<Minijuego> RegistroMinijuegos::crear(
    const std::string& id, StateStack* pila,
    Minijuego::AlTerminar alTerminar) const
{
    auto it = entradas_.find(id);
    if (it == entradas_.end() || pila == nullptr)
        return nullptr;
    return it->second.fabrica(pila, std::move(alTerminar));
}

bool RegistroMinijuegos::contiene(const std::string& id) const
{
    return entradas_.find(id) != entradas_.end();
}

std::vector<std::string> RegistroMinijuegos::idsDeCategoria(
    const std::string& categoria) const
{
    std::vector<std::string> ids;
    for (const auto& id : orden_)
        if (entradas_.at(id).categoria == categoria)
            ids.push_back(id);
    return ids;
}
