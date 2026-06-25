#pragma once
#include <Arduino.h>
#include <SD.h>
#include <M5CoreS3.h>
#include "utility/GC0308.h"
#include "img_converters.h"  // fmt2jpg用

static GC0308    s_cam;
static bool      s_camInited = false;
static uint16_t* s_rgbBuf    = nullptr;
static size_t    s_rgbSize   = 0;

// RGB565バイトスワップ（赤青反転を修正）
static void swapBytes(uint16_t* buf, size_t count) {
    for (size_t i = 0; i < count; i++) {
        buf[i] = (buf[i] >> 8) | (buf[i] << 8);
    }
}

inline bool camera_init() {
    if (s_camInited) return true;
    bool ok = s_cam.begin();
    Serial.printf("[Cam] begin=%d\n", ok);
    s_camInited = ok;
    return ok;
}

// 画面5: フルスクリーンプレビュー（バイトスワップなし）
inline bool previewFrameFull() {
    if (!camera_init()) return false;
    if (!s_cam.get()) return false;

    // プレビューはそのまま表示（スワップしない）
    CoreS3.Display.pushImage(0, 0, 320, 210, (uint16_t*)s_cam.fb->buf);
    s_cam.free();

    CoreS3.Display.fillRect(0, 210, 320, 30, 0x1082);
    CoreS3.Display.setTextColor(TFT_WHITE);
    CoreS3.Display.setTextDatum(MC_DATUM);
    CoreS3.Display.setTextSize(2);
    CoreS3.Display.drawString("Touch to shoot!", 160, 225);
    return true;
}

// 撮影してPSRAMに保持
inline bool capturePhoto() {
    if (!camera_init()) return false;
    for (int i = 0; i < 3; i++) {
        if (s_cam.get()) s_cam.free();
        delay(100);
    }
    if (!s_cam.get()) { Serial.println("[Cam] get failed"); return false; }

    if (s_rgbBuf) { free(s_rgbBuf); s_rgbBuf = nullptr; s_rgbSize = 0; }

    s_rgbSize = s_cam.fb->len;
    s_rgbBuf  = (uint16_t*)ps_malloc(s_rgbSize);
    if (!s_rgbBuf) { s_cam.free(); return false; }

    // スワップせずそのままコピー（表示はそのまま、保存時にスワップ）
    memcpy(s_rgbBuf, s_cam.fb->buf, s_rgbSize);
    s_cam.free();

    Serial.printf("[Cam] Captured %d bytes\n", s_rgbSize);
    return true;
}

// 画面6: 撮影した写真を上部144pxに表示
inline void showPhotoScreen7() {
    if (!s_rgbBuf || s_rgbSize == 0) return;
    CoreS3.Display.fillScreen(TFT_BLACK);
    CoreS3.Display.pushImage(0, 0, 320, 144, s_rgbBuf);
}

// JPEGとしてSDカードに保存
inline bool savePhotoAs(const char* path) {
    if (!s_rgbBuf || s_rgbSize == 0) return false;

    // フォルダ作成
    char dir[60];
    strncpy(dir, path, sizeof(dir));
    char* slash = strrchr(dir, '/');
    if (slash && slash != dir) {
        *slash = '\0';
        if (!SD.exists(dir)) SD.mkdir(dir);
    }

    // RGB565 → JPEG変換（スワップなし）
    uint8_t* jpegBuf = nullptr;
    size_t   jpegLen = 0;
    bool ok = fmt2jpg((uint8_t*)s_rgbBuf, s_rgbSize,
                      320, 240, PIXFORMAT_RGB565, 80,
                      &jpegBuf, &jpegLen);
    if (!ok || !jpegBuf) {
        Serial.println("[Cam] JPEG convert failed");
        return false;
    }

    File f = SD.open(path, FILE_WRITE);
    if (!f) {
        free(jpegBuf);
        Serial.printf("[Cam] Cannot open: %s\n", path);
        return false;
    }
    f.write(jpegBuf, jpegLen);
    f.close();
    free(jpegBuf);

    free(s_rgbBuf); s_rgbBuf = nullptr; s_rgbSize = 0;
    Serial.printf("[Cam] Saved JPEG: %s (%d bytes)\n", path, jpegLen);
    return true;
}

inline void discardPhoto() {
    if (s_rgbBuf) { free(s_rgbBuf); s_rgbBuf = nullptr; s_rgbSize = 0; }
}

inline void drawScreen7Buttons();  // ui.hで定義
