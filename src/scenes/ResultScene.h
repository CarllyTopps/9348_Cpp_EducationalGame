#pragma once
#include "Scene.h"
#include "../Game.h"
#include "UIStyle.h"
#include "raylib.h"
#include <cmath>
#include <cstdio>
#include <string>

class ResultScene : public Scene {
public:
    ResultScene(GameState& gs)
        : state(gs), pulse(0.0f), sceneNext(SCENE_STAY),
          hPlay(false), hQuit(false) {}

    void update() override {
        pulse += GetFrameTime() * 1.6f;
        Vector2 m = GetMousePosition();
        hPlay = CheckCollisionPointRec(m, btnPlay);
        hQuit = CheckCollisionPointRec(m, btnQuit);
        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
            if (hPlay) sceneNext = SCENE_MENU;
            if (hQuit) sceneNext = SCENE_MENU;
        }
    }

    void draw() override {
        UIStyle::drawBackground(1280, 720);
        UIStyle::drawStars(1280, 720, pulse);

        int p1 = state.players[0].score;
        int p2 = state.players[1].score;
        bool pvp = (state.playerCount == 2);

        // ── Header bar ────────────────────────────────────────
        DrawRectangle(0, 0, 1280, 64, Fade(UIStyle::BG_SPACE, 0.95f));
        DrawRectangle(0, 63, 1280, 1, Fade(UIStyle::ACCENT_GOLD, 0.5f));
        UIStyle::drawTextC("Game Over", 640, 16, 32, UIStyle::ACCENT_GOLD);

        // ── Planet ────────────────────────────────────────────
        UIStyle::drawPlanet(640, 120, 48, pulse,
            {195, 148, 18, 255}, {255, 200, 60, 255});

        // ── Title + divider ───────────────────────────────────
        UIStyle::drawTextC("Results", 640, 180, 40, UIStyle::TEXT_PRIMARY);
        DrawRectangle(240, 230, 800, 1, Fade(WHITE, 0.10f));

        // ── Content ───────────────────────────────────────────
        if (pvp) drawPvP(p1, p2);
        else     drawSolo(p1);

        // ── Buttons ───────────────────────────────────────────
        UIStyle::drawButton(btnPlay, "Play Again", hPlay,
                             {25, 14, 90, 255}, UIStyle::ACCENT_CYAN);
        UIStyle::drawButton(btnQuit, "Main Menu",  hQuit,
                             {80, 12, 12, 255}, UIStyle::ACCENT_RED);

        // ── Blink ─────────────────────────────────────────────
        UIStyle::drawBlink("Insert coin to continue", 640, 694,
                            18, UIStyle::TEXT_DIM, pulse, 0.7f);
    }

    int nextScene() override { return sceneNext; }

