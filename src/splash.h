#pragma once
#include <M5CoreS3.h>

inline void drawSplash() {
    auto& d = CoreS3.Display;
    d.fillScreen(0x4DA8);
    d.fillRect(0, 0, 320, 40, d.color565(125, 207, 248));

    // ソラミルン（上部）
    d.setTextDatum(MC_DATUM);
    d.setTextColor(TFT_WHITE);
    d.setFont(&fonts::efontJA_16);
    d.setTextSize(2);
    d.drawString("ソラミルン", 160, 20);
    d.setFont(nullptr);

    // 顔（上に寄せて小さく）
    d.fillCircle(160, 90, 80, d.color565(254, 239, 200));

    uint16_t sun  = d.color565(247, 200, 32);
    uint16_t blk  = d.color565(58,  32,  0);
    uint16_t moon = d.color565(200, 223, 245);
    uint16_t face = d.color565(254, 239, 200);
    uint16_t navy = d.color565(58,  80,  128);
    uint16_t star = d.color565(245, 224, 48);

    // ほっぺ左：太陽
    d.fillRoundRect(63, 77, 6, 13, 3, sun);
    d.fillRoundRect(63, 107, 6, 13, 3, sun);
    d.fillRoundRect(46, 90, 13, 6, 3, sun);
    d.fillRoundRect(72, 90, 13, 6, 3, sun);
    d.fillRoundRect(50, 80, 9, 9, 3, sun);
    d.fillRoundRect(72, 80, 9, 9, 3, sun);
    d.fillRoundRect(50, 107, 9, 9, 3, sun);
    d.fillRoundRect(72, 107, 9, 9, 3, sun);
    d.fillCircle(67, 93, 10, sun);

    // ほっぺ右：太陽
    d.fillRoundRect(251, 77, 6, 13, 3, sun);
    d.fillRoundRect(251, 107, 6, 13, 3, sun);
    d.fillRoundRect(235, 90, 13, 6, 3, sun);
    d.fillRoundRect(258, 90, 13, 6, 3, sun);
    d.fillRoundRect(238, 80, 9, 9, 3, sun);
    d.fillRoundRect(258, 80, 9, 9, 3, sun);
    d.fillRoundRect(238, 107, 9, 9, 3, sun);
    d.fillRoundRect(258, 107, 9, 9, 3, sun);
    d.fillCircle(252, 93, 10, sun);

    // 左目：星
    d.fillTriangle(99,28, 116,42, 82,42, sun);
    d.fillTriangle(82,42, 116,42, 99,56, sun);
    d.fillTriangle(89,64, 109,64, 99,28, sun);
    d.fillCircle(99, 46, 12, sun);
    for (int i = 0; i <= 20; i++) {
        float t = i / 20.0f;
        float x = 90 + t * 18;
        float y = 49 - 12 * 4 * t * (1 - t);
        d.fillCircle((int)x, (int)y, 2, blk);
    }

    // 右目：上弦の月
    d.fillCircle(210, 55, 17, moon);
    d.fillCircle(200, 55, 17, face);
    for (int i = 0; i <= 20; i++) {
        float t = i / 20.0f;
        float x = 199 + t * 20;
        float y = 62 - 15 * 4 * t * (1 - t);
        d.fillCircle((int)x, (int)y, 2, navy);
    }
    d.fillRect(192, 41, 4, 4, star);
    d.fillRect(194, 39, 2, 8, star);
    d.fillRect(190, 43, 8, 2, star);

    // 口：丸い雲
    d.fillCircle(160, 132, 17, TFT_WHITE);

    // 下段3ボタン
    // はじめに
    d.fillRoundRect(4, 158, 96, 76, 8, d.color565(41, 69, 149));
    d.drawRoundRect(4, 158, 96, 76, 8, 0x4A69);
    d.setFont(&fonts::efontJA_16);
    d.setTextSize(1);
    d.setTextColor(TFT_WHITE);
    d.setTextDatum(MC_DATUM);
    d.drawString("はじめに", 52, 196);
    d.setFont(nullptr);

    // 名付け
    d.fillRoundRect(112, 158, 96, 76, 8, d.color565(41, 69, 149));
    d.drawRoundRect(112, 158, 96, 76, 8, 0x4A69);
    d.setFont(&fonts::efontJA_16);
    d.setTextSize(1);
    d.setTextColor(TFT_WHITE);
    d.setTextDatum(MC_DATUM);
    d.drawString("名付け", 160, 196);
    d.setFont(nullptr);

    // End
    d.fillRoundRect(220, 158, 96, 76, 8, d.color565(100, 10, 10));
    d.drawRoundRect(220, 158, 96, 76, 8, 0x4A69);
    d.setTextSize(2);
    d.setTextColor(TFT_WHITE);
    d.setTextDatum(MC_DATUM);
    d.drawString("End", 268, 185);
    d.setTextSize(1);
    d.drawString("m(_ _)m", 268, 210);
}
