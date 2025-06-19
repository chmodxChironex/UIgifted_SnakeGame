/**
 * @file game.cpp
 * @brief Implementation of the Snake game.
 * @author chmodxChironex
 * @date 2025
 */

#include "game.hpp"
#include <fstream>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <map>
#include <cmath>

// Constructor: Initializes the game, loads resources and settings
Game::Game(const std::string& playerName) 
    : currentState(GameState::MENU), score(0), overallHighestScore(0), personalBestScore(0),
      moveTimer(0.0f), moveInterval(0.15f), animationTimer(0.0f),
      playerName(playerName), selectedMenuItem(0), gameRunning(true), showGrid(true) {
    
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Snake Game - Modern Edition");
    SetTargetFPS(60);
    
    font = LoadFont("resources/roboto.ttf");
    if (font.texture.id == 0) {
        font = GetFontDefault();
    }
    
    srand(time(nullptr));
    
    loadSettings();
    loadHighestScores();
    loadLeaderboard();
    loadObstacles("obstacles.txt");
    reset();
}

// Destructor: Saves session data and releases resources
Game::~Game() {
    saveSessionBestScore();
    saveSettings();
    UnloadFont(font);
    CloseWindow();
}

// Main game loop
void Game::run() {
    while (!WindowShouldClose() && gameRunning) {
        float deltaTime = GetFrameTime();
        
        handleInput();
        
        if (currentState == GameState::PLAYING) {
            update(deltaTime);
        }
        
        updateAnimations(deltaTime);
        draw();
    }
}

// Handles input based on current game state
void Game::handleInput() {
    switch (currentState) {
        case GameState::MENU:         handleMenuInput(); break;
        case GameState::PLAYING:      handleGameInput(); break;
        case GameState::LEADERBOARD:  handleLeaderboardInput(); break;
        case GameState::SETTINGS:     handleSettingsInput(); break;
        case GameState::GAME_OVER:
            if (IsKeyPressed(KEY_R)) {
                reset();
                changeState(GameState::PLAYING);
            }
            if (IsKeyPressed(KEY_Q) || IsKeyPressed(KEY_ESCAPE)) changeState(GameState::MENU);
            if (IsKeyPressed(KEY_L)) changeState(GameState::LEADERBOARD);
            break;
        case GameState::PAUSED:
            if (IsKeyPressed(KEY_P) || IsKeyPressed(KEY_SPACE)) changeState(GameState::PLAYING);
            if (IsKeyPressed(KEY_Q) || IsKeyPressed(KEY_ESCAPE)) changeState(GameState::MENU);
            break;
    }
}

// Handles input in the main menu
void Game::handleMenuInput() {
    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
        selectedMenuItem = (selectedMenuItem + 1) % 4;
    }
    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
        selectedMenuItem = (selectedMenuItem - 1 + 4) % 4;
    }
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
        switch (selectedMenuItem) {
            case 0: reset(); changeState(GameState::PLAYING); break;
            case 1: changeState(GameState::LEADERBOARD); break;
            case 2: changeState(GameState::SETTINGS); break;
            case 3: gameRunning = false; break;
        }
    }
}

// Handles input during gameplay
void Game::handleGameInput() {
    if (IsKeyPressed(KEY_P) || IsKeyPressed(KEY_SPACE)) {
        changeState(GameState::PAUSED);
        return;
    }
    if (IsKeyPressed(KEY_Q) || IsKeyPressed(KEY_ESCAPE)) {
        changeState(GameState::MENU);
        return;
    }
    
    if ((IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP)) && currentDirection != Direction::DOWN) nextDirection = Direction::UP;
    if ((IsKeyPressed(KEY_S) || IsKeyPressed(KEY_DOWN)) && currentDirection != Direction::UP) nextDirection = Direction::DOWN;
    if ((IsKeyPressed(KEY_A) || IsKeyPressed(KEY_LEFT)) && currentDirection != Direction::RIGHT) nextDirection = Direction::LEFT;
    if ((IsKeyPressed(KEY_D) || IsKeyPressed(KEY_RIGHT)) && currentDirection != Direction::LEFT) nextDirection = Direction::RIGHT;
}

// Handles input in the leaderboard screen
void Game::handleLeaderboardInput() {
    if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_Q) || IsKeyPressed(KEY_ENTER)) {
        changeState(GameState::MENU);
    }
}

