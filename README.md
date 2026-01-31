# ESP32智慧門鎖學習專案

一個基於ESP32的智慧門鎖系統，整合RFID卡片辨識與指紋辨識，使用TFT顯示器提供視覺回饋。這是一個為學習嵌入式系統設計的教育專案，程式碼中包含詳細的datasheet章節標註與技術說明。

## 📋 專案概述

### 功能特色
- 🔐 **雙重認證**: RFID卡片 + 指紋辨識
- 📺 **視覺回饋**: TFT彩色顯示器即時顯示狀態
- 🔓 **自動上鎖**: 可設定解鎖時間，自動重新上鎖
- 📚 **學習導向**: 每個模組都有詳細的datasheet章節註解

### 硬體組成
| 模組 | 型號 | 用途 | 通訊介面 |
|------|------|------|----------|
| 主控制器 | ESP32-WROOM-32 | 系統核心 | - |
| RFID讀卡器 | MFRC522 | 卡片辨識 | SPI |
| 指紋感應器 | AS608 | 指紋辨識 | UART |
| 伺服馬達 | SG90 | 門鎖控制 | PWM |
| TFT顯示器 | ILI9341/ST7789 | 狀態顯示 | SPI |

## 🗂️ 專案結構

```
smart_lock_junior/
├── platformio.ini          # PlatformIO配置文件
├── src/
│   └── main.cpp           # 主程式 (狀態機邏輯)
├── include/
│   ├── config.h           # 硬體接腳定義與系統配置
│   ├── rfid_handler.h     # RFID模組介面 (MFRC522)
│   ├── fingerprint_handler.h  # 指紋模組介面 (AS608)
│   ├── servo_handler.h    # 伺服馬達控制 (SG90)
│   └── display_handler.h  # 顯示器介面 (TFT_eSPI)
└── README.md              # 本文件
```

## 🔌 硬體連接

### ESP32 GPIO接腳分配

```
┌─────────────────────────────────────────────────────────────┐
│                        ESP32-WROOM-32                        │
├──────────┬──────────────────────────────────────────────────┤
│ GPIO Pin │ 功能                       │ 連接模組            │
├──────────┼──────────────────────────────────────────────────┤
│    23    │ SPI MOSI (共用)            │ MFRC522 + TFT       │
│    19    │ SPI MISO                   │ MFRC522             │
│    18    │ SPI SCK (共用)             │ MFRC522 + TFT       │
│    21    │ RFID SS (Chip Select)      │ MFRC522             │
│    22    │ RFID RST (Reset)           │ MFRC522             │
├──────────┼──────────────────────────────────────────────────┤
│    17    │ UART TX                    │ AS608 RX            │
│    16    │ UART RX                    │ AS608 TX            │
├──────────┼──────────────────────────────────────────────────┤
│    13    │ PWM (Servo Control)        │ SG90 Signal         │
├──────────┼──────────────────────────────────────────────────┤
│    15    │ TFT CS (Chip Select)       │ ILI9341/ST7789      │
│     2    │ TFT DC (Data/Command)      │ ILI9341/ST7789      │
│     4    │ TFT RST (Reset)            │ ILI9341/ST7789      │
└──────────┴──────────────────────────────────────────────────┘
```

### 電源連接
```
ESP32 3.3V  →  MFRC522 VCC, AS608 VCC, TFT VCC
ESP32 5V    →  SG90 VCC (建議外部5V電源)
ESP32 GND   →  所有模組 GND (務必共地！)
```

### 詳細接線圖

#### MFRC522 RFID模組
```
MFRC522    ESP32
-----------------
VCC    →   3.3V
GND    →   GND
RST    →   GPIO22
MISO   →   GPIO19
MOSI   →   GPIO23
SCK    →   GPIO18
SDA(SS)→   GPIO21
```

#### AS608指紋感應器
```
AS608      ESP32
-----------------
VCC    →   3.3V
GND    →   GND
TX     →   GPIO16 (RX)
RX     →   GPIO17 (TX)
```

#### SG90伺服馬達
```
SG90       電源
-----------------
橙色/黃色  →   GPIO13 (Signal)
紅色       →   5V (外部電源建議)
棕色/黑色  →   GND
```
⚠️ **重要**: SG90需要較大電流，建議使用外部5V電源供應器，並與ESP32共地。

