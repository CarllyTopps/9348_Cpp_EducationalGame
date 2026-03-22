#pragma once
#include "../entities/Question.h"
#include "../../lib/json.hpp"
#include <vector>
#include <string>
#include <fstream>
#include <algorithm>
#include <random>
#include <set>

using json = nlohmann::json;

class QuizManager {
public:
    std::vector<Question> questions;
    int currentIndex = 0;

    // Load all questions from JSON and collect available categories
    bool loadFromJSON(const std::string& path, std::vector<std::string>& outCategories) {
        std::ifstream file(path);
        if (!file.is_open()) return false;

        json data;
        try { data = json::parse(file); }
        catch (...) { return false; }

        questions.clear();
        currentIndex = 0;      
        std::set<std::string> cats;

        for (const auto& item : data) {
            Question q;
            q.text         = item["text"].get<std::string>();
            q.correctIndex = item["correct"].get<int>();
            q.category     = item["category"].get<std::string>();
            for (const auto& c : item["choices"])
                q.choices.push_back(c.get<std::string>());
            questions.push_back(q);
            cats.insert(q.category);
        }

        outCategories.clear();
        outCategories.push_back("All");
        for (const auto& c : cats)
            outCategories.push_back(c);

        shuffle();                
        return !questions.empty();
    }


    void filterByCategory(const std::string& category) {
        if (category != "All") {
            std::vector<Question> filtered;
            for (const auto& q : questions)
                if (q.category == category)
                    filtered.push_back(q);
            questions = filtered;
        }
        currentIndex = 0;
        shuffle();
    }

    const Question& current() const {
        return questions[currentIndex];
    }

    void advance() {
        currentIndex++;
    }

    bool hasNext() const {
        return currentIndex < (int)questions.size();
    }

    int totalQuestions() const {
        return (int)questions.size();
    }

    void reset() {
        currentIndex = 0;
        shuffle();               
    }

private:
    void shuffle() {
        std::mt19937 rng(std::random_device{}());
        std::shuffle(questions.begin(), questions.end(), rng);
    }
};