#include <Arduino.h>
#include <M5CoreS3.h>
#include <SD.h>
#include <time.h>
#include <WiFi.h>
#include "splash.h"
#include "ui.h"
#include "audio_player.h"
#include "recorder.h"
#include "camera_utils.h"

enum class State {
    SCREEN0_TENS,    // 参加者番号：十の位選択
    SCREEN0_ONES,    // 参加者番号：一の位選択
    SCREEN0_CONFIRM, // 参加者番号：確認
    SCREEN1,         // スプラッシュ
    SCREEN2,         // メインメニュー
    SCREEN3,         // Task4・Task5
    SCREEN5_READY,   // 録音待機
    SCREEN5_REC,     // 録音中
    SCREEN6,         // カメラプレビュー
    SCREEN7,         // 撮影確認
};

State    g_state     = State::SCREEN1;
char     g_lastTask[20] = "task1";
uint32_t g_previewMs = 0;
uint8_t  g_volumeLevel = 0; // 0=大, 1=中, 2=小
uint8_t  g_partTens = 0;
uint8_t  g_partOnes = 0;
uint32_t g_fileSeq = 0;

static void loadSeq() {
    File f = SD.open("/seq.txt", FILE_READ);
    if (f) {
        g_fileSeq = f.parseInt();
        f.close();
    }
}

static void saveSeq() {
    SD.remove("/seq.txt");
    File f = SD.open("/seq.txt", FILE_WRITE);
    if (f) {
        f.printf("%lu", (unsigned long)g_fileSeq);
        f.close();
    }
}

