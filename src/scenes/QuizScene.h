#pragma once
#include "Scene.h"
#include "../Game.h"
#include "../Timer.h"
#include "UIStyle.h"
#include "raylib.h"
#include <cmath>
#include <string>
#include <vector>

enum class FeedbackState { NONE, CORRECT, WRONG, TIMEOUT };

class QuizScene : public Scene {
public:
    QuizScene(GameState& gs) : state(gs), timer(15.0f) {
        questionsAnswered = 0;
        activeTurn        = 0;
        feedbackState     = FeedbackState::NONE;
        feedbackTimer     = 0.0f;
        selectedChoice    = -1;
        correctChoice     = -1;
        sceneNext         = SCENE_STAY;
        pulse             = 0.0f;
        buildChoiceRects();
    }

    void update() override {
        if (sceneNext != SCENE_STAY) return;
        pulse += GetFrameTime() * 1.6f;

        if (feedbackState != FeedbackState::NONE) {
            feedbackTimer += GetFrameTime();
            if (feedbackTimer >= FEEDBACK_DURATION) advanceQuestion();
            return;
        }

        timer.update();

        int pressed = -1;
        if (state.playerCount == 1 || activeTurn == 0) {
            if      (IsKeyPressed(KEY_A)) pressed = 0;
            else if (IsKeyPressed(KEY_B)) pressed = 1;
            else if (IsKeyPressed(KEY_C)) pressed = 2;
            else if (IsKeyPressed(KEY_D)) pressed = 3;
        }
        if (state.playerCount == 2 && activeTurn == 1) {
            if      (IsKeyPressed(KEY_LEFT))  pressed = 0;
            else if (IsKeyPressed(KEY_UP))    pressed = 1;
            else if (IsKeyPressed(KEY_RIGHT)) pressed = 2;
            else if (IsKeyPressed(KEY_DOWN))  pressed = 3;
        }
        if (pressed == -1 && IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
            Vector2 m = GetMousePosition();
            for (int i = 0; i < 4; i++)
                if (CheckCollisionPointRec(m, choiceRects[i])) { pressed = i; break; }
        }

        const Question& q = state.quizManager.current();
        correctChoice = q.correctIndex;

        if (pressed != -1) {
            selectedChoice = pressed;
            Player& p = state.players[activeTurn];
            if (pressed == q.correctIndex) {
                p.addScore(calculatePoints(p));
                p.correct++;
                feedbackState = FeedbackState::CORRECT;
            } else {
                p.resetStreak();
                feedbackState = FeedbackState::WRONG;
            }
            p.total++;
            feedbackTimer = 0.0f;
        } else if (timer.isDone()) {
            selectedChoice = -1;
            state.players[activeTurn].resetStreak();
            state.players[activeTurn].total++;
            feedbackState = FeedbackState::TIMEOUT;
            feedbackTimer = 0.0f;
        }
    }

