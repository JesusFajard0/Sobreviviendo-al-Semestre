#include "core/GestorRecursos.hpp"

#include <iostream>

GestorRecursos::GestorRecursos()
    : texturas_(IgualNombre<sf::Texture>(),     &hashEntrada<sf::Texture>),
      sonidos_ (IgualNombre<sf::SoundBuffer>(), &hashEntrada<sf::SoundBuffer>),
      fuentes_ (IgualNombre<sf::Font>(),        &hashEntrada<sf::Font>)
{
}

// ---------------------------------------------------------------
// Lógica común
// ---------------------------------------------------------------

template <typename T, typename Cargador>
bool GestorRecursos::cargarGenerico(Tabla<T>& tabla, const std::string& nombre,
                                    const std::string& ruta, const char* tipo,
                                    Cargador cargar)
{
    if (nombre.empty())
    {
        std::cerr << "[GestorRecursos] Nombre vacio al cargar " << tipo << " desde '" << ruta << "'\n";
        return false;
    }

    // Entrada "sonda": solo lleva el nombre, sirve para buscar en la tabla.
    const Entrada<T> sonda{nombre, nullptr};

    // Si ya estaba cargado no se vuelve a cargar (así los punteros ya entregados
    // siguen siendo válidos).
    if (tabla.search(sonda) != nullptr)
        return true;

    std::shared_ptr<T> recurso = std::make_shared<T>();
    if (!cargar(*recurso, ruta))
    {
        std::cerr << "[GestorRecursos] No se pudo cargar " << tipo << " '" << nombre
                  << "' desde '" << ruta << "'\n";
        return false;
    }

    tabla.insert(Entrada<T>{nombre, recurso});
    return true;
}

template <typename T>
T* GestorRecursos::obtenerGenerico(const Tabla<T>& tabla, const std::string& nombre) const
{
    const Entrada<T> sonda{nombre, nullptr};
    const Entrada<T>* encontrada = tabla.search(sonda);
    return encontrada != nullptr ? encontrada->recurso.get() : nullptr;
}

// ---------------------------------------------------------------
// Texturas
// ---------------------------------------------------------------

bool GestorRecursos::cargarTextura(const std::string& nombre, const std::string& ruta)
{
    return cargarGenerico<sf::Texture>(texturas_, nombre, ruta, "textura",
        [](sf::Texture& t, const std::string& r) { return t.loadFromFile(r); });
}

sf::Texture* GestorRecursos::obtenerTextura(const std::string& nombre) const
{
    return obtenerGenerico<sf::Texture>(texturas_, nombre);
}

// ---------------------------------------------------------------
// Sonidos
// ---------------------------------------------------------------

bool GestorRecursos::cargarSonido(const std::string& nombre, const std::string& ruta)
{
    return cargarGenerico<sf::SoundBuffer>(sonidos_, nombre, ruta, "sonido",
        [](sf::SoundBuffer& s, const std::string& r) { return s.loadFromFile(r); });
}

sf::SoundBuffer* GestorRecursos::obtenerSonido(const std::string& nombre) const
{
    return obtenerGenerico<sf::SoundBuffer>(sonidos_, nombre);
}

// ---------------------------------------------------------------
// Fuentes (en SFML 3 el método se llama openFromFile)
// ---------------------------------------------------------------

bool GestorRecursos::cargarFuente(const std::string& nombre, const std::string& ruta)
{
    return cargarGenerico<sf::Font>(fuentes_, nombre, ruta, "fuente",
        [](sf::Font& f, const std::string& r)
        {
#if SFML_VERSION_MAJOR >= 3
            return f.openFromFile(r);
#else
            return f.loadFromFile(r);
#endif
        });
}

sf::Font* GestorRecursos::obtenerFuente(const std::string& nombre) const
{
    return obtenerGenerico<sf::Font>(fuentes_, nombre);
}

// ---------------------------------------------------------------

void GestorRecursos::liberarTodo()
{
    texturas_.clear();
    sonidos_.clear();
    fuentes_.clear();
}