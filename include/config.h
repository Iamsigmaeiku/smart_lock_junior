/*
 * ESP32智慧門鎖 - 硬體配置文件
 * 
 * 本文件定義所有硬體模組的接腳配置與系統參數
 * 學習重點: 了解ESP32 GPIO多工功能與硬體通訊協定
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ============================================================================
// 系統配置
// ============================================================================

// Serial通訊速率 (用於Debug輸出)
#define SERIAL_BAUD_RATE 115200

// 系統狀態更新間隔 (毫秒)
#define SYSTEM_UPDATE_INTERVAL 100

// 門鎖解鎖持續時間 (毫秒)
#define UNLOCK_DURATION 5000

// ============================================================================
// MFRC522 RFID模組配置
// ============================================================================
// 參考文檔: MFRC522 Datasheet
// - Section 8.1: SPI Interface (SPI通訊協定)
// - Section 8.1.2: SPI Timing (時序要求)
// - Table 27: Pin Description (接腳說明)
//
// MFRC522使用SPI通訊，工作電壓3.3V
// SPI模式: Mode 0 (CPOL=0, CPHA=0)
// 最大時鐘頻率: 10MHz
// ============================================================================

// ESP32 VSPI接腳定義 (硬體SPI)
// ESP32 Technical Reference Manual Section 3.2.4 (SPI Peripheral)
//#define SPI_MOSI_PIN  23  // Master Out Slave In (主設備輸出)
//#define SPI_MISO_PIN  19  // Master In Slave Out (主設備輸入)
//#define SPI_SCK_PIN   18  // Serial Clock (時鐘訊號)

// MFRC522專用控制腳位
#define RFID_SS_PIN   21  // Slave Select / Chip Select (片選腳位)
                          // Datasheet Section 8.1: 低電位啟用MFRC522
#define RFID_RST_PIN  22  // Reset (重置腳位)
                          // Datasheet Section 7.5: 低電位重置，需上拉電阻

// RFID卡片UID長度 (字節)
// ISO/IEC 14443A標準: 4字節、7字節或10字節
#define RFID_UID_SIZE 4

// ============================================================================
// AS608指紋感應器模組配置
// ============================================================================
// 參考文檔: AS608 User Manual V2.0
// - Section 4.1: UART Communication (UART通訊協定)
// - Section 4.2: Package Protocol (封包協定)
// - Table 1: Default Parameters (預設參數)
//
// AS608使用UART通訊，工作電壓3.3V
// 預設波特率: 57600 bps (可配置 9600~115200)
// 資料格式: 8N1 (8資料位，無校驗位，1停止位)
// ============================================================================

// ESP32 UART2接腳 (使用Hardware Serial)
// ESP32 Technical Reference Manual Section 10.3.1 (UART Pins)
#define FP_TX_PIN     17  // ESP32 TX -> AS608 RX (傳送資料到感應器)
#define FP_RX_PIN     16  // ESP32 RX -> AS608 TX (接收感應器資料)

// AS608 UART通訊參數
// User Manual Section 4.1.1: Communication Parameter
#define FP_BAUD_RATE  57600  // 預設波特率

// AS608模組地址 (預設為0xFFFFFFFF)
// User Manual Section 4.2: 每個封包都需包含模組地址
#define FP_ADDRESS    0xFFFFFFFF

// 指紋比對信心閾值 (0-255，值越高越嚴格)
// User Manual Section 5.4: Match命令返回的confidence score
#define FP_CONFIDENCE_THRESHOLD 50

// 指紋資料庫容量
#define FP_DATABASE_SIZE 127  // AS608標準版: 0~126 (共127個位置)

// ============================================================================
// SG90伺服馬達配置
// ============================================================================
// 參考文檔: SG90 Servo Datasheet
// - Section 3: Control Signal (控制訊號規格)
// - 工作電壓: 4.8V ~ 6V (建議5V)
// - 控制週期: 20ms (50Hz)
// - 脈衝寬度: 1ms(0°) ~ 1.5ms(90°) ~ 2ms(180°)
//
// ESP32 PWM控制器參考:
// - ESP32 Technical Reference Manual Section 14: LED PWM Controller
// - 解析度: 最高16位元 (0-65535)
// - 頻率範圍: 支援1Hz ~ 40MHz
// ============================================================================

#define SERVO_PIN     13  // 伺服馬達PWM控制腳位

// PWM參數設定
// TRM Section 14.3: PWM Timer Configuration
#define SERVO_PWM_FREQ    50    // PWM頻率: 50Hz (週期20ms)
#define SERVO_PWM_CHANNEL 0     // LEDC通道 (ESP32有16個PWM通道)
#define SERVO_PWM_RESOLUTION 16 // 解析度: 16位元 (0-65535)

// 角度對應的脈衝寬度 (微秒)
// SG90 Datasheet Section 3: Pulse Width vs Angle
#define SERVO_PULSE_MIN  1000   // 0度位置 (1ms)
#define SERVO_PULSE_MID  1500   // 90度位置 (1.5ms)
#define SERVO_PULSE_MAX  2000   // 180度位置 (2ms)

// 門鎖角度定義
#define SERVO_LOCK_ANGLE    0   // 上鎖位置 (0度)
#define SERVO_UNLOCK_ANGLE 90   // 解鎖位置 (90度)

// ============================================================================
// ILI9341 / ST7789顯示器配置
// ============================================================================
// 參考文檔: ST7789V2 Datasheet
// - Section 8.4: 4-Line Serial Interface (SPI通訊)
// - Section 9: Display Commands (顯示命令集)
// - Section 13.2: Pixel Format (像素格式設定)
//
// 顯示器使用SPI通訊，工作電壓3.3V
// SPI模式: Mode 3 (CPOL=1, CPHA=1)
// 最大時鐘頻率: 15MHz (寫入)
// ============================================================================

// 顯示器SPI控制腳位 (與MFRC522共用SPI MOSI/SCK)
// 但使用不同的CS (Chip Select) 實現多設備共存
#define TFT_CS_PIN    15  // Chip Select (片選)
                          // Datasheet Section 8.4.1: 低電位選中顯示器
#define TFT_DC_PIN     2  // Data/Command (資料/命令選擇)
                          // DC=0: 傳送命令 | DC=1: 傳送資料
#define TFT_RST_PIN    4  // Reset (重置)
                          // Datasheet Section 8.5: 低電位重置顯示器

// 顯示器解析度
// ST7789V2 Section 8.2: Display Resolution
#define TFT_WIDTH   240   // 寬度 (像素)
#define TFT_HEIGHT  320   // 高度 (像素)

// 顯示器顏色定義 (RGB565格式)
// ST7789V2 Section 13.2.1: 16-bit/pixel (65K colors)
// 格式: RRRRRGGGGGGBBBBB
#define COLOR_BLACK     0x0000
#define COLOR_WHITE     0xFFFF
#define COLOR_RED       0xF800
#define COLOR_GREEN     0x07E0
#define COLOR_BLUE      0x001F
#define COLOR_YELLOW    0xFFE0
#define COLOR_CYAN      0x07FF
#define COLOR_MAGENTA   0xF81F
#define COLOR_GRAY      0x8410
#define COLOR_DARKGRAY  0x4208

// UI參數
#define TEXT_SIZE_SMALL   2
#define TEXT_SIZE_MEDIUM  3
#define TEXT_SIZE_LARGE   4

// ============================================================================
// 硬體連接示意圖
// ============================================================================
/*
 * ESP32 GPIO分配總覽:
 * 
 * ┌─────────────────────────────────────────────────────────────┐
 * │                        ESP32-WROOM-32                        │
 * ├──────────┬──────────────────────────────────────────────────┤
 * │ GPIO Pin │ 功能                       │ 連接模組            │
 * ├──────────┼──────────────────────────────────────────────────┤
 * │    23    │ SPI MOSI (共用)            │ MFRC522 + TFT       │
 * │    19    │ SPI MISO                   │ MFRC522             │
 * │    18    │ SPI SCK (共用)             │ MFRC522 + TFT       │
 * │    21    │ RFID SS (Chip Select)      │ MFRC522             │
 * │    22    │ RFID RST (Reset)           │ MFRC522             │
 * ├──────────┼──────────────────────────────────────────────────┤
 * │    17    │ UART TX                    │ AS608 RX            │
 * │    16    │ UART RX                    │ AS608 TX            │
 * ├──────────┼──────────────────────────────────────────────────┤
 * │    13    │ PWM (Servo Control)        │ SG90 Signal         │
 * ├──────────┼──────────────────────────────────────────────────┤
 * │    15    │ TFT CS (Chip Select)       │ ILI9341/ST7789      │
 * │     2    │ TFT DC (Data/Command)      │ ILI9341/ST7789      │
 * │     4    │ TFT RST (Reset)            │ ILI9341/ST7789      │
 * └──────────┴──────────────────────────────────────────────────┘
 * 
 * 電源連接:
 * - ESP32 3.3V    -> MFRC522 VCC, AS608 VCC, TFT VCC
 * - ESP32 5V/VIN  -> SG90 VCC (紅線)
 * - ESP32 GND     -> 所有模組GND (黑線/棕線)
 * 
 * 注意事項:
 * 1. SPI匯流排共用: MFRC522與TFT共用MOSI(23)與SCK(18)
 *    透過不同的CS腳位(21 vs 15)區分設備
 * 2. ESP32的VSPI預設腳位: MOSI=23, MISO=19, SCK=18
 * 3. 所有模組務必共地(GND連接在一起)
 * 4. SG90需要5V供電，電流需求較大，建議使用外部電源
 * 5. AS608使用Hardware Serial(UART2)，避免與USB Serial(UART0)衝突
 */

// ============================================================================
// 狀態機定義
// ============================================================================
// 用於main.cpp的系統狀態管理

enum SystemState {
    STATE_INIT,            // 初始化狀態
    STATE_IDLE,            // 待機狀態 (等待RFID或指紋)
    STATE_RFID_DETECTED,   // 偵測到RFID卡片
    STATE_FP_DETECTED,     // 偵測到指紋
    STATE_VERIFYING,       // 驗證中
    STATE_ACCESS_GRANTED,  // 驗證通過，解鎖
    STATE_ACCESS_DENIED,   // 驗證失敗
    STATE_ERROR            // 錯誤狀態
};

// ============================================================================
// 除錯巨集
// ============================================================================

// 根據編譯模式啟用/停用Debug輸出
#ifdef DEBUG_MODE
    #define DEBUG_PRINT(x)    Serial.print(x)
    #define DEBUG_PRINTLN(x)  Serial.println(x)
    #define DEBUG_PRINTF(...) Serial.printf(__VA_ARGS__)
#else
    #define DEBUG_PRINT(x)
    #define DEBUG_PRINTLN(x)
    #define DEBUG_PRINTF(...)
#endif

#endif // CONFIG_H
