#include <Arduino.h>
#include <M5CoreS3.h>
#include <SD.h>
#include <time.h>
#include "splash.h"
#include "ui.h"
#include "audio_player.h"
#include "recorder.h"
#include "camera_utils.h"

enum class State {
    SCREEN1,
    SCREEN2,
    SCREEN3,
    SCREEN5_READY,
    SCREEN5_REC,
    SCREEN6,
    SCREEN7,
};

State    g_state     = State::SCREEN1;
char     g_lastTask[20] = "task1";
uint32_t g_previewMs = 0;
uint8_t  g_volumeLevel = 0; // 0=大, 1=中, 2=小

static void buildFilename(char* buf, size_t bufSize,
                          const char* taskName, const char* ext) {
    const char* folder = (strcmp(ext,"jpg")==0) ? "photos" : "recordings";
    auto dt = CoreS3.Rtc.getDateTime();
    snprintf(buf, bufSize, "/%s/%s_%04d%02d%02d_%02d%02d%02d.%s",
             folder, taskName,
             dt.date.year, dt.date.month, dt.date.date,
             dt.time.hours, dt.time.minutes, dt.time.seconds,
             ext);
}

static bool initSD() {
    if (!SD.begin(GPIO_NUM_4)) {
        CoreS3.Display.fillScreen(TFT_BLACK);
        CoreS3.Display.setTextColor(TFT_RED);
        CoreS3.Display.setTextDatum(MC_DATUM);
        CoreS3.Display.setTextSize(2);
        CoreS3.Display.drawString("SD Error!", 160, 120);
        return false;
    }
    if (!SD.exists("/recordings")) SD.mkdir("/recordings");
    if (!SD.exists("/photos"))     SD.mkdir("/photos");
    return true;
}

static void applyVolume() {
    extern bool s_speakerInited;
    s_speakerInited = false;
    CoreS3.Speaker.end();
    ensureSpeaker();
}

void setup() {
    auto cfg = M5.config();
    CoreS3.begin(cfg);
    Serial.begin(115200);
    delay(2000);
    Serial.println("=== Soramirun Start ===");
    CoreS3.Display.setRotation(1);
    if (!initSD()) { while (true) delay(1000); }

    drawSplash();

    // RTCの時刻確認・必要なら更新
    const char* months[] = {"Jan","Feb","Mar","Apr","May","Jun",
                            "Jul","Aug","Sep","Oct","Nov","Dec"};
    char mon[4]; int day, year, hour, min, sec;
    sscanf(__DATE__, "%s %d %d", mon, &day, &year);
    sscanf(__TIME__, "%d:%d:%d", &hour, &min, &sec);
    auto cur = CoreS3.Rtc.getDateTime();
    if (cur.date.year < 2020 || cur.date.year < year) {
        m5::rtc_datetime_t dt;
        dt.date.year = year; dt.date.month = 1; dt.date.date = day;
        for (int i = 0; i < 12; i++) {
            if (strncmp(mon, months[i], 3) == 0) { dt.date.month = i+1; break; }
        }
        dt.time.hours = hour; dt.time.minutes = min; dt.time.seconds = sec;
        CoreS3.Rtc.setDateTime(dt);
        Serial.printf("[RTC] Set: %04d/%02d/%02d %02d:%02d:%02d\n",
                      year, dt.date.month, day, hour, min, sec);
    }

    g_state = State::SCREEN1;
}

