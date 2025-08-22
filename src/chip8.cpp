#include "chip8.hpp"
#include <chrono>
#include <cstring>
#include <fstream>
#include <random>

#define VIDEO_WIDTH 64
#define VIDEO_HEIGHT 32

const unsigned int START_ADDRESS = 0x200;
const unsigned int FONTSET_START_ADDRESS = 0x50;

// There are 16 characters at 5 bytes each, so 80 bytes of size
const unsigned int FONTSET_SIZE = 80;

uint8_t fontset[FONTSET_SIZE] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0,  // 0
    0x20, 0x60, 0x20, 0x20, 0x70,  // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0,  // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0,  // 3
    0x90, 0x90, 0xF0, 0x10, 0x10,  // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0,  // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0,  // 6
    0xF0, 0x10, 0x20, 0x40, 0x40,  // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0,  // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0,  // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90,  // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0,  // B
    0xF0, 0x80, 0x80, 0x80, 0xF0,  // C
    0xE0, 0x90, 0x90, 0x90, 0xE0,  // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0,  // E
    0xF0, 0x80, 0xF0, 0x80, 0x80   // F
};

Chip8::Chip8()
    : randGen(std::chrono::system_clock::now().time_since_epoch().count()) {
    // Initialize RNG
    randByte = std::uniform_int_distribution<uint8_t>(0, 255U);

    // Initialize PC
    pc = START_ADDRESS;

    // Load fonts into memory
    for (unsigned int i = 0; i < FONTSET_SIZE; ++i) {
        memory[FONTSET_START_ADDRESS + 1] = fontset[i];
    }

    // Default: all point to OP_NULL
    for (auto &f : table) {
        f = &Chip8::OP_NULL;
    }

    for (auto &f : table0) {
        f = &Chip8::OP_NULL;
    }

    for (auto &f : table8) {
        f = &Chip8::OP_NULL;
    }

    for (auto &f : tableE) {
        f = &Chip8::OP_NULL;
    }

    for (auto &f : tableF) {
        f = &Chip8::OP_NULL;
    }

    // Primary table dispatch by first nibble
    table[0x0] = &Chip8::Table0;   // 00E0, 00EE
    table[0x1] = &Chip8::OP_1nnn;  // JP addr
    table[0x2] = &Chip8::OP_2nnn;  // CALL addr
    table[0x3] = &Chip8::OP_3xkk;  // SE Vx, byte
    table[0x4] = &Chip8::OP_4xkk;  // SNE Vx, byte
    table[0x5] = &Chip8::OP_5xy0;  // SE Vx, Vy
    table[0x6] = &Chip8::OP_6xkk;  // LD Vx, byte
    table[0x7] = &Chip8::OP_7xkk;  // ADD Vx, byte
    table[0x8] = &Chip8::Table8;   // 8xy*
    table[0x9] = &Chip8::OP_9xy0;  // SNE Vx, Vy
    table[0xA] = &Chip8::OP_Annn;  // LD I, addr
    table[0xB] = &Chip8::OP_Bnnn;  // JP V0, addr
    table[0xC] = &Chip8::OP_Cxkk;  // RND Vx, byte
    table[0xD] = &Chip8::OP_Dxyn;  // DRW Vx, Vy, nibble
    table[0xE] = &Chip8::TableE;   // Ex9E / ExA1
    table[0xF] = &Chip8::TableF;   // Fx**

    // Table0 0x0xxx family
    table0[0x0] = &Chip8::OP_00E0;
    table0[0xE] = &Chip8::OP_00EE;

    // Table8 0x8xy family
    table8[0x0] = &Chip8::OP_8xy0;
    table8[0x1] = &Chip8::OP_8xy1;
    table8[0x2] = &Chip8::OP_8xy2;
    table8[0x3] = &Chip8::OP_8xy3;
    table8[0x4] = &Chip8::OP_8xy4;
    table8[0x5] = &Chip8::OP_8xy5;
    table8[0x6] = &Chip8::OP_8xy6;
    table8[0x7] = &Chip8::OP_8xy7;
    table8[0xE] = &Chip8::OP_8xyE;

    // TableE 0xEx** family
    tableE[0xE] = &Chip8::OP_Ex9E;
    tableE[0x1] = &Chip8::OP_ExA1;

    // TableF 0xFx** family
    tableF[0x07] = &Chip8::OP_Fx07;
    tableF[0x0A] = &Chip8::OP_Fx0A;
    tableF[0x15] = &Chip8::OP_Fx15;
    tableF[0x18] = &Chip8::OP_Fx18;
    tableF[0x1E] = &Chip8::OP_Fx1E;
    tableF[0x29] = &Chip8::OP_Fx29;
    tableF[0x33] = &Chip8::OP_Fx33;
    tableF[0x55] = &Chip8::OP_Fx55;
    tableF[0x65] = &Chip8::OP_Fx65;
}

