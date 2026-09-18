#pragma once
#include <M5CoreS3.h>
#include "ui.h"

inline void drawSplash() {
    auto& d = CoreS3.Display;

    // 背景：クールなミント・ラベンダー
    d.fillScreen(d.color565(190, 225, 255));
    d.fillRect(0, 0, 320, 40, d.color565(50, 110, 200));

    uint16_t skin  = d.color565(215, 235, 255);
    uint16_t eyeC  = d.color565(40,  70, 140);
    uint16_t chk   = d.color565(150, 210, 255);
    uint16_t smile = d.color565(60,  90, 170);
    uint16_t drop  = d.color565(100, 170, 240);

    // 顔（やや青みがかった肌）
    d.fillCircle(160, 95, 78, skin);

    // 少し震えるイメージ：輪郭をわずかにずらした二重円
    d.drawCircle(160, 95, 78, d.color565(170, 200, 240));
    d.drawCircle(161, 96, 76, d.color565(170, 200, 240));

    // ほっぺ：水色の柔らかい丸
    d.fillCircle(92,  112, 20, chk);
    d.fillCircle(228, 112, 20, chk);

    // 左目：三日月（閉じた目）
    d.fillCircle(118, 72, 16, eyeC);
    d.fillCircle(118, 82, 15, skin);

    // 右目：三日月（閉じた目）
    d.fillCircle(202, 72, 16, eyeC);
    d.fillCircle(202, 82, 15, skin);

    // 瞳の線（細いまつ毛ライン）
    for (int i = 0; i <= 16; i++) {
        float t = i / 16.0f;
        float x = 108 + t * 20;
        float y = 72 - 5 * 4 * t * (1 - t);
        d.fillCircle((int)x, (int)y, 1, eyeC);
    }
    for (int i = 0; i <= 16; i++) {
        float t = i / 16.0f;
        float x = 192 + t * 20;
        float y = 72 - 5 * 4 * t * (1 - t);
        d.fillCircle((int)x, (int)y, 1, eyeC);
    }

    // 口：小さな優しいほほえみ
    for (int i = 0; i <= 16; i++) {
        float t = i / 16.0f;
        float x = 143 + t * 34;
        float y = 128 + 8 * 4 * t * (1 - t);
        d.fillCircle((int)x, (int)y, 2, smile);
    }

    // 装飾：しずく型（縦長楕円 + 三角の組み合わせ）
    auto drawDrop = [&](int cx, int cy, uint16_t col) {
        d.fillCircle(cx, cy, 6, col);
        d.fillTriangle(cx-5, cy+1, cx+5, cy+1, cx, cy+14, col);
    };
    drawDrop(45,  60, drop);
    drawDrop(38, 118, drop);
    drawDrop(270,  60, drop);
    drawDrop(276, 118, drop);

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