// Handles input in the settings screen
void Game::handleSettingsInput() {
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
        showGrid = !showGrid;
    }
    
    if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_Q)) {
        saveSettings();
        changeState(GameState::MENU);
    }
}

// Updates game logic (snake movement, food, collision)
void Game::update(float deltaTime) {
    moveTimer += deltaTime;
    float currentSpeed = calculateSpeed();
    
    if (moveTimer >= currentSpeed) {
        moveTimer = 0.0f;
        currentDirection = nextDirection;
        moveSnake();
        
        if (checkCollision()) {
            saveToLeaderboard();
            savePersonalBest();
            changeState(GameState::GAME_OVER);
            return;
        }
        
        if (snake.front() == food) {
            score += 10;
            generateFood();
        } else {
            snake.pop_back();
        }
    }
}

// Draws the current frame based on game state
void Game::draw() {
    BeginDrawing();
    ClearBackground(colors.background);
    
    switch (currentState) {
        case GameState::MENU:         drawMenu(); break;
        case GameState::PLAYING:      drawGameField(); drawUI(); break;
        case GameState::PAUSED:       drawGameField(); drawUI(); drawPauseOverlay(); break;
        case GameState::GAME_OVER:    drawGameField(); drawUI(); drawGameOver(); break;
        case GameState::LEADERBOARD:  drawLeaderboard(); break;
        case GameState::SETTINGS:     drawSettings(); break;
    }
    
    EndDrawing();
}

// Draws the game field, snake, food, and obstacles
void Game::drawGameField() {
    if (showGrid) drawGrid();
    
    for (const auto& obs : obstacles) {
        DrawRectangle(obs.x * CELL_SIZE, obs.y * CELL_SIZE, CELL_SIZE - 2, CELL_SIZE - 2, colors.obstacle);
        drawGlowEffect(obs, colors.obstacle, 0.3f);
    }
    
    for (size_t i = 0; i < snake.size(); ++i) {
        Color segmentColor = (i == 0) ? colors.snakeHead : colors.snakeBody;
        if (i > 0) {
            float fade = 1.0f - (static_cast<float>(i) / snake.size());
            segmentColor.a = static_cast<unsigned char>(255 * fade * 0.8f + 51);
        }
        DrawRectangle(snake[i].x * CELL_SIZE, snake[i].y * CELL_SIZE, CELL_SIZE - 2, CELL_SIZE - 2, segmentColor);
        if (i == 0) drawGlowEffect(snake[i], colors.snakeHead, 0.5f);
    }
    
    float pulse = sin(animationTimer * 8.0f) * 0.3f + 0.7f;
    Color pulsedFood = colors.food;
    pulsedFood.a = static_cast<unsigned char>(255 * pulse);
    
    DrawCircle(food.x * CELL_SIZE + CELL_SIZE/2, food.y * CELL_SIZE + CELL_SIZE/2, (CELL_SIZE/2 - 2) * pulse, pulsedFood);
    drawGlowEffect(food, colors.food, pulse * 0.6f);
}

// Draws the grid overlay
void Game::drawGrid() {
    for (int x = 0; x <= GRID_WIDTH; ++x) {
        DrawLine(x * CELL_SIZE, 0, x * CELL_SIZE, GRID_HEIGHT * CELL_SIZE, colors.grid);
    }
    for (int y = 0; y <= GRID_HEIGHT; ++y) {
        DrawLine(0, y * CELL_SIZE, GRID_WIDTH * CELL_SIZE, y * CELL_SIZE, colors.grid);
    }
}

