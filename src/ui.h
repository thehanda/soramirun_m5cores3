#pragma once
#include <M5CoreS3.h>

constexpr uint16_t COL_BG     = TFT_BLACK;
constexpr uint16_t COL_STS_BG = 0x1082;
constexpr uint16_t COL_BLUE   = 0x2945;
constexpr uint16_t COL_RED    = 0x8000;
constexpr uint16_t COL_GREEN  = 0x3186;
constexpr uint16_t COL_ORANGE = 0xC4A0;
constexpr uint16_t COL_GRAY   = 0x4208;
constexpr uint16_t COL_CYAN   = 0x0451;
constexpr uint16_t COL_PURPLE = 0x6014;

struct Rect { int16_t x, y, w, h; };

static void drawBtn(const Rect& r, uint16_t bg,
                    const char* line1, const char* line2 = nullptr,
                    uint8_t sz1 = 2, uint8_t sz2 = 2) {
    auto& d = CoreS3.Display;
    d.fillRoundRect(r.x, r.y, r.w, r.h, 8, bg);
    d.drawRoundRect(r.x, r.y, r.w, r.h, 8, 0x4A69);
    d.setTextColor(TFT_WHITE);
    d.setTextDatum(MC_DATUM);
    if (line2) {
        d.setTextSize(sz1);
        d.drawString(line1, r.x+r.w/2, r.y+r.h/2-12);
        d.setTextSize(sz2);
        d.drawString(line2, r.x+r.w/2, r.y+r.h/2+12);
    } else {
        d.setTextSize(sz1);
        d.drawString(line1, r.x+r.w/2, r.y+r.h/2);
    }
}

static void drawBtnJP(const Rect& r, uint16_t bg, const char* jp) {
    auto& d = CoreS3.Display;
    d.fillRoundRect(r.x, r.y, r.w, r.h, 8, bg);
    d.drawRoundRect(r.x, r.y, r.w, r.h, 8, 0x4A69);
    d.setTextColor(TFT_WHITE);
    d.setTextDatum(MC_DATUM);
    d.setFont(&fonts::efontJA_16);
    d.setTextSize(1);
    d.drawString(jp, r.x+r.w/2, r.y+r.h/2);
    d.setFont(nullptr);
}

inline void ui_setStatus(const char* msg, uint16_t col = TFT_WHITE) {
    auto& d = CoreS3.Display;
    d.fillRect(0, 0, 320, 36, COL_STS_BG);
    d.setTextColor(col);
    d.setTextDatum(ML_DATUM);
    d.setTextSize(2);
    d.drawString(msg, 8, 18);
}

// ── Screen 1: ボタン判定のみ（描画はsplash.hで） ─────
constexpr Rect S1_HAJIME = {   4, 158,  96, 76 };
constexpr Rect S1_NAZUKE = { 112, 158,  96, 76 };
constexpr Rect S1_END    = { 220, 158,  96, 76 };

enum class Btn1 { NONE, HAJIME, NAZUKE, END };

inline Btn1 hitScreen1(int16_t x, int16_t y) {
    auto h = [&](const Rect& r) {
        return x>=r.x && x<r.x+r.w && y>=r.y && y<r.y+r.h;
    };
    if (h(S1_HAJIME)) return Btn1::HAJIME;
    if (h(S1_NAZUKE)) return Btn1::NAZUKE;
    if (h(S1_END))    return Btn1::END;
    return Btn1::NONE;
}

// ── Screen 2 ──────────────────────────────────────────
// 上段: Task1 Task2 Task3
// 下段: Option Next Back
constexpr Rect S2_TASK1  = {   4,  38, 100, 90 };
constexpr Rect S2_TASK2  = { 110,  38, 100, 90 };
constexpr Rect S2_TASK3  = { 216,  38, 100, 90 };
constexpr Rect S2_OPTION = {   4, 134, 100, 90 };
constexpr Rect S2_NEXT   = { 110, 134, 100, 90 };
constexpr Rect S2_BACK   = { 216, 134, 100, 90 };