/* Dispatcher functions */
void Chip8::Table0(void) {
    (this->*table0[opcode & 0x000Fu])();
}

void Chip8::Table8(void) {
    (this->*table8[opcode & 0x000Fu])();
}

void Chip8::TableE(void) {
    (this->*tableE[opcode & 0x000Fu])();
}

void Chip8::TableF(void) {
    (this->*tableF[opcode & 0x00FFu])();
}

void Chip8::LoadROM(char const *filename) {
    // Open the file as a stream of binary and move the file pointer to the end
    std::ifstream file(filename, std::ios::binary | std::ios::ate);

    if (file.is_open()) {
        // Get size of file and allocate a buffer to hold the contents
        std::streampos size = file.tellg();
        char *buffer = new char[size];

        // Go back to the beginning of the file and fill the buffer
        file.seekg(0, std::ios::beg);
        file.read(buffer, size);
        file.close();

        // Load the ROM contents into the Chip8's memory, starting at 0x200
        for (long i = 0; i < size; ++i) {
            memory[START_ADDRESS + i] = buffer[i];
        }

        // Free the buffer
        delete[] buffer;
    }
}

void Chip8::Cycle() {
    // Fetch
    opcode = (memory[pc] << 8u) | memory[pc + 1];

    // Increment the PC before we execute anything
    pc += 2;

    // Decode and Execute
    ((*this).*(table[(opcode & 0xF000u) >> 12u]))();

    // Decrement the delay timer if it's been set
    if (delayTimer > 0) {
        --delayTimer;
    }

    // Decrement the sound timer if it's been set
    if (soundTimer > 0) {
        --soundTimer;
    }
}

/* Clears the screen by setting all of the screen bits to zero. */
void Chip8::OP_00E0(void) {
    memset(video, 0, sizeof(video));
}

void Chip8::OP_00EE(void) {
    --sp;
    pc = stack[sp];
}

void Chip8::OP_1nnn(void) {
    uint16_t address = opcode & 0xFFFu;

    pc = address;
}

void Chip8::OP_2nnn(void) {
    uint16_t address = opcode & 0xFFFu;

    stack[sp] = pc;
    ++sp;
    pc = address;
}

void Chip8::OP_3xkk(void) {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t kk = opcode & 0x00FFu;

    if (registers[Vx] == kk) {
        pc += 2;
    }
}

void Chip8::OP_4xkk(void) {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t kk = opcode & 0x00FFu;

    if (registers[Vx] != kk) {
        pc += 2;
    }
}

void Chip8::OP_5xy0(void) {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    if (registers[Vx] == registers[Vy]) {
        pc += 2;
    }
}

void Chip8::OP_6xkk(void) {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t kk = opcode & 0x00FFu;

    registers[Vx] = kk;
}

void Chip8::OP_7xkk(void) {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t kk = opcode & 0x00FFu;

    registers[Vx] = registers[Vx] + kk;
}

void Chip8::OP_8xy0(void) {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    registers[Vx] = registers[Vy];
}

void Chip8::OP_8xy1(void) {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    registers[Vx] |= registers[Vy];
}

void Chip8::OP_8xy2(void) {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    registers[Vx] &= registers[Vy];
}

void Chip8::OP_8xy3(void) {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    registers[Vx] ^= registers[Vy];
}

void Chip8::OP_8xy4(void) {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    uint8_t prevVx = registers[Vx];
    registers[Vx] += registers[Vy];
    if (prevVx > registers[Vx]) {
        registers[0xF] = 1;
    } else {
        registers[0xF] = 0;
    }
}

