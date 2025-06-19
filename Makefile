#
# Makefile for the Snake Game - Modern Edition
# Author: chmodxChironex
# Date: 2025
#

# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -I/usr/local/include
LDFLAGS = -L/usr/local/lib
LIBS = $(shell pkg-config --libs raylib) -lm

# Source files and object files
SRCS = main.cpp game.cpp
OBJS = $(SRCS:.cpp=.o)

# Target executable name
TARGET = snake_game

# Default target
all: $(TARGET)

# Rule to link the executable
$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET) $(LDFLAGS) $(LIBS)
	@echo "Snake game compiled successfully as '$(TARGET)'"

# Rule to compile source files into object files
%.o: %.cpp game.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Rule to run the game
run: all
	./$(TARGET)

# Rule to clean up the build directory
clean:
	rm -f $(OBJS) $(TARGET)
	@echo "Cleaned up build artifacts."

# Phony targets
.PHONY: all run clean