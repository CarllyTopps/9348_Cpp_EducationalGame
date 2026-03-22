#pragma once
#include <string>

class Player {
public:
    std::string name;
    int score   = 0;
    int streak  = 0;   // consecutive correct answers
    int correct = 0;   // total correct this game
    int total   = 0;   // total questions answered
    int sabotageCharges = 0; // charges based on streak

    void addScore(int points) {
        score += points;
        streak++;
        if (streak > 0 && streak % 3 == 0) {
            sabotageCharges++;
        }
    }

    void resetStreak() {
        streak = 0;
    }

    void reset() {
        score  = 0;
        streak = 0;
        correct = 0;
        total   = 0;
        sabotageCharges = 0;
    }
};