private:
    GameState& state;
    float pulse;
    int   sceneNext;
    bool  hPlay, hQuit;

    Rectangle btnPlay = {340, 615, 280, 64};
    Rectangle btnQuit = {660, 615, 280, 64};

    // ── Solo layout (190–448) ─────────────────────────────────
    //   190–374  Score card (184px)
    //   382–420  Grade strip (38px)
    //   428–443  Motivational line
    void drawSolo(int score) {
        const char* grade = gradeLabel(score);
        Color gc          = gradeColor(grade);
        int correct = state.players[0].correct;
        int total   = state.players[0].total;

        // ── Score card ────────────────────────────────────────
        Rectangle card = {240, 230, 800, 250};
        UIStyle::drawCard(card, UIStyle::BG_CARD, Fade(gc, 0.55f), 0.05f);
        DrawRectangleRounded({card.x+2, card.y+2, card.width-4, 5},
                              0.5f, 4, Fade(gc, 0.75f));

        // "Your Score" label
        UIStyle::drawTextC("Your Score",
            (int)(card.x + card.width/2), (int)card.y + 24, 24,
            Fade(gc, 0.85f));

        // Big score number
        char sc[24]; snprintf(sc, sizeof(sc), "%d", score);
        int ssw = UIStyle::textW(sc, 100);
        DrawTextEx(UIStyle::FONT_REG, sc,
                   {card.x + card.width/2 - ssw/2 + 3, card.y + 70},
                   100, 2.0f, Fade(BLACK, 0.7f));
        DrawTextEx(UIStyle::FONT_REG, sc,
                   {card.x + card.width/2 - ssw/2, card.y + 68},
                   100, 2.0f, UIStyle::TEXT_PRIMARY);

        // Correct count
        char acc[48];
        snprintf(acc, sizeof(acc), "Correct:  %d / %d", correct, total);
        UIStyle::drawTextC(acc,
            (int)(card.x + card.width/2), (int)card.y + 180, 22,
            Fade(gc, 0.92f));

        // Accuracy bar
        float frac = (total > 0) ? (float)correct / total : 0.0f;
        UIStyle::drawBar((int)card.x + 60, (int)card.y + 214,
                          (int)card.width - 120, 18,
                          frac, gc, Fade(BLACK, 0.35f));

        // ── Motivational text ─────────────────────────────────
        UIStyle::drawTextC(motivMsg(correct, total),
            640, 505, 20, UIStyle::TEXT_SECONDARY);

        // ── Grade strip ───────────────────────────────────────
        DrawRectangle(0, 530, 1280, 60, Fade(gc, 0.11f));
        DrawRectangle(0, 530, 1280, 2,  Fade(gc, 0.55f));
        DrawRectangle(0, 588, 1280, 2,  Fade(gc, 0.55f));

        float sc2 = 1.0f + 0.04f * sinf(pulse * 2.4f);
        int   gfs = (int)(32 * sc2);
        UIStyle::drawTextC(grade, 640, 560 - gfs/2, gfs, gc);
    }

    // ── PvP layout (190–450) ──────────────────────────────────
    //   190–392  Two player cards (202px)
    //   400–450  Winner strip (50px)
    void drawPvP(int p1, int p2) {
        bool p1w = (p1 > p2), p2w = (p2 > p1);

        const float CARD_Y = 230, CARD_H = 250;

        // Player 1 card
        Color c1a = UIStyle::ACCENT_CYAN;
        Color c1f = p1w ? Fade((Color){18,38,110,255},0.92f)
                        : Fade((Color){10,18,55,255},0.72f);
        Rectangle card1 = {40, CARD_Y, 580, CARD_H};
        UIStyle::drawCard(card1, c1f,
                           p1w ? Fade(c1a,0.70f) : Fade(c1a,0.28f), 0.05f);
        drawPlayerCard(card1, "Player 1", p1,
                       state.players[0].correct, state.players[0].total,
                       c1a, p1w);

        // Player 2 card
        Color c2a = UIStyle::ACCENT_PINK;
        Color c2f = p2w ? Fade((Color){75,10,58,255},0.92f)
                        : Fade((Color){28,5,24,255},0.72f);
        Rectangle card2 = {660, CARD_Y, 580, CARD_H};
        UIStyle::drawCard(card2, c2f,
                           p2w ? Fade(c2a,0.70f) : Fade(c2a,0.28f), 0.05f);
        drawPlayerCard(card2, "Player 2", p2,
                       state.players[1].correct, state.players[1].total,
                       c2a, p2w);

        // Winner strip
        const char* wt;
        Color wc;
        if      (p1w) { wt = "Player 1 Wins!"; wc = UIStyle::ACCENT_CYAN; }
        else if (p2w) { wt = "Player 2 Wins!"; wc = UIStyle::ACCENT_PINK; }
        else          { wt = "It's a Tie!";    wc = UIStyle::ACCENT_GOLD; }

        DrawRectangle(0, 530, 1280, 60, Fade(UIStyle::BG_SPACE, 0.82f));
        DrawRectangle(0, 530, 1280, 1,  Fade(wc, 0.45f));
        DrawRectangle(0, 588, 1280, 1,  Fade(wc, 0.45f));

        float sc3  = 1.0f + 0.04f * sinf(pulse * 3.0f);
        int   fsize = (int)(40 * sc3);
        UIStyle::drawTextC(wt, 640, 560 - fsize/2, fsize, wc);
    }

    // No key labels — just name, score, correct count, bar
    void drawPlayerCard(Rectangle r, const char* name,
                        int score, int correct, int total,
                        Color accent, bool winner) {
        int ix = (int)r.x + 30;
        int iy = (int)r.y + 20;

        // Winner badge
        if (winner) {
            int bw = UIStyle::textW("Winner!", 16) + 24;
            DrawRectangleRounded({(float)ix, (float)iy, (float)bw, 28},
                                  0.4f, 4, Fade(UIStyle::ACCENT_GOLD, 0.22f));
            DrawRectangleRoundedLines({(float)ix,(float)iy,(float)bw,28},
                                       0.4f, 4, Fade(UIStyle::ACCENT_GOLD, 0.6f));
            UIStyle::drawText("Winner!", ix + 12, iy + 6, 16, UIStyle::ACCENT_GOLD);
            iy += 36;
        }

        // Player name (bigger)
        UIStyle::drawText(name, ix, iy, 26, accent);

        // Score number
        char sc[16]; snprintf(sc, sizeof(sc), "%d", score);
        int nw = UIStyle::textW(sc, 84);
        DrawTextEx(UIStyle::FONT_REG, sc,
                   {(float)ix + 2, (float)(iy + 40)}, 84, 2.0f, Fade(BLACK, 0.65f));
        DrawTextEx(UIStyle::FONT_REG, sc,
                   {(float)ix, (float)(iy + 38)}, 84, 2.0f, UIStyle::TEXT_PRIMARY);

        // "pts" label
        UIStyle::drawText("pts", ix + nw + 10, iy + 84, 18, UIStyle::TEXT_DIM);

        // Correct count
        char acc[40];
        snprintf(acc, sizeof(acc), "Correct:  %d / %d", correct, total);
        UIStyle::drawText(acc, ix, (int)r.y + 194, 20, Fade(accent, 0.88f));

        // Accuracy bar
        float frac = (total > 0) ? (float)correct / total : 0.0f;
        UIStyle::drawBar(ix, (int)r.y + 220,
                          (int)r.width - 60, 16,
                          frac, accent, Fade(BLACK, 0.35f));
    }

    const char* gradeLabel(int score) {
        int ms = state.questionsPerGame * 250;
        float p = ms > 0 ? (float)score/ms : 0.0f;
        if (p >= 0.85f) return "S Rank  —  Perfect!";
        if (p >= 0.65f) return "A Rank  —  Great!";
        if (p >= 0.40f) return "B Rank  —  Good";
        return                  "C Rank  —  Try Again";
    }

    Color gradeColor(const char* g) {
        if (g[0]=='S') return UIStyle::ACCENT_GOLD;
        if (g[0]=='A') return UIStyle::ACCENT_GREEN;
        if (g[0]=='B') return (Color){255,140,50,255};
        return UIStyle::ACCENT_RED;
    }

    const char* motivMsg(int correct, int total) {
        if (correct == total)                           return "Flawless victory!";
        if (total > 0 && correct >= (int)(total*0.8f)) return "So close to perfect!";
        if (total > 0 && correct >= total/2)            return "Keep grinding, you got this!";
        return "Study hard and try again!";
    }
};