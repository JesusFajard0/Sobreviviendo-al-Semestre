
#pragma once

#include "DatosEscena.hpp"

#include <string>
#include <vector>

#include <chainedhash.hpp>
#include <mtreenode.hpp>

#include <unordered_map>

// Representa una opción de destino disponible desde una escena.
struct Destino
{
    std::string id;
    std::string nombre;

    bool bloqueado = false;
    bool esPadre = false;
    bool esAnimacion = false;
};

// Resultado de intentar cambiar de escena.
enum class ResultadoCambio
{
    Ok,
    Bloqueada,
    Animacion,
    NoExiste
};

class MapaEscenas
{
private:

    // Nodo del árbol general de escenas.
    using Nodo = Designar::MTreeNode<std::string>;

    // Entrada almacenada en la tabla hash.
    struct EntradaEscena
    {
        std::string id;
        DatosEscena datos;
    };

    // Comparador: dos entradas son iguales si tienen el mismo ID.
    struct IgualId
    {
        bool operator()(const EntradaEscena& a,
                        const EntradaEscena& b) const
        {
            return a.id == b.id;
        }
    };

    // Función hash basada en el identificador.
    static Designar::nat_t hashEntrada(const EntradaEscena& entrada)
    {
        return Designar::super_fast_hash(entrada.id);
    }

    using TablaDatos =
        Designar::SeparateChainingHashTable<EntradaEscena, IgualId>;

    Nodo* raiz_;
    std::string idRaiz_;
    TablaDatos datos_;
    std::unordered_map<std::string, std::vector<DatosRectangulo>> rectangulos_;

    // Imprime recursivamente el árbol.
    void imprimirRecursivo(Nodo* nodo, int nivel) const;

    // Busca un nodo del árbol por su ID (NUEVO).
    Nodo* buscarNodo(Nodo* nodo, const std::string& id) const;


public:

    MapaEscenas();
    ~MapaEscenas();

    // Evitamos copias superficiales de un árbol con memoria dinámica.
    MapaEscenas(const MapaEscenas&) = delete;
    MapaEscenas& operator=(const MapaEscenas&) = delete;

    bool cargar(const std::string& rutaDatos);

    const DatosEscena* datos(const std::string& id) const;

    const std::vector<DatosRectangulo>& rectangulos(
    const std::string& id) const;

    std::vector<Destino> destinos(
        const std::string& idActual,
        int faseActual
    ) const;

    const std::string& raiz() const;

    void imprimirArbol() const;
};
