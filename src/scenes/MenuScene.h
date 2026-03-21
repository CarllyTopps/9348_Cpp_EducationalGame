#pragma once
#include "Scene.h"
#include "../Game.h"
#include "UIStyle.h"
#include "raylib.h"
#include <cmath>
#include <cstdlib>

class MenuScene : public Scene {
public:
    MenuScene(GameState& gs)
        : state(gs), pulse(0.0f), sceneNext(SCENE_STAY),
          hover1P(false), hover2P(false),
          shootX(-100), shootY(100), shootVX(0), shootVY(0), shootTimer(0) {}

    void update() override {
        pulse      += GetFrameTime() * 1.6f;
        shootTimer += GetFrameTime();

        if (shootTimer > 3.2f) {
            shootTimer = 0;
            shootX  = (float)(rand() % 400 + 50);
            shootY  = (float)(rand() % 100 + 20);
            shootVX = 3.5f + rand() % 4;
            shootVY = 1.2f + rand() % 2;
        }
        shootX += shootVX;
        shootY += shootVY;

        Vector2 mouse = GetMousePosition();
        hover1P = CheckCollisionPointRec(mouse, btn1P);
        hover2P = CheckCollisionPointRec(mouse, btn2P);

        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
            if (hover1P) { state.playerCount = 1; sceneNext = SCENE_CATEGORY; }
            if (hover2P) { state.playerCount = 2; sceneNext = SCENE_CATEGORY; }
        }
    }

    void draw() override {
        UIStyle::drawBackground(1280, 720);
        UIStyle::drawStars(1280, 720, pulse);

        // Shooting star
        if (shootX > 0 && shootX < 1280) {
            for (int t = 0; t < 20; t++) {
                float fx = shootX - shootVX * t * 0.65f;
                float fy = shootY - shootVY * t * 0.65f;
                DrawCircleV({fx, fy}, 2.5f - t*0.10f,
                             Fade(WHITE, (1.0f - t/20.0f) * 0.75f));
            }
        }

        // ── Planet ───────────────────────────────────────────
        UIStyle::drawPlanet(640, 220, 85, pulse,
            {85, 38, 195, 255}, {160, 80, 255, 255});

        // ── Logo: QUIZ (gold) + PLANET (white) ───────────────
        // Uses Raylib default font intentionally for the chunky pixel logo look
        const int LOGO_FS = 96;
        int tw1 = MeasureText("QUIZ",   LOGO_FS);
        int tw2 = MeasureText("PLANET", LOGO_FS);

        // Drop shadows
        DrawText("QUIZ",   640 - tw1/2 + 4, 343, LOGO_FS, Fade(BLACK, 0.85f));
        DrawText("PLANET", 640 - tw2/2 + 4, 433, LOGO_FS, Fade(BLACK, 0.85f));
        // Glow layer
        DrawText("QUIZ",   640 - tw1/2 - 3, 339, LOGO_FS, Fade(UIStyle::ACCENT_GOLD, 0.22f));
        DrawText("QUIZ",   640 - tw1/2 + 3, 345, LOGO_FS, Fade(UIStyle::ACCENT_GOLD, 0.22f));
        // Fill
        DrawText("QUIZ",   640 - tw1/2,     342, LOGO_FS, UIStyle::ACCENT_GOLD);
        DrawText("PLANET", 640 - tw2/2,     432, LOGO_FS, UIStyle::TEXT_PRIMARY);

        // ── Subtitle + divider ────────────────────────────────
        UIStyle::drawTextC("Trivia Challenge", 640, 530, 24, UIStyle::TEXT_SECONDARY);
        DrawRectangle(400, 564, 480, 1, Fade(WHITE, 0.10f));

        // ── Buttons ───────────────────────────────────────────
        drawModeBtn(btn1P, "Single Player", hover1P,
                    {20, 14, 80, 255}, UIStyle::ACCENT_CYAN);
        drawModeBtn(btn2P, "PvP  Duel",     hover2P,
                    {60,  8, 50, 255}, UIStyle::ACCENT_PINK);

        // ── Hint ─────────────────────────────────────────────
        UIStyle::drawBlink("Click to select a mode", 640, 686,
                            18, UIStyle::TEXT_DIM, pulse, 0.85f);
        UIStyle::drawText("v1.1", 16, 692, 16, UIStyle::TEXT_DIM);
    }

    int nextScene() override { return sceneNext; }

private:
    GameState& state;
    float pulse;
    int   sceneNext;
    bool  hover1P, hover2P;
    float shootX, shootY, shootVX, shootVY, shootTimer;

    Rectangle btn1P = {260, 590, 340, 72};
    Rectangle btn2P = {680, 590, 340, 72};

    void drawModeBtn(Rectangle r, const char* label, bool hov,
                     Color fill, Color accent) {
        if (hov)
            DrawRectangleRounded({r.x-4, r.y-4, r.width+8, r.height+8},
                                  0.18f, 6, Fade(accent, 0.14f));
        DrawRectangleRounded(r, 0.18f, 6,
                              hov ? Fade(fill, 0.95f) : Fade(fill, 0.60f));
        DrawRectangleRoundedLines(r, 0.18f, 6,
                                   hov ? accent : Fade(accent, 0.45f));
        DrawRectangleRounded({r.x+2, r.y+2, r.width-4, 3}, 0.5f, 4,
                              Fade(WHITE, 0.09f));

        float fs2 = 26.0f;
        while (UIStyle::textW(label, fs2) > (int)r.width-28 && fs2 > 16) fs2 -= 1;
        int tw = UIStyle::textW(label, fs2);
        DrawTextEx(UIStyle::FONT_REG, label,
                   {r.x+r.width/2-tw/2+1, r.y+r.height/2-fs2/2+1},
                   fs2, 1.5f, Fade(BLACK, 0.55f));
        DrawTextEx(UIStyle::FONT_REG, label,
                   {r.x+r.width/2-tw/2, r.y+r.height/2-fs2/2},
                   fs2, 1.5f,
                   hov ? UIStyle::TEXT_PRIMARY : Fade(UIStyle::TEXT_PRIMARY, 0.82f));

        if (hov) {
            float ax = r.x + r.width/2;
            float ay = r.y + r.height - 7;
            DrawTriangle({ax-5,ay-4},{ax+5,ay-4},{ax,ay}, Fade(accent,0.75f));
        }
    }
};