    void draw() override {
        UIStyle::drawBackground(1280, 720);
        UIStyle::drawStars(1280, 720, pulse);

        const Question& q = state.quizManager.current();

        // ── Top bar ───────────────────────────────────────────
        DrawRectangle(0, 0, 1280, 50, Fade(UIStyle::BG_SPACE, 0.94f));
        DrawRectangle(0, 49, 1280, 1, Fade(WHITE, 0.07f));

        // Mode label
        const char* modeLabel = (state.playerCount == 2) ? "PvP Mode" : "Solo";
        Color modeCol = (state.playerCount == 2) ? UIStyle::ACCENT_PINK
                                                  : UIStyle::ACCENT_CYAN;
        UIStyle::drawText(modeLabel, 16, 14, 18, modeCol);

        // Q counter pill
        char qcnt[32];
        snprintf(qcnt, sizeof(qcnt), "Q %d / %d",
                 questionsAnswered+1, state.questionsPerGame);
        int qcw = UIStyle::textW(qcnt, 18);
        DrawRectangleRounded({(float)(1280-qcw-32), 10, (float)(qcw+24), 30},
                              0.5f, 4, Fade(WHITE, 0.10f));
        UIStyle::drawText(qcnt, 1280-qcw-20, 16, 18,
                           Fade(UIStyle::TEXT_PRIMARY, 0.82f));

        // PvP turn label (centered)
        if (state.playerCount == 2) {
            bool p1 = (activeTurn == 0);
            Color tc = p1 ? UIStyle::ACCENT_CYAN : UIStyle::ACCENT_PINK;
            const char* tt = p1 ? "Player 1  —  [ A / B / C / D ]"
                                : "Player 2  —  [ ← / ↑ / → / ↓ ]";
            UIStyle::drawTextC(tt, 640, 14, 18, tc);
        }

        // ── Main card ─────────────────────────────────────────
        // Shadow
        DrawRectangleRounded({63, 64, 1154, 520}, 0.05f, 8, Fade(BLACK, 0.50f));
        // Card
        DrawRectangleRounded({60, 60, 1154, 520}, 0.05f, 8, UIStyle::BG_CARD);
        DrawRectangleRoundedLines({60, 60, 1154, 520}, 0.05f, 8, Fade(WHITE, 0.07f));
        DrawRectangleRounded({62, 61, 1150, 3}, 0.5f, 4, Fade(WHITE, 0.06f));

        // ── Category label ────────────────────────────────────
        UIStyle::drawText(q.category.c_str(), 84, 80, 18,
                           UIStyle::TEXT_SECONDARY);

        // ── Question text ─────────────────────────────────────
        UIStyle::drawWrapped(q.text.c_str(), 84, 110, 1106, 32,
                              UIStyle::TEXT_PRIMARY, 160);

        // Divider
        DrawRectangle(84, 286, 1106, 1, Fade(WHITE, 0.07f));

        // ── Answer buttons (y: 238–482, 2×2) ─────────────────
        const char* kP1[4] = {"A","B","C","D"};
        const char* kP2[4] = {"←","↑","→","↓"};
        for (int i = 0; i < 4; i++)
            drawAnswer(i, q,
                       (state.playerCount==2 && activeTurn==1) ? kP2[i] : kP1[i]);

        // ── Timer bar ─────────────────────────────────────────
        float prog = timer.progress();
        Color tFill = (prog < 0.5f) ? UIStyle::ACCENT_CYAN
                    : (prog < 0.75f) ? (Color){255,160,50,255}
                    :                  UIStyle::ACCENT_RED;

        DrawRectangle(0, 600, 1280, 16, Fade(BLACK, 0.4f));
        UIStyle::drawBar(0, 600, 1280, 16, 1.0f - prog, tFill, {20,12,50,255});

        // Time number above bar
        char tStr[12]; snprintf(tStr, sizeof(tStr), "%02.0f", timer.remaining());
        int tsw = UIStyle::textW(tStr, 20);
        DrawRectangleRounded({(float)(640-tsw/2-10), 570, (float)(tsw+20), 28},
                              0.4f, 4, Fade(UIStyle::BG_SPACE, 0.95f));
        UIStyle::drawText(tStr, 640-tsw/2, 574, 20, tFill);

        // ── Scores (514–570) ──────────────────────────────────
        drawScores();

        // ── Feedback ──────────────────────────────────────────
        if (feedbackState != FeedbackState::NONE) drawFeedback(q);
    }

    int nextScene() override { return sceneNext; }

private:
    GameState&    state;
    Timer         timer;
    int           questionsAnswered, activeTurn, selectedChoice, correctChoice;
    FeedbackState feedbackState;
    float         feedbackTimer, pulse;
    int           sceneNext;
    Rectangle     choiceRects[4];
    static constexpr float FEEDBACK_DURATION = 1.8f;

    void buildChoiceRects() {
        float cx = 84.0f, sy = 296.0f;
        float bw = (1106.0f - 16.0f) / 2.0f, bh = 130.0f, gap = 16.0f;
        choiceRects[0] = {cx,       sy,          bw, bh};
        choiceRects[1] = {cx+bw+gap,sy,          bw, bh};
        choiceRects[2] = {cx,       sy+bh+gap,   bw, bh};
        choiceRects[3] = {cx+bw+gap,sy+bh+gap,   bw, bh};
    }

    int calculatePoints(const Player& p) {
        return 100 + (int)(timer.remaining()*10.0f) + p.streak*10;
    }

