#pragma once

// Gestor de recursos: carga texturas, buffers de sonido y fuentes UNA sola vez,
// los guarda por nombre en tablas hash de DeSiGNAR y los entrega cuando se piden.
//
// Convención de errores (sin excepciones):
//   - cargar...  devuelve false y escribe un mensaje en std::cerr si falla.
//   - obtener... devuelve nullptr si el nombre no está cargado.
// Los punteros devueltos siguen siendo válidos mientras el gestor exista
// (o hasta llamar a liberarTodo()).

#include <functional>
#include <memory>
#include <string>

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>

#include <chainedhash.hpp>   // DeSiGNAR: SeparateChainingHashTable

class GestorRecursos
{
public:
    GestorRecursos();

    bool cargarTextura(const std::string& nombre, const std::string& ruta);
    bool cargarSonido (const std::string& nombre, const std::string& ruta);
    bool cargarFuente (const std::string& nombre, const std::string& ruta);

    sf::Texture*     obtenerTextura(const std::string& nombre) const;
    sf::SoundBuffer* obtenerSonido (const std::string& nombre) const;
    sf::Font*        obtenerFuente (const std::string& nombre) const;

    // Libera todos los recursos cargados.
    void liberarTodo();

private:
    // Elemento que guarda la tabla: el nombre es la clave y el recurso el dato.
    template <typename T>
    struct Entrada
    {
        std::string        nombre;
        std::shared_ptr<T> recurso;
    };

    // Dos entradas son "iguales" si tienen el mismo nombre.
    template <typename T>
    struct IgualNombre
    {
        bool operator()(const Entrada<T>& a, const Entrada<T>& b) const
        {
            return a.nombre == b.nombre;
        }
    };

    // Función hash: se calcula solo sobre el nombre.
    template <typename T>
    static Designar::nat_t hashEntrada(const Entrada<T>& e)
    {
        return Designar::super_fast_hash(e.nombre);
    }

    template <typename T>
    using Tabla = Designar::SeparateChainingHashTable<Entrada<T>, IgualNombre<T>>;

    // Lógica común a los tres tipos de recurso (definidas en el .cpp).
    template <typename T, typename Cargador>
    bool cargarGenerico(Tabla<T>& tabla, const std::string& nombre, const std::string& ruta,
                        const char* tipo, Cargador cargar);

    template <typename T>
    T* obtenerGenerico(const Tabla<T>& tabla, const std::string& nombre) const;

    Tabla<sf::Texture>     texturas_;
    Tabla<sf::SoundBuffer> sonidos_;
    Tabla<sf::Font>        fuentes_;
};