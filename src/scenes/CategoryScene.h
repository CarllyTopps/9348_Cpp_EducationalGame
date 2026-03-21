#pragma once
#include "Scene.h"
#include "../Game.h"
#include "UIStyle.h"
#include "raylib.h"
#include <cmath>
#include <vector>
#include <string>

class CategoryScene : public Scene {
public:
    CategoryScene(GameState& gs)
        : state(gs), sceneNext(SCENE_STAY), pulse(0.0f) {
        buildButtons();
    }

    void update() override {
        pulse += GetFrameTime() * 1.6f;
        Vector2 mouse = GetMousePosition();
        for (int i = 0; i < (int)buttons.size(); i++)
            hovered[i] = CheckCollisionPointRec(mouse, buttons[i]);

        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
            for (int i = 0; i < (int)buttons.size(); i++) {
                if (hovered[i]) {
                    state.selectedCategory = state.availableCategories[i];
                    state.quizManager.loadFromJSON("assets/questions.json",
                                                   state.availableCategories);
                    state.quizManager.filterByCategory(state.selectedCategory);
                    state.players[0].reset();
                    state.players[1].reset();
                    sceneNext = SCENE_QUESTION_COUNT;
                    break;
                }
            }
        }
    }

    void draw() override {
        UIStyle::drawBackground(1280, 720);
        UIStyle::drawStars(1280, 720, pulse);

        // ── Planet ───────────────────────────────────────────
        UIStyle::drawPlanet(640, 100, 50, pulse,
            {70, 30, 180, 255}, {150, 70, 255, 255});

        // ── Header ────────────────────────────────────────────
        UIStyle::drawTextC("Select Category", 640, 160, 46, UIStyle::TEXT_PRIMARY);

        // Mode badge (top left)
        const char* modeTxt = (state.playerCount == 2) ? "PvP Mode" : "Solo";
        Color modeAcc = (state.playerCount == 2) ? UIStyle::ACCENT_PINK
                                                  : UIStyle::ACCENT_CYAN;
        int mw = UIStyle::textW(modeTxt, 18) + 24;
        DrawRectangleRounded({12, 12, (float)mw, 32}, 0.35f, 4,
                              Fade(modeAcc, 0.15f));
        DrawRectangleRoundedLines({12, 12, (float)mw, 32}, 0.35f, 4,
                                   Fade(modeAcc, 0.6f));
        UIStyle::drawText(modeTxt, 12 + (mw - UIStyle::textW(modeTxt,18))/2,
                          18, 18, modeAcc);

        // Divider
        DrawRectangle(340, 220, 600, 1, Fade(WHITE, 0.10f));
        UIStyle::drawTextC("Choose your topic", 640, 234, 20, UIStyle::TEXT_DIM);

        // ── Category buttons ──────────────────────────────────
        for (int i = 0; i < (int)buttons.size(); i++)
            drawCategoryBtn(buttons[i],
                            state.availableCategories[i].c_str(),
                            hovered[i], i);

        // ── Hint ─────────────────────────────────────────────
        UIStyle::drawBlink("Click a category to begin", 640, 670,
                            18, UIStyle::TEXT_DIM, pulse, 0.8f);
    }

    int nextScene() override { return sceneNext; }