    void advanceQuestion() {
        feedbackState = FeedbackState::NONE;
        selectedChoice = correctChoice = -1;
        feedbackTimer = 0.0f;
        timer.reset();
        questionsAnswered++;
        if (state.playerCount == 2) activeTurn = 1 - activeTurn;
        
        if (questionsAnswered >= state.questionsPerGame ||
            state.quizManager.currentIndex + 1 >= state.quizManager.totalQuestions()) {
            sceneNext = SCENE_RESULT;
        } else {
            state.quizManager.advance();
        }
    }

    void drawAnswer(int idx, const Question& q, const char* key) {
        Rectangle r = choiceRects[idx];
        bool hov = false;
        Color bg, border, textCol;

        if (feedbackState == FeedbackState::NONE) {
            hov = CheckCollisionPointRec(GetMousePosition(), r);
            bg      = hov ? Fade(WHITE, 0.18f) : Fade(WHITE, 0.08f);
            border  = hov ? Fade(WHITE, 0.45f) : Fade(WHITE, 0.14f);
            textCol = UIStyle::TEXT_PRIMARY;
        } else {
            if (idx == correctChoice) {
                bg = {22, 130, 60, 215}; border = UIStyle::ACCENT_GREEN;
                textCol = UIStyle::TEXT_PRIMARY;
            } else if (idx == selectedChoice && feedbackState == FeedbackState::WRONG) {
                bg = {170, 32, 32, 200}; border = UIStyle::ACCENT_RED;
                textCol = UIStyle::TEXT_PRIMARY;
            } else {
                bg = Fade(WHITE, 0.04f); border = Fade(WHITE, 0.07f);
                textCol = UIStyle::TEXT_DIM;
            }
        }

        // Shadow + body
        DrawRectangleRounded({r.x+2, r.y+2, r.width, r.height},
                              0.10f, 6, Fade(BLACK, 0.35f));
        DrawRectangleRounded(r, 0.10f, 6, bg);
        DrawRectangleRoundedLines(r, 0.10f, 6, border);
        // Top highlight
        DrawRectangleRounded({r.x+2, r.y+2, r.width-4, 2},
                              0.5f, 4, Fade(WHITE, 0.07f));

        // Correct badge (top-right, shown after answer)
        if (feedbackState != FeedbackState::NONE && idx == correctChoice) {
            Rectangle ck = {r.x + r.width - 50, r.y + 10, 38, 24};
            DrawRectangleRounded(ck, 0.4f, 4, UIStyle::ACCENT_GREEN);
            int cw = UIStyle::textW("OK", 13);
            UIStyle::drawText("OK",
                (int)(ck.x + ck.width/2 - cw/2),
                (int)(ck.y + ck.height/2 - 7),
                13, WHITE);
        }

        // Answer text
        UIStyle::drawWrapped(q.choices[idx].c_str(),
                              (int)r.x + 24, (int)r.y + 16,
                              (int)r.width - 48, 28, textCol,
                              (int)r.height - 32);
    }

    void drawScores() {
        if (state.playerCount == 2) {
            UIStyle::drawScorePill(20, 634, 280, 70,
                "Player 1", state.players[0].score, state.players[0].streak,
                UIStyle::ACCENT_CYAN, activeTurn == 0);
            UIStyle::drawScorePill(980, 634, 280, 70,
                "Player 2", state.players[1].score, state.players[1].streak,
                UIStyle::ACCENT_PINK, activeTurn == 1);
            // Turn dot
            Color dc = (activeTurn==0) ? UIStyle::ACCENT_CYAN : UIStyle::ACCENT_PINK;
            DrawCircle(640, 665, 6, dc);
            DrawCircleLines(640, 665, 10, Fade(dc, 0.35f));
        } else {
            char sc[32];
            snprintf(sc, sizeof(sc), "%d pts", state.players[0].score);
            int sw = UIStyle::textW(sc, 24);
            DrawRectangleRounded({(float)(640-sw/2-24), 634,
                                   (float)(sw+48), 40},
                                  0.4f, 4, Fade(WHITE, 0.07f));
            UIStyle::drawText(sc, 640-sw/2, 642, 24,
                               Fade(UIStyle::TEXT_PRIMARY, 0.82f));
            if (state.players[0].streak >= 2) {
                char stk[24];
                snprintf(stk, sizeof(stk), "x%d streak",
                         state.players[0].streak);
                UIStyle::drawTextC(stk, 640, 690, 18, UIStyle::ACCENT_GOLD);
            }
        }
    }

