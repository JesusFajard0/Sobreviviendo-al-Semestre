# Sobreviviendo al Semestre

Juego 2.5D de simulación de un semestre universitario de un estudiante de Ingeniería de Sistemas.
El jugador administra su tiempo, energía, estrés, dopamina, salud mental y dinero durante 8 semanas,
estudia mediante minijuegos y presenta tres exámenes. Al final recibe un resumen y uno de seis finales.

Proyecto de **Programación 3** · Grupo **CLOUDNOVA**
Integrantes: Jesús Fajardo, Daniel Freitas y Luis Ortega (Toñito)

## Tecnologías

- C++17
- [SFML](https://www.sfml-dev.org/) (gráficos, ventana, entrada y audio)
- [DeSiGNAR](https://github.com/R3mmurd/DeSiGNAR) (estructuras de datos, incluida como submódulo)
- CMake ≥ 3.20
- Git y GitHub

## Cómo obtener el proyecto

DeSiGNAR está incluida como **submódulo de Git**, así que hay que clonar con la opción `--recurse-submodules`:

```bash
git clone --recurse-submodules <URL-DEL-REPOSITORIO>
cd sobreviviendo-al-semestre
```

Si ya se clonó sin esa opción:

```bash
git submodule update --init --recursive
```

## Requisitos

Un compilador de C++17 (GCC ≥ 9, Clang ≥ 14 o MSVC 2019+), CMake ≥ 3.20 y SFML 2.5 o superior (también funciona con SFML 3).

**Ubuntu / Debian**

```bash
sudo apt install build-essential cmake git libsfml-dev
```

**macOS**

```bash
brew install cmake sfml
```

**Windows**: con Visual Studio 2019+ y [vcpkg](https://vcpkg.io): `vcpkg install sfml`, y luego configurar con
`-DCMAKE_TOOLCHAIN_FILE=<ruta-de-vcpkg>/scripts/buildsystems/vcpkg.cmake`.
También sirve MSYS2 (`pacman -S mingw-w64-ucrt-x86_64-sfml`).

## Compilar y ejecutar

```bash
cmake -S . -B build
cmake --build build --parallel
./build/sobreviviendo            # en Windows: build\Debug\sobreviviendo.exe
```

La versión actual abre una ventana de prueba con un cuadrado que se mueve con las flechas y escribe
`DeSiGNAR OK` en la consola. Sirve para comprobar que el entorno funciona.

## Estructura del proyecto

| Carpeta | Contenido |
|---|---|
| `src/core` | Ventana, bucle principal, pila de estados, recursos |
| `src/world` | Escenarios, personaje, mapa (árbol), colisiones |
| `src/interactables` | Clase base `Interactuable`, puertas, NPCs, objetos |
| `src/systems` | Estadísticas, tiempo, fases, notas, finales |
| `src/activities` | Actividades y sus efectos |
| `src/minigames` | Clase base `Minijuego` y minijuegos concretos |
| `src/ui` | HUD, menús, diálogos, pantalla de resumen |
| `src/audio` | Música y efectos |
| `assets/` | Imágenes, audio y fuentes |
| `data/` | Archivos de texto con actividades, fases, finales y diálogos |
| `docs/` | Documento técnico y diagramas |
| `external/` | Librerías de terceros (DeSiGNAR) |

## Flujo de trabajo con Git

- `main`: versión estable, solo se actualiza al cumplir la meta de cada viernes.
- `develop`: integración del trabajo en curso.
- Una rama por tarea, con el código de la tarea en el nombre: `s1-3-personaje`, `s2-5-fin-de-semana`...
- Commits pequeños y con mensajes claros en español.
- Cada viernes se integran las ramas en `develop`, se compila y se prueba una demostración.

## Estado

Semana 0 (preparación): repositorio, entorno de compilación y documento técnico.
