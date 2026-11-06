
#include "world/MapaEscenas.hpp"
#include "world/LectorEscenas.hpp"

#include <iostream>
#include <memory>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <vector>

// Constructor
MapaEscenas::MapaEscenas()
    : raiz_(nullptr),
      idRaiz_(),
      datos_(IgualId(), &MapaEscenas::hashEntrada)
{
}

// Destructor
MapaEscenas::~MapaEscenas()
{
    if (raiz_ != nullptr)
    {
        Nodo::destroy_tree(raiz_);
    }
}

// Busca los datos de una escena mediante su ID.
const DatosEscena* MapaEscenas::datos(const std::string& id) const
{
    EntradaEscena clave{};
    clave.id = id;

    const EntradaEscena* encontrada = datos_.search(clave);

    if (encontrada == nullptr)
    {
        return nullptr;
    }

    return &encontrada->datos;
}

// Devuelve el identificador de la escena raíz.
const std::string& MapaEscenas::raiz() const
{
    return idRaiz_;
}

// Carga y construye el árbol de escenas.
bool MapaEscenas::cargar(const std::string& rutaDatos)
{
    // Liberar una carga anterior, si existe.
    if (raiz_ != nullptr)
    {
        Nodo::destroy_tree(raiz_);
        raiz_ = nullptr;      
    }

    datos_.clear();
    rectangulos_.clear();
    idRaiz_.clear();

    // 1. Leer los datos del archivo.
    std::vector<DatosEscena> escenas;
    std::vector<DatosRectangulo> rectangulos;

    if (!LectorEscenas::cargarArchivo(rutaDatos, escenas, rectangulos))
    {
        std::cerr << "[MapaEscenas] Error al leer el archivo.\n";
        return false;
    }

    // 2. Crear estructuras auxiliares.
    std::unordered_map<std::string, const DatosEscena*> porId;
    std::unordered_map<std::string, std::vector<std::string>> hijos;

    std::string idRaizTemporal;
    int cantidadRaices = 0;

    // 3. Registrar escenas y detectar IDs duplicados.
    for (const DatosEscena& escena : escenas)
    {
        if (escena.id.empty())
        {
            std::cerr << "[MapaEscenas] Existe una escena sin ID.\n";
            return false;
        }

        auto resultado = porId.emplace(escena.id, &escena);

        if (!resultado.second)
        {
            std::cerr << "[MapaEscenas] ID duplicado: "
                      << escena.id << '\n';
            return false;
        }

        if (escena.padre == "-")
        {
            ++cantidadRaices;
            idRaizTemporal = escena.id;
        }
    }

    // 4. Debe existir exactamente una raíz.
    if (cantidadRaices != 1)
    {
        std::cerr << "[MapaEscenas] Debe existir exactamente una raiz.\n";
        return false;
    }

    // 5. Comprobar padres y construir las relaciones padre-hijo.
    for (const DatosEscena& escena : escenas)
    {
        if (escena.padre == "-")
        {
            continue;
        }

        if (porId.find(escena.padre) == porId.end())
        {
            std::cerr << "[MapaEscenas] El padre de "
                      << escena.id << " no existe: "
                      << escena.padre << '\n';
            return false;
        }

        hijos[escena.padre].push_back(escena.id);
    }

       // 6. Validar rectángulos y que haya una sola salida por escena.
    std::unordered_map<std::string, int> salidasPorEscena;

    for (const DatosRectangulo& rectangulo : rectangulos)
    {
        if (porId.find(rectangulo.idEscena) == porId.end())
        {
            std::cerr << "[MapaEscenas] Rectangulo asociado "
                      << "a una escena inexistente: "
                      << rectangulo.idEscena << '\n';
            return false;
        }

        if (rectangulo.tipo == "rect_salida" &&
            ++salidasPorEscena[rectangulo.idEscena] > 1)
        {
            std::cerr << "[MapaEscenas] La escena "
                      << rectangulo.idEscena
                      << " tiene mas de una salida.\n";
            return false;
        }
    }

    // 7. Comprobar conectividad y detectar ciclos
    //    antes de construir el árbol.
    std::unordered_set<std::string> visitados;
    std::queue<std::string> pendientes;

    pendientes.push(idRaizTemporal);
    visitados.insert(idRaizTemporal);

    while (!pendientes.empty())
    {
        std::string actual = pendientes.front();
        pendientes.pop();

        auto it = hijos.find(actual);

        if (it == hijos.end())
        {
            continue;
        }

        for (const std::string& idHijo : it->second)
        {
            if (visitados.insert(idHijo).second)
            {
                pendientes.push(idHijo);
            }
        }
    }

    if (visitados.size() != escenas.size())
    {
        std::cerr << "[MapaEscenas] Hay escenas desconectadas "
                  << "o ciclos en la jerarquia.\n";
        return false;
    }

    // 8. Crear todos los nodos independientes.
    std::unordered_map<std::string, Nodo*> nodos;
    std::vector<std::unique_ptr<Nodo>> propiedadNodos;

    for (const DatosEscena& escena : escenas)
    {
        std::unique_ptr<Nodo> nuevo =
            std::make_unique<Nodo>(escena.id);

        nodos[escena.id] = nuevo.get();

        propiedadNodos.push_back(std::move(nuevo));
    }

    // 9. Conectar los nodos mediante append_child().
    Nodo* raizTemporal = nodos.at(idRaizTemporal);

    for (const DatosEscena& escena : escenas)
    {
        auto it = hijos.find(escena.id);

        if (it == hijos.end())
        {
            continue;
        }

        Nodo* padre = nodos.at(escena.id);

        for (const std::string& idHijo : it->second)
        {
            padre->append_child(nodos.at(idHijo));
        }
    }

    // El árbol ya está conectado.
    // Transferimos la propiedad de los nodos al árbol.
    for (auto& nodo : propiedadNodos)
    {
        nodo.release();
    }

    // 10. Guardar los datos completos en la tabla hash.
    for (const DatosEscena& escena : escenas)
    {
        EntradaEscena entrada;
        entrada.id = escena.id;
        entrada.datos = escena;

        if (datos_.insert(entrada) == nullptr)
        {
            std::cerr << "[MapaEscenas] Error al insertar: "
                      << escena.id << '\n';

            Nodo::destroy_tree(raizTemporal);
            datos_.clear();

            return false;
        }
    }

    // 11. Publicar el árbol construido.
    for (const DatosRectangulo& r : rectangulos)
        rectangulos_[r.idEscena].push_back(r);


    raiz_ = raizTemporal;
    idRaiz_ = idRaizTemporal;

    std::cout << "[MapaEscenas] Arbol cargado correctamente.\n";
    std::cout << "[MapaEscenas] Escenas: "
              << escenas.size() << '\n';

    return true;
}

