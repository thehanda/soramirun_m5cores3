# ソラミルン・Futocoron プロジェクト

## 担当者
羽鳥研究室（愛媛大学 地域レジリエンス学環）

## プロジェクト概要
- ソラミルン：M5Stack CoreS3搭載ロボット（録音・カメラ・音声再生・タッチUI）
- Futocoron：人形にスピーカー内蔵のロボット（音声再生のみ）

## 開発環境
- Mac（Apple Silicon）
- VS Code + PlatformIO
- アップロードコマンド：
  cd ~/Downloads/soramiru_m5cores3 && ~/.platformio/penv/bin/platformio run --target upload --upload-port /dev/cu.usbmodem1101
- シリアルモニタ：
  ~/.platformio/penv/bin/platformio device monitor --port /dev/cu.usbmodem1101 --baud 115200

## SDカード
- マウント先：/Volumes/NO NAME/
- 音声ファイル：/audio/
- 録音：/recordings/
- 写真：/photos/
- 連番管理：/seq.txt

## ファイル名形式
0001_p03_task2.wav
0002_p03_task1.jpg
（連番_p参加者番号_タスク種別）

## WiFi（自宅・NTP同期用）
SSID: Buffalo-A-35C0
PW: 6a5idibtfxdtk

## GitHub
https://github.com/thehanda/soramirun_m5cores3

## 説明書PDF（GitHub Pages）
https://thehanda.github.io/soramirun_m5cores3/docs/soramirun_manual.pdf

## Futocoronフォーム
URL: https://forms.gle/ASTz9iCwfL1BAPV4A
オーナー: futocoron1@gmail.com
