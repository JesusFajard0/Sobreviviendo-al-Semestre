cmake_minimum_required(VERSION 3.20)
project(SobreviviendoAlSemestre VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---------------------------------------------------------------
# DeSiGNAR (submódulo de Git en external/DeSiGNAR)
# Se desactivan sus ejemplos y pruebas para compilar solo la librería
# ---------------------------------------------------------------
set(DESIGNAR_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
set(DESIGNAR_BUILD_TESTS   OFF CACHE BOOL "" FORCE)
add_subdirectory(external/DeSiGNAR)

# ---------------------------------------------------------------
# SFML (se intenta primero la versión 3 y luego la 2.x)
# ---------------------------------------------------------------
find_package(SFML 3 COMPONENTS Graphics Window System Audio QUIET)
if(SFML_FOUND)
    set(SFML_LIBS SFML::Graphics SFML::Window SFML::System SFML::Audio)
else()
    find_package(SFML 2.5 COMPONENTS graphics window system audio REQUIRED)
    set(SFML_LIBS sfml-graphics sfml-window sfml-system sfml-audio)
endif()

# ---------------------------------------------------------------
# Ejecutable
# ---------------------------------------------------------------
file(GLOB_RECURSE FUENTES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/src/*.cpp)
add_executable(sobreviviendo ${FUENTES})
target_include_directories(sobreviviendo PRIVATE ${CMAKE_SOURCE_DIR}/src)

if(TARGET Designar::Designar)
    target_link_libraries(sobreviviendo PRIVATE Designar::Designar)
elseif(TARGET Designar)
    target_link_libraries(sobreviviendo PRIVATE Designar)
else()
    message(FATAL_ERROR
        "No se encontró el target de DeSiGNAR. ¿Se inicializó el submódulo? "
        "Ejecuta: git submodule update --init --recursive")
endif()
target_link_libraries(sobreviviendo PRIVATE ${SFML_LIBS})

if(MSVC)
    target_compile_options(sobreviviendo PRIVATE /W4)
else()
    target_compile_options(sobreviviendo PRIVATE -Wall -Wextra)
endif()

# Copiar assets y data junto al ejecutable
add_custom_command(TARGET sobreviviendo POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:sobreviviendo>/assets
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/data $<TARGET_FILE_DIR:sobreviviendo>/data)

# En Windows, copiar las DLL de SFML junto al ejecutable
if(WIN32 AND CMAKE_VERSION VERSION_GREATER_EQUAL 3.21)
    add_custom_command(TARGET sobreviviendo POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_RUNTIME_DLLS:sobreviviendo> $<TARGET_FILE_DIR:sobreviviendo>
        COMMAND_EXPAND_LISTS)
endif()
