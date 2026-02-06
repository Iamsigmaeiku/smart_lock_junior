#pragma once
#include <Arduino.h>
#include <TFT_eSPI.h>
#include "config.h"
#include <XPT2046_Touchscreen.h>

class Screen {
public:
  Screen();

  void init();
  void initTouch();
struct Rect { int16_t x, y, w, h; };
Rect getMenuButtonRect(uint8_t index); // 0..5
  void showWelcome();
  void showWaitingForFinger();
  void showWaitingForCard();
  void showSuccess();
  void showFailed();
  TFT_eSPI& display() { return tft; }  // ← 加這行

  // 觸控相關
  bool isTouched();                                   // 檢測是否被觸摸
  void getTouchPoint(int16_t &x, int16_t &y);         // 獲取觸摸座標（已映射到螢幕像素）
  void printTouchDebug();                             // 調試用：印出觸摸座標

  // 密碼輸入UI
  void showMainMenu();                                // 主選單（選擇驗證方式）
  void showPasswordInput();                           // 顯示密碼輸入介面
  void updatePasswordDisplay(String maskedPW);        // 更新密碼顯示
  int8_t getKeypadPress(int16_t x, int16_t y);        // 判斷按了哪個鍵（返回0-9或特殊鍵）

  // 校準測試
  void showCalibrationMarkers();                      // 顯示校準標記點

  // Header 相關
  enum HeaderTouch { HEADER_NONE = 0, HEADER_BACK = 1, HEADER_SETTING = 2 };
  void drawHeader(const char* title, bool showBack, bool showSettings);
  HeaderTouch getHeaderTouch(int16_t x, int16_t y);

  // Setting 相關頁面
  void showSettingMenu();
  int8_t getSettingMenuPress(int16_t x, int16_t y); // 返回 0-7 (0-3=add, 4-7=remove) 或 -1
  void showAddFingerprint();
  void showAddFace();
  void showAddRFID();
  void showAddPassword();
  void showPasswordKeypad(const char* title, const char* hint);
  void showRemoveMenu(uint8_t type, uint8_t* ids, uint8_t count); // 顯示刪除選單，傳入 ID 列表和數量
  int8_t getRemoveMenuPress(int16_t x, int16_t y, uint8_t count); // 檢測刪除選單的按鈕點擊，返回索引或 -1
  void showEnrollStep(uint8_t step); // 顯示指紋註冊步驟 (1=第一次按壓, 2=移開, 3=第二次按壓)

  // 輔助函數
  bool isButtonPressed(int16_t x, int16_t y, int16_t btnX, int16_t btnY,
                       int16_t btnW, int16_t btnH);   // 判斷是否按中按鈕
  void drawButton(int16_t x, int16_t y, int16_t w, int16_t h,
                  uint16_t color, const char* label, uint8_t textSize);

private:
  TFT_eSPI tft;
  XPT2046_Touchscreen ts;

  // ===== 你「以 0 為基底」的邏輯尺寸 =====
  static constexpr int16_t LOG_W = 240;
  static constexpr int16_t LOG_H = 320;

  // ===== 繪圖用：把 rotation=0 的座標/矩形，轉成目前實體 rotation=1 的座標/矩形 =====
  void mapPoint0ToPhys(int16_t x0, int16_t y0, int16_t &xp, int16_t &yp);
  void mapRect0ToPhys(int16_t x0, int16_t y0, int16_t w0, int16_t h0,
                      int16_t &xp, int16_t &yp, int16_t &wp, int16_t &hp);

  // ===== 只在 Screen 內部用的「以 0 為基底」繪圖 API =====
  void fillScreen0(uint16_t color);
  void setCursor0(int16_t x0, int16_t y0);
  void fillRoundRect0(int16_t x0, int16_t y0, int16_t w0, int16_t h0, int16_t r, uint16_t color);
  void drawRoundRect0(int16_t x0, int16_t y0, int16_t w0, int16_t h0, int16_t r, uint16_t color);

  void drawButton0(int16_t x0, int16_t y0, int16_t w0, int16_t h0,
                   uint16_t color, const String &text, bool pressed,
                   uint8_t textSize = 2);
};
