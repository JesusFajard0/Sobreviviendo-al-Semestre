#include "world/Rastros.hpp"

bool cumpleCondicion(CondicionRastro condicion, EstadoExamen estado, bool aprobado)
{
    // Mientras el examen esta pendiente no queda ningun rastro.
    if (estado == EstadoExamen::Pendiente)
        return false;

    const bool presento = (estado == EstadoExamen::Presentado);

    switch (condicion)
    {
        case CondicionRastro::Aprobado:   return presento && aprobado;
        case CondicionRastro::Reprobado:  return presento && !aprobado;
        case CondicionRastro::NoPresento: return !presento;
        case CondicionRastro::Presentado: return presento;
        case CondicionRastro::Cualquiera: return true;
    }
    return false;
}

std::string rellenarPlantilla(const std::string& texto,
                              const std::map<std::string, std::string>& valores)
{
    std::string resultado;
    std::size_t i = 0;

    while (i < texto.size())
    {
        if (texto[i] == '{')
        {
            const std::size_t cierre = texto.find('}', i + 1);
            if (cierre != std::string::npos)
            {
                const std::string clave = texto.substr(i + 1, cierre - i - 1);
                const auto it = valores.find(clave);
                if (it != valores.end())
                {
                    resultado += it->second;
                    i = cierre + 1;
                    continue;
                }
            }
        }
        resultado += texto[i];
        ++i;
    }

    return resultado;
}
