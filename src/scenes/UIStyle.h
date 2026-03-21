#pragma once
#include "raylib.h"
#include <cmath>
#include <string>
#include <vector>
#include <cstdio>

// ═══════════════════════════════════════════════════════════
//  UIStyle  —  clean, readable, Discord-inspired helpers
//
//  SETUP: put "font.ttf" in your working directory.
//  Recommended free fonts (rename to font.ttf):
//    • Nunito-Regular.ttf   (rounded, very readable)
//    • Outfit-Regular.ttf   (clean, modern)
//    • Rubik-Regular.ttf    (friendly, game-like)
//  Download from fonts.google.com
//
//  Call UIStyle::load()  once in Game::init()
//  Call UIStyle::unload() once in Game::shutdown()
// ═══════════════════════════════════════════════════════════

namespace UIStyle {

// ── Fonts ─────────────────────────────────────────────────
inline Font FONT_REG;   // regular weight
inline Font FONT_BOLD;  // same file loaded at larger base size (raylib trick)
inline bool fontLoaded = false;

inline void load() {
    // Try to load custom font; fall back to raylib default gracefully
    if (FileExists("assets/fonts/Outfit-Bold.ttf") && FileExists("assets/fonts/Outfit-Regular.ttf")) {
        FONT_REG  = LoadFontEx("assets/fonts/Outfit-Regular.ttf", 40, nullptr, 250);
        FONT_BOLD = LoadFontEx("assets/fonts/Outfit-Bold.ttf", 64, nullptr, 250);
        SetTextureFilter(FONT_REG.texture,  TEXTURE_FILTER_BILINEAR);
        SetTextureFilter(FONT_BOLD.texture, TEXTURE_FILTER_BILINEAR);
        fontLoaded = true;
    } else {
        // Use raylib's built-in font as fallback
        FONT_REG  = GetFontDefault();
        FONT_BOLD = GetFontDefault();
        fontLoaded = false;
    }
}

inline void unload() {
    if (fontLoaded) {
        UnloadFont(FONT_REG);
        UnloadFont(FONT_BOLD);
    }
}

// ── Palette ───────────────────────────────────────────────
constexpr Color BG_SPACE     = {10,  6,  28, 255};   // deep space
constexpr Color BG_CARD      = {22, 14,  56, 245};   // card surface
constexpr Color BG_CARD_MID  = {30, 20,  72, 245};   // slightly lighter card
constexpr Color BG_OVERLAY   = {8,   4,  20, 210};   // dim overlay

constexpr Color ACCENT_BLUE  = {80, 160, 255, 255};  // primary accent
constexpr Color ACCENT_GREEN = {60, 210, 120, 255};  // correct / success
constexpr Color ACCENT_RED   = {255, 75,  75, 255};  // wrong / danger
constexpr Color ACCENT_GOLD  = {255, 200,  50, 255}; // gold / winner
constexpr Color ACCENT_PINK  = {220,  80, 255, 255}; // P2 / PvP
constexpr Color ACCENT_CYAN  = {60,  210, 255, 255}; // P1 / info

constexpr Color TEXT_PRIMARY   = {240, 238, 255, 255}; // near-white
constexpr Color TEXT_SECONDARY = {160, 155, 190, 200}; // muted
constexpr Color TEXT_DIM       = {100,  96, 130, 170}; // very muted
constexpr Color BORDER_SUBTLE  = {255, 255, 255,  20}; // 8% white

// Per-choice button fill colors (A B C D) — muted, not neon
inline Color choiceFill(int i) {
    Color p[4] = {
        {35,  70, 180, 200},   // A – blue
        {25, 120,  65, 200},   // B – green
        {150, 75,  15, 200},   // C – amber
        {140,  20,  80, 200},  // D – rose
    };
    return p[i % 4];
}
inline Color choiceAccent(int i) {
    Color p[4] = {
        {100, 160, 255, 255},
        { 80, 210, 130, 255},
        {255, 160,  50, 255},
        {255,  80, 160, 255},
    };
    return p[i % 4];
}

// ── Font drawing helpers ──────────────────────────────────
// Draw text with the custom font, centered on cx
inline void drawTextC(const char* text, int cx, int y, float fs,
                       Color col, float spacing = 1.5f) {
    Vector2 size = MeasureTextEx(FONT_REG, text, fs, spacing);
    DrawTextEx(FONT_REG, text, {(float)cx - size.x/2, (float)y}, fs, spacing, col);
}

// Draw text left-aligned
inline void drawText(const char* text, int x, int y, float fs,
                      Color col, float spacing = 1.5f) {
    DrawTextEx(FONT_REG, text, {(float)x, (float)y}, fs, spacing, col);
}

// Measure text width
inline int textW(const char* text, float fs, float spacing = 1.5f) {
    return (int)MeasureTextEx(FONT_REG, text, fs, spacing).x;
}

// ── Background ────────────────────────────────────────────
inline void drawBackground(int w, int h) {
    // Deep space gradient (manual two-rect approximation)
    DrawRectangleGradientV(0, 0, w, h, {12, 7, 34, 255}, {6, 3, 18, 255});
    // Scanlines (very subtle — just depth)
    for (int y = 0; y < h; y += 3)
        DrawRectangle(0, y, w, 1, Fade(BLACK, 0.12f));
    // Perspective grid floor
    int horizon = h * 2 / 3;
    for (int row = 0; row < 12; row++) {
        float t  = (float)row / 11.0f;
        float yy = horizon + t * t * (h - horizon);
        DrawLine(0, (int)yy, w, (int)yy,
                 Fade({100, 60, 200, 255}, 0.04f + 0.06f * t));
    }
    for (int col = 0; col <= 14; col++) {
        float cx2 = w / 2.0f + (col - 7) * 96.0f;
        DrawLine((int)cx2, horizon,
                 (int)(w / 2.0f + (col - 7) * 700.0f), h,
                 Fade({100, 60, 200, 255}, 0.04f));
    }
    // Vignette
    DrawRectangleGradientH(0, 0, w/6, h, Fade(BLACK, 0.5f), Fade(BLACK, 0));
    DrawRectangleGradientH(w*5/6, 0, w/6, h, Fade(BLACK, 0), Fade(BLACK, 0.5f));
}

// ── Stars ─────────────────────────────────────────────────
inline void drawStars(int w, int h, float time) {
    for (int i = 0; i < 120; i++) {
        float x   = (float)((i * 173 + 31) % w);
        float y   = (float)((i * 97  + 17) % (h * 2 / 3));
        float twk = sinf(time * 1.1f + i * 0.4f);
        float a   = 0.25f + 0.45f * ((twk + 1.0f) / 2.0f);
        float sz  = (i % 4 == 0) ? 2.0f : 1.0f;
        DrawCircleV({x, y}, sz, Fade(WHITE, a));
    }
}

// ── Planet ────────────────────────────────────────────────
inline void drawPlanet(float cx, float cy, float r, float pulse,
                        Color body, Color ring) {
    float g = 0.12f + 0.08f * sinf(pulse);
    DrawCircle((int)cx, (int)cy, (int)(r + 14 + 5*sinf(pulse*0.4f)),
               Fade(body, g * 0.5f));
    DrawCircle((int)cx, (int)cy, (int)(r + 7), Fade(body, g));
    DrawCircle((int)cx, (int)cy, (int)r,
               {(unsigned char)(body.r/2),(unsigned char)(body.g/2),
                (unsigned char)(body.b/2), 255});
    DrawCircle((int)cx, (int)cy, (int)(r * 0.85f), body);
    DrawEllipse((int)cx, (int)(cy - r*0.14f), (int)(r*0.68f), (int)(r*0.11f),
                Fade(WHITE, 0.06f));
    DrawCircle((int)(cx - r*0.26f), (int)(cy - r*0.26f),
               (int)(r*0.16f), Fade(WHITE, 0.20f));
    DrawEllipseLines((int)cx, (int)(cy + r*0.14f),
                     (int)(r*1.52f), (int)(r*0.26f), ring);
    DrawEllipseLines((int)cx, (int)(cy + r*0.14f),
                     (int)(r*1.55f), (int)(r*0.29f), Fade(ring, 0.35f));
}

// ── Card ──────────────────────────────────────────────────
// Clean rounded dark card
inline void drawCard(Rectangle r, Color fill, Color border, float radius = 0.05f) {
    // Shadow
    DrawRectangleRounded({r.x + 3, r.y + 4, r.width, r.height},
                          radius, 6, Fade(BLACK, 0.4f));
    // Body
    DrawRectangleRounded(r, radius, 6, fill);
    // Border
    DrawRectangleRoundedLines(r, radius, 6, border);
    // Subtle top highlight
    DrawRectangleRounded({r.x + 2, r.y + 1, r.width - 4, 3},
                          radius, 4, Fade(WHITE, 0.07f));
}

// ── Clean button ──────────────────────────────────────────
inline void drawButton(Rectangle r, const char* label, bool hovered,
                        Color fill, Color accent, float fs = 22.0f) {
    Color bg = hovered ? Fade(fill, 0.95f) : Fade(fill, 0.65f);
    Color bd = hovered ? accent : Fade(accent, 0.5f);

    // Hover glow
    if (hovered)
        DrawRectangleRounded({r.x-3, r.y-3, r.width+6, r.height+6},
                              0.15f, 6, Fade(accent, 0.14f));

    DrawRectangleRounded(r, 0.15f, 6, bg);
    DrawRectangleRoundedLines(r, 0.15f, 6, bd);

    // Top highlight
    DrawRectangleRounded({r.x+2, r.y+2, r.width-4, 3},
                          0.3f, 4, Fade(WHITE, 0.10f));

    // Label
    float actualFs = fs;
    while (textW(label, actualFs) > (int)r.width - 24 && actualFs > 14) actualFs -= 1;
    int tw2 = textW(label, actualFs);
    Color tc = hovered ? TEXT_PRIMARY : Fade(TEXT_PRIMARY, 0.80f);
    // Shadow
    DrawTextEx(FONT_REG, label,
               {r.x + r.width/2 - tw2/2 + 1, r.y + r.height/2 - actualFs/2 + 1},
               actualFs, 1.5f, Fade(BLACK, 0.6f));
    DrawTextEx(FONT_REG, label,
               {r.x + r.width/2 - tw2/2, r.y + r.height/2 - actualFs/2},
               actualFs, 1.5f, tc);
}

// ── Blinking text ─────────────────────────────────────────
inline void drawBlink(const char* text, int cx, int y, float fs,
                       Color col, float time, float rate = 1.4f) {
    if (sinf(time * rate * 3.14159f) > 0.0f)
        drawTextC(text, cx, y, fs, col);
}

// ── Progress bar (clean, rounded) ─────────────────────────
inline void drawBar(int x, int y, int w, int h,
                     float progress, Color fill, Color bg) {
    DrawRectangleRounded({(float)x,(float)y,(float)w,(float)h}, 0.5f, 4, bg);
    if (progress > 0.01f)
        DrawRectangleRounded({(float)x,(float)y,(float)(int)(w*progress),(float)h},
                              0.5f, 4, fill);
}

// ── Word-wrap draw ────────────────────────────────────────
inline void drawWrapped(const char* text, int x, int y,
                         int maxW, float fs, Color col, int maxH,
                         float spacing = 1.5f) {
    std::string full(text);
    int lineH    = (int)(fs + 6);
    int maxLines = maxH / lineH;
    if (maxLines < 1) maxLines = 1;

    std::vector<std::string> words;
    std::string cur;
    for (char c : full) {
        if (c == ' ' || c == '\n') {
            if (!cur.empty()) { words.push_back(cur); cur.clear(); }
        } else cur += c;
    }
    if (!cur.empty()) words.push_back(cur);

    std::vector<std::string> lines;
    std::string line;
    for (auto& w : words) {
        std::string cand = line.empty() ? w : (line + " " + w);
        if ((int)MeasureTextEx(FONT_REG, cand.c_str(), fs, spacing).x <= maxW)
            line = cand;
        else { if (!line.empty()) lines.push_back(line); line = w; }
    }
    if (!line.empty()) lines.push_back(line);

    int count  = (int)lines.size() < maxLines ? (int)lines.size() : maxLines;
    int totalH = count * lineH;
    int sy     = y + (maxH - totalH) / 2;
    if (sy < y) sy = y;
    for (int i = 0; i < count; i++)
        DrawTextEx(FONT_REG, lines[i].c_str(),
                   {(float)x, (float)(sy + i * lineH)}, fs, spacing, col);
}

// ── Score pill ────────────────────────────────────────────
inline void drawScorePill(int x, int y, int w, int h,
                           const char* who, int score, int streak,
                           Color accent, bool active) {
    Color bg = active ? Fade(accent, 0.16f) : Fade(WHITE, 0.05f);
    Color bd = active ? Fade(accent, 0.65f) : Fade(WHITE, 0.10f);
    DrawRectangleRounded({(float)x,(float)y,(float)w,(float)h}, 0.18f, 4, bg);
    DrawRectangleRoundedLines({(float)x,(float)y,(float)w,(float)h}, 0.18f, 4, bd);
    if (active)
        DrawRectangleRounded({(float)x+2,(float)y+2,(float)w-4, 3},
                              0.5f, 2, Fade(accent, 0.7f));

    drawText(who,   x+12, y+8,  13, Fade(accent, 0.85f));

    char sc[24]; snprintf(sc, sizeof(sc), "%d", score);
    drawText(sc, x+12, y+26, 20, TEXT_PRIMARY);
    drawText("PTS", x + textW(sc, 20) + 18, y+32, 13, TEXT_DIM);

    if (streak >= 2) {
        char stk[16]; snprintf(stk, sizeof(stk), "x%d", streak);
        int sw = textW(stk, 15);
        drawText(stk, x + w - sw - 10, y+28, 15, ACCENT_GOLD);
    }
}

} // namespace UIStyle