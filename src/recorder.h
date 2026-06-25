#pragma once
#include <Arduino.h>
#include <SD.h>
#include <M5CoreS3.h>
#include <time.h>

constexpr uint32_t REC_SAMPLE_RATE = 16000;
constexpr uint16_t REC_CHANNELS    = 1;
constexpr uint16_t REC_BIT_DEPTH   = 16;
constexpr size_t   REC_CHUNK_SAMP  = 256;

struct WavHeader {
    char     riff[4]       = {'R','I','F','F'};
    uint32_t fileSize      = 0;
    char     wave[4]       = {'W','A','V','E'};
    char     fmt[4]        = {'f','m','t',' '};
    uint32_t fmtSize       = 16;
    uint16_t audioFmt      = 1;
    uint16_t numChannels   = REC_CHANNELS;
    uint32_t sampleRate    = REC_SAMPLE_RATE;
    uint32_t byteRate      = REC_SAMPLE_RATE * REC_CHANNELS * 2;
    uint16_t blockAlign    = REC_CHANNELS * 2;
    uint16_t bitsPerSample = REC_BIT_DEPTH;
    char     data[4]       = {'d','a','t','a'};
    uint32_t dataSize      = 0;
};

static File      s_recFile;
static bool      s_recording          = false;
static uint32_t  s_dataBytesWritten   = 0;
static TaskHandle_t s_recTaskHandle   = nullptr;

// FreeRTOSタスク：別スレッドで録音し続ける
static void recTask(void* arg) {
    int16_t samples[REC_CHUNK_SAMP];
    while (s_recording) {
        if (CoreS3.Mic.record(samples, REC_CHUNK_SAMP, REC_SAMPLE_RATE)) {
            size_t bytes = REC_CHUNK_SAMP * sizeof(int16_t);
            s_recFile.write((uint8_t*)samples, bytes);
            s_dataBytesWritten += bytes;
        }
        vTaskDelay(1 / portTICK_PERIOD_MS);
    }
    vTaskDelete(nullptr);
}

inline void buildRecFilename(char* buf, size_t bufSize) {
    struct tm ti;
    if (getLocalTime(&ti)) {
        strftime(buf, bufSize, "/recordings/REC_%Y%m%d_%H%M%S.wav", &ti);
    } else {
        snprintf(buf, bufSize, "/recordings/REC_%lu.wav", millis());
    }
}

inline void rec_start(const char* path) {
    Serial.printf("[Rec] Start: %s\n", path);

    CoreS3.Mic.end();
    CoreS3.Speaker.end();  // スピーカーI2Sを確実に解放
    s_speakerInited = false;
    delay(2000);           // I2S完全解放待ち
    bool micOk = false;
    for (int retry = 0; retry < 5; retry++) {
        if (CoreS3.Mic.begin()) { micOk = true; break; }
        Serial.printf("[Rec] Mic retry %d\n", retry + 1);
        CoreS3.Mic.end();
        delay(500);
    }
    if (!micOk) {
        Serial.println("[Rec] ERROR: Mic init failed");
        return;
    }
    // マイクゲインを最大に設定
    auto mic_cfg = CoreS3.Mic.config();
    mic_cfg.magnification = 16; // 1〜16 デフォルトは1
    CoreS3.Mic.config(mic_cfg);
    delay(200);

    s_recFile = SD.open(path, FILE_WRITE);
    if (!s_recFile) {
        Serial.println("[Rec] ERROR: Cannot open file");
        CoreS3.Mic.end();
        return;
    }

    WavHeader hdr;
    s_recFile.write((uint8_t*)&hdr, sizeof(hdr));
    s_dataBytesWritten = 0;
    s_recording = true;

    // Core1で録音タスクを起動
    xTaskCreatePinnedToCore(
        recTask, "recTask",
        4096, nullptr, 1,
        &s_recTaskHandle, 1
    );
    Serial.println("[Rec] Task started");
}

inline void rec_stop() {
    if (!s_recording) return;
    s_recording = false;
    delay(200); // タスク終了を待つ

    CoreS3.Mic.end();

    WavHeader hdr;
    hdr.dataSize = s_dataBytesWritten;
    hdr.fileSize = s_dataBytesWritten + sizeof(WavHeader) - 8;
    s_recFile.seek(0);
    s_recFile.write((uint8_t*)&hdr, sizeof(hdr));
    s_recFile.flush();
    s_recFile.close();

    Serial.printf("[Rec] Saved %u bytes\n", s_dataBytesWritten);
}

// loop()からは呼ばなくてよい（タスクが自動で録音する）
inline void rec_write_chunk() {}

// 録音したファイルを再生する
static char s_lastRecPath[60] = "";

inline void rec_play() {
    if (strlen(s_lastRecPath) == 0) return;
    Serial.printf("[Rec] Play: %s\n", s_lastRecPath);

    File f = SD.open(s_lastRecPath, FILE_READ);
    if (!f) { Serial.println("[Rec] Play: file not found"); return; }

    size_t fileSize = f.size();
    uint8_t* buf = (uint8_t*)ps_malloc(fileSize);
    if (!buf) { f.close(); return; }
    f.read(buf, fileSize);
    f.close();

    uint32_t sr = buf[24]|(buf[25]<<8)|(buf[26]<<16)|(buf[27]<<24);
    const size_t HDR = 44;

    CoreS3.Speaker.begin();
    CoreS3.Speaker.setVolume(200);
    CoreS3.Speaker.playRaw((const int16_t*)(buf+HDR),
                           (fileSize-HDR)/2, sr, false, 1, 0);
    while (CoreS3.Speaker.isPlaying()) {
        CoreS3.update();
        delay(10);
    }
    CoreS3.Speaker.end();
    s_speakerInited = false;
    free(buf);
    Serial.println("[Rec] Play done");
}
