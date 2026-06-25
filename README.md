# そらみる — M5Stack CoreS3 実装

天気の観察を促すロボット「そらみる」のスタンドアロン録音版です。

## ファイル構成

```
soramiru_m5cores3/
├── platformio.ini
└── src/
    ├── main.cpp          メインループ・状態管理
    ├── ui.h              タッチUI描画・ボタンヒット判定
    ├── audio_player.h    SDカードMP3再生
    └── recorder.h        マイク録音 → SDカードWAV保存
```

---

## microSDカードの準備

FAT32でフォーマットしたmicroSDに、以下のフォルダとファイルを作成してください。

```
/
├── audio/
│   ├── startup.mp3          起動時に再生
│   ├── intro.mp3            [Intro]ボタンで再生
│   ├── end.mp3              [End]ボタンで再生
│   │   └──（推奨スクリプト）
│   │       「私と一緒に天気を感じてくれてありがとう。
│   │        君が話してくれた内容は、しっかり僕の中に記録したよ。
│   │        次の人に僕を渡してね！」
│   └── task/
│       ├── task01.mp3       タスク1（例:「今日の空の色は何色ですか？」）
│       ├── task02.mp3
│       ⋮
│       └── task10.mp3
│           └──（各タスクには「タスクの実行報告時は、
│               名前とタスク番号を明示してから話してね。」の文言を含める）
└── recordings/              ← プログラムが自動生成します（手動作成不要）
    └── REC_20240601_103045.wav   ← 録音のたびに自動保存される
```

### MP3ファイルの作り方（例）
- **テキスト読み上げ**: Google Text-to-Speech、VOICEVOX、CoeFont 等でMP3を生成
- **手動録音**: スマートフォンの録音アプリで録音 → MP3変換ツールで変換
- **推奨フォーマット**: MP3 / 128kbps / 44.1kHz / モノラルまたはステレオ

---

## VSCode + PlatformIO セットアップ（具体的な操作）

### Step 1: 拡張機能のインストール

1. VSCode を起動する
2. 左サイドバーの 🧩 **Extensions**（拡張機能）アイコンをクリック
3. 検索ボックスに `PlatformIO IDE` と入力
4. 「**PlatformIO IDE**」（platformio.org 製）を選択 → **Install** をクリック
5. インストール完了のポップアップが出たら **Reload Window** をクリック

> ⚠️ **Python 3.6以上** が必要です。  
> インストールされていない場合は https://www.python.org/ から先にインストールしてください。

---

### Step 2: プロジェクトを開く

1. `File` → `Open Folder`
2. `soramiru_m5cores3` フォルダを選択して「**フォルダーを選択**」
3. しばらく待つと、画面下部に PlatformIO のツールバーが現れる

---

### Step 3: M5Stack CoreS3 を接続する

1. USB Type-C ケーブルで CoreS3 と PC を繋ぐ
2. CoreS3 の側面ボタンを**長押し**して起動
3. VSCode 下部のバーに `env:m5stack-cores3` と表示されることを確認

---

### Step 4: ビルドとアップロード

VSCode 下部のツールバーにあるアイコンを使います：

```
 ✓  ビルド（コンパイルのみ）    Ctrl + Alt + B
 →  アップロード（書き込み）    Ctrl + Alt + U
 🔌  シリアルモニタ              Ctrl + Alt + S
```

**手順:**
1. まず ✓ **ビルド**を押してエラーがないか確認
2. 次に → **アップロード**を押して CoreS3 に書き込む
3. 書き込み完了後、CoreS3 が自動で再起動する

---

### Step 5: シリアルモニタで確認

🔌 シリアルモニタを開くと以下のようなログが流れます：

```
[Audio] Done: /audio/startup.mp3
[Rec] Start: /recordings/REC_1234567.wav
[Rec] Saved 163840 bytes of audio data
```

ボーレートが合わない場合は、右下のドロップダウンで **115200** を選択してください。

---

## 録音ファイルの後処理（文字起こし）

SDカードを回収後、録音ファイル（.wav）をテキスト化する方法：

### 方法A: OpenAI Whisper（ローカル・無料）
```bash
# インストール
pip install openai-whisper

# 文字起こし（日本語）
whisper REC_20240601_103045.wav --language Japanese --output_format txt
```

### 方法B: OpenAI Whisper API（クラウド）
```python
import openai
client = openai.OpenAI()

with open("REC_20240601_103045.wav", "rb") as f:
    result = client.audio.transcriptions.create(
        model="whisper-1",
        file=f,
        language="ja"
    )
print(result.text)
```

### 方法C: 一括変換スクリプト
```python
# batch_transcribe.py  —  /recordings/ 内の全WAVを一括変換してCSV出力
import os, csv, openai

client = openai.OpenAI()
output = []

for fname in sorted(os.listdir("recordings")):
    if not fname.endswith(".wav"):
        continue
    with open(f"recordings/{fname}", "rb") as f:
        res = client.audio.transcriptions.create(
            model="whisper-1", file=f, language="ja"
        )
    output.append({"filename": fname, "text": res.text})
    print(f"{fname}: {res.text}")

with open("results.csv", "w", newline="", encoding="utf-8") as f:
    w = csv.DictWriter(f, fieldnames=["filename", "text"])
    w.writeheader()
    w.writerows(output)

print("results.csv に保存しました")
```

---

## トラブルシューティング

| 症状 | 対処法 |
|---|---|
| ビルドエラー「M5CoreS3.h not found」 | ビルド実行で自動DL。または PlatformIO: Update All Libraries |
| SDが認識されない | FAT32フォーマット確認。microSDを一度抜き差し |
| MP3が再生されない | ファイルパスのスペルを確認。`/audio/intro.mp3` (小文字) |
| 録音ファイルが0バイト | `rec_stop()` が呼ばれているか確認。シリアルログを確認 |
| アップロードできない | CoreS3のボタンを押しながらUSBを挿す（ブートローダーモード） |
