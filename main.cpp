#include "chip8.h"

int main()
{
    sf::RenderWindow window(sf::VideoMode(1024, 512, 8), "Chip8");
    window.setFramerateLimit(60);

    Chip8 chip8(&window, "../chip8/Chip8-Games/PONG");

    while(window.isOpen())
    {
        sf::Event e;
        while(window.pollEvent(e))
        {
            if(e.type == sf::Event::Closed)
                window.close();

            if(e.type == sf::Event::KeyPressed)
            {
                if(e.key.code >= sf::Keyboard::A && e.key.code <= sf::Keyboard::F)
                switch(e.key.code)
                {
                case sf::Keyboard::A:
                    chip8.keyboard[12] = true;
                    break;
                case sf::Keyboard::B:
                    chip8.keyboard[14] = true;
                    break;
                case sf::Keyboard::C:
                    chip8.keyboard[3] = true;
                    break;
                case sf::Keyboard::D:
                    chip8.keyboard[7] = true;
                    break;
                case sf::Keyboard::E:
                    chip8.keyboard[11] = true;
                    break;
                case sf::Keyboard::F:
                    chip8.keyboard[15] = true;
                    break;
                }
                else if(e.key.code >= sf::Keyboard::Num0 && e.key.code <= sf::Keyboard::Num9)
                switch(e.key.code)
                {
                case sf::Keyboard::Num0:
                    chip8.keyboard[13] = true;
                    break;
                case sf::Keyboard::Num1:
                case sf::Keyboard::Num2:
                case sf::Keyboard::Num3:
                    chip8.keyboard[e.key.code - 26 - 1] = true;
                    break;
                case sf::Keyboard::Num4:
                case sf::Keyboard::Num5:
                case sf::Keyboard::Num6:
                    chip8.keyboard[e.key.code - 26] = true;
                    break;
                case sf::Keyboard::Num7:
                case sf::Keyboard::Num8:
                case sf::Keyboard::Num9:
                    chip8.keyboard[e.key.code - 26 + 1] = true;
                    break;
                }
            } else if(e.type == sf::Event::KeyReleased)
            {
                if(e.key.code >= sf::Keyboard::A && e.key.code <= sf::Keyboard::F)
                switch(e.key.code)
                {
                case sf::Keyboard::A:
                    chip8.keyboard[12] = false;
                    break;
                case sf::Keyboard::B:
                    chip8.keyboard[14] = false;
                    break;
                case sf::Keyboard::C:
                    chip8.keyboard[3] = false;
                    break;
                case sf::Keyboard::D:
                    chip8.keyboard[7] = false;
                    break;
                case sf::Keyboard::E:
                    chip8.keyboard[11] = false;
                    break;
                case sf::Keyboard::F:
                    chip8.keyboard[15] = false;
                    break;
                }
                else if(e.key.code >= sf::Keyboard::Num0 && e.key.code <= sf::Keyboard::Num9)
                switch(e.key.code)
                {
                case sf::Keyboard::Num0:
                    chip8.keyboard[13] = false;
                    break;
                case sf::Keyboard::Num1:
                case sf::Keyboard::Num2:
                case sf::Keyboard::Num3:
                    chip8.keyboard[e.key.code - 26 - 1] = false;
                    break;
                case sf::Keyboard::Num4:
                case sf::Keyboard::Num5:
                case sf::Keyboard::Num6:
                    chip8.keyboard[e.key.code - 26] = false;
                    break;
                case sf::Keyboard::Num7:
                case sf::Keyboard::Num8:
                case sf::Keyboard::Num9:
                    chip8.keyboard[e.key.code - 26 + 1] = false;
                    break;
                }
            }
        }

        chip8.Iterate();
    }

    return 0;
}
