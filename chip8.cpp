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

    // Initialize memory to 0
    for(int i = 80; i < 4096; i++)
        memory[i] = 0;

    // Reset registers and stack
    for(int i = 0; i < 16; i++)
    {
        registers[i] = 0;
        stack[i] = 0;
    }

    PC = 0x200;
    I = 0;
    SP = 0;

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
    fread(buffer, 1, size, game);
    fclose(game);

    // Store game into memory
    for(int i = 0; i < size; i++)
        memory[i + 0x200] = buffer[i];
}

// TODO: Possibly move to switch case
void Chip8::Decode(uint16_t &instruction)
{
    unsigned int leading = (instruction & 0xF000) >> 12;
    unsigned int regX = (instruction & 0x0F00) >> 8;
    unsigned int regY = (instruction & 0x00F0) >> 4;
    if(leading == 0)
    {
        unsigned int last = instruction & 0x000F;
        if(last == 0)   // Screen clear
        {
            for(int i = 0; i < 4; i++)
            for(int j = 0; j < 8; j++)
                screen[i][j] = 0x00;
        }
        else if(last == 0x000E) // Return from subroutine
        {
            PC = stack[SP];
            SP--;
        } else
            printf("Unknown instruction! %#06X\n", instruction);
    } else if(leading == 1) // Jump
    {
        uint16_t address = instruction & 0x0FFF;
        PC = address;
    } else if(leading == 2) // Call subroutine
    {
        uint16_t address = instruction & 0x0FFF;
        stack[++SP] = PC;
        PC = address;
    } else if(leading == 3) // Skip ==
    {
        uint8_t kk = instruction & 0x00FF;
        if(registers[regX] == kk)
            PC+=2;
    } else if(leading == 4) // Skip !=
    {
        uint8_t kk = instruction & 0x00FF;
        if(registers[regX] != kk)
            PC+=2;
    } else if(leading == 5) // Skip == registers
    {
        if(registers[regX] == registers[regY])
            PC+=2;
    } else if(leading == 6) // Set Vx = kk
    {
        uint8_t kk = instruction & 0x00FF;
        registers[regX] = kk;
    } else if(leading == 7) // Vx += kk
    {
        uint8_t kk = instruction & 0x00FF;
        registers[regX] += kk;
    } else if(leading == 8)
    {
        unsigned int subtype = instruction & 0x000F;
        if(subtype == 0)    // Vx = Vy
            registers[regX] = registers[regY];
        else if(subtype == 1) // Vx |= Vy
            registers[regX] |= registers[regY];
        else if(subtype == 2) // Vx &= Vy
            registers[regX] &= registers[regY];
        else if(subtype == 3) // Vx ^= Vy
            registers[regX] ^= registers[regY];
        else if(subtype == 4) // Vx += Vy
        {
            registers[0xF] = (registers[regX] + registers[regY] > 255);
            registers[regX] += registers[regY];
        } else if(subtype == 5) // Vx -= Vy
        {
            registers[0xF] = registers[regX] > registers[regY];
            registers[regX] -= registers[regY];
        } else if(subtype == 6) // SHR
        {
            registers[0xF] = ((registers[regX] & 0x01) == 1);
            registers[regX] >>= 1;
        } else if(subtype == 7) // SUBN
        {
            registers[0xF] = (registers[regY] > registers[regX]);
            registers[regX] = registers[regY] - registers[regX];
        } else if(subtype == 0xE)   // SHL
        {
            registers[0xF] = ((registers[regX] & 0x80) == 1);
            registers[regX] <<= 1;
        }
    } else if(leading == 9) // Skip != registers
    {
        if(registers[regX] != registers[regY])
            PC+=2;
    } else if(leading == 0xA) // I = nnn
    {
        uint16_t address = instruction & 0x0FFF;
        I = address;
    } else if(leading == 0xB) // Jump nnn + v0
    {
        uint16_t address = instruction & 0x0FFF;
        PC = registers[0] + address;
    } else if(leading == 0xC) // rand & kk
    {
        uint8_t random = (unsigned int)rand() % 256;
        uint8_t kk = instruction & 0x00FF;
        registers[regX] = random & kk;
    } else if(leading == 0xD) // Draw
    {
        unsigned int n = instruction & 0x000F;

        uint8_t bytes[n];
        for(uint16_t i = 0; i < n; i++)
            bytes[i] = memory[i+I];

        uint8_t coordX = registers[regX];
        uint8_t coordY = registers[regY];

        for(unsigned i = 0; i < n; i++)
        {
            registers[0xF] = 0;

            // Check every bit in bytes (screen byte and to-be XORed byte)
            for(int j = 0; j < 8; j++)
            {
                uint8_t screenBit = (screen[(coordY+i) % 32][coordX % 64] & (0b00000001 << j)) >> j;
                uint8_t xorBit = (bytes[i] % (0x00000001 << j)) >> j;

                if(screenBit == 1 && xorBit == 0)
                {
                    registers[0xF] = 1;
                    break;
                }
            }

            screen[(coordY+i) % 32][coordX % 64] ^= bytes[i];
        }
    } else if(leading == 0xE)
    {
        if(keyboard[registers[regX]])
            PC+=2;
    } else if(leading == 0xF)
    {
        uint8_t lastByte = instruction & 0x00FF;
        if(lastByte == 0x07)
            registers[regX] = delayTimer;
        else if(lastByte == 0x0A)   // Wait for key press
        {
            sf::Event e;
            while(true)
            {
                window->pollEvent(e);
                if(e.type == sf::Event::KeyPressed)
                {
                    if(e.key.code >= sf::Keyboard::A && e.key.code <= sf::Keyboard::F)
                    {
                        keyboard[e.key.code + 10] = true;
                        registers[regX] = e.key.code + 10;
                        break;
                    }
                    else if(e.key.code >= sf::Keyboard::Num0 && e.key.code <= sf::Keyboard::Num9)
                    {
                        keyboard[e.key.code - 26] = true;
                        registers[regX] = e.key.code - 26;
                        break;
                    }
                }
            }
        } else if(lastByte == 0x15)
            delayTimer = registers[regX];
        else if(lastByte == 0x18)
            soundTimer = registers[regX];
        else if(lastByte == 0x1E)
            I += registers[regX];
        else if(lastByte == 0x29)
            I = memory[5*registers[regX]];
        else if(lastByte == 0x33)
        {
            unsigned int decimal = registers[regX];
            memory[I] = decimal / 100;
            memory[I+1] = decimal / 10 % 10;
            memory[I+2] = decimal % 10;
        } else if(lastByte == 0x55)
        {
            for(unsigned i = 0; i <= regX; i++)
                memory[I+i] = registers[i];
        } else if(lastByte == 0x65)
        {
            for(unsigned i = 0; i <= regX; i++)
                registers[i] = memory[I+i];
        }
    }

    PC+=2; // Fetch the next instruction
}

void Chip8::Iterate()
{
    uint16_t instruction = memory[PC] << 8 | memory[PC+1];
    Decode(instruction);

    // Transfer screen to SFML screen
    sf::Image image;
    image.create(64, 32);
    for(int i = 0; i < 32; i++)
    for(int j = 0; j < 64; j++)
        if(screen[i][j])
            image.setPixel(j, i, sf::Color::White);
        else
            image.setPixel(j, i, sf::Color::Black);

    sf::Texture tex;
    tex.loadFromImage(image);

    sf::Sprite spr(tex);
    spr.scale(4.0f, 4.0f);
    window->draw(spr);
}
