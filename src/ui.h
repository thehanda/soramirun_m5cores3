#pragma once
#include <M5CoreS3.h>

constexpr uint16_t COL_BG     = TFT_BLACK;
constexpr uint16_t COL_STS_BG = 0x1082;
constexpr uint16_t COL_BLUE   = 0x035F; // 明るい青
constexpr uint16_t COL_RED    = 0x8000;
constexpr uint16_t COL_GREEN  = 0x3186;
constexpr uint16_t COL_ORANGE = 0xC4A0;
constexpr uint16_t COL_GRAY   = 0x4208;
constexpr uint16_t COL_CYAN   = 0x0451;
constexpr uint16_t COL_PURPLE = 0x6014;
constexpr uint16_t COL_BLACK  = 0x0861;

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

static void drawBtnJP(const Rect& r, uint16_t bg, const char* jp,
                      const char* sub = nullptr) {
    auto& d = CoreS3.Display;
    d.fillRoundRect(r.x, r.y, r.w, r.h, 8, bg);
    d.drawRoundRect(r.x, r.y, r.w, r.h, 8, 0x4A69);
    d.setTextColor(TFT_WHITE);
    d.setTextDatum(MC_DATUM);
    d.setFont(&fonts::efontJA_16);
    d.setTextSize(1);
    if (sub) {
        d.drawString(jp, r.x+r.w/2, r.y+r.h/2-10);
        d.setFont(nullptr);
        d.setTextSize(1);
        d.drawString(sub, r.x+r.w/2, r.y+r.h/2+10);
    } else {
        d.drawString(jp, r.x+r.w/2, r.y+r.h/2);
    }
    d.setFont(nullptr);
}

// 音量レベル: 0=大, 1=中, 2=小
extern uint8_t g_volumeLevel;
const uint8_t VOL_VALUES[] = {200, 120, 60};
const char*   VOL_LABELS[] = {"HI", "MD", "LO"};

inline void ui_setStatus(const char* msg, uint16_t col = TFT_WHITE) {
    auto& d = CoreS3.Display;
    d.fillRect(0, 0, 320, 36, COL_STS_BG);
    d.setTextColor(col);
    d.setTextDatum(ML_DATUM);
    d.setTextSize(2);
    d.drawString(msg, 8, 18);
}

// 画面1右上の音量ボタンを描画
inline void drawVolBtn() {
    auto& d = CoreS3.Display;
    d.fillRoundRect(220, 4, 96, 76, 8, d.color565(30, 80, 160));
    d.drawRoundRect(220, 4, 96, 76, 8, 0x4A69);
    d.setTextColor(TFT_WHITE);
    d.setTextDatum(MC_DATUM);
    d.setTextSize(1);
    d.drawString("VOL", 268, 22);
    d.setTextSize(3);
    d.drawString(VOL_LABELS[g_volumeLevel], 268, 52);
}

// ── Screen 1 ──────────────────────────────────────────
// 下段: はじめに(緑) ユーザーネーム(オレンジ) Next(青)
constexpr Rect S1_HAJIME  = {   4, 158,  96, 76 };
constexpr Rect S1_USERNAME= { 112, 158,  96, 76 };
constexpr Rect S1_NEXT    = { 220, 158,  96, 76 };
constexpr Rect S1_VOL     = { 220,   4,  96, 76 }; // 右上音量ボタン

enum class Btn1 { NONE, HAJIME, USERNAME, NEXT, VOLUME };

inline Btn1 hitScreen1(int16_t x, int16_t y) {
    auto h = [&](const Rect& r) {
        return x>=r.x && x<r.x+r.w && y>=r.y && y<r.y+r.h;
    };
    if (h(S1_VOL))      return Btn1::VOLUME;
    if (h(S1_HAJIME))   return Btn1::HAJIME;
    if (h(S1_USERNAME)) return Btn1::USERNAME;
    if (h(S1_NEXT))     return Btn1::NEXT;
    return Btn1::NONE;
}

// ── Screen 2 ──────────────────────────────────────────
// 上段: Task1(灰) Task2(灰) Task3(灰)
// 下段: Option(紫) Next(青) End(赤)
constexpr Rect S2_TASK1  = {   4,  38, 100, 90 };
constexpr Rect S2_TASK2  = { 110,  38, 100, 90 };
constexpr Rect S2_TASK3  = { 216,  38, 100, 90 };
constexpr Rect S2_OPTION = {   4, 134, 100, 90 };
constexpr Rect S2_NEXT   = { 110, 134, 100, 90 };
constexpr Rect S2_END    = { 216, 134, 100, 90 };

enum class Btn2 { NONE, TASK1, TASK2, TASK3, OPTION, NEXT, END };

inline void drawScreen2() {
    CoreS3.Display.fillScreen(COL_BG);
    ui_setStatus(" Soramirun", TFT_WHITE);
    drawBtn(S2_TASK1,  COL_GRAY,   "Task 1", "(^-^)");
    drawBtn(S2_TASK2,  COL_GRAY,   "Task 2", "(^-^)");
    drawBtn(S2_TASK3,  COL_GRAY,   "Task 3", "(^-^)");
    drawBtn(S2_OPTION, COL_PURPLE, "Option", "(o^^o)");
    drawBtn(S2_NEXT,   COL_BLUE,   "Next",   "(>>)");
    drawBtn(S2_END,    COL_RED,    "End",    "m(_ _)m");
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
    if (h(S2_END))    return Btn2::END;
    return Btn2::NONE;
}

// ── Screen 3 ──────────────────────────────────────────
// 上段: Task4(灰・左) Task5(灰・右)
// 下段: Back(黒)
constexpr Rect S3_TASK4 = {   4,  38, 154, 112 };
constexpr Rect S3_TASK5 = { 162,  38, 154, 112 };
constexpr Rect S3_BACK  = {   4, 156, 312,  78 };

enum class Btn3 { NONE, TASK4, TASK5, BACK };

inline void drawScreen3() {
    CoreS3.Display.fillScreen(COL_BG);
    ui_setStatus(" Soramirun", TFT_WHITE);
    drawBtn(S3_TASK4, COL_GRAY,  "Task 4", "(^-^)");
    drawBtn(S3_TASK5, COL_GRAY,  "Task 5", "(^-^)");
    drawBtn(S3_BACK,  COL_BLACK, "Back",   nullptr);
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

// ── Screen 5 ──────────────────────────────────────────
// 上段: Start REC(赤・左) 再生(緑・右)
// 下段: Save(オレンジ) やりなおし(紫) Back(黒)
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
    drawBtnJP(S5_PLAY, COL_GREEN,  "再生");
    drawBtn(S5_SAVE,   COL_ORANGE, "Save", "(^-^)");
    drawBtnJP(S5_YARI, COL_PURPLE, "やりなおし");
    drawBtn(S5_BACK,   COL_BLACK,  "Back", nullptr);
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
// 下段: Save(オレンジ) やりなおし(紫) Back(黒)
constexpr Rect S7_SAVE = {   4, 148, 100, 86 };
constexpr Rect S7_YARI = { 110, 148, 100, 86 };
constexpr Rect S7_BACK = { 216, 148, 100, 86 };

enum class Btn7 { NONE, SAVE, YARI, BACK };

inline void drawScreen7Buttons() {
    drawBtn(S7_SAVE,   COL_ORANGE, "Save",  "(^-^)");
    drawBtnJP(S7_YARI, COL_PURPLE, "やりなおし");
    drawBtn(S7_BACK,   COL_BLACK,  "Back",  nullptr);
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
