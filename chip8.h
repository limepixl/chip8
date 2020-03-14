#pragma once
#include <inttypes.h>
#include <SFML/Graphics.hpp>

struct Chip8
{
    // Monochrome 64x32 screen
    uint8_t screen[32][64];
    bool shouldRedraw = false;

    // 0x000 to 0xFFF
    // Non-interpreter memory starts at 0x200
    uint8_t memory[4096];

    // V0 to VF
    // VF should be avoided as it is the carry flag
    uint8_t registers[16];

    // Address register
    uint16_t I;

    // Special purpose
    uint8_t delayTimer = 0, soundTimer = 0;

    // Program counter
    uint16_t PC;

    // Stack pointer
    uint8_t SP;

    // Stack
    uint16_t stack[16];

    // Keyboard values (0, 1, 2, .., E, F)
    bool keyboard[16];

    // SFML related things
    sf::RenderWindow* window;
    sf::Image screenImage;
    sf::Texture tex;
    sf::Sprite spr;

    Chip8(sf::RenderWindow* window, const char* gamePath);

    void Decode(uint16_t instruction);
    void Iterate();
};
