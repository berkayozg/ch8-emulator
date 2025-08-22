# Makefile for CHIP-8 Emulator

# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -g
SDL_FLAGS = $(shell sdl2-config --cflags --libs)

# Source files
SRC = main.cpp chip8.cpp platform.cpp
OBJ = $(SRC:.cpp=.o)
DEP = $(SRC:.cpp=.d)

# Executable
TARGET = chip8

# Default target
all: $(TARGET)

# Link object files into executable
$(TARGET): $(OBJ)
	$(CXX) $(OBJ) -o $@ $(SDL_FLAGS)

# Compile source files into object files with dependency generation
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -MMD -c $< -o $@

# Include dependency files
-include $(DEP)

# Clean build files
clean:
	rm -f $(OBJ) $(DEP) $(TARGET)

.PHONY: all clean