// Imprime recursivamente un nodo y sus descendientes.
void MapaEscenas::imprimirRecursivo(Nodo* nodo, int nivel) const
{
    if (nodo == nullptr)
    {
        return;
    }

    for (int i = 0; i < nivel; ++i)
    {
        std::cout << "  ";
    }

    std::cout << nodo->get_key() << '\n';

    Nodo* hijo = nodo->get_first_child();

    while (hijo != nullptr)
    {
        imprimirRecursivo(hijo, nivel + 1);
        hijo = hijo->get_right_sibling();
    }
}

// Imprime el árbol completo.
void MapaEscenas::imprimirArbol() const
{
    if (raiz_ == nullptr)
    {
        std::cout << "[MapaEscenas] El arbol esta vacio.\n";
        return;
    }

    std::cout << "\n=== ARBOL DE ESCENAS ===\n";

    imprimirRecursivo(raiz_, 0);

    std::cout << "========================\n";
}

auto MapaEscenas::buscarNodo(Nodo* nodo, const std::string& id) const -> Nodo*
{
    if (nodo == nullptr)
        return nullptr;

    if (nodo->get_key() == id)
        return nodo;

    for (Nodo* h = nodo->get_first_child(); h != nullptr;
         h = h->get_right_sibling())
    {
        if (Nodo* r = buscarNodo(h, id))
            return r;
    }

    return nullptr;
}

std::vector<Destino> MapaEscenas::destinos(
    const std::string& idActual, int faseActual) const
{
    std::vector<Destino> resultado;

    const DatosEscena* actual = datos(idActual);
    Nodo* nodo = buscarNodo(raiz_, idActual);

    if (actual == nullptr || nodo == nullptr)
        return resultado;

    auto agregar = [&](const std::string& id, bool esPadre)
    {
        const DatosEscena* d = datos(id);
        if (d == nullptr)
            return;

        Destino dest;
        dest.id = id;
        dest.nombre = d->nombre;
        dest.esPadre = esPadre;
        dest.esAnimacion = (d->tipo == "animacion");
        dest.bloqueado = (faseActual < d->faseMinima);
        resultado.push_back(dest);
    };

    if (actual->padre != "-")
        agregar(actual->padre, true);

    for (Nodo* h = nodo->get_first_child(); h != nullptr;
         h = h->get_right_sibling())
    {
        agregar(h->get_key(), false);
    }

    return resultado;
}

const std::vector<DatosRectangulo>& MapaEscenas::rectangulos(
    const std::string& id) const
{
    static const std::vector<DatosRectangulo> vacio;

    auto it = rectangulos_.find(id);
    return it == rectangulos_.end() ? vacio : it->second;
}