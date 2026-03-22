#include "Game.h"
#include "scenes/MenuScene.h"
#include "scenes/CategoryScene.h"
#include "scenes/QuestionCountScene.h"
#include "scenes/QuizScene.h"
#include "scenes/ResultScene.h"
#include <stdexcept>

void Game::run() {
    InitWindow(screenW, screenH, "Quiz Planet");
    SetExitKey(KEY_NULL); // Prevent ESC from closing the window immediately
    UIStyle::load();  
    SetTargetFPS(60);

    // Load questions once
    if (!state.loaded) {
        state.quizManager.loadFromJSON("assets/questions.json", state.availableCategories);
        state.players[0].name = "Player 1";
        state.players[1].name = "Player 2";
        state.loaded = true;
    }

    // Start at the menu
    switchScene(SCENE_MENU);

    while (!WindowShouldClose()) {
        currentScene->update();

        BeginDrawing();
        ClearBackground(BLACK);
        currentScene->draw();
        EndDrawing();

        int next = currentScene->nextScene();
        if (next == SCENE_EXIT) break;
        if (next != SCENE_STAY) switchScene(next);
    }

    UIStyle::unload();
    CloseWindow();
}

void Game::switchScene(int id) {
    switch (id) {
        case SCENE_MENU:
            currentScene = std::make_unique<MenuScene>(state);
            break;
        case SCENE_CATEGORY:
            currentScene = std::make_unique<CategoryScene>(state);
            break;
        case SCENE_QUESTION_COUNT:
            currentScene = std::make_unique<QuestionCountScene>(state);
            break;
        case SCENE_QUIZ:
            currentScene = std::make_unique<QuizScene>(state);
            break;
        case SCENE_RESULT:
            currentScene = std::make_unique<ResultScene>(state);
            break;
        default:
            break;
    }
}