// Draws the UI panel with score, player info, and controls
void Game::drawUI() {
    int uiX = GRID_WIDTH * CELL_SIZE + 20;
    int currentY = 20;

    DrawTextEx(font, "SNAKE GAME", { (float)uiX, (float)currentY }, 22, 1, colors.accent);
    currentY += 25;
    DrawTextEx(font, "Modern Edition", { (float)uiX, (float)currentY }, 16, 1, colors.ui);
    currentY += 35;

    DrawTextEx(font, ("Player: " + playerName).c_str(), { (float)uiX, (float)currentY }, 18, 1, colors.success);
    currentY += 25;

    DrawTextEx(font, ("Score: " + std::to_string(score)).c_str(), { (float)uiX, (float)currentY }, 20, 1, WHITE);
    currentY += 25;
    
    DrawTextEx(font, ("Your Best: " + std::to_string(personalBestScore)).c_str(), { (float)uiX, (float)currentY }, 16, 1, colors.warning);
    currentY += 25;
    
    DrawTextEx(font, ("Best Overall: " + std::to_string(overallHighestScore)).c_str(), { (float)uiX, (float)currentY }, 16, 1, colors.accent);
    currentY += 35;

    int speedLevel = getDifficultyLevel();
    DrawTextEx(font, ("Speed Level: " + std::to_string(speedLevel)).c_str(), { (float)uiX, (float)currentY }, 16, 1, colors.warning);
    currentY += 35;

    DrawTextEx(font, "Controls:", { (float)uiX, (float)currentY }, 18, 1, colors.accent);
    currentY += 25;
    
    const char* controls[] = {"WASD/Arrows - Move", "P/Space - Pause", "Q/Esc - Menu", "L - Leaderboard"};
    for (int i = 0; i < 4; ++i) {
        DrawTextEx(font, controls[i], { (float)uiX, (float)currentY }, 14, 1, colors.ui);
        currentY += 20;
    }
}

// Draws the main menu
void Game::drawMenu() {
    int centerX = SCREEN_WIDTH / 2;
    int centerY = SCREEN_HEIGHT / 2;
    
    DrawTextEx(font, "SNAKE GAME", { (float)centerX - 120, (float)centerY - 150 }, 40, 2, colors.accent);
    DrawTextEx(font, "Modern Edition", { (float)centerX - 75, (float)centerY - 100 }, 20, 1, colors.ui);
    
    std::string welcomeText = "Welcome, " + playerName + "!";
    Vector2 textSize = MeasureTextEx(font, welcomeText.c_str(), 18, 1);
    DrawTextEx(font, welcomeText.c_str(), { (float)centerX - textSize.x/2, (float)centerY - 60 }, 18, 1, colors.success);
    
    std::string highestText = "Highest Score: " + std::to_string(overallHighestScore);
    Vector2 highestSize = MeasureTextEx(font, highestText.c_str(), 14, 1);
    DrawTextEx(font, highestText.c_str(), { (float)centerX - highestSize.x/2, (float)centerY - 35 }, 14, 1, colors.warning);
    
    std::string personalText = "Your Best: " + std::to_string(personalBestScore);
    Vector2 personalSize = MeasureTextEx(font, personalText.c_str(), 14, 1);
    DrawTextEx(font, personalText.c_str(), { (float)centerX - personalSize.x/2, (float)centerY - 15 }, 14, 1, colors.ui);
    
    const char* menuItems[] = {"Start Game", "Leaderboard", "Settings", "Exit"};
    for (int i = 0; i < 4; ++i) {
        bool selected = (i == selectedMenuItem);
        Color textColor = selected ? colors.accent : colors.ui;
        if (selected) DrawRectangle(centerX - 100, centerY + 20 + i * 40 - 5, 200, 30, Fade(colors.accent, 0.2f));
        DrawTextEx(font, menuItems[i], { (float)centerX - 60, (float)centerY + 20 + i * 40 }, 18, 1, textColor);
        if (selected) DrawTextEx(font, ">", { (float)centerX - 80, (float)centerY + 20 + i * 40 }, 18, 1, colors.accent);
    }
    
    DrawTextEx(font, "Use W/S or Arrows to navigate, Enter to select", { (float)centerX - 200, (float)SCREEN_HEIGHT - 60 }, 14, 1, colors.ui);
    DrawTextEx(font, "Speed increases automatically as you eat!", { (float)centerX - 150, (float)SCREEN_HEIGHT - 40 }, 12, 1, colors.warning);
}

