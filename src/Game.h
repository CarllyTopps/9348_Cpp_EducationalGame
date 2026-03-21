#pragma once
#include "raylib.h"
#include "scenes/Scene.h"
#include "Player.h"
#include "QuizManager.h"
#include <memory>
#include <string>
#include <vector>

// Shared game state accessible by all scenes
struct GameState {
    int  playerCount = 1;       // 1 or 2
    int  questionsPerGame = 10;
    Player players[2];
    QuizManager quizManager;
    bool loaded = false;

    // Category selection
    std::string selectedCategory = "All";
    std::vector<std::string> availableCategories;
};

class Game {
public:
    int screenW = 1280;
    int screenH = 720;

    // Shared state passed to scenes
    GameState state;

    void run();

private:
    std::unique_ptr<Scene> currentScene;
    void switchScene(int id);
};
