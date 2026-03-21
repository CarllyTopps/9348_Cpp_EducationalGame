#pragma once
#include <string>
#include <vector>

struct Question {
    std::string text;
    std::vector<std::string> choices; // 4 choices
    int correctIndex;                 // 0-3
    std::string category;
};
