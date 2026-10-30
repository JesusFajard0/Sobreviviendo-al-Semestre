// Sobreviviendo al Semestre - prueba de entorno (semana 0)
// Comprueba que SFML y DeSiGNAR compilan y enlazan. Se reemplaza en la semana 1.

#include <SFML/Graphics.hpp>
#include <iostream>
#include <optional>

#include <mtreenode.hpp>   // DeSiGNAR: árbol general (se usará para el mapa)

using Designar::MTreeNode;

int main()
{
    // --- Prueba de DeSiGNAR: un árbol con una raíz y dos hijos ---
    MTreeNode<int>* raiz = new MTreeNode<int>(0);
    raiz->append_child(new MTreeNode<int>(1));
    raiz->append_child(new MTreeNode<int>(2));
    std::cout << "DeSiGNAR OK -> primer hijo: " << raiz->get_first_child()->get_key()
              << ", ultimo hijo: " << raiz->get_last_child()->get_key() << '\n';
    MTreeNode<int>::destroy_tree(raiz);

    // --- Prueba de SFML: ventana 1280x720 con vista logica de 640x360 ---
#if SFML_VERSION_MAJOR >= 3
    sf::RenderWindow ventana(sf::VideoMode({1280u, 720u}), "Sobreviviendo al Semestre");
#else
    sf::RenderWindow ventana(sf::VideoMode(1280, 720), "Sobreviviendo al Semestre");
#endif
    ventana.setFramerateLimit(60);

    sf::View vista;
    vista.setSize(sf::Vector2f(640.f, 360.f));
    vista.setCenter(sf::Vector2f(320.f, 180.f));
    ventana.setView(vista);

    sf::RectangleShape jugador(sf::Vector2f(24.f, 36.f));
    jugador.setFillColor(sf::Color(59, 130, 246));
    jugador.setPosition(sf::Vector2f(308.f, 160.f));

    sf::Clock reloj;
    while (ventana.isOpen())
    {
#if SFML_VERSION_MAJOR >= 3
        while (const std::optional evento = ventana.pollEvent())
        {
            if (evento->is<sf::Event::Closed>())
                ventana.close();
        }
#else
        sf::Event evento;
        while (ventana.pollEvent(evento))
        {
            if (evento.type == sf::Event::Closed)
                ventana.close();
        }
#endif

        const float dt = reloj.restart().asSeconds();
        const float velocidad = 120.f;
        sf::Vector2f mov(0.f, 0.f);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))  mov.x -= velocidad * dt;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) mov.x += velocidad * dt;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))    mov.y -= velocidad * dt;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))  mov.y += velocidad * dt;
        jugador.move(mov);

        ventana.clear(sf::Color(30, 30, 40));
        ventana.draw(jugador);
        ventana.display();
    }
    return 0;
}
