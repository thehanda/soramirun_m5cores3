#pragma once
#include <M5CoreS3.h>
#include "ui.h"

inline void drawSplash() {
    auto& d = CoreS3.Display;

    // 背景：暖かい黄色
    d.fillScreen(d.color565(255, 240, 130));
    d.fillRect(0, 0, 320, 40, d.color565(220, 100, 20));

    uint16_t skin  = d.color565(255, 215, 160);
    uint16_t eyeC  = d.color565(50,  25,  0);
    uint16_t shine = TFT_WHITE;
    uint16_t chk   = d.color565(255, 155, 155);
    uint16_t smile = d.color565(180,  50,  50);
    uint16_t starC = d.color565(255, 200,  10);

    // 顔
    d.fillCircle(160, 95, 78, skin);

    // ほっぺ：ローズ色の丸
    d.fillCircle(92,  112, 22, chk);
    d.fillCircle(228, 112, 22, chk);

    // 左目：大きな丸目 + キラキラ
    d.fillCircle(115, 72, 20, eyeC);
    d.fillCircle(110, 66,  9, shine);
    d.fillCircle(119, 63,  4, shine);

    // 右目：大きな丸目 + キラキラ
    d.fillCircle(205, 72, 20, eyeC);
    d.fillCircle(200, 66,  9, shine);
    d.fillCircle(209, 63,  4, shine);

    // 眉毛（上がり眉・元気な表情）
    for (int i = 0; i <= 20; i++) {
        float t = i / 20.0f;
        float x = 100 + t * 30;
        float y = 48 - 8 * 4 * t * (1 - t);
        d.fillCircle((int)x, (int)y, 2, eyeC);
    }
    for (int i = 0; i <= 20; i++) {
        float t = i / 20.0f;
        float x = 190 + t * 30;
        float y = 48 - 8 * 4 * t * (1 - t);
        d.fillCircle((int)x, (int)y, 2, eyeC);
    }

    // 口：大きな笑顔アーク
    for (int i = 0; i <= 24; i++) {
        float t = i / 24.0f;
        float x = 130 + t * 60;
        float y = 130 + 18 * 4 * t * (1 - t);
        d.fillCircle((int)x, (int)y, 3, smile);
    }

    // 装飾：小さな星（4方向）
    auto drawStar4 = [&](int cx, int cy, int r) {
        d.fillRect(cx-1, cy-r, 3, 2*r+1, starC);
        d.fillRect(cx-r, cy-1, 2*r+1, 3, starC);
        d.fillCircle(cx, cy, r/2+1, starC);
    };
    drawStar4(42,  58, 9);
    drawStar4(36, 118, 6);
    drawStar4(272,  62, 9);
    drawStar4(278, 118, 6);

    // 機体名（左寄せ、ヘッダー内）
    d.setTextDatum(ML_DATUM);
    d.setTextColor(TFT_WHITE);
    d.setFont(&fonts::efontJA_16);
    d.setTextSize(2);
    d.drawString(DEVICE_NAME, 8, 20);
    d.setFont(nullptr);

    // 音量ボタン（右上）
    drawVolBtn();

    // 下段3ボタン
    drawBtnJP(S1_HAJIME,   d.color565(49, 134, 48),  "はじめに");
    drawBtnJP(S1_USERNAME, d.color565(196, 164, 0),  "ユーザー", "番号設定");
    drawBtn(S1_NEXT,       d.color565(41, 69, 149),  "Next", "(>>)");
}