enum class Btn2 { NONE, TASK1, TASK2, TASK3, OPTION, NEXT, BACK };

inline void drawScreen2() {
    CoreS3.Display.fillScreen(COL_BG);
    ui_setStatus(" Soramirun", TFT_WHITE);
    drawBtn(S2_TASK1,  COL_BLUE,   "Task 1", "(^-^)");
    drawBtn(S2_TASK2,  COL_BLUE,   "Task 2", "(^-^)");
    drawBtn(S2_TASK3,  COL_BLUE,   "Task 3", "(^-^)");
    drawBtn(S2_OPTION, COL_PURPLE, "Option", "(o^^o)");
    drawBtn(S2_NEXT,   COL_GREEN,  "Next",   "(>>)");
    drawBtn(S2_BACK,   COL_GRAY,   "Back",   nullptr);
}

inline Btn2 hitScreen2(int16_t x, int16_t y) {
    auto h = [&](const Rect& r) {
        return x>=r.x && x<r.x+r.w && y>=r.y && y<r.y+r.h;
    };
    if (h(S2_TASK1))  return Btn2::TASK1;
    if (h(S2_TASK2))  return Btn2::TASK2;
    if (h(S2_TASK3))  return Btn2::TASK3;
    if (h(S2_OPTION)) return Btn2::OPTION;
    if (h(S2_NEXT))   return Btn2::NEXT;
    if (h(S2_BACK))   return Btn2::BACK;
    return Btn2::NONE;
}

// ── Screen 3 ──────────────────────────────────────────
// 上段: Task4(左) Task5(右)
// 下段: Back
constexpr Rect S3_TASK4 = {   4,  38, 154, 112 };
constexpr Rect S3_TASK5 = { 162,  38, 154, 112 };
constexpr Rect S3_BACK  = {   4, 156, 312,  78 };

enum class Btn3 { NONE, TASK4, TASK5, BACK };

inline void drawScreen3() {
    CoreS3.Display.fillScreen(COL_BG);
    ui_setStatus(" Soramirun", TFT_WHITE);
    drawBtn(S3_TASK4, COL_BLUE, "Task 4", "(^-^)");
    drawBtn(S3_TASK5, COL_BLUE, "Task 5", "(^-^)");
    drawBtn(S3_BACK,  COL_GRAY, "Back",   nullptr);
}

inline Btn3 hitScreen3(int16_t x, int16_t y) {
    auto h = [&](const Rect& r) {
        return x>=r.x && x<r.x+r.w && y>=r.y && y<r.y+r.h;
    };
    if (h(S3_TASK4)) return Btn3::TASK4;
    if (h(S3_TASK5)) return Btn3::TASK5;
    if (h(S3_BACK))  return Btn3::BACK;
    return Btn3::NONE;
}

// ── Screen 4 ──────────────────────────────────────────
// 上段: Record(左) Camera(右)
// 下段: Back
constexpr Rect S4_RECORD = {   4,  38, 154, 112 };
constexpr Rect S4_CAMERA = { 162,  38, 154, 112 };
constexpr Rect S4_BACK   = {   4, 156, 312,  78 };

enum class Btn4 { NONE, RECORD, CAMERA, BACK };

inline void drawScreen4() {
    CoreS3.Display.fillScreen(COL_BG);
    ui_setStatus(" Report", TFT_WHITE);
    drawBtn(S4_RECORD, COL_RED,  "Record", "(^o^)");
    drawBtn(S4_CAMERA, COL_CYAN, "Camera", "[^]");
    drawBtn(S4_BACK,   COL_GRAY, "Back",   nullptr);
}

