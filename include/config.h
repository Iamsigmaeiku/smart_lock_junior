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
#define RFID_RST  22  // Reset



// 觸控校準參數（需根據實際硬體調整）
#define TS_MINX 380
#define TS_MAXX 3680
#define TS_MINY 340
#define TS_MAXY 3740

// HUSKYLENS AI 辨識鏡頭接腳 (UART)
#define HUSKYLENS_RX 26   // ESP32 RX <- HuskyLens TX (藍線)
#define HUSKYLENS_TX 25  // ESP32 TX -> HuskyLens RX (綠線)

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

// 主選單按鈕佈局 (2x2 - 已棄用，改用 getMenuButtonRect)
// Header 高度
#define HEADER_HEIGHT      40

// Setting 頁面按鈕區域
#define SETTING_ADD_TOP    50
#define SETTING_ADD_BOTTOM 170
#define SETTING_DEL_TOP    180
#define SETTING_DEL_BOTTOM 310


//WiFi推波設定
#define DISCORD_WEBHOOK_URL "https://discord.com/api/webhooks/1467471815801831516/vm3TmYFn-mKYoXaDZ5kdK4E4k_RnmcbSSUCYrT2hNGBr8-0sx76Tl5xaFPfRaEbxNyF-"

#define LED_R_PIN  32

#define LED_G_PIN  33

#define BUZZ_PIN   27
#define BUZZ_ACTIVE 1   // 1=有源蜂鳴器(給HIGH就叫)；0=無源(用tone)
#define UI_RESULT_HOLD_MS 800   // 成功/失敗 LED 維持時間(ms)，800=0.8秒

#endif
