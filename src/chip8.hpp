#pragma once

#include <cstdint>

class Chip8 {
public:
    Chip8();

    // ROM
    void LoadROM(char const *filename);

    // Registers & Memory
    uint8_t registers[16]{};
    uint8_t memory[4096]{};
    uint16_t index{};
    uint16_t pc{};

    // Stack
    uint16_t stack[16]{};
    uint8_t sp{};

    // Timers
    uint8_t delayTimer{};
    uint8_t soundTimer{};

    // I/O
    uint8_t keypad[16]{};
    uint32_t video[64 * 32]{};

    // Opcode
    uint16_t opcode;

    // RNG
    std::default_random_engine randGen;
    std::uniform_int_distribution<uint8_t> randByte;

    // CHIP-8 Instructions
    void OP_00E0(void);  // Clear the display
    void OP_00EE(void);  // Return from a subroutine
    void OP_1nnn(void);  // Jump to location nnn
    void OP_2nnn(void);  // Call subroutine at nnn
    void OP_3xkk(void);  // Skip next instruction if Vx = kk
    void OP_4xkk(void);
    void OP_5xy0(void);
    void OP_6xkk(void);
    void OP_7xkk(void);
    void OP_8xy0(void);
    void OP_8xy1(void);
    void OP_8xy2(void);
    void OP_8xy3(void);
    void OP_8xy4(void);
    void OP_8xy5(void);
    void OP_8xy6(void);
    void OP_8xy7(void);
    void OP_8xyE(void);
};