inline Btn4 hitScreen4(int16_t x, int16_t y) {
    auto h = [&](const Rect& r) {
        return x>=r.x && x<r.x+r.w && y>=r.y && y<r.y+r.h;
    };
    if (h(S4_RECORD)) return Btn4::RECORD;
    if (h(S4_CAMERA)) return Btn4::CAMERA;
    if (h(S4_BACK))   return Btn4::BACK;
    return Btn4::NONE;
}

// ── Screen 5 ──────────────────────────────────────────
// 上段: Start REC(左) 再生(右)
// 下段: Save やりなおし Back
constexpr Rect S5_STARTREC = {   4,  38, 154,  90 };
constexpr Rect S5_PLAY     = { 162,  38, 154,  90 };
constexpr Rect S5_SAVE     = {   4, 134, 100,  90 };
constexpr Rect S5_YARI     = { 110, 134, 100,  90 };
constexpr Rect S5_BACK     = { 216, 134, 100,  90 };

enum class Btn5 { NONE, STARTREC, PLAY, SAVE, YARI, BACK };

inline void drawScreen5(bool isRecording) {
    auto& d = CoreS3.Display;
    d.fillScreen(COL_BG);
    ui_setStatus(isRecording ? " REC..." : " Record",
                 isRecording ? TFT_RED : TFT_WHITE);
    if (isRecording) {
        drawBtn(S5_STARTREC, COL_ORANGE, "Stop",      "(>_<)", 2, 2);
    } else {
        drawBtn(S5_STARTREC, COL_RED,    "Start REC", "(^o^)", 2, 2);
    }
    // 再生ボタン（日本語）
    drawBtnJP(S5_PLAY, COL_CYAN, "再生");
    drawBtn(S5_SAVE, COL_GREEN, "Save", "(^-^)");
    // やりなおしボタン
    drawBtnJP(S5_YARI, COL_BLUE, "やりなおし");
    drawBtn(S5_BACK, COL_GRAY, "Back", nullptr);
}

inline void updateRecStatus5(bool isRecording) {
    ui_setStatus(isRecording ? " REC..." : " Stopped",
                 isRecording ? TFT_RED : TFT_WHITE);
    if (isRecording) {
        drawBtn(S5_STARTREC, COL_ORANGE, "Stop",      "(>_<)", 2, 2);
    } else {
        drawBtn(S5_STARTREC, COL_RED,    "Start REC", "(^o^)", 2, 2);
    }
}

inline Btn5 hitScreen5(int16_t x, int16_t y) {
    auto h = [&](const Rect& r) {
        return x>=r.x && x<r.x+r.w && y>=r.y && y<r.y+r.h;
    };
    if (h(S5_STARTREC)) return Btn5::STARTREC;
    if (h(S5_PLAY))     return Btn5::PLAY;
    if (h(S5_SAVE))     return Btn5::SAVE;
    if (h(S5_YARI))     return Btn5::YARI;
    if (h(S5_BACK))     return Btn5::BACK;
    return Btn5::NONE;
}

// ── Screen 7 ──────────────────────────────────────────
constexpr Rect S7_SAVE = {   4, 148, 100, 86 };
constexpr Rect S7_YARI = { 110, 148, 100, 86 };
constexpr Rect S7_BACK = { 216, 148, 100, 86 };

enum class Btn7 { NONE, SAVE, YARI, BACK };

inline void drawScreen7Buttons() {
    drawBtn(S7_SAVE, COL_GREEN, "Save", "(^-^)");
    drawBtnJP(S7_YARI, COL_BLUE, "やりなおし");
    drawBtn(S7_BACK, COL_GRAY,  "Back", nullptr);
}

inline Btn7 hitScreen7(int16_t x, int16_t y) {
    auto h = [&](const Rect& r) {
        return x>=r.x && x<r.x+r.w && y>=r.y && y<r.y+r.h;
    };
    if (h(S7_SAVE)) return Btn7::SAVE;
    if (h(S7_YARI)) return Btn7::YARI;
    if (h(S7_BACK)) return Btn7::BACK;
    return Btn7::NONE;
}
