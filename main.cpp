#include "chip8.h"

int main()
{
    sf::RenderWindow window(sf::VideoMode(1024, 512, 8), "Chip8");
    window.setFramerateLimit(60);

    Chip8 chip8(&window, "../chip8/Chip8-Games/TICTAC");

    while(window.isOpen())
    {
        sf::Event e;
        while(window.pollEvent(e))
        {
            if(e.type == sf::Event::Closed)
                window.close();
        }

        chip8.Iterate();
    }

    return 0;
}