// Draws the game over screen
void Game::drawGameOver() {
    int centerX = SCREEN_WIDTH / 2;
    int centerY = SCREEN_HEIGHT / 2;

    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(BLACK, 0.7f));
    DrawRectangle(centerX - 200, centerY - 150, 400, 300, Fade(colors.background, 0.95f));
    DrawRectangleLines(centerX - 200, centerY - 150, 400, 300, colors.accent);

    DrawTextEx(font, "GAME OVER", { (float)centerX - 80, (float)centerY - 120 }, 28, 2, colors.warning);
    DrawTextEx(font, ("Final Score: " + std::to_string(score)).c_str(), { (float)centerX - 70, (float)centerY - 80 }, 20, 1, WHITE);

    if (score > personalBestScore && personalBestScore > 0) {
        DrawTextEx(font, "NEW PERSONAL BEST!", { (float)centerX - 85, (float)centerY - 40 }, 18, 1, colors.success);
    } else if (score > overallHighestScore && overallHighestScore > 0) {
        DrawTextEx(font, "NEW HIGHEST SCORE!", { (float)centerX - 85, (float)centerY - 20 }, 18, 1, colors.warning);
    }

    DrawTextEx(font, "R - Restart Game", { (float)centerX - 65, (float)centerY + 75 }, 16, 1, colors.accent);
    DrawTextEx(font, "L - View Leaderboard", { (float)centerX - 80, (float)centerY + 95 }, 16, 1, colors.accent);
    DrawTextEx(font, "Q - Return to Menu", { (float)centerX - 75, (float)centerY + 115 }, 16, 1, colors.accent);
}

// Draws the leaderboard screen
void Game::drawLeaderboard() {
    int centerX = SCREEN_WIDTH / 2;
    int startY = 100;

    DrawTextEx(font, "LEADERBOARD", { (float)centerX - 90, 50 }, 32, 2, colors.accent);

    if (leaderboard.empty()) {
        DrawTextEx(font, "No scores yet. Be the first!", { (float)centerX - 100, (float)startY + 50 }, 20, 1, colors.ui);
    } else {
        DrawTextEx(font, "Rank", { 50, (float)startY }, 16, 1, colors.accent);
        DrawTextEx(font, "Player", { 150, (float)startY }, 16, 1, colors.accent);
        DrawTextEx(font, "Score", { 350, (float)startY }, 16, 1, colors.accent);
        DrawLine(50, startY + 25, SCREEN_WIDTH - 50, startY + 25, colors.grid);

        for (size_t i = 0; i < std::min(leaderboard.size(), (size_t)MAX_LEADERBOARD_ENTRIES); ++i) {
            int y = startY + 40 + i * 30;
            bool isCurrentPlayer = (leaderboard[i].name == playerName);
            if (isCurrentPlayer) DrawRectangle(40, y - 5, SCREEN_WIDTH - 80, 25, Fade(colors.accent, 0.2f));

            DrawTextEx(font, std::to_string(i + 1).c_str(), { 50, (float)y }, 16, 1, colors.ui);
            DrawTextEx(font, leaderboard[i].name.c_str(), { 150, (float)y }, 16, 1, isCurrentPlayer ? colors.success : colors.ui);
            DrawTextEx(font, std::to_string(leaderboard[i].score).c_str(), { 350, (float)y }, 16, 1, isCurrentPlayer ? colors.success : colors.ui);
        }
    }
    DrawTextEx(font, "Press Q or Escape to return to menu", { (float)centerX - 140, (float)SCREEN_HEIGHT - 50 }, 14, 1, colors.ui);
}

// Draws the settings screen
void Game::drawSettings() {
    int centerX = SCREEN_WIDTH / 2;
    int startY = 200;

    DrawTextEx(font, "SETTINGS", { (float)centerX - 70, 50 }, 32, 2, colors.accent);
    
    DrawTextEx(font, "Speed increases automatically with score!", { (float)centerX - 140, 100 }, 14, 1, colors.warning);
    
    DrawTextEx(font, "Show Grid", { (float)centerX - 100, (float)startY }, 18, 1, colors.accent);
    DrawTextEx(font, showGrid ? "ON" : "OFF", { (float)centerX + 50, (float)startY }, 18, 1, showGrid ? colors.success : colors.warning);
    DrawTextEx(font, "Press Enter to toggle", { (float)centerX - 80, (float)startY + 25 }, 12, 1, colors.ui);
    
    DrawTextEx(font, "Q/Escape - Back to Menu", { (float)centerX - 90, (float)SCREEN_HEIGHT - 50 }, 14, 1, colors.ui);
}

// Draws the pause overlay
void Game::drawPauseOverlay() {
    int centerX = SCREEN_WIDTH / 2;
    int centerY = SCREEN_HEIGHT / 2;
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(BLACK, 0.5f));
    DrawRectangle(centerX - 100, centerY - 60, 200, 120, Fade(colors.background, 0.95f));
    DrawRectangleLines(centerX - 100, centerY - 60, 200, 120, colors.accent);
    DrawTextEx(font, "PAUSED", { (float)centerX - 35, (float)centerY - 40 }, 20, 1, colors.accent);
    DrawTextEx(font, "Press P to continue", { (float)centerX - 75, (float)centerY - 10 }, 14, 1, colors.ui);
    DrawTextEx(font, "Press Q for menu", { (float)centerX - 70, (float)centerY + 15 }, 14, 1, colors.ui);
}

// Draws a progress bar (not used in main UI)
void Game::drawProgressBar(int x, int y, int width, int height, float progress, Color color) {
    progress = std::max(0.0f, std::min(1.0f, progress));
    DrawRectangle(x, y, width, height, Fade(colors.ui, 0.3f));
    DrawRectangle(x, y, static_cast<int>(width * progress), height, color);
    DrawRectangleLines(x, y, width, height, colors.ui);
}

// Draws a glow effect around a position
void Game::drawGlowEffect(Position pos, Color color, float intensity) {
    int centerX = pos.x * CELL_SIZE + CELL_SIZE / 2;
    int centerY = pos.y * CELL_SIZE + CELL_SIZE / 2;
    Color glowColor = color;
    glowColor.a = static_cast<unsigned char>(100 * intensity);
    DrawCircle(centerX, centerY, CELL_SIZE * 0.8f * intensity, glowColor);
}

// Updates animation timers
void Game::updateAnimations(float deltaTime) {
    animationTimer += deltaTime;
}

// Initializes the snake to starting position and direction
void Game::initializeSnake() {
    snake.clear();
    snake.push_back({GRID_WIDTH / 2, GRID_HEIGHT / 2});
    snake.push_back({GRID_WIDTH / 2, GRID_HEIGHT / 2 + 1});
    snake.push_back({GRID_WIDTH / 2, GRID_HEIGHT / 2 + 2});
    currentDirection = Direction::UP;
    nextDirection = Direction::UP;
}

// Generates food at a random empty position
void Game::generateFood() {
    std::vector<Position> emptyPositions;
    for (int x = 0; x < GRID_WIDTH; ++x) {
        for (int y = 0; y < GRID_HEIGHT; ++y) {
            Position pos = {x, y};
            bool isEmpty = true;
            for (const auto& segment : snake) if (segment == pos) { isEmpty = false; break; }
            if (isEmpty) {
                for (const auto& obs : obstacles) if (obs == pos) { isEmpty = false; break; }
            }
            if (isEmpty) emptyPositions.push_back(pos);
        }
    }
    if (!emptyPositions.empty()) {
        food = emptyPositions[rand() % emptyPositions.size()];
    }
}

// Moves the snake in the current direction
void Game::moveSnake() {
    Position newHead = snake.front();
    switch (currentDirection) {
        case Direction::UP:    newHead.y--; break;
        case Direction::DOWN:  newHead.y++; break;
        case Direction::LEFT:  newHead.x--; break;
        case Direction::RIGHT: newHead.x++; break;
    }
    snake.push_front(newHead);
}

// Checks for collisions with walls, self, or obstacles
bool Game::checkCollision() {
    Position head = snake.front();
    if (head.x < 0 || head.x >= GRID_WIDTH || head.y < 0 || head.y >= GRID_HEIGHT) return true;
    for (size_t i = 1; i < snake.size(); ++i) if (snake[i] == head) return true;
    for (const auto& obs : obstacles) if (obs == head) return true;
    return false;
}

// Loads obstacles from a file
void Game::loadObstacles(const std::string& filename) {
    obstacles.clear();
    std::ifstream file(filename);
    if (file.is_open()) {
        int x, y;
        while (file >> x >> y && obstacles.size() < MAX_OBSTACLES) {
            if (x >= 0 && x < GRID_WIDTH && y >= 0 && y < GRID_HEIGHT) {
                obstacles.push_back({x, y});
            }
        }
    }
}

