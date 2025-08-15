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
            uint32_t* screenPixel = &video[(yPos + row) * VIDEO_WIDTH + (xPos + col)];

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