void Chip8::OP_8xy5(void) {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    if (registers[Vx] > registers[Vy]) {
        registers[0xF] = 1;
    } else {
        registers[0xF] = 0;
    }
    registers[Vx] -= registers[Vy];
}

void Chip8::OP_8xy6(void) {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    registers[0xF] = (registers[Vx] & 0x1u);
    registers[Vx] >>= 1;
}

void Chip8::OP_8xy7(void) {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    if (registers[Vy] > registers[Vx]) {
        registers[0xF] = 1;
    } else {
        registers[0xF] = 0;
    }

    registers[Vx] = registers[Vy] - registers[Vx];
}

void Chip8::OP_8xyE(void) {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    registers[0xF] = (registers[Vx] & 0xFFu) >> 7u;
    registers[Vx] <<= 1;
}

void Chip8::OP_9xy0(void) {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    if (registers[Vx] != registers[Vy]) {
        pc += 2;
    }
}

void Chip8::OP_Annn(void) {
    uint16_t nnnValue = opcode & 0x0FFFu;

    index = nnnValue;
}

void Chip8::OP_Bnnn(void) {
    uint16_t nnnValue = opcode & 0x0FFFu;

    pc = nnnValue + registers[0];
}

void Chip8::OP_Cxkk(void) {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t kk = opcode & 0x00FFu;

    registers[Vx] = randByte(randGen) & kk;
}

void Chip8::OP_Dxyn(void) {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;
    uint8_t height = opcode & 0x000Fu;

    // making sure that it is in screen boundaries
    uint8_t xPos = registers[Vx] % VIDEO_WIDTH;
    uint8_t yPos = registers[Vy] % VIDEO_HEIGHT;

    registers[0xF] = 0;

    for (unsigned int row = 0; row < height; ++row) {
        uint8_t spriteByte = memory[index + row];

        for (uint8_t col = 0; col < 8; ++col) {
            uint8_t spritePixel = spriteByte & (0x80u >> col);
            uint32_t *screenPixel =
                &video[(yPos + row) * VIDEO_WIDTH + (xPos + col)];

            // Sprite pixel is on
            if (spritePixel) {
                // Screen pixel also on - collision
                if (*screenPixel == 0xFFFFFFFF) {
                    registers[0xF] = 1;
                }

                // XOR with the sprite pixel
                *screenPixel ^= 0xFFFFFFFF;
            }
        }
    }
}

void Chip8::OP_Ex9E(void) {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    uint8_t key = registers[Vx];

    if (keypad[key]) {
        pc += 2;
    }
}

void Chip8::OP_ExA1(void) {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    uint8_t key = registers[Vx];

    if (!keypad[key]) {
        pc += 2;
    }
}

void Chip8::OP_Fx07(void) {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    registers[Vx] = delayTimer;
}

void Chip8::OP_Fx0A(void) {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    for (uint8_t i = 0; i < 16; i++) {
        if (keypad[i]) {
            registers[Vx] = i;
            return;
        }
    }
    pc -= 2;
}

void Chip8::OP_Fx15(void) {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    delayTimer = registers[Vx];
}

void Chip8::OP_Fx18(void) {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    soundTimer = registers[Vx];
}

void Chip8::OP_Fx1E(void) {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    index = registers[Vx] + index;
}

void Chip8::OP_Fx29(void) {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    // Each character sprite is 5 bytes
    index = FONTSET_START_ADDRESS + (registers[Vx] * 5);
}

void Chip8::OP_Fx33(void) {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t value = registers[Vx];

    // Hundreds digit
    memory[index] = value / 100;

    // Tens digit
    memory[index + 1] = (value % 100) / 10;

    // Ones digit
    memory[index + 2] = value % 10;
}

void Chip8::OP_Fx55(void) {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    for (uint8_t i = 0; i < Vx; i++) {
        memory[index + i] = registers[i];
    }
}

void Chip8::OP_Fx65(void) {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    for (uint8_t i = 0; i < Vx; i++) {
        registers[i] = memory[index + i];
    }
}