// Loads the highest scores from file
void Game::loadHighestScores() {
    overallHighestScore = 0;
    personalBestScore = 0;
    std::ifstream file("user_scores.txt");
    if (file.is_open()) {
        std::string name;
        int score;
        while (file >> name >> score) {
            if (score > overallHighestScore) {
                overallHighestScore = score;
            }
            if (name == playerName && score > personalBestScore) {
                personalBestScore = score;
            }
        }
    }
}

// Saves the player's personal best score
void Game::savePersonalBest() {
    if (score <= personalBestScore) return;

    std::map<std::string, int> userScores;
    std::ifstream inFile("user_scores.txt");
    if (inFile.is_open()) {
        std::string name;
        int existingScore;
        while (inFile >> name >> existingScore) {
            userScores[name] = existingScore;
        }
    }
    
    userScores[playerName] = score;
    
    std::ofstream outFile("user_scores.txt");
    if (outFile.is_open()) {
        for (const auto& pair : userScores) {
            outFile << pair.first << " " << pair.second << "\n";
        }
    }
    
    personalBestScore = score;
    if (score > overallHighestScore) {
        overallHighestScore = score;
    }
}

// Saves the best score for the session
void Game::saveSessionBestScore() {
    std::map<std::string, int> userScores;
    std::ifstream inFile("user_scores.txt");
    if (inFile.is_open()) {
        std::string name;
        int existingScore;
        while (inFile >> name >> existingScore) {
            if (userScores.find(name) == userScores.end() || existingScore > userScores[name]) {
                userScores[name] = existingScore;
            }
        }
    }
    
    std::ofstream outFile("user_scores.txt");
    if (outFile.is_open()) {
        for (const auto& pair : userScores) {
            outFile << pair.first << " " << pair.second << "\n";
        }
    }
}

// Loads the leaderboard from file
void Game::loadLeaderboard() {
    leaderboard.clear();
    std::map<std::string, int> bestScores;
    
    std::ifstream file("scores.txt");
    if (file.is_open()) {
        std::string name;
        int scoreVal;
        while (file >> name >> scoreVal) {
            if (bestScores.find(name) == bestScores.end() || scoreVal > bestScores[name]) {
                bestScores[name] = scoreVal;
            }
        }
        
        for (const auto& pair : bestScores) {
            leaderboard.push_back({pair.first, pair.second});
        }
        
        std::sort(leaderboard.begin(), leaderboard.end());
    }
}

// Saves the current score to the leaderboard
void Game::saveToLeaderboard() {
    if (score > 0) {
        bool playerExists = false;
        for (auto& entry : leaderboard) {
            if (entry.name == playerName) {
                playerExists = true;
                if (score > entry.score) {
                    entry.score = score;
                }
                break;
            }
        }
        
        if (!playerExists) {
            leaderboard.push_back({playerName, score});
        }
        
        std::sort(leaderboard.begin(), leaderboard.end());
        if (leaderboard.size() > MAX_LEADERBOARD_ENTRIES) {
            leaderboard.resize(MAX_LEADERBOARD_ENTRIES);
        }
        
        std::ofstream file("scores.txt");
        if (file.is_open()) {
            for (const auto& entry : leaderboard) {
                file << entry.name << " " << entry.score << "\n";
            }
        }
    }
}

// Loads settings from file
void Game::loadSettings() {
    std::ifstream file("settings.txt");
    if (file.is_open()) {
        file >> showGrid;
    }
}

// Saves settings to file
void Game::saveSettings() {
    std::ofstream file("settings.txt");
    if (file.is_open()) {
        file << showGrid;
    }
}

// Changes the current game state
void Game::changeState(GameState newState) {
    currentState = newState;
    selectedMenuItem = 0;
    if (newState == GameState::LEADERBOARD) {
        loadLeaderboard();
    }
}

// Returns the current difficulty level based on score
int Game::getDifficultyLevel() const {
    return (score / 50) + 1;
}

// Calculates the snake's movement speed based on score
float Game::calculateSpeed() const {
    float baseSpeed = 0.15f;
    float speedIncrease = (score / 30) * 0.02f;
    float minSpeed = 0.05f;
    return std::max(minSpeed, baseSpeed - speedIncrease);
}

// Resets the game to initial state
void Game::reset() {
    initializeSnake();
    generateFood();
    score = 0;
    moveTimer = 0.0f;
    animationTimer = 0.0f;
}