static void buildFilename(char* buf, size_t bufSize,
                          const char* taskName, const char* ext) {
    const char* folder = (strcmp(ext,"jpg")==0) ? "photos" : "recordings";
    g_fileSeq++;
    saveSeq();
    uint8_t pnum = g_partTens * 10 + g_partOnes;
    snprintf(buf, bufSize, "/%s/%04lu_p%02d_%s.%s",
             folder, g_fileSeq, pnum, taskName, ext);
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
    loadSeq();

    drawSplash();

    // RTCの時刻確認・必要なら更新
    const char* months[] = {"Jan","Feb","Mar","Apr","May","Jun",
                            "Jul","Aug","Sep","Oct","Nov","Dec"};
    char mon[4]; int day, year, hour, min, sec;
    sscanf(__DATE__, "%s %d %d", mon, &day, &year);
    sscanf(__TIME__, "%d:%d:%d", &hour, &min, &sec);
    // まずコンパイル時刻でRTCを設定（フォールバック）
    m5::rtc_datetime_t dt;
    dt.date.year = year; dt.date.month = 1; dt.date.date = day;
    for (int i = 0; i < 12; i++) {
        if (strncmp(mon, months[i], 3) == 0) { dt.date.month = i+1; break; }
    }
    dt.time.hours = hour; dt.time.minutes = min + 2; dt.time.seconds = sec;
    if (dt.time.minutes >= 60) { dt.time.hours++; dt.time.minutes -= 60; }
    CoreS3.Rtc.setDateTime(dt);
    Serial.printf("[RTC] Compile time: %04d/%02d/%02d %02d:%02d:%02d\n",
                  year, dt.date.month, day, dt.time.hours, dt.time.minutes, sec);

    // WiFiに接続できればNTPで正確な時刻に上書き
    Serial.println("[WiFi] Trying...");
    WiFi.begin("Buffalo-A-35C0", "6a5idibtfxdtk");
    uint8_t retry = 0;
    while (WiFi.status() != WL_CONNECTED && retry < 10) {
        delay(300); retry++;
    }
    if (WiFi.status() == WL_CONNECTED) {
        configTime(9 * 3600, 0, "ntp.nict.jp");
        struct tm ti;
        bool ntpOk = false;
        for (int i = 0; i < 20; i++) {
            if (getLocalTime(&ti) && ti.tm_year > 100) { ntpOk = true; break; }
            delay(300);
        }
        if (ntpOk) {
            m5::rtc_datetime_t ndt;
            ndt.date.year  = ti.tm_year + 1900;
            ndt.date.month = ti.tm_mon + 1;
            ndt.date.date  = ti.tm_mday;
            ndt.time.hours   = ti.tm_hour;
            ndt.time.minutes = ti.tm_min;
            ndt.time.seconds = ti.tm_sec;
            CoreS3.Rtc.setDateTime(ndt);
            Serial.printf("[RTC] NTP sync: %04d/%02d/%02d %02d:%02d:%02d\n",
                          ndt.date.year, ndt.date.month, ndt.date.date,
                          ndt.time.hours, ndt.time.minutes, ndt.time.seconds);
        }
        WiFi.disconnect(true);
        WiFi.mode(WIFI_OFF);
    } else {
        Serial.println("[WiFi] No WiFi - using compile time");
        WiFi.disconnect(true);
        WiFi.mode(WIFI_OFF);
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

    // 画面0: 参加者番号設定（十の位）
    if (g_state == State::SCREEN0_TENS) {
        if (CoreS3.Touch.getCount() > 0) {
            auto t0 = CoreS3.Touch.getDetail();
            if (t0.wasPressed()) {
                Serial.printf("[S0_TENS] touch x=%d y=%d\n", t0.x, t0.y);
                int hit = hitScreen0(t0.x, t0.y, NumStep::TENS);
                Serial.printf("[S0_TENS] hit=%d\n", hit);
                if (hit >= 0) {
                    g_partTens = hit;
                    g_state = State::SCREEN0_ONES;
                    drawScreen0(NumStep::ONES, g_partTens, 0);
                }
            }
        }
        return;
    }

    // 画面0: 参加者番号設定（一の位）
    if (g_state == State::SCREEN0_ONES) {
        if (CoreS3.Touch.getCount() > 0) {
            auto t0 = CoreS3.Touch.getDetail();
            if (t0.wasPressed()) {
                Serial.printf("[S0_ONES] touch x=%d y=%d\n", t0.x, t0.y);
                int hit = hitScreen0(t0.x, t0.y, NumStep::ONES);
                if (hit >= 0) {
                    g_partOnes = hit;
                    g_state = State::SCREEN0_CONFIRM;
                    drawScreen0(NumStep::CONFIRM, g_partTens, g_partOnes);
                }
            }
        }
        return;
    }

    // 画面0: 参加者番号確認
    if (g_state == State::SCREEN0_CONFIRM) {
        if (CoreS3.Touch.getCount() > 0) {
            auto t0 = CoreS3.Touch.getDetail();
            if (t0.wasPressed()) {
                int hit = hitScreen0(t0.x, t0.y, NumStep::CONFIRM);
                if (hit == -2) {
                    g_state = State::SCREEN0_TENS;
                    drawScreen0(NumStep::TENS, 0, 0);
                } else if (hit == -3) {
                    drawSplash();
                    g_state = State::SCREEN1;
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
            // ユーザー番号設定→画面0へ
            g_state = State::SCREEN0_TENS;
            drawScreen0(NumStep::TENS, 0, 0);
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
            // 一時ファイルに録音（Saveで正式保存）
            strncpy(s_lastRecPath, "/recordings/_tmp.wav", sizeof(s_lastRecPath));
            rec_start("/recordings/_tmp.wav");
            drawScreen5(true);
            g_state = State::SCREEN5_REC;
            break;
        }
        case Btn5::PLAY:
            ui_setStatus(" Playing...", TFT_CYAN);
            rec_play();
            ui_setStatus(" Stopped", TFT_WHITE);
            break;
        case Btn5::SAVE: {
            // 一時ファイルを正式ファイル名にリネーム
            char fname[60];
            buildFilename(fname, sizeof(fname), g_lastTask, "wav");
            if (SD.exists("/recordings/_tmp.wav")) {
                SD.rename("/recordings/_tmp.wav", fname);
                Serial.printf("[Save] %s\n", fname);
            }
            ui_setStatus(" Saved! (^-^)", TFT_GREEN);
            delay(1500);
            updateRecStatus5(false);
            break;
        }
        case Btn5::YARI: {
            ui_setStatus(" Preparing...", TFT_YELLOW);
            // 一時ファイルに上書き録音
            strncpy(s_lastRecPath, "/recordings/_tmp.wav", sizeof(s_lastRecPath));
            rec_start("/recordings/_tmp.wav");
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
