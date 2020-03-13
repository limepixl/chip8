#include "chip8.h"

int main()
{
    sf::RenderWindow window(sf::VideoMode(512, 256, 8), "Chip8");
    window.setFramerateLimit(60);

    Chip8 chip8(&window, "../chip8/Chip8-Games/logo.ch8");

    while(window.isOpen())
    {
        sf::Event e;
        while(window.pollEvent(e))
        {
            if(e.type == sf::Event::Closed)
                window.close();
        }

        window.clear();
        chip8.Iterate();
        window.display();
    }

    return 0;
}
