#ifndef CONFIG_H
#define CONFIG_H

// ========================================
// 接腳定義 (根據你的硬體設定)
// ========================================

// SG90 伺服馬達接腳
#define MOTOR_PIN 13  // 橘色線 (PWM 訊號)

// AS608 指紋模組接腳 (UART2)
#define FINGERPRINT_RX 16  // ESP32 RX <- AS608 TX
#define FINGERPRINT_TX 17  // ESP32 TX -> AS608 RX

// See SetupX_Template.h for all options available


#define LOAD_GLCD    // Font 1. Original Adafruit 8 pixel font needs ~1820 bytes in FLASH
#define LOAD_FONT2   // Font 2. Small 16 pixel high font, needs ~3534 bytes in FLASH, 96 characters
#define LOAD_FONT4   // Font 4. Medium 26 pixel high font, needs ~5848 bytes in FLASH, 96 characters
#define LOAD_FONT6   // Font 6. Large 48 pixel font, needs ~2666 bytes in FLASH, only characters 1234567890:-.apm
#define LOAD_FONT7   // Font 7. 7 segment 48 pixel font, needs ~2438 bytes in FLASH, only characters 1234567890:.
#define LOAD_FONT8   // Font 8. Large 75 pixel font needs ~3256 bytes in FLASH, only characters 1234567890:-.
#define LOAD_GFXFF   // FreeFonts. Include access to the 48 Adafruit_GFX free fonts FF1 to FF48 and custom fonts

#define SMOOTH_FONT

// TFT SPI clock frequency

// RFID RC522 接腳 (SPI - 與螢幕共用 MOSI/MISO/SCLK)
#define RFID_CS   5   // Chip Select (SDA)
#define RFID_RST  27  // Reset



// 觸控校準參數（需根據實際硬體調整）
#define TS_MINX 380
#define TS_MAXX 3680
#define TS_MINY 340
#define TS_MAXY 3740

// HUSKYLENS AI 辨識鏡頭接腳 (I2C)
#define HUSKYLENS_SDA 21  // I2C 資料線
#define HUSKYLENS_SCL 22  // I2C 時鐘線
#define HUSKYLENS_I2C_ADDR 0x32  // I2C 地址（預設）

// ========================================
// 系統設定
// ========================================
#define SERIAL_BAUD 115200
#define UNLOCK_DURATION 5000  // 開鎖持續時間 (毫秒)

// ========================================
// 螢幕設定
// ========================================
#define SCREEN_WIDTH  240
#define SCREEN_HEIGHT 320

// 主選單按鈕佈局
#define MENU_BTN_LEFT_X    10
#define MENU_BTN_RIGHT_X   130
#define MENU_BTN_WIDTH     110
#define MENU_BTN_HEIGHT    60

#define MENU_BTN_ROW1_Y    50   // Finger, RFID
#define MENU_BTN_ROW2_Y    120  // Password, Face
#define MENU_BTN_ROW3_Y    190  // Enroll, Setting


//WiFi推波設定
#define DISCORD_WEBHOOK_URL "https://discord.com/api/webhooks/1467471815801831516/vm3TmYFn-mKYoXaDZ5kdK4E4k_RnmcbSSUCYrT2hNGBr8-0sx76Tl5xaFPfRaEbxNyF-"


#endif
