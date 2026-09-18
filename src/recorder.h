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
    delay(300); // I2S解放待ち（この後スピーカー側で再利用するため）

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

// 再生用バッファ：毎回ps_malloc/freeすると長時間使用でPSRAMが断片化し
// 確保に失敗することがあったため、起動時に一度だけ確保して使い回す。
constexpr size_t REC_PLAY_BUF_CAP = 2 * 1024 * 1024; // 16kHz/mono/16bitで約65秒分
static uint8_t*  s_playBuf    = nullptr;
static size_t    s_playBufCap = 0;

inline void rec_initPlayBuffer() {
    if (s_playBuf) return;
    s_playBuf = (uint8_t*)ps_malloc(REC_PLAY_BUF_CAP);
    s_playBufCap = s_playBuf ? REC_PLAY_BUF_CAP : 0;
    Serial.printf("[Rec] Play buffer init: %s (%u bytes)\n",
                  s_playBuf ? "OK" : "FAILED", (unsigned)REC_PLAY_BUF_CAP);
}

inline void rec_play() {
    if (strlen(s_lastRecPath) == 0) return;
    Serial.printf("[Rec] Play: %s\n", s_lastRecPath);

    File f = SD.open(s_lastRecPath, FILE_READ);
    if (!f) { Serial.println("[Rec] Play: file not found"); return; }

    size_t fileSize = f.size();
    if (!s_playBuf || fileSize > s_playBufCap) {
        Serial.printf("[Rec] Play: buffer unavailable (need %u, cap %u)\n",
                      (unsigned)fileSize, (unsigned)s_playBufCap);
        f.close();
        return;
    }
    f.read(s_playBuf, fileSize);
    f.close();

    uint32_t sr = s_playBuf[24]|(s_playBuf[25]<<8)|(s_playBuf[26]<<16)|(s_playBuf[27]<<24);
    const size_t HDR = 44;
    size_t numSamples = (fileSize - HDR) / 2;
    // 再生時間の見積り＋余裕を持たせた上限。ドライバ異常でisPlaying()が
    // 返り続けても無限ループにならないようにする安全弁。
    uint32_t expectedMs = (sr > 0) ? (uint32_t)((uint64_t)numSamples * 1000 / sr) : 0;
    uint32_t maxWaitMs  = expectedMs + 2000;

    CoreS3.Speaker.begin();
    CoreS3.Speaker.setVolume(200);
    CoreS3.Speaker.playRaw((const int16_t*)(s_playBuf+HDR),
                           numSamples, sr, false, 1, 0);
    uint32_t startMs = millis();
    while (CoreS3.Speaker.isPlaying()) {
        CoreS3.update();
        if (millis() - startMs > maxWaitMs) {
            Serial.println("[Rec] Play: timeout, forcing stop");
            CoreS3.Speaker.stop();
            break;
        }
        delay(10);
    }
    CoreS3.Speaker.end();
    s_speakerInited = false;
    Serial.println("[Rec] Play done");
}
