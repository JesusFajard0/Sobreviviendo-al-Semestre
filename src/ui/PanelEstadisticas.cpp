#include "ui/PanelEstadisticas.hpp"

namespace
{
    constexpr int N_GENERALES = 4;

    // Mismo orden que las barras 0 a 3 del panel.
    const Stat ORDEN[N_GENERALES] = {
        Stat::Energia, Stat::Estres, Stat::SaludMental, Stat::Dopamina
    };

    const char* const ETIQUETAS[5] = {
        "Energía", "Estrés", "Salud mental", "Dopamina", "Conocimiento"
    };

    const sf::Color COLORES[5] = {
        sf::Color(240, 200,  60),   // Energía: amarillo
        sf::Color(230,  90,  50),   // Estrés: naranja-rojo
        sf::Color( 90, 200, 110),   // Salud mental: verde
        sf::Color(170, 110, 220),   // Dopamina: violeta
        sf::Color( 80, 140, 230)    // Conocimiento: azul
    };

    const sf::Color COLOR_FONDO(15, 15, 22, 215);
    const sf::Color COLOR_BORDE(90, 90, 110);

    constexpr float  GROSOR_BORDE_PANEL = 1.f;
    constexpr float  MAXIMO_CONOCIMIENTO = 100.f;
}

void PanelEstadisticas::iniciar(const sf::Font* fuente, sf::Vector2f posicion)
{
    fondo_.setSize(sf::Vector2f(ANCHO, ALTO));
    fondo_.setPosition(posicion);
    fondo_.setFillColor(COLOR_FONDO);
    fondo_.setOutlineColor(COLOR_BORDE);
    fondo_.setOutlineThickness(GROSOR_BORDE_PANEL);

    for (std::size_t i = 0; i < barras_.size(); ++i)
    {
        barras_[i].iniciar(fuente, ETIQUETAS[i], COLORES[i]);
        barras_[i].colocar(sf::Vector2f(
            posicion.x + MARGEN,
            posicion.y + MARGEN + static_cast<float>(i) * BarraEstadistica::ALTO_FILA));
    }
}

void PanelEstadisticas::actualizar(const Estadisticas& stats,
                                   Materia materiaActual)
{
    for (int i = 0; i < N_GENERALES; ++i)
    {
        const Stat s = ORDEN[i];
        barras_[i].establecer(stats.obtener(s), stats.maximo(s),
                              stats.esCritica(s));
    }

    // El conocimiento nunca genera alerta.
    barras_[N_GENERALES].establecer(stats.conocimiento(materiaActual),
                                    MAXIMO_CONOCIMIENTO, false);
}

void PanelEstadisticas::dibujar(sf::RenderTarget& destino) const
{
    destino.draw(fondo_);

    for (const BarraEstadistica& b : barras_)
        b.dibujar(destino);
}