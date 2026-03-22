#pragma once
#include "../core/Scene.h"
#include "../core/Game.h"
#include "../service/Timer.h"
#include "../ui/UIStyle.h"
#include "raylib.h"
#include <cmath>
#include <string>
#include <vector>

enum class FeedbackState { NONE, CORRECT, WRONG, TIMEOUT };

class QuizScene : public Scene {
public:
    QuizScene(GameState& gs) : state(gs), timer(15.0f) {
        questionsAnswered = 0;
        selectedP1        = -1;
        selectedP2        = -1;
        timeP1            = 0.0f;
        timeP2            = 0.0f;
        shakeTimer        = 0.0f;
        blackoutTimer     = 0.0f;
        sabotageMessage   = "";
        feedbackState     = FeedbackState::NONE;
        feedbackTimer     = 0.0f;
        correctChoice     = -1;
        sceneNext         = SCENE_STAY;
        pulse             = 0.0f;
        buildChoiceRects();
    }

    void update() override {
        if (sceneNext != SCENE_STAY) return;
        
        if (IsKeyPressed(KEY_ESCAPE)) {
            isPaused = !isPaused;
        }

        if (isPaused) {
            Vector2 m = GetMousePosition();
            hResume = CheckCollisionPointRec(m, btnResume);
            hQuitMenu = CheckCollisionPointRec(m, btnQuitMenu);

            if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
                if (hResume) isPaused = false;
                if (hQuitMenu) sceneNext = SCENE_MENU;
            }
            return;
        }
        
        pulse += GetFrameTime() * 1.6f;

        if (feedbackState != FeedbackState::NONE) {
            feedbackTimer += GetFrameTime();
            if (feedbackTimer >= FEEDBACK_DURATION) advanceQuestion();
            return;
        }

        if (shakeTimer > 0) shakeTimer -= GetFrameTime();
        if (blackoutTimer > 0) blackoutTimer -= GetFrameTime();

        timer.update();

        int pressedP1 = -1;
        int pressedP2 = -1;

        if (selectedP1 == -1) {
            if      (IsKeyPressed(KEY_A)) pressedP1 = 0;
            else if (IsKeyPressed(KEY_B)) pressedP1 = 1;
            else if (IsKeyPressed(KEY_C)) pressedP1 = 2;
            else if (IsKeyPressed(KEY_D)) pressedP1 = 3;
            
            if (pressedP1 == -1 && IsMouseButtonReleased(MOUSE_LEFT_BUTTON) && state.playerCount == 1) {
                Vector2 m = GetMousePosition();
                for (int i = 0; i < 4; i++)
                    if (CheckCollisionPointRec(m, choiceRects[i])) { pressedP1 = i; break; }
            }
        }
        
        if (state.playerCount == 2 && selectedP2 == -1) {
            if      (IsKeyPressed(KEY_KP_1))  pressedP2 = 0;
            else if (IsKeyPressed(KEY_KP_2))  pressedP2 = 1;
            else if (IsKeyPressed(KEY_KP_3))  pressedP2 = 2;
            else if (IsKeyPressed(KEY_KP_4))  pressedP2 = 3;
        }

        const Question& q = state.quizManager.current();
        correctChoice = q.correctIndex;

        if (pressedP1 != -1) {
            selectedP1 = pressedP1;
            timeP1 = timer.remaining();
            if (state.players[0].sabotageCharges > 0 && state.playerCount == 2) {
                state.players[0].sabotageCharges--;
                if (GetRandomValue(0, 1) == 0) { shakeTimer = 2.0f; sabotageMessage = "P1 used SHAKE!"; }
                else { blackoutTimer = 2.0f; sabotageMessage = "P1 used BLACKOUT!"; }
            }
        }
        
        if (pressedP2 != -1) {
            selectedP2 = pressedP2;
            timeP2 = timer.remaining();
            if (state.players[1].sabotageCharges > 0) {
                state.players[1].sabotageCharges--;
                if (GetRandomValue(0, 1) == 0) { shakeTimer = 2.0f; sabotageMessage = "P2 used SHAKE!"; }
                else { blackoutTimer = 2.0f; sabotageMessage = "P2 used BLACKOUT!"; }
            }
        }

        bool p1Done = (selectedP1 != -1);
        bool p2Done = (state.playerCount == 1) ? true : (selectedP2 != -1);

        if ((p1Done && p2Done) || timer.isDone()) {
            if (selectedP1 == q.correctIndex) {
                state.players[0].addScore(calculatePoints(state.players[0], timeP1));
                state.players[0].correct++;
                if (state.playerCount == 1) feedbackState = FeedbackState::CORRECT;
            } else {
                state.players[0].resetStreak();
                if (state.playerCount == 1) feedbackState = (selectedP1 == -1) ? FeedbackState::TIMEOUT : FeedbackState::WRONG;
            }
            state.players[0].total++;

            if (state.playerCount == 2) {
                if (selectedP2 == q.correctIndex) {
                    state.players[1].addScore(calculatePoints(state.players[1], timeP2));
                    state.players[1].correct++;
                } else {
                    state.players[1].resetStreak();
                }
                state.players[1].total++;
                
                feedbackState = FeedbackState::CORRECT; 
            }
            feedbackTimer = 0.0f;
        }
    }

    void draw() override {
        UIStyle::drawBackground(1280, 720);
        UIStyle::drawStars(1280, 720, pulse);

        if (shakeTimer > 0 && feedbackState == FeedbackState::NONE) {
            float dx = (float)GetRandomValue(-12, 12);
            float dy = (float)GetRandomValue(-12, 12);
            Camera2D cam = {0};
            cam.offset = {dx, dy};
            cam.target = {0,0};
            cam.rotation = 0.0f;
            cam.zoom = 1.0f;
            BeginMode2D(cam);
        }

        const Question& q = state.quizManager.current();

        // ── Top bar ───────────────────────────────────────────
        DrawRectangle(0, 0, 1280, 50, Fade(UIStyle::BG_SPACE, 0.94f));
        DrawRectangle(0, 49, 1280, 1, Fade(WHITE, 0.07f));

        const char* modeLabel = (state.playerCount == 2) ? "PvP Mode" : "Solo";
        Color modeCol = (state.playerCount == 2) ? UIStyle::ACCENT_PINK : UIStyle::ACCENT_CYAN;
        UIStyle::drawText(modeLabel, 16, 14, 18, modeCol);

        char qcnt[32];
        snprintf(qcnt, sizeof(qcnt), "Q %d / %d",
                 questionsAnswered+1, state.questionsPerGame);
        int qcw = UIStyle::textW(qcnt, 18);
        DrawRectangleRounded({(float)(1280-qcw-32), 10, (float)(qcw+24), 30},
                              0.5f, 4, Fade(WHITE, 0.10f));
        UIStyle::drawText(qcnt, 1280-qcw-20, 16, 18,
                           Fade(UIStyle::TEXT_PRIMARY, 0.82f));

        if (state.playerCount == 2) {
            UIStyle::drawTextC("Real-time PvP  —  [P1: A-D | P2: Numpad 1-4]", 640, 14, 18, WHITE);
        }

        // Pause Instruction
        UIStyle::drawText("[ESC] to Pause", 1130, 20, 18, Fade(UIStyle::TEXT_DIM, 0.6f));

        // ── Main card ─────────────────────────────────────────
        DrawRectangleRounded({63, 64, 1154, 520}, 0.05f, 8, Fade(BLACK, 0.50f));
        DrawRectangleRounded({60, 60, 1154, 520}, 0.05f, 8, UIStyle::BG_CARD);
        DrawRectangleRoundedLines({60, 60, 1154, 520}, 0.05f, 8, Fade(WHITE, 0.07f));
        DrawRectangleRounded({62, 61, 1150, 3}, 0.5f, 4, Fade(WHITE, 0.06f));

        UIStyle::drawText(q.category.c_str(), 84, 80, 18, UIStyle::TEXT_SECONDARY);
        UIStyle::drawWrapped(q.text.c_str(), 84, 110, 1106, 32, UIStyle::TEXT_PRIMARY, 160);
        DrawRectangle(84, 286, 1106, 1, Fade(WHITE, 0.07f));

        // Sabotage UI indicator
        if (!sabotageMessage.empty() && feedbackState == FeedbackState::NONE) {
            Color sCol = sabotageMessage.find("P1") != std::string::npos ? UIStyle::ACCENT_CYAN : UIStyle::ACCENT_PINK;
            int smw = UIStyle::textW(sabotageMessage.c_str(), 24);
            DrawRectangleRounded({(float)(640-smw/2-24), 220, (float)(smw+48), 40}, 0.5f, 4, Fade(BLACK, 0.8f));
            UIStyle::drawText(sabotageMessage.c_str(), 640-smw/2, 227, 24, sCol);
        }

        // Apply Blackout mask over answer area IF blackout timer is active
        if (blackoutTimer > 0 && feedbackState == FeedbackState::NONE) {
            DrawRectangleRounded({60, 270, 1154, 310}, 0.05f, 8, {(unsigned char)10,(unsigned char)10,(unsigned char)12,(unsigned char)245});
            int cw = UIStyle::textW("BLACKOUT!", 50);
            DrawTextEx(UIStyle::FONT_BOLD, "BLACKOUT!", {(float)(640 - cw/2), 380}, 50, 2.0f, UIStyle::ACCENT_RED);
        } else {
            const char* kSolo[4] = {"A","B","C","D"};
            const char* kPvP[4] = {"A|1","B|2","C|3","D|4"};
            for (int i = 0; i < 4; i++)
                drawAnswer(i, q, (state.playerCount==2) ? kPvP[i] : kSolo[i]);
        }

        float prog = timer.progress();
        Color tFill = (prog < 0.5f) ? UIStyle::ACCENT_CYAN
                    : (prog < 0.75f) ? (Color){255,160,50,255}
                    :                  UIStyle::ACCENT_RED;

        DrawRectangle(0, 600, 1280, 16, Fade(BLACK, 0.4f));
        UIStyle::drawBar(0, 600, 1280, 16, 1.0f - prog, tFill, {(unsigned char)20,(unsigned char)12,(unsigned char)50,(unsigned char)255});

        char tStr[12]; snprintf(tStr, sizeof(tStr), "%02.0f", timer.remaining());
        int tsw = UIStyle::textW(tStr, 20);
        DrawRectangleRounded({(float)(640-tsw/2-10), 570, (float)(tsw+20), 28},
                              0.4f, 4, Fade(UIStyle::BG_SPACE, 0.95f));
        UIStyle::drawText(tStr, 640-tsw/2, 574, 20, tFill);

        drawScores();
        if (shakeTimer > 0 && feedbackState == FeedbackState::NONE) EndMode2D();
        
        if (feedbackState != FeedbackState::NONE) drawFeedback(q);

        if (isPaused) {
            DrawRectangle(0, 0, 1280, 720, Fade(BLACK, 0.94f));
            UIStyle::drawTextC("PAUSED", 640, 220, 64, UIStyle::ACCENT_CYAN);
            UIStyle::drawButton(btnResume, "Resume", hResume, {(unsigned char)25, (unsigned char)14, (unsigned char)90, (unsigned char)255}, UIStyle::ACCENT_CYAN);
            UIStyle::drawButton(btnQuitMenu, "Quit to Menu", hQuitMenu, {(unsigned char)80, (unsigned char)12, (unsigned char)12, (unsigned char)255}, UIStyle::ACCENT_RED);
        }
    }

    int nextScene() override { return sceneNext; }

