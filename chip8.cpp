#include "chip8.h"
#include <cstdio>
#include <ctime>
#include <cmath>

Chip8::Chip8(sf::RenderWindow& window) : window(window)
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

    // Seed random number generation
    srand(time(nullptr));
}

// TODO: Possibly move to switch case
void Chip8::Decode(uint16_t &instruction)
{
    unsigned int leading = instruction & 0xF000;
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
        unsigned int reg = instruction & 0x0F00;
        uint8_t kk = instruction & 0x00FF;
        if(registers[reg] == kk)
            PC+=2;
    } else if(leading == 4) // Skip !=
    {
        unsigned int reg = instruction & 0x0F00;
        uint8_t kk = instruction & 0x00FF;
        if(registers[reg] != kk)
            PC+=2;
    } else if(leading == 5) // Skip == registers
    {
        unsigned int regX = instruction & 0x0F00;
        unsigned int regY = instruction & 0x00F0;
        if(registers[regX] == registers[regY])
            PC+=2;
    } else if(leading == 6) // Set Vx = kk
    {
        unsigned int reg = instruction & 0x0F00;
        uint8_t kk = instruction & 0x00FF;
        registers[reg] = kk;
    } else if(leading == 7) // Vx += kk
    {
        unsigned int reg = instruction & 0x0F00;
        uint8_t kk = instruction & 0x00FF;
        registers[reg] += kk;
    } else if(leading == 8)
    {
        unsigned int regX = instruction & 0x0F00;
        unsigned int regY = instruction & 0x00F0;

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
        unsigned int regX = instruction & 0x0F00;
        unsigned int regY = instruction & 0x00F0;
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
        unsigned int reg = instruction & 0x0F00;
        registers[reg] = random & kk;
    } else if(leading == 0xD) // Draw
    {
        unsigned int n = instruction & 0x000F;

        int bCount = 0;
        uint8_t bytes[n];
        for(uint16_t i = I; i < n; i++)
            bytes[bCount++] = memory[i];

        unsigned int regX = instruction & 0x0F00;
        unsigned int regY = instruction & 0x00F0;
        uint8_t coordX = registers[regX];
        uint8_t coordY = registers[regY];

        for(unsigned i = 0; i < n; i++)
        {
            // TODO: Find another way to check if bit flipped from 1 to 0
            for(int i = 0; i < 8; i++)
                if((screen[coordY+i][coordX] & (unsigned char)pow(2, i)) && !((screen[coordY+i][coordX]^bytes[i]) & (unsigned char)pow(2, i)))
                    registers[0xF] |= 1;

            screen[coordY+i % 32][coordX % 64] ^= bytes[i];
        }
    } else if(leading == 0xE)
    {
        // TODO: Add keyboard
    } else if(leading == 0xF)
    {
        uint8_t lastByte = instruction & 0x00FF;
        unsigned int reg = instruction & 0x0F00;
        if(lastByte == 0x07)
            registers[reg] = delayTimer;
        else if(lastByte == 0x0A)
        {
            // TODO: Add keyboard
        } else if(lastByte == 0x15)
            delayTimer = registers[reg];
        else if(lastByte == 0x18)
            soundTimer = registers[reg];
        else if(lastByte == 0x1E)
            I += registers[reg];
        else if(lastByte == 0x29)
        {
            // TODO: What location?
        } else if(lastByte == 0x33)
        {
            unsigned int decimal = registers[reg];
            memory[I] = decimal / 100;
            memory[I+1] = decimal / 10 % 10;
            memory[I+2] = decimal % 10;
        } else if(lastByte == 0x55) // TODO: Check
        {
            for(unsigned i = 0; i <= reg; i++)
                memory[I+i] = registers[i];
        } else if(lastByte == 0x65)
        {
            for(unsigned i = 0; i <= reg; i++)
                registers[i] = memory[I+i];
        }
    }
}