private:
    GameState& state;
    int   sceneNext;
    float pulse;
    std::vector<Rectangle> buttons;
    std::vector<bool>      hovered;

    Color accentFor(int idx) {
        Color p[] = {
            {160, 80,  255, 255},   // All      – purple
            UIStyle::ACCENT_CYAN,   // Computing
            {255, 140,  60, 255},   // Geography – orange
            UIStyle::ACCENT_GREEN,  // History
            UIStyle::ACCENT_PINK,   // Math
            UIStyle::ACCENT_GOLD,   // Science
        };
        return p[idx % 6];
    }
    Color fillFor(int idx) {
        Color p[] = {
            {38, 12, 80,  220},
            {10, 42, 90,  220},
            {80, 38,  8,  220},
            {10, 62, 34,  220},
            {80,  8, 60,  220},
            {70, 58,  6,  220},
        };
        return p[idx % 6];
    }

    void buildButtons() {
        buttons.clear(); hovered.clear();
        int count = (int)state.availableCategories.size();
        float btnW = 280.0f, btnH = 90.0f;
        float gapX = 30.0f,  gapY = 24.0f;
        float startY = 280.0f;
        int cols = (count <= 3) ? count : 3;
        float totalW = cols * btnW + (cols-1) * gapX;
        float startX = 640.0f - totalW / 2.0f;
        for (int i = 0; i < count; i++) {
            int row = i/cols, col = i%cols;
            buttons.push_back({startX + col*(btnW+gapX),
                                startY + row*(btnH+gapY), btnW, btnH});
            hovered.push_back(false);
        }
    }

    // ── Pixel art icons (same as before) ─────────────────────
    void iconAll(int cx, int cy, Color c) {
        int s = 12;
        DrawTriangle({(float)cx,(float)(cy-s)},{(float)(cx-s),(float)(cy+s/2)},
                     {(float)(cx+s),(float)(cy+s/2)}, Fade(c,0.9f));
        DrawTriangle({(float)cx,(float)(cy+s)},{(float)(cx+s),(float)(cy-s/2)},
                     {(float)(cx-s),(float)(cy-s/2)}, Fade(c,0.5f));
    }
    void iconComputing(int cx, int cy, Color c) {
        DrawRectangle(cx-13,cy-10,26,19, Fade(c,0.22f));
        DrawRectangleLinesEx({(float)(cx-13),(float)(cy-10),26,19}, 2, c);
        DrawRectangle(cx-4, cy-5, 9, 7,  Fade(c,0.7f));
        DrawRectangle(cx-1, cy+9, 3, 4,  Fade(c,0.8f));
        DrawRectangle(cx-7, cy+12,15, 3, c);
    }
    void iconGeography(int cx, int cy, Color c) {
        DrawCircleLines(cx, cy, 13, c);
        DrawLine(cx-13,cy, cx+13,cy, Fade(c,0.65f));
        DrawLine(cx,cy-13, cx,cy+13, Fade(c,0.65f));
        DrawLine(cx-10,cy-6, cx+10,cy-6, Fade(c,0.38f));
        DrawLine(cx-10,cy+6, cx+10,cy+6, Fade(c,0.38f));
        DrawCircle(cx,cy,2,c);
    }
    void iconHistory(int cx, int cy, Color c) {
        DrawRectangle(cx-11,cy-8,22,17, Fade(c,0.20f));
        DrawRectangleLinesEx({(float)(cx-11),(float)(cy-8),22,17}, 2, c);
        DrawLine(cx-6,cy-3, cx+6,cy-3, Fade(c,0.9f));
        DrawLine(cx-6,cy+1, cx+6,cy+1, Fade(c,0.9f));
        DrawLine(cx-6,cy+5, cx+4,cy+5, Fade(c,0.9f));
        DrawRectangle(cx-14,cy-8,4,17, Fade(c,0.45f));
        DrawRectangle(cx+10,cy-8,4,17, Fade(c,0.45f));
    }
    void iconMath(int cx, int cy, Color c) {
        DrawRectangle(cx-9,cy-2,18,4,c);
        DrawRectangle(cx-2,cy-9,4,18,c);
        DrawRectangle(cx+5,cy+4, 8,2, Fade(c,0.82f));
        DrawRectangle(cx+5,cy+8, 8,2, Fade(c,0.82f));
    }
    void iconScience(int cx, int cy, Color c) {
        DrawCircle(cx,cy,3,c);
        DrawEllipseLines(cx,cy,12,5,c);
        DrawLine(cx-9,cy-8, cx+9,cy+8, Fade(c,0.65f));
        DrawLine(cx+9,cy-8, cx-9,cy+8, Fade(c,0.65f));
        DrawCircle(cx+12,cy,  2, Fade(c,0.85f));
        DrawCircle(cx-12,cy,  2, Fade(c,0.85f));
        DrawCircle(cx,cy-11,  2, Fade(c,0.85f));
    }

    void drawCategoryBtn(Rectangle r, const char* label, bool hov, int idx) {
        Color accent = accentFor(idx);
        Color fill   = fillFor(idx);

        // Hover glow
        if (hov)
            DrawRectangleRounded({r.x-4,r.y-4,r.width+8,r.height+8},
                                  0.14f, 6, Fade(accent, 0.15f));

        // Card body
        UIStyle::drawCard(r, hov ? Fade(fill, 0.95f) : Fade(fill, 0.70f),
                           hov ? Fade(accent, 0.7f) : Fade(accent, 0.30f), 0.12f);

        // Left accent stripe
        DrawRectangleRounded({r.x, r.y+2, 5, r.height-4}, 0.5f, 4,
                              Fade(accent, 0.85f));

        // Icon zone (50px wide)
        int icx = (int)r.x + 30;
        int icy = (int)(r.y + r.height/2);
        DrawRectangleRounded({r.x+5, r.y, 48, r.height}, 0.12f, 4,
                              Fade(accent, 0.08f));
        DrawLine((int)r.x+53, (int)r.y+6,
                 (int)r.x+53, (int)(r.y+r.height-6),
                 Fade(accent, 0.25f));

        std::string cat(label);
        if      (cat=="Computing")  iconComputing(icx, icy, accent);
        else if (cat=="Geography")  iconGeography(icx, icy, accent);
        else if (cat=="History")    iconHistory(icx, icy, accent);
        else if (cat=="Math")       iconMath(icx, icy, accent);
        else if (cat=="Science")    iconScience(icx, icy, accent);
        else                        iconAll(icx, icy, accent);

        // Label
        float fs = 24.0f;
        int labelAreaW = (int)r.width - 64;
        while (UIStyle::textW(label, fs) > labelAreaW && fs > 16) fs -= 1;
        int lw = UIStyle::textW(label, fs);
        int lx = (int)r.x + 56 + labelAreaW/2 - lw/2;
        int ly = (int)(r.y + r.height/2 - fs/2);

        // Shadow + text
        DrawTextEx(UIStyle::FONT_REG, label, {(float)lx+1,(float)ly+1},
                   fs, 1.5f, Fade(BLACK, 0.7f));
        DrawTextEx(UIStyle::FONT_REG, label, {(float)lx,(float)ly},
                   fs, 1.5f,
                   hov ? UIStyle::TEXT_PRIMARY : Fade(UIStyle::TEXT_PRIMARY, 0.85f));

        // Hover arrow
        if (hov) {
            float ay = r.y + r.height/2;
            DrawTriangle({r.x+r.width+10, ay},
                         {r.x+r.width+3,  ay-5},
                         {r.x+r.width+3,  ay+5}, accent);
        }
    }
};