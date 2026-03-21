#pragma once
#include "Scene.h"
#include "../Game.h"
#include "UIStyle.h"
#include <vector>
#include <string>
#include <algorithm>

class QuestionCountScene : public Scene {
public:
    QuestionCountScene(GameState& gs) : state(gs) {
        sceneNext = SCENE_STAY;
        pulse = 0.0f;

        int maxQ = state.quizManager.totalQuestions();
        if (maxQ >= 10) options.push_back(10);
        if (maxQ >= 20) options.push_back(20);
        if (maxQ >= 30) options.push_back(30);
        if (options.empty() || options.back() != maxQ) {
            // Only add maxQ if it's not already in the list
            if (std::find(options.begin(), options.end(), maxQ) == options.end()) {
                options.push_back(maxQ);
            }
        }
        std::sort(options.begin(), options.end());

        // Layout buttons vertically
        float startY = 320.0f;
        float gap = 24.0f;
        float bh = 80.0f;
        float bw = 400.0f;
        float bx = 640.0f - bw / 2.0f;

        for (size_t i = 0; i < options.size(); i++) {
            buttons.push_back({bx, startY + i * (bh + gap), bw, bh});
            hovered.push_back(false);
        }
    }

    void update() override {
        if (sceneNext != SCENE_STAY) return;
        pulse += GetFrameTime() * 1.5f;

        Vector2 m = GetMousePosition();
        for (size_t i = 0; i < buttons.size(); i++) {
            hovered[i] = CheckCollisionPointRec(m, buttons[i]);
            if (hovered[i] && IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
                state.questionsPerGame = options[i];
                sceneNext = SCENE_QUIZ;
                break;
            }
        }
    }

    void draw() override {
        UIStyle::drawBackground(1280, 720);
        UIStyle::drawStars(1280, 720, pulse);

        // Big planet background element
        UIStyle::drawPlanet(640, 720, 400, pulse, 
                            {30, 20, 60, 255}, {100, 80, 200, 50});

        char title[128];
        snprintf(title, sizeof(title), "Category: %s", state.selectedCategory.c_str());
        UIStyle::drawTextC(title, 640, 100, 24, UIStyle::ACCENT_CYAN);
        
        UIStyle::drawTextC("How many questions?", 640, 180, 56, UIStyle::TEXT_PRIMARY);
        UIStyle::drawTextC("Choose the length of your game", 640, 240, 22, UIStyle::TEXT_SECONDARY);

        for (size_t i = 0; i < buttons.size(); i++) {
            char label[64];
            if (options[i] == state.quizManager.totalQuestions()) {
                snprintf(label, sizeof(label), "%d Questions (Max)", options[i]);
            } else {
                snprintf(label, sizeof(label), "%d Questions", options[i]);
            }
            
            Color fill = UIStyle::choiceFill(i);
            Color accent = UIStyle::choiceAccent(i);
            UIStyle::drawButton(buttons[i], label, hovered[i], fill, accent, 32.0f);
        }
    }

    int nextScene() override { return sceneNext; }

private:
    GameState& state;
    int sceneNext;
    float pulse;
    std::vector<int> options;
    std::vector<Rectangle> buttons;
    std::vector<bool> hovered;
};