void loop() {
    CoreS3.update();

    // 画面6: カメラプレビュー
    if (g_state == State::SCREEN6) {
        if (millis() - g_previewMs >= 1000) {
            previewFrameFull();
            g_previewMs = millis();
        }
        if (CoreS3.Touch.getCount() > 0) {
            auto t = CoreS3.Touch.getDetail();
            if (t.wasPressed()) {
                if (capturePhoto()) {
                    CoreS3.Display.fillScreen(TFT_BLACK);
                    showPhotoScreen7();
                    drawScreen7Buttons();
                    g_state = State::SCREEN7;
                } else {
                    drawScreen2();
                    ui_setStatus(" Cam Error", TFT_RED);
                    delay(1500);
                    ui_setStatus(" Soramirun", TFT_WHITE);
                    g_state = State::SCREEN2;
                }
            }
        }
        return;
    }

    // タッチ読み取り
    if (CoreS3.Touch.getCount() == 0) {
        if (g_state == State::SCREEN5_REC) rec_write_chunk();
        delay(5);
        return;
    }
    auto t = CoreS3.Touch.getDetail();
    if (!t.wasPressed()) {
        if (g_state == State::SCREEN5_REC) rec_write_chunk();
        delay(5);
        return;
    }

    // 画面1: スプラッシュ
    if (g_state == State::SCREEN1) {
        Btn1 btn = hitScreen1(t.x, t.y);
        switch (btn) {
        case Btn1::HAJIME:
            ensureSpeaker();
            playMp3("/audio/startup2.wav");
            drawSplash();
            break;
        case Btn1::USERNAME:
            strncpy(g_lastTask, "name", sizeof(g_lastTask));
            drawScreen5(false);
            g_state = State::SCREEN5_READY;
            break;
        case Btn1::NEXT:
            drawScreen2();
            g_state = State::SCREEN2;
            break;
        case Btn1::VOLUME:
            g_volumeLevel = (g_volumeLevel + 1) % 3;
            CoreS3.Speaker.setVolume(VOL_VALUES[g_volumeLevel]);
            // 音量ボタンを再描画
            drawVolBtn();
            break;
        default: break;
        }
        return;
    }

    // 画面2: メインメニュー
    if (g_state == State::SCREEN2) {
        Btn2 btn = hitScreen2(t.x, t.y);
        switch (btn) {
        case Btn2::TASK1:
            strncpy(g_lastTask, "task1", sizeof(g_lastTask));
            ui_setStatus(" Task 1...", TFT_YELLOW);
            playMp3("/audio/task1.wav");
            if (!camera_init()) {
                ui_setStatus(" Cam Error", TFT_RED);
                delay(1500);
                drawScreen2();
                ui_setStatus(" Soramirun", TFT_WHITE);
                break;
            }
            CoreS3.Display.fillScreen(TFT_BLACK);
            g_previewMs = 0;
            g_state = State::SCREEN6;
            break;
        case Btn2::TASK2:
            strncpy(g_lastTask, "task2", sizeof(g_lastTask));
            ui_setStatus(" Task 2...", TFT_YELLOW);
            playMp3("/audio/task2.wav");
            drawScreen5(false);
            g_state = State::SCREEN5_READY;
            break;
        case Btn2::TASK3:
            strncpy(g_lastTask, "task3", sizeof(g_lastTask));
            ui_setStatus(" Task 3...", TFT_YELLOW);
            playMp3("/audio/task3.wav");
            drawScreen5(false);
            g_state = State::SCREEN5_READY;
            break;
        case Btn2::OPTION: {
            uint8_t idx = (esp_random() % 5) + 1;
            char optLabel = 'A' + (idx - 1);
            snprintf(g_lastTask, sizeof(g_lastTask), "option%c", optLabel);
            char path[32];
            snprintf(path, sizeof(path), "/audio/option%d.wav", idx);
            ui_setStatus(" Option...", TFT_YELLOW);
            playMp3(path);
            drawScreen5(false);
            g_state = State::SCREEN5_READY;
            break;
        }
        case Btn2::NEXT:
            drawScreen3();
            g_state = State::SCREEN3;
            break;
        case Btn2::END:
            ui_setStatus(" Good bye...", TFT_YELLOW);
            ensureSpeaker();
            playMp3("/audio/end.wav");
            delay(500);
            CoreS3.Power.powerOff();
            break;
        default: break;
        }
        return;
    }

    // 画面3: Task4/Task5
    if (g_state == State::SCREEN3) {
        Btn3 btn = hitScreen3(t.x, t.y);
        switch (btn) {
        case Btn3::TASK4:
            strncpy(g_lastTask, "task4", sizeof(g_lastTask));
            ui_setStatus(" Task 4...", TFT_YELLOW);
            playMp3("/audio/task4.wav");
            ui_setStatus(" Soramirun", TFT_WHITE);
            // 画面3のまま
            break;
        case Btn3::TASK5:
            strncpy(g_lastTask, "task5", sizeof(g_lastTask));
            ui_setStatus(" Task 5...", TFT_YELLOW);
            playMp3("/audio/task5.wav");
            drawScreen5(false);
            g_state = State::SCREEN5_READY;
            break;
        case Btn3::BACK:
            drawScreen2();
            g_state = State::SCREEN2;
            break;
        default: break;
        }
        return;
    }

    // 画面5: 録音待機
    if (g_state == State::SCREEN5_READY) {
        Btn5 btn = hitScreen5(t.x, t.y);
        switch (btn) {
        case Btn5::STARTREC: {
            ui_setStatus(" Preparing...", TFT_YELLOW);
            char fname[60];
            buildFilename(fname, sizeof(fname), g_lastTask, "wav");
            strncpy(s_lastRecPath, fname, sizeof(s_lastRecPath));
            rec_start(fname);
            drawScreen5(true);
            g_state = State::SCREEN5_REC;
            break;
        }
        case Btn5::PLAY:
            ui_setStatus(" Playing...", TFT_CYAN);
            rec_play();
            ui_setStatus(" Stopped", TFT_WHITE);
            break;
        case Btn5::SAVE:
            ui_setStatus(" Saved! (^-^)", TFT_GREEN);
            delay(1500);
            updateRecStatus5(false);
            break;
        case Btn5::YARI: {
            ui_setStatus(" Preparing...", TFT_YELLOW);
            char fname[60];
            buildFilename(fname, sizeof(fname), g_lastTask, "wav");
            strncpy(s_lastRecPath, fname, sizeof(s_lastRecPath));
            rec_start(fname);
            drawScreen5(true);
            g_state = State::SCREEN5_REC;
            break;
        }
        case Btn5::BACK:
            drawScreen2();
            g_state = State::SCREEN2;
            break;
        default: break;
        }
        return;
    }

    // 画面5: 録音中
    if (g_state == State::SCREEN5_REC) {
        Btn5 btn = hitScreen5(t.x, t.y);
        if (btn == Btn5::STARTREC) {
            rec_stop();
            updateRecStatus5(false);
            g_state = State::SCREEN5_READY;
        }
        return;
    }

    // 画面7: 撮影確認
    if (g_state == State::SCREEN7) {
        Btn7 btn = hitScreen7(t.x, t.y);
        switch (btn) {
        case Btn7::SAVE: {
            char fname[60];
            buildFilename(fname, sizeof(fname), g_lastTask, "jpg");
            bool ok = savePhotoAs(fname);
            ensureSpeaker();
            CoreS3.Speaker.tone(ok ? 1200 : 600, 200);
            delay(300);
            drawScreen2();
            ui_setStatus(ok ? " Saved!(^-^)" : " Save Error",
                         ok ? TFT_GREEN : TFT_RED);
            delay(1000);
            ui_setStatus(" Soramirun", TFT_WHITE);
            g_state = State::SCREEN2;
            break;
        }
        case Btn7::YARI:
            discardPhoto();
            CoreS3.Display.fillScreen(TFT_BLACK);
            g_previewMs = 0;
            g_state = State::SCREEN6;
            break;
        case Btn7::BACK:
            discardPhoto();
            drawScreen2();
            g_state = State::SCREEN2;
            break;
        default: break;
        }
        return;
    }

    delay(5);
}