#### ILI9341/ST7789顯示器
```
TFT        ESP32
-----------------
VCC    →   3.3V
GND    →   GND
CS     →   GPIO15
RESET  →   GPIO4
DC     →   GPIO2
MOSI   →   GPIO23 (與MFRC522共用)
SCK    →   GPIO18 (與MFRC522共用)
LED    →   3.3V (或GPIO控制亮度)
```

## 🚀 快速開始

### 環境需求
- [PlatformIO](https://platformio.org/) (VSCode擴展 或 CLI)
- USB傳輸線 (連接ESP32)
- 各硬體模組 (如上述列表)

### 安裝步驟

1. **克隆/下載專案**
   ```bash
   cd smart_lock_junior
   ```

2. **開啟PlatformIO**
   - VSCode: 安裝PlatformIO IDE擴展
   - 或使用PlatformIO CLI

3. **連接硬體**
   - 按照上述接線圖連接所有模組
   - 確保電源與GND正確連接
   - **檢查共地**: 所有模組GND必須連接在一起

4. **編譯專案**
   ```bash
   pio run
   ```

5. **上傳到ESP32**
   ```bash
   pio run --target upload
   ```

6. **開啟Serial監視器 (Debug用)**
   ```bash
   pio device monitor
   ```
   或在VSCode中使用PlatformIO的Serial Monitor。

### 首次運行

首次上傳後，你會在Serial Monitor看到:
```
=================================
  ESP32 智慧門鎖系統啟動
  Smart Lock System v1.0
=================================

ESP32 Chip ID: XXXXXXXXXXXX
CPU Frequency: 240 MHz
Flash Size: 4 MB

開始初始化硬體模組...

[1/5] 初始化SPI匯流排...
      ✓ SPI初始化完成
[2/5] 初始化TFT顯示器...
      ✓ 顯示器初始化成功
[3/5] 初始化MFRC522 RFID模組...
      ✓ RFID模組初始化成功
...
```

## 📖 學習路徑

這個專案採用**骨架程式**設計，提供完整的架構與詳細註解，但核心邏輯需要你自行實作。這是一個**邊做邊學**的專案。

### 第一階段：理解架構
1. **閱讀 `config.h`**
   - 了解各模組的接腳定義
   - 學習ESP32 GPIO多工功能
   - 查看硬體連接示意圖

2. **閱讀各Handler頭文件**
   - `rfid_handler.h`: RFID通訊協定 (SPI + ISO14443A)
   - `fingerprint_handler.h`: 指紋辨識流程 (UART + 封包協定)
   - `servo_handler.h`: PWM控制原理
   - `display_handler.h`: TFT顯示器命令集

3. **閱讀 `main.cpp`**
   - 理解Arduino的setup()與loop()架構
   - 學習狀態機設計模式
   - 了解非阻塞程式設計

### 第二階段：實作模組功能

每個模組的頭文件都包含：
- ✅ **完整的函式介面**
- ✅ **詳細的datasheet章節標註**
- ✅ **技術原理說明**
- ✅ **TODO標記** (需要你實作的部分)

建議實作順序：

#### 1. 伺服馬達控制 (最簡單)
- 檔案: `servo_handler.h` / `servo_handler.cpp`
- 學習重點: PWM原理、ESP32 LEDC
- Datasheet參考:
  - SG90 Datasheet Section 3
  - ESP32 TRM Section 14 (LEDC)
- 實作函式:
  - `init()`: 初始化PWM通道
  - `setAngle()`: 設定角度 (0~180°)
  - `lock()` / `unlock()`: 上鎖/解鎖

#### 2. TFT顯示器 (視覺回饋)
- 檔案: `display_handler.h` / `display_handler.cpp`
- 學習重點: SPI通訊、RGB565色彩、圖形渲染
- Datasheet參考:
  - ST7789V2 Section 8.4 (SPI Interface)
  - ST7789V2 Section 9 (Command Table)
- 實作函式:
  - `init()`: 初始化TFT_eSPI
  - `showWelcome()`: 設計歡迎畫面
  - `showUnlocked()` / `showAccessDenied()`: 狀態顯示
  - 繪製圖示函式

#### 3. RFID讀卡器 (中等難度)
- 檔案: `rfid_handler.h` / `rfid_handler.cpp`
- 學習重點: SPI通訊、ISO14443A協定、防碰撞演算法
- Datasheet參考:
  - MFRC522 Section 8.1 (SPI Interface)
  - MFRC522 Section 9.3 (Card Detection)
  - MFRC522 Section 10 (PICC Commands)
- 實作函式:
  - `init()`: 初始化MFRC522，啟動天線
  - `isCardPresent()`: 發送REQA命令偵測卡片
  - `readCardUID()`: 讀取UID (實作防碰撞)
  - `verifyCard()`: 白名單驗證

#### 4. 指紋感應器 (較複雜)
- 檔案: `fingerprint_handler.h` / `fingerprint_handler.cpp`
- 學習重點: UART通訊、封包協定、指紋辨識演算法
- Datasheet參考:
  - AS608 Manual Section 4.2 (Package Protocol)
  - AS608 Manual Section 5 (Instruction System)
- 實作函式:
  - `init()`: 初始化UART，驗證密碼
  - `detectFinger()`: GenImg命令偵測手指
  - `captureFingerprint()`: Img2Tz提取特徵
  - `verifyFingerprint()`: Search命令搜尋指紋庫
  - `enrollFingerprint()`: 註冊新指紋 (進階)

### 第三階段：整合與測試

1. **單元測試**
   - 分別測試每個模組
   - 使用Serial.print()輸出除錯資訊

2. **整合測試**
   - 在`main.cpp`中啟用各模組的函式呼叫
   - 測試完整的解鎖流程

3. **優化與擴展**
   - 加入錯誤處理
   - 實作平滑動畫
   - 新增進階功能 (見下方)

## 📚 Datasheet與參考文檔

### 必讀文檔
1. **ESP32技術參考手冊**
   - [ESP32 Technical Reference Manual](https://www.espressif.com/sites/default/files/documentation/esp32_technical_reference_manual_en.pdf)
   - 重點章節:
     - Section 3.2.4: SPI Peripheral
     - Section 10: UART Controller
     - Section 14: LED PWM Controller (LEDC)

2. **MFRC522 Datasheet**
   - [MFRC522數據手冊](https://www.nxp.com/docs/en/data-sheet/MFRC522.pdf)
   - 重點章節:
     - Section 8.1: SPI Interface
     - Section 9.3: Card Detection and Anti-collision
     - Section 10: MIFARE Commands

3. **AS608指紋模組用戶手冊**
   - AS608 User Manual V2.0
   - 重點章節:
     - Section 4: Communication Protocol
     - Section 5: Instruction System
     - Section 6: Application Notes

4. **SG90伺服馬達規格書**
   - SG90 Datasheet
   - 重點: Section 3 (Control Signal)

5. **ST7789V2 Datasheet**
   - [ST7789V2數據手冊](https://www.displayfuture.com/Display/datasheet/controller/ST7789V2.pdf)
   - 重點章節:
     - Section 8.4: 4-Line Serial Interface
     - Section 9: Command Table

### 函式庫文檔
- [Arduino ESP32 Core](https://docs.espressif.com/projects/arduino-esp32/en/latest/)
- [MFRC522 Library](https://github.com/miguelbalboa/rfid)
- [Adafruit Fingerprint Library](https://github.com/adafruit/Adafruit-Fingerprint-Sensor-Library)
- [TFT_eSPI Library](https://github.com/Bodmer/TFT_eSPI)
- [ESP32Servo Library](https://github.com/madhephaestus/ESP32Servo)

## 🛠️ 故障排除

### RFID模組無法初始化
- **症狀**: Serial顯示 "✗ RFID模組初始化失敗！"
- **檢查**:
  1. SPI接線是否正確 (MOSI, MISO, SCK)
  2. SS腳位是否連接到GPIO21
  3. RST腳位是否連接到GPIO22
  4. 3.3V供電是否穩定
- **測試**: 讀取Version Register (0x37)，應返回0x92

### 指紋感應器無回應
- **症狀**: UART無法通訊
- **檢查**:
  1. TX/RX是否交叉連接 (ESP32 TX → AS608 RX)
  2. 波特率是否為57600
  3. 電源是否穩定 (3.3V)
  4. 模組地址是否正確 (預設0xFFFFFFFF)
- **測試**: 發送VfyPwd命令驗證密碼

### 伺服馬達抖動或無力
- **症狀**: 馬達無法穩定保持角度
- **原因**: 電流不足
- **解決**:
  1. 使用外部5V電源 (至少1A)
  2. 在VCC與GND間並聯100μF電容
  3. 確保ESP32與外部電源共地

### TFT顯示器無顯示
- **症狀**: 螢幕全白或全黑
- **檢查**:
  1. `platformio.ini`的build_flags是否正確
  2. CS, DC, RST接腳是否正確
  3. SPI MOSI/SCK是否與MFRC522共用正確
  4. 背光腳位是否連接
- **測試**: 執行TFT_eSPI的範例程式

### Serial Monitor亂碼
- **原因**: 波特率不匹配
- **解決**: 確認波特率為115200

## 🎯 進階功能擴展

完成基礎功能後，可以嘗試以下擴展：

### 1. Wi-Fi遠端控制
```cpp
#include <WiFi.h>
#include <WebServer.h>

// 建立Web Server
WebServer server(80);

void setup() {
    WiFi.begin("SSID", "PASSWORD");
    server.on("/unlock", handleUnlock);
    server.begin();
}
```
- 透過手機APP或網頁遠端開鎖
- 即時查看門鎖狀態
- 接收開門通知

### 2. 資料記錄與分析
- 整合RTC模組 (DS3231) 記錄時間戳記
- 使用SD卡儲存開門記錄
- 分析使用模式

### 3. 多重認證模式
- **雙因子認證**: RFID + 指紋必須同時驗證
- **臨時授權**: 設定單次使用的臨時卡片
- **時間限制**: 特定時段才能開門

### 4. 省電模式
```cpp
#include "esp_sleep.h"

void enterDeepSleep() {
    esp_sleep_enable_ext0_wakeup(RFID_IRQ_PIN, 0);
    esp_deep_sleep_start();
}
```
- 使用Deep Sleep降低功耗
- 外部中斷喚醒 (PIR感應器)

### 5. 安全強化
- 指紋+密碼雙重認證
- 失敗次數限制與鎖定機制
- 防撬警報 (加速度感應器)
- 加密通訊 (HTTPS/MQTT with TLS)

### 6. UI/UX優化
- Sprite雙緩衝消除閃爍
- 平滑動畫效果
- 多語言支援
- 自訂主題與圖示

## 📝 程式碼規範

專案遵循以下規範：
- **命名規則**: 
  - 類別名稱: `PascalCase`
  - 函式名稱: `camelCase`
  - 變數名稱: `camelCase`
  - 常數: `UPPER_CASE`
- **縮排**: 4空格
- **註解**: 
  - 函式前有完整說明
  - 關鍵程式碼有行內註解
  - Datasheet章節標註
- **TODO標記**: 需要實作的部分

## 🤝 貢獻與學習交流

這是一個學習專案，歡迎：
- 分享你的實作經驗
- 提出改進建議
- 回報Bug或問題

## 📄 授權

本專案採用教育用途開源，可自由學習與修改。

## 💡 學習建議

1. **不要急於求成**: 一次專注一個模組，確實理解原理
2. **多讀Datasheet**: 程式碼中的章節標註是學習重點
3. **動手測試**: 單獨測試每個模組再整合
4. **記錄問題**: 建立除錯日誌，記錄解決方案
5. **參考範例**: 各函式庫的Examples資料夾有很多範例

## 📞 支援

遇到問題時：
1. 檢查Serial Monitor的除錯訊息
2. 參考程式碼中的TODO註解
3. 查閱對應的Datasheet章節
4. 使用示波器或邏輯分析儀檢查訊號 (進階)

---

**祝你學習順利！🚀**

從零開始建構一個智慧門鎖系統，你會學到：
- 嵌入式系統架構設計
- 多種硬體通訊協定 (SPI, UART, PWM)
- 狀態機程式設計
- 感測器整合
- 實用的專案開發經驗

這些技能是成為嵌入式工程師的基礎！
