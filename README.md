# Snake Game - Modern Edition (C++ with Raylib)

This is a personal side project made just for fun. It builds upon a terminal-based Snake game I originally created in C. The original version focused on raw terminal control and basic logic, while this new version aims to modernize the experience with graphics, animations, UI, and additional features.

## Evolution from the C Version

- **From Terminal to Graphics**: The C version used raw input and terminal drawing; this version uses the Raylib graphics library.
- **Enhanced UI**: Added custom fonts, game panels, settings menu, and visual feedback.
- **Leaderboard**: More advanced score tracking with persistent best scores and player identification.
- **Obstacles & Effects**: Retained the obstacle system from the original, but added visuals, glow effects, and animations.
- **Improved Structure**: Moved from procedural design to a structured C++ class-based architecture.
- **Game States**: Introduced menu, pause, game over, settings, and leaderboard states.

## Features

- Smooth animations and responsive controls
- Dynamic difficulty (speed increases as you play)
- Player name input and persistent personal bests
- Grid toggle, obstacles, leaderboard
- Custom font and UI panel with info
- Cleaned-up code with basic error handling

## Requirements

- Linux / WSL (tested on Ubuntu)
- Raylib 4.5.0 (installed automatically)
- g++, make, CMake

To install dependencies:

```bash
chmod +x install_dependencies.sh
./install_dependencies.sh
```

## Build and Run

```bash
chmod +x build.sh
./build.sh        # compiles via Makefile
./snake_game      # or run: make run
```

## Notes

- Highscores are saved locally to `user_scores.txt` and `scores.txt`
- Settings are stored in `settings.txt`
- Obstacles can be modified in `obstacles.txt`
- The code uses Raylib for graphics and input – no additional frameworks