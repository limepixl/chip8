#include "chip8.h"
#include <cstdio>
#include <ctime>
#include <cmath>

Chip8::Chip8(sf::RenderWindow* window, const char* gamePath) : window(window)
{
    // Hexadecimal digits in interpreter memory 5*8 (5B)
    uint8_t digits[] = {
        0xF0, 0x90, 0x90, 0x90, 0xF0,   // 0
        0x20, 0x60, 0x20, 0x20, 0x70,   // 1
        0xF0, 0x10, 0xF0, 0x80, 0xF0,   // 2
        0xF0, 0x10, 0xF0, 0x10, 0xF0,   // 3
        0x90, 0x90, 0xF0, 0x10, 0x10,   // 4
        0xF0, 0x80, 0xF0, 0x10, 0xF0,   // 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0,   // 6
        0xF0, 0x10, 0x20, 0x40, 0x40,   // 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0,   // 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0,   // 9
        0xF0, 0x90, 0xF0, 0x90, 0x90,   // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0,   // B
        0xF0, 0x80, 0x80, 0x80, 0x80,   // C
        0xE0, 0x90, 0x90, 0x90, 0xE0,   // D
        0xF0, 0x80, 0xF0, 0x80, 0xF0,   // E
        0xF0, 0x80, 0xF0, 0x80, 0x80    // F
    };

    // Store digits in first 80 bytes of memory
    for(int i = 0; i < 80; i++)
        memory[i] = digits[i];

    // Initialize memory (after 80) to 0
    for(int i = 80; i < 4096; i++)
        memory[i] = 0;

    // Reset registers and stack
    for(int i = 0; i < 16; i++)
    {
        registers[i] = 0;
        stack[i] = 0;
        keyboard[i] = false;
    }

    PC = 0x200;
    I = 0;
    SP = 0;
    delayTimer = 0;
    soundTimer = 0;

    // Clear display
    for(int i = 0; i < 32; i++)
    for(int j = 0; j < 64; j++)
        screen[i][j] = 0;

    // Seed random number generation
    srand(time(nullptr));

    // Load game from file
    FILE* game = fopen(gamePath, "rb");
    if(game == nullptr)
    {
        printf("Failed to load game from file!");
        exit(-1);
    }

    fseek(game, 0L, SEEK_END);
    unsigned long size = ftell(game);
    rewind(game);

    uint8_t buffer[size];
    unsigned long res = fread(buffer, 1, size, game);
    if(res != size)
        exit(-1);
    fclose(game);

    // Store game into memory
    for(unsigned i = 0; i < size; i++)
        memory[i + 0x200] = (uint8_t)buffer[i];

    screenImage.create(64, 32);
}

// I know, I know. Switch cases are evil.
// But in this case, they're a bit faster!
void Chip8::Decode(uint16_t instruction)
{
    uint8_t leading = (instruction & 0xF000) >> 12;
    uint8_t regX = (instruction & 0x0F00) >> 8;
    uint8_t regY = (instruction & 0x00F0) >> 4;
    uint16_t address = instruction & 0x0FFF;

    switch(leading)
    {
    case 0:
    {
        uint8_t last = instruction & 0x000F;
        if(last == 0)   // Screen clear
        {
            for(int i = 0; i < 32; i++)
            for(int j = 0; j < 64; j++)
            {
                screen[i][j] = 0x00;
                screenImage.setPixel(j, i, sf::Color::Black);
            }

            shouldRedraw = true;
            PC+=2;
        }
        else if(last == 0xE) // Return from subroutine
        {
            PC = stack[SP];
            SP--;
            PC+=2;
        }

        break;
    }
    case 1: // Jump
        PC = address;
        break;
    case 2: // Call subroutine
    {
        stack[++SP] = PC;
        PC = address;
        break;
    }
    case 3: // Skip ==
    {
        uint8_t kk = instruction & 0x00FF;
        if(registers[regX] == kk)
            PC+=4;
        else
            PC+=2;
        break;
    }
    case 4: // Skip !=
    {
        uint8_t kk = instruction & 0x00FF;
        if(registers[regX] != kk)
            PC+=4;
        else
            PC+=2;
        break;
    }
    case 5: // Skip == registers
    {
        if(registers[regX] == registers[regY])
            PC+=4;
        else
            PC+=2;
        break;
    }
    case 6: // Set Vx = kk
    {
        uint8_t kk = instruction & 0x00FF;
        registers[regX] = kk;
        PC+=2;
        break;
    }
    case 7: // Vx += kk
    {
        uint8_t kk = instruction & 0x00FF;
        registers[regX] += kk;
        PC+=2;
        break;
    }
    case 8:
    {
        uint8_t subtype = instruction & 0x000F;
        switch(subtype)
        {
        case 0:    // Vx = Vy
        {
            registers[regX] = registers[regY];
            PC+=2;
            break;
        }
        case 1: // Vx |= Vy
        {
            registers[regX] |= registers[regY];
            PC+=2;
            break;
        }
        case 2: // Vx &= Vy
        {
            registers[regX] &= registers[regY];
            PC+=2;
            break;
        }
        case 3: // Vx ^= Vy
        {
            registers[regX] ^= registers[regY];
            PC+=2;
            break;
        }
        case 4: // Vx += Vy
        {
            registers[0xF] = ((registers[regX] + registers[regY]) > 0xFF);
            registers[regX] += registers[regY];
            PC+=2;
            break;
        }
        case 5: // Vx -= Vy
        {
            registers[0xF] = registers[regX] > registers[regY];
            registers[regX] -= registers[regY];
            PC+=2;
            break;
        }
        case 6: // SHR
        {
            registers[0xF] = (registers[regX] & 0x01);
            registers[regX] >>= 1;
            PC+=2;
            break;
        }
        case 7: // SUBN
        {
            registers[0xF] = (registers[regY] > registers[regX]);
            registers[regX] = registers[regY] - registers[regX];
            PC+=2;
            break;
        }
        case 0xE: // SHL
        {
            registers[0xF] = (registers[regX] & 0x80);
            registers[regX] <<= 1;
            PC+=2;
            break;
        }
        default:
            printf("Unknown subtype %#4X for instruction %#6X\n", subtype, instruction);
            break;
        }
        break;
    }
    case 9: // Skip != registers
    {
        if(registers[regX] != registers[regY])
            PC+=4;
        else
            PC+=2;
        break;
    }
    case 0xA: // I = nnn
    {
        I = address;
        PC+=2;
        break;
    }
    case 0xB: // Jump nnn + v0
    {
        PC = registers[0] + address;
        break;
    }
    case 0xC: // rand & kk
    {
        uint8_t random = rand() % 256;
        uint8_t kk = instruction & 0x00FF;
        registers[regX] = random & kk;

        PC+=2;
        break;
    }
    case 0xD: // Draw
    {
        unsigned int height = instruction & 0x000F;

        uint8_t bytes[height];
        for(unsigned i = 0; i < height; i++)
            bytes[i] = memory[i+I];

        uint8_t coordX = registers[regX];
        uint8_t coordY = registers[regY];

        registers[0xF] = 0;
        for(unsigned yLine = 0; yLine < height; yLine++)
        {
            for(unsigned xLine = 0; xLine < 8; xLine++)
            {
                uint8_t screenByte = screen[(coordY + yLine) % 32][(coordX + xLine) % 64];
                uint8_t XORByte = bytes[yLine] & (0x80 >> xLine);

                // If bit was white but got flipped, collision
                registers[0xF] |= screenByte && XORByte;
                if(screenByte ^ XORByte)
                    screenImage.setPixel((coordX + xLine) % 64, (coordY + yLine) % 32, sf::Color::White);
                else
                    screenImage.setPixel((coordX + xLine) % 64, (coordY + yLine) % 32, sf::Color::Black);

                screen[(coordY + yLine) % 32][(coordX + xLine) % 64] = (screenByte ^ XORByte);
            }
        }

        shouldRedraw = true;
        PC+=2;
        break;
    }
    case 0xE:
    {
        uint8_t subtype = instruction & 0x000F;
        if(subtype == 0xE)
        {
            if(keyboard[registers[regX]])
                PC+=4;
            else
                PC+=2;

        } else if(subtype == 0x1)
        {
            if(!keyboard[registers[regX]])
                PC+=4;
            else
                PC+=2;
        }
        break;
    }
    case 0xF:
    {
        uint8_t lastByte = instruction & 0x00FF;
        switch(lastByte)
        {
        case 0x07:
        {
            registers[regX] = delayTimer;
            PC+=2;
            break;
        }
        case 0x0A:   // Wait for key press
        {
            bool pressed = false;
            for(int i = 0; i < 16; i++)
                pressed &= keyboard[i];

            if(!pressed)
                return;

            PC+=2;
            break;
        }
        case 0x15:
        {
            delayTimer = registers[regX];
            PC+=2;
            break;
        }
        case 0x18:
        {
            soundTimer = registers[regX];
            PC+=2;
            break;
        }
        case 0x1E:
        {
            registers[0xF] = (I + registers[regX] > 0xFFF);
            I += registers[regX];
            PC+=2;
            break;
        }
        case 0x29:
        {
            I = registers[regX]*5;
            PC+=2;
            break;
        }
        case 0x33:
        {
            unsigned int decimal = registers[regX];
            memory[I] = decimal / 100;
            memory[I+1] = (decimal / 10) % 10;
            memory[I+2] = decimal % 10;

            PC+=2;
            break;
        }
        case 0x55:
        {
            for(unsigned i = 0; i <= regX; i++)
                memory[I+i] = registers[i];

            PC+=2;
            break;
        }
        case 0x65:
        {
            for(unsigned i = 0; i <= regX; i++)
                registers[i] = memory[I+i];

            PC+=2;
            break;
        }
        }
        break;
    }
    default:
        printf("Unknown instruction: %#6X\n", instruction);
    }
}

void Chip8::Iterate()
{
    uint16_t instruction = memory[PC] << 8 | memory[PC+1];

    printf("Instruction: %#6X\n", instruction);
    Decode(instruction);

    // TODO: Not working!
    // Update keys
    for(int i = 0; i < 16; i++)
        keyboard[i] = false;
    sf::Event e;
    while(window->pollEvent(e))
    {
        if(e.type == sf::Event::KeyPressed)
        {
            if(e.key.code >= sf::Keyboard::A && e.key.code <= sf::Keyboard::F)
                keyboard[e.key.code + 10] = true;
            else if(e.key.code >= sf::Keyboard::Num0 && e.key.code <= sf::Keyboard::Num9)
                keyboard[e.key.code - 26] = true;
        } else if(e.type == sf::Event::KeyReleased)
        {
            if(e.key.code >= sf::Keyboard::A && e.key.code <= sf::Keyboard::F)
                keyboard[e.key.code + 10] = false;
            else if(e.key.code >= sf::Keyboard::Num0 && e.key.code <= sf::Keyboard::Num9)
                keyboard[e.key.code - 26] = false;
        }
    }

    // Transfer screen to SFML screen
    // TODO: SFML texture updating is slow
    if(shouldRedraw)
    {
        shouldRedraw = false;

        tex.loadFromImage(screenImage);
        sf::Sprite sprite(tex);
        sprite.scale(16.0f, 16.0f);

        window->clear();
        window->draw(sprite);
        window->display();
    }

    // Update timers
    delayTimer -= (delayTimer > 0);
    soundTimer -= (soundTimer > 0);
}
