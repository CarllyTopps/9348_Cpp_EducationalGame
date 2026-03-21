#pragma once
#include <string>

class Player {
public:
    std::string name;
    int score   = 0;
    int streak  = 0;   // consecutive correct answers
    int correct = 0;   // total correct this game
    int total   = 0;   // total questions answered

    void addScore(int points) {
        score += points;
        streak++;
    }

    void resetStreak() {
        streak = 0;
    }

    void reset() {
        score  = 0;
        streak = 0;
        correct = 0;
        total   = 0;
    }
};