private:
    GameState&    state;
    Timer         timer;
    int           questionsAnswered;
    int           selectedP1, selectedP2, correctChoice;
    float         timeP1, timeP2;
    float         shakeTimer, blackoutTimer;
    std::string   sabotageMessage;
    
    bool          isPaused = false;
    Rectangle     btnResume = {500, 340, 280, 64};
    Rectangle     btnQuitMenu = {500, 430, 280, 64};
    bool          hResume = false, hQuitMenu = false;
    
    FeedbackState feedbackState;
    float         feedbackTimer, pulse;
    int           sceneNext;
    Rectangle     choiceRects[4];
    static constexpr float FEEDBACK_DURATION = 3.0f; // Extended slightly for PvP reading time

    void buildChoiceRects() {
        float cx = 84.0f, sy = 296.0f;
        float bw = (1106.0f - 16.0f) / 2.0f, bh = 130.0f, gap = 16.0f;
        choiceRects[0] = {cx,       sy,          bw, bh};
        choiceRects[1] = {cx+bw+gap,sy,          bw, bh};
        choiceRects[2] = {cx,       sy+bh+gap,   bw, bh};
        choiceRects[3] = {cx+bw+gap,sy+bh+gap,   bw, bh};
    }

    int calculatePoints(const Player& p, float timeRem) {
        if (timeRem < 0) timeRem = 0.0f;
        return 100 + (int)(timeRem*10.0f) + p.streak*10;
    }

    void advanceQuestion() {
        feedbackState = FeedbackState::NONE;
        selectedP1 = selectedP2 = correctChoice = -1;
        timeP1 = timeP2 = shakeTimer = blackoutTimer = feedbackTimer = 0.0f;
        sabotageMessage = "";
        
        timer.reset();
        questionsAnswered++;
        
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
            
            if (state.playerCount == 2) {
                if (selectedP1 == idx && selectedP2 == idx) border = Fade(WHITE, 0.9f);
                else if (selectedP1 == idx) border = UIStyle::ACCENT_CYAN;
                else if (selectedP2 == idx) border = UIStyle::ACCENT_PINK;
                else border = hov ? Fade(WHITE, 0.45f) : Fade(WHITE, 0.14f);
            } else {
                if (selectedP1 == idx) border = UIStyle::ACCENT_CYAN;
                else border = hov ? Fade(WHITE, 0.45f) : Fade(WHITE, 0.14f);
            }
            textCol = UIStyle::TEXT_PRIMARY;
        } else {
            bool p1C = (selectedP1 == idx);
            bool p2C = (state.playerCount == 2 && selectedP2 == idx);
            
            if (idx == correctChoice) {
                bg = {(unsigned char)22, (unsigned char)130, (unsigned char)60, (unsigned char)215}; border = UIStyle::ACCENT_GREEN;
                textCol = UIStyle::TEXT_PRIMARY;
            } else if (p1C || p2C) {
                bg = {(unsigned char)170, (unsigned char)32, (unsigned char)32, (unsigned char)200}; border = UIStyle::ACCENT_RED;
                textCol = UIStyle::TEXT_PRIMARY;
            } else {
                bg = Fade(WHITE, 0.04f); border = Fade(WHITE, 0.07f);
                textCol = UIStyle::TEXT_DIM;
            }
        }

        DrawRectangleRounded({r.x+2, r.y+2, r.width, r.height}, 0.10f, 6, Fade(BLACK, 0.35f));
        DrawRectangleRounded(r, 0.10f, 6, bg);
        DrawRectangleRoundedLines(r, 0.10f, 6, border);
        DrawRectangleRounded({r.x+2, r.y+2, r.width-4, 2}, 0.5f, 4, Fade(WHITE, 0.07f));

        UIStyle::drawWrapped(q.choices[idx].c_str(), (int)r.x + 24, (int)r.y + 16, (int)r.width - 48, 28, textCol, (int)r.height - 32);

        if (feedbackState == FeedbackState::NONE) {
            DrawRectangleRounded({r.x + r.width - 50, r.y + r.height - 30, 40, 20}, 0.3f, 4, Fade(BLACK, 0.5f));
            UIStyle::drawText(key, (int)(r.x + r.width - 45), (int)(r.y + r.height - 28), 14, Fade(WHITE, 0.5f));
        }

        if (feedbackState != FeedbackState::NONE) {
            if (idx == correctChoice) {
                Rectangle ck = {r.x + r.width - 50, r.y + 10, 38, 24};
                DrawRectangleRounded(ck, 0.4f, 4, UIStyle::ACCENT_GREEN);
                int cw = UIStyle::textW("OK", 13);
                UIStyle::drawText("OK", (int)(ck.x + ck.width/2 - cw/2), (int)(ck.y + ck.height/2 - 7), 13, WHITE);
            }
            if (selectedP1 == idx) {
                Rectangle ck = {r.x + r.width - (idx==correctChoice?95:50), r.y + 10, 38, 24};
                DrawRectangleRounded(ck, 0.4f, 4, UIStyle::ACCENT_CYAN);
                int cw = UIStyle::textW("P1", 13);
                UIStyle::drawText("P1", (int)(ck.x + ck.width/2 - cw/2), (int)(ck.y + ck.height/2 - 7), 13, BLACK);
            }
            if (state.playerCount == 2 && selectedP2 == idx) {
                int off = 50 + (idx==correctChoice?45:0) + (selectedP1==idx?45:0);
                Rectangle ck = {r.x + r.width - off, r.y + 10, 38, 24};
                DrawRectangleRounded(ck, 0.4f, 4, UIStyle::ACCENT_PINK);
                int cw = UIStyle::textW("P2", 13);
                UIStyle::drawText("P2", (int)(ck.x + ck.width/2 - cw/2), (int)(ck.y + ck.height/2 - 7), 13, BLACK);
            }
        }
    }

    void drawScores() {
        if (state.playerCount == 2) {
            UIStyle::drawScorePill(20, 634, 280, 70, "Player 1", state.players[0].score, state.players[0].streak, UIStyle::ACCENT_CYAN, selectedP1==-1);
            UIStyle::drawScorePill(980, 634, 280, 70, "Player 2", state.players[1].score, state.players[1].streak, UIStyle::ACCENT_PINK, selectedP2==-1);
            
            if (state.players[0].sabotageCharges > 0) {
                char chg[32]; snprintf(chg, sizeof(chg), "%d ATK", state.players[0].sabotageCharges);
                UIStyle::drawTextC(chg, 160, 712, 16, UIStyle::ACCENT_GOLD);
            }
            if (state.players[1].sabotageCharges > 0) {
                char chg[32]; snprintf(chg, sizeof(chg), "%d ATK", state.players[1].sabotageCharges);
                UIStyle::drawTextC(chg, 1120, 712, 16, UIStyle::ACCENT_GOLD);
            }
        } else {
            char sc[32];
            snprintf(sc, sizeof(sc), "%d pts", state.players[0].score);
            int sw = UIStyle::textW(sc, 24);
            DrawRectangleRounded({(float)(640-sw/2-24), 634, (float)(sw+48), 40}, 0.4f, 4, Fade(WHITE, 0.07f));
            UIStyle::drawText(sc, 640-sw/2, 642, 24, Fade(UIStyle::TEXT_PRIMARY, 0.82f));
            
            if (state.players[0].streak >= 2) {
                char stk[24]; snprintf(stk, sizeof(stk), "x%d streak", state.players[0].streak);
                UIStyle::drawTextC(stk, 640, 690, 18, UIStyle::ACCENT_GOLD);
            }
            if (state.players[0].sabotageCharges > 0) {
                char chg[32]; snprintf(chg, sizeof(chg), "%d ATK CHARGES (N/A IN SOLO)", state.players[0].sabotageCharges);
                UIStyle::drawTextC(chg, 640, 710, 14, Fade(WHITE, 0.3f));
            }
        }
    }

    void drawFeedback(const Question& q) {
        float frac = feedbackTimer / FEEDBACK_DURATION;
        float ease = (frac * 6.0f);
        if (ease > 1.0f) ease = 1.0f;
        float alpha = 1.0f - powf(1.0f - ease, 3.0f);

        int bh = 140; 
        int by = 360 - bh/2;
        
        Color mc = (state.playerCount == 2) ? UIStyle::ACCENT_GOLD : 
                  (feedbackState == FeedbackState::CORRECT ? UIStyle::ACCENT_GREEN : 
                   (feedbackState == FeedbackState::WRONG ? UIStyle::ACCENT_RED : (Color){(unsigned char)255,(unsigned char)160,(unsigned char)50,(unsigned char)255}));

        DrawRectangle(0, by, 1280, bh, Fade(UIStyle::BG_SPACE, 0.94f * alpha));
        DrawRectangle(0, by, 1280, 2,  Fade(mc, 0.8f * alpha));
        DrawRectangle(0, by+bh-2, 1280, 2,  Fade(mc, 0.8f * alpha));

        if (alpha > 0.05f) {
            float fs = 64.0f;
            const char* centerMsg = (state.playerCount == 2) ? "Round Over!" :
                                    (feedbackState == FeedbackState::CORRECT ? "Correct!" : 
                                     (feedbackState == FeedbackState::WRONG ? "Wrong!" : "Time's Up!"));

            int mw = UIStyle::textW(centerMsg, fs);
            DrawTextEx(UIStyle::FONT_REG, centerMsg, {(float)(640 - mw/2), (float)(360 - fs/2 - 20)}, fs, 2.0f, Fade(mc, alpha));

            if (state.playerCount == 2) {
                char p1M[64];
                if (selectedP1 == q.correctIndex) snprintf(p1M, sizeof(p1M), "P1: +%d", calculatePoints(state.players[0], timeP1));
                else if (selectedP1 == -1) snprintf(p1M, sizeof(p1M), "P1: Time Out");
                else snprintf(p1M, sizeof(p1M), "P1: Miss");
                UIStyle::drawTextC(p1M, 320, 370, 30, UIStyle::ACCENT_CYAN);

                char p2M[64];
                if (selectedP2 == q.correctIndex) snprintf(p2M, sizeof(p2M), "P2: +%d", calculatePoints(state.players[1], timeP2));
                else if (selectedP2 == -1) snprintf(p2M, sizeof(p2M), "P2: Time Out");
                else snprintf(p2M, sizeof(p2M), "P2: Miss");
                UIStyle::drawTextC(p2M, 960, 370, 30, UIStyle::ACCENT_PINK);
            } else {
                if (feedbackState == FeedbackState::CORRECT) {
                    char pts[32]; snprintf(pts, sizeof(pts), "+%d pts", calculatePoints(state.players[0], timeP1));
                    UIStyle::drawTextC(pts, 640, 380, 40, Fade(mc, alpha));
                } else {
                    char ans[200]; snprintf(ans, sizeof(ans), "Answer: %s", q.choices[q.correctIndex].c_str());
                    UIStyle::drawTextC(ans, 640, 380, 26, Fade(UIStyle::TEXT_PRIMARY, 0.95f * alpha));
                }
            }
        }

        DrawRectangle(60, 574, 1154, 6, Fade(WHITE, 0.07f));
        DrawRectangle(60, 574, (int)(1154*(1.0f - frac)), 6, Fade(mc, 0.85f));
    }
};