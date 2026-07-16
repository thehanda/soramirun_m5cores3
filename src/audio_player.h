#pragma once
#include <M5CoreS3.h>
#include <SD.h>

static bool s_speakerInited = false;

extern uint8_t g_volumeLevel;
extern const uint8_t VOL_VALUES[];

inline void ensureSpeaker() {
    if (!s_speakerInited) {
        CoreS3.Speaker.begin();
        CoreS3.Speaker.setVolume(VOL_VALUES[g_volumeLevel]);
        s_speakerInited = true;
    }
}

static uint32_t getWavSampleRate(const uint8_t* buf) {
    return buf[24]|(buf[25]<<8)|(buf[26]<<16)|(buf[27]<<24);
}
static uint16_t getWavChannels(const uint8_t* buf) {
    return buf[22]|(buf[23]<<8);
}

// 通常再生
inline void playMp3(const char* path) {
    if (!SD.exists(path)) {
        Serial.printf("[Audio] Not found: %s\n", path);
        ensureSpeaker();
        CoreS3.Speaker.tone(880, 200);
        delay(250);
        return;
    }
    File f = SD.open(path, FILE_READ);
    if (!f) return;
    size_t fileSize = f.size();
    uint8_t* buf = (uint8_t*)ps_malloc(fileSize);
    if (!buf) { f.close(); return; }
    f.read(buf, fileSize); f.close();
    uint32_t sr = getWavSampleRate(buf);
    uint16_t ch = getWavChannels(buf);
    const size_t HDR = 44;
    if (fileSize > HDR) {
        ensureSpeaker();
        CoreS3.Speaker.playRaw((const int16_t*)(buf+HDR),
                               (fileSize-HDR)/2, sr, ch==2, 1, 0);
        while (CoreS3.Speaker.isPlaying()) {
            CoreS3.update(); delay(10);
        }
    }
    free(buf);
    // スピーカータスクを完全に解放してからリターン
    CoreS3.Speaker.end();
    s_speakerInited = false;
    delay(100);
    Serial.printf("[Audio] Done: %s\n", path);
}

// タッチで中断できる再生（trueを返したら中断された）
inline bool playMp3Interruptible(const char* path) {
    if (!SD.exists(path)) {
        Serial.printf("[Audio] Not found: %s\n", path);
        return false;
    }
    File f = SD.open(path, FILE_READ);
    if (!f) return false;
    size_t fileSize = f.size();
    uint8_t* buf = (uint8_t*)ps_malloc(fileSize);
    if (!buf) { f.close(); return false; }
    f.read(buf, fileSize); f.close();
    uint32_t sr = getWavSampleRate(buf);
    uint16_t ch = getWavChannels(buf);
    const size_t HDR = 44;
    bool interrupted = false;
    if (fileSize > HDR) {
        ensureSpeaker();
        CoreS3.Speaker.playRaw((const int16_t*)(buf+HDR),
                               (fileSize-HDR)/2, sr, ch==2, 1, 0);
        while (CoreS3.Speaker.isPlaying()) {
            CoreS3.update();
            // タッチで中断
            if (CoreS3.Touch.getCount() > 0) {
                auto t = CoreS3.Touch.getDetail();
                if (t.wasPressed()) {
                    CoreS3.Speaker.stop();
                    interrupted = true;
                    break;
                }
            }
            delay(10);
        }
    }
    free(buf);
    CoreS3.Speaker.end();
    s_speakerInited = false;
    delay(100);
    return interrupted;
}
