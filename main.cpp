#include "chip8.h"
#include <unordered_map>

int main()
{
    sf::RenderWindow window(sf::VideoMode(1024, 512, 8), "Chip8");
    window.setFramerateLimit(120);

#ifdef _WIN32
    Chip8 chip8(&window, "../../../Chip8-Games/PONG");
#else
    Chip8 chip8(&window, "../chip8/Chip8-Games/PONG");
#endif

    std::unordered_map<sf::Keyboard::Key, int> keymap
    {
        {sf::Keyboard::Num1, 0},
        {sf::Keyboard::Num2, 1},
        {sf::Keyboard::Num3, 2},
        {sf::Keyboard::Num4, 3},
        {sf::Keyboard::Q, 4},
        {sf::Keyboard::W, 5},
        {sf::Keyboard::E, 6},
        {sf::Keyboard::R, 7},
        {sf::Keyboard::A, 8},
        {sf::Keyboard::S, 9},
        {sf::Keyboard::D, 10},
        {sf::Keyboard::F, 11},
        {sf::Keyboard::Z, 12},
        {sf::Keyboard::X, 13},
        {sf::Keyboard::C, 14},
        {sf::Keyboard::V, 15}
    };

    while(window.isOpen())
    {
        sf::Event e;
        while(window.pollEvent(e))
        {
            if(e.type == sf::Event::Closed)
                window.close();

            if(e.type == sf::Event::KeyPressed)
                chip8.keyboard[keymap[e.key.code]] = true;
            else if(e.type == sf::Event::KeyReleased)
                chip8.keyboard[keymap[e.key.code]] = false;
        }

        chip8.Iterate();
    }

    return 0;
}