    void drawFeedback(const Question& q) {
        const char* msg;
        Color mc;
        switch (feedbackState) {
            case FeedbackState::CORRECT: msg="Correct!";  mc=UIStyle::ACCENT_GREEN; break;
            case FeedbackState::WRONG:   msg="Wrong!";    mc=UIStyle::ACCENT_RED;   break;
            default:                     msg="Time's Up!";mc={255,160,50,255};      break;
        }

        // Animate banner sliding in
        float frac = feedbackTimer / FEEDBACK_DURATION;
        // Pop in quickly during the first 1/6th of the duration
        float ease = (frac * 6.0f);
        if (ease > 1.0f) ease = 1.0f;
        float alpha = 1.0f - powf(1.0f - ease, 3.0f); // cubic ease out

        // Banner dimensions
        int bh = 140; 
        int by = 360 - bh/2;

        // Draw horizontal sweep background over the middle, avoiding full screen blackout
        DrawRectangle(0, by, 1280, bh, Fade(UIStyle::BG_SPACE, 0.94f * alpha));
        DrawRectangle(0, by, 1280, 2,  Fade(mc, 0.8f * alpha));
        DrawRectangle(0, by+bh-2, 1280, 2,  Fade(mc, 0.8f * alpha));

        if (alpha > 0.05f) {
            // Big result text (Left Side)
            float fs = 64.0f;
            int mw = UIStyle::textW(msg, fs);
            int txtX = 320 - mw/2;
            int txtY = 360 - (int)fs/2;
            
            // Shadow
            DrawTextEx(UIStyle::FONT_REG, msg,
                       {(float)(txtX+3), (float)(txtY+3)}, fs, 2.0f, Fade(BLACK, 0.8f * alpha));
            // Glow
            DrawTextEx(UIStyle::FONT_REG, msg,
                       {(float)(txtX-2), (float)(txtY-2)}, fs, 2.0f, Fade(mc, 0.28f * alpha));
            // Fill
            DrawTextEx(UIStyle::FONT_REG, msg,
                       {(float)txtX, (float)txtY}, fs, 2.0f, Fade(mc, alpha));

            // Points or Answer string (Right Side)
            if (feedbackState == FeedbackState::CORRECT) {
                char pts[32];
                snprintf(pts, sizeof(pts), "+%d pts",
                         calculatePoints(state.players[activeTurn]));
                int pw = UIStyle::textW(pts, 40);
                int px = 960 - pw/2;
                int py = 360 - 20;

                // Points badge
                DrawRectangleRounded({(float)(px-24), (float)(py-12), (float)(pw+48), 64},
                                      0.4f, 4, Fade(mc, 0.18f * alpha));
                DrawRectangleRoundedLines({(float)(px-24), (float)(py-12), (float)(pw+48), 64},
                                           0.4f, 4, Fade(mc, 0.50f * alpha));
                UIStyle::drawTextC(pts, 960, py, 40, Fade(mc, alpha));
            } else {
                char ans[200];
                snprintf(ans, sizeof(ans), "Answer: %s",
                         q.choices[q.correctIndex].c_str());
                while (UIStyle::textW(ans, 26) > 520 && strlen(ans) > 8)
                    ans[strlen(ans)-1] = 0;
                
                UIStyle::drawTextC(ans, 960, 360 - 13, 26, Fade(UIStyle::TEXT_PRIMARY, 0.95f * alpha));
            }

            // Middle vertical divider
            DrawRectangle(639, by+20, 2, bh-40, Fade(mc, 0.35f * alpha));
        }

        // Progress drain bar shrinking at the bottom of the card
        DrawRectangle(60, 574, 1154, 6, Fade(WHITE, 0.07f));
        DrawRectangle(60, 574, (int)(1154*(1.0f - frac)), 6, Fade(mc, 0.85f));
    }
};