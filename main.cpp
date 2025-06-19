/**
 * @file main.cpp
 * @brief The entry point for the Snake Game - Modern Edition.
 * @author chmodxChironex
 * @date 2025
 */

#include <iostream>
#include <string>
#include "game.hpp"

/**
 * @brief The main function, serving as the program's entry point.
 * @details It prompts the user for their name, creates a Game instance,
 * and starts the main game loop.
 * @return 0 on successful execution, 1 on error.
 */
int main() {
    std::string playerName;
    
    std::cout << "Welcome to Snake Game - Modern Edition!" << std::endl;
    std::cout << "Enter your name: ";
    std::getline(std::cin, playerName);
    
    if (playerName.empty()) {
        playerName = "Player";
    }
    
    try {
        Game game(playerName);
        game.run();
    } catch (const std::exception& e) {
        std::cerr << "An unhandled exception occurred: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}