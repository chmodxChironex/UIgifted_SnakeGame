/**
 * @file game.hpp
 * @brief Game class definition for Snake game using Raylib.
 * @author chmodxChironex
 * @date 2025
 */

#ifndef GAME_HPP
#define GAME_HPP

#include <vector>
#include <string>
#include <deque>
#include <array>
#include "raylib.h"

struct Position {
    int x, y;
    bool operator==(const Position& other) const {
        return x == other.x && y == other.y;
    }
};

struct ScoreEntry {
    std::string name;
    int score;
    bool operator<(const ScoreEntry& other) const {
        return score > other.score;
    }
};

enum class Direction {
    UP, DOWN, LEFT, RIGHT
};

enum class GameState {
    MENU,
    PLAYING,
    PAUSED,
    GAME_OVER,
    LEADERBOARD,
    SETTINGS
};

class Game {
public:
    Game(const std::string& playerName);
    ~Game();
    void run();

private:
    // Game Constants
    static constexpr int CELL_SIZE = 20;
    static constexpr int GRID_WIDTH = 30;
    static constexpr int GRID_HEIGHT = 20;
    static constexpr int SCREEN_WIDTH = GRID_WIDTH * CELL_SIZE + 250;
    static constexpr int SCREEN_HEIGHT = GRID_HEIGHT * CELL_SIZE + 150;
    static constexpr int MAX_LEADERBOARD_ENTRIES = 10;
    static constexpr int MAX_OBSTACLES = 100;
    
    // Game State
    GameState currentState;
    std::deque<Position> snake;
    Position food;
    std::vector<Position> obstacles;
    Direction currentDirection;
    Direction nextDirection;
    
    // Scoring & Timers
    int score;
    int overallHighestScore;
    int personalBestScore;
    float moveTimer;
    float moveInterval;
    float animationTimer;
    
    // Player & UI
    std::string playerName;
    std::vector<ScoreEntry> leaderboard;
    int selectedMenuItem;
    
    // System & Settings
    bool gameRunning;
    bool showGrid;

    // Resources
    Font font;

    struct Colors {
        Color background = {15, 15, 25, 255};
        Color snakeHead = {100, 255, 100, 255};
        Color snakeBody = {50, 200, 50, 255};
        Color food = {255, 100, 100, 255};
        Color obstacle = {120, 120, 120, 255};
        Color grid = {30, 30, 40, 255};
        Color ui = {200, 200, 200, 255};
        Color accent = {100, 150, 255, 255};
        Color warning = {255, 200, 100, 255};
        Color success = {100, 255, 150, 255};
    } colors;

    // Core Logic
    void update(float deltaTime);
    void handleInput();
    void reset();
    void changeState(GameState newState);

    // Initialization and Data Management
    void initializeSnake();
    void generateFood();
    void loadObstacles(const std::string& filename);
    void loadHighestScores();
    void savePersonalBest();
    void saveSessionBestScore();
    void loadLeaderboard();
    void saveToLeaderboard();
    void loadSettings();
    void saveSettings();

    // Collision and Movement
    void moveSnake();
    bool checkCollision();

    // Drawing
    void draw();
    void drawGameField();
    void drawGrid();
    void drawUI();
    void drawMenu();
    void drawGameOver();
    void drawLeaderboard();
    void drawSettings();
    void drawPauseOverlay();
    
    // UI Helpers
    void drawProgressBar(int x, int y, int width, int height, float progress, Color color);
    void drawGlowEffect(Position pos, Color color, float intensity);
    void updateAnimations(float deltaTime);
    
    // Input Handlers
    void handleMenuInput();
    void handleGameInput();
    void handleLeaderboardInput();
    void handleSettingsInput();

    // Utility
    int getDifficultyLevel() const;
    float calculateSpeed() const;
};

#endif // GAME_HPP