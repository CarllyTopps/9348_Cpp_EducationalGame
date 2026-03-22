#pragma once
#include "raylib.h"

class Timer {
public:
    float duration;
    float elapsed = 0.0f;

    Timer() : duration(15.0f) {}
    Timer(float seconds) : duration(seconds) {}

    void update()  { elapsed += GetFrameTime(); }
    void reset()   { elapsed = 0.0f; }
    bool isDone()  const { return elapsed >= duration; }
    float remaining() const { return duration - elapsed; }
    float progress()  const { return elapsed / duration; }  // 0.0 -> 1.0
};
