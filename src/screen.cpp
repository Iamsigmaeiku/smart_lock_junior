#include "screen.h"
#include "config.h"
#include <Arduino.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

// 顏色定義 (RGB565)
#define BLACK   0x0000
#define WHITE   0xFFFF
#define RED     0xF800
#define GREEN   0x07E0
#define BLUE    0x001F
#define YELLOW  0xFFE0
#define CYAN    0x07FF
#define ORANGE  0xFD20
#define GRAY    0x8410

// Pastel 色系 - 現代化 UI
#define BG_DARK       0x18E5  // #1a1d2e - 深色背景
#define HEADER_BG     0xFFDF  // #f8f9fa - 淺色 header
#define PASTEL_PINK   0xFD97  // #ffb3ba - 指紋
#define PASTEL_ORANGE 0xFEB4  // #ffd4a3 - RFID
#define PASTEL_BLUE   0xA6BF  // #a3d4ff - 密碼
#define PASTEL_GREEN  0xBF16  // #bae1b3 - 人臉
#define PASTEL_PURPLE 0xD63F  // #d4c4fb - Setting
#define SUCCESS_GREEN 0xBF37  // #b8e6b8 - 成功
#define FAIL_PINK     0xFD97  // #ffb3ba - 失敗
#define TEXT_DARK     0x2DA9  // #2d3748 - 深色文字
#define SHADOW_LIGHT  0xD69A  // #d0d0d0 - 淺陰影


#define TOUCH_ROT   1

// 鏡像（不用就 false）
#define TOUCH_MIRROR_X false
#define TOUCH_MIRROR_Y false

// 若你不確定 TS_MIN/MAX 是否抄反：保留防呆
static inline void normalizeMinMax(int16_t &mn, int16_t &mx) {
  if (mn > mx) { int16_t t = mn; mn = mx; mx = t; }
}

// map + clamp（杜絕負號/超界外插）
static inline int16_t mapClamped(int32_t v, int16_t inMin, int16_t inMax, int16_t outMin, int16_t outMax) {
  normalizeMinMax(inMin, inMax);
  v = constrain(v, inMin, inMax);
  return (int16_t)map((long)v, (long)inMin, (long)inMax, (long)outMin, (long)outMax);
}

// 旋轉座標（針對螢幕座標 W×H）
static inline void applyRotation(int16_t &x, int16_t &y, int16_t W, int16_t H, uint8_t rot) {
  int16_t nx, ny;
  switch (rot & 3) {
    case 0: // 0°
      nx = x; ny = y; break;
    case 1: // 90° CW
      nx = H - 1 - y; ny = x; break;
    case 2: // 180°
      nx = W - 1 - x; ny = H - 1 - y; break;
    case 3: // 90° CCW
      nx = y; ny = W - 1 - x; break;
  }
  x = nx; y = ny;
}

// 建構子：TFT_eSPI 不需要在這裡塞 CS/DC/RST（這些都在 User_Setup.h）
Screen::Screen()
  : tft(),
    ts(TOUCH_CS) {
}

// ========================================
// 初始化函數
// ========================================
void Screen::init() {
  // ST7789 + TFT_eSPI
  tft.init();

  // 你想要「直立 240x320」以 0 為基底：就用 rotation 0
  tft.setRotation(0);

  // 色調修正（你前面測到 RGB/BGR 會影響）
  // 這行是針對「顏色怪 / 黃紫海藍」最常見的補救
  // 如果顏色還怪，改成 false 試一次
  tft.setSwapBytes(true);

  // 若出現反相（像負片），把這個切 true/false
  tft.invertDisplay(false);

  tft.fillScreen(BLACK);
  tft.setTextWrap(false);

  Serial.printf("TFT init done (w=%d, h=%d)\n", tft.width(), tft.height());
}

void Screen::initTouch() {
  ts.begin();

  // 觸控旋轉要跟螢幕一致（你螢幕 rotation=0 就先設 0）
  ts.setRotation(0);

  Serial.println("XPT2046 觸控初始化完成");
}

// ========================================
// UI 輔助函數
// ========================================
void Screen::drawButton(int16_t x, int16_t y, int16_t w, int16_t h,
                        uint16_t color, const char* label, uint8_t textSize) {
  // 繪製陰影（微妙的立體效果）
  tft.fillRoundRect(x + 2, y + 2, w, h, 12, SHADOW_LIGHT);
  
  // 繪製按鈕本體
  tft.fillRoundRect(x, y, w, h, 12, color);

  // 文字
  tft.setTextSize(textSize);
  int16_t textWidth = strlen(label) * 6 * textSize;
  int16_t textHeight = 8 * textSize;
  int16_t textX = x + (w - textWidth) / 2;
  int16_t textY = y + (h - textHeight) / 2;

  tft.setCursor(textX, textY);
  tft.setTextColor(TEXT_DARK);
  tft.print(label);
}

// ========================================
// 顯示函數
// ========================================
void Screen::showWelcome() {
  tft.fillScreen(BG_DARK);

  // 主標題卡片
  tft.fillRoundRect(22, 102, 200, 70, 16, SHADOW_LIGHT); // 陰影
  tft.fillRoundRect(20, 100, 200, 70, 16, PASTEL_BLUE);

  tft.setTextSize(3);
  tft.setTextColor(TEXT_DARK);
  tft.setCursor(35, 125);
  tft.print("Smart Lock");

  // 提示文字
  tft.setTextSize(2);
  tft.setTextColor(HEADER_BG);
  tft.setCursor(25, 220);
  tft.print("Touch screen");
  tft.setCursor(50, 245);
  tft.print("to begin");

  Serial.println("顯示：歡迎畫面");
}

// ========================================
// Header 組件
// ========================================
void Screen::drawHeader(const char* title, bool showBack, bool showSettings) {
  // 繪製 header 背景
  tft.fillRect(0, 0, 240, 40, HEADER_BG);
  tft.drawFastHLine(0, 40, 240, SHADOW_LIGHT); // 底部分隔線

  // 左側：Back 按鈕
  if (showBack) {
    tft.setTextSize(2);
    tft.setTextColor(TEXT_DARK);
    tft.setCursor(8, 12);
    tft.print("<");
    tft.setTextSize(1);
    tft.setCursor(22, 15);
    tft.print("Back");
  }

  // 中央：標題
  tft.setTextSize(2);
  tft.setTextColor(TEXT_DARK);
  int16_t titleWidth = strlen(title) * 12; // 估算寬度
  int16_t titleX = (240 - titleWidth) / 2;
  tft.setCursor(titleX, 12);
  tft.print(title);

  // 右側：Setting 按鈕
  if (showSettings) {
    tft.setTextSize(2);
    tft.setTextColor(TEXT_DARK);
    tft.setCursor(200, 12);
    tft.print("=");
    tft.setTextSize(1);
    tft.setCursor(165, 15);
    tft.print("Set");
  }
}

Screen::HeaderTouch Screen::getHeaderTouch(int16_t x, int16_t y) {
  if (y > 40) return HEADER_NONE; // 不在 header 區域
  
  // Back 區域：左側 0-70px
  if (x >= 0 && x <= 70) {
    return HEADER_BACK;
  }
  
  // Setting 區域：右側 160-240px
  if (x >= 160 && x <= 240) {
    return HEADER_SETTING;
  }
  
  return HEADER_NONE;
}

static void computeMenuLayout(int16_t W, int16_t H,
                              int16_t &marginX, int16_t &topY,
                              int16_t &gapX, int16_t &gapY,
                              int16_t &btnW, int16_t &btnH) {
  // 新的 2x2 佈局，header 下方開始
  marginX = 16;
  topY    = 50;   // header 之後 (40 + 10px gap)
  gapX    = 16;
  gapY    = 16;

  const int16_t cols = 2;
  const int16_t rows = 2;  // 改為 2 行
  const int16_t bottomMargin = 20;

  btnW = (W - 2 * marginX - (cols - 1) * gapX) / cols;
  btnH = (H - topY - bottomMargin - (rows - 1) * gapY) / rows;
}

static void drawCenteredText(TFT_eSPI &tft, const char* txt, int16_t cx, int16_t cy, uint8_t size, uint16_t color) {
  tft.setTextSize(size);
  tft.setTextColor(color);
  int16_t w = strlen(txt) * 6 * size;
  int16_t h = 8 * size;
  tft.setCursor(cx - w/2, cy - h/2);
  tft.print(txt);
}

void Screen::showMainMenu() {
  tft.fillScreen(BG_DARK);

  // 繪製 Header
  drawHeader("Smart Lock", false, true);

  int16_t W = tft.width();   // 240
  int16_t H = tft.height();  // 320

  // Layout (2x2)
  int16_t marginX, topY, gapX, gapY, btnW, btnH;
  computeMenuLayout(W, H, marginX, topY, gapX, gapY, btnW, btnH);

  // Colors + labels by index 0..3 (只有 4 個按鈕)
  const uint16_t colors[4] = { PASTEL_PINK, PASTEL_ORANGE, PASTEL_BLUE, PASTEL_GREEN };
  const char*    labels[4] = { "Finger", "RFID", "Password", "Face" };
  const char*    icons[4]  = { "F", "R", "P", "A" }; // 大圖標字母
  const uint8_t  textSize  = 2;

  for (uint8_t i = 0; i < 4; i++) {
    Rect r = getMenuButtonRect(i);
    
    // 繪製按鈕背景（帶陰影效果）
    tft.fillRoundRect(r.x+2, r.y+2, r.w, r.h, 16, SHADOW_LIGHT); // 陰影
    tft.fillRoundRect(r.x, r.y, r.w, r.h, 16, colors[i]);
    
    // 繪製大圖標（字母）
    tft.setTextSize(4);
    tft.setTextColor(TEXT_DARK);
    int16_t iconW = strlen(icons[i]) * 24;
    int16_t iconX = r.x + (r.w - iconW) / 2;
    int16_t iconY = r.y + 30;
    tft.setCursor(iconX, iconY);
    tft.print(icons[i]);
    
    // 繪製功能名稱
    tft.setTextSize(1);
    tft.setTextColor(TEXT_DARK);
    int16_t labelW = strlen(labels[i]) * 6;
    int16_t labelX = r.x + (r.w - labelW) / 2;
    int16_t labelY = r.y + r.h - 20;
    tft.setCursor(labelX, labelY);
    tft.print(labels[i]);
  }

  Serial.println("顯示：主選單 (2x2 layout with header)");
}

void Screen::showSettingMenu() {
  tft.fillScreen(BG_DARK);

  // 繪製 Header
  drawHeader("Settings", true, false);

  // 上半部：新增區塊
  const int16_t addTop = 50;
  const int16_t addHeight = 120;
  
  // 區塊背景
  tft.fillRoundRect(10, addTop, 220, addHeight, 12, HEADER_BG);
  
  // 標題
  tft.setTextSize(2);
  tft.setTextColor(TEXT_DARK);
  tft.setCursor(15, addTop + 8);
  tft.print("Add New");
  
  // 4 個小按鈕 (2x2)
  const int16_t btnStartY = addTop + 35;
  const int16_t btnW = 95;
  const int16_t btnH = 35;
  const int16_t gapX = 10;
  const int16_t gapY = 8;
  const char* addLabels[4] = { "+ Finger", "+ Face", "+ RFID", "+ Pass" };
  const uint16_t addColors[4] = { PASTEL_PINK, PASTEL_GREEN, PASTEL_ORANGE, PASTEL_BLUE };
  
  for (uint8_t i = 0; i < 4; i++) {
    int16_t col = i % 2;
    int16_t row = i / 2;
    int16_t x = 20 + col * (btnW + gapX);
    int16_t y = btnStartY + row * (btnH + gapY);
    
    // 按鈕
    tft.fillRoundRect(x, y, btnW, btnH, 8, addColors[i]);
    
    // 文字
    tft.setTextSize(1);
    tft.setTextColor(TEXT_DARK);
    int16_t labelW = strlen(addLabels[i]) * 6;
    int16_t labelX = x + (btnW - labelW) / 2;
    int16_t labelY = y + (btnH - 8) / 2;
    tft.setCursor(labelX, labelY);
    tft.print(addLabels[i]);
  }
  
  // 下半部：刪除區塊
  const int16_t delTop = 180;
  const int16_t delHeight = 120;
  
  // 區塊背景（稍深）
  tft.fillRoundRect(10, delTop, 220, delHeight, 12, SHADOW_LIGHT);
  
  // 標題
  tft.setTextSize(2);
  tft.setTextColor(TEXT_DARK);
  tft.setCursor(15, delTop + 8);
  tft.print("Remove");
  
  // 4 個小按鈕 (2x2)
  const int16_t delBtnStartY = delTop + 35;
  const char* delLabels[4] = { "- Finger", "- Face", "- RFID", "- Pass" };
  const uint16_t delColors[4] = { FAIL_PINK, 0xFFAA, 0xFFC0, 0xFFD0 }; // 較淺的色調
  
  for (uint8_t i = 0; i < 4; i++) {
    int16_t col = i % 2;
    int16_t row = i / 2;
    int16_t x = 20 + col * (btnW + gapX);
    int16_t y = delBtnStartY + row * (btnH + gapY);
    
    // 按鈕
    tft.fillRoundRect(x, y, btnW, btnH, 8, delColors[i]);
    
    // 文字
    tft.setTextSize(1);
    tft.setTextColor(TEXT_DARK);
    int16_t labelW = strlen(delLabels[i]) * 6;
    int16_t labelX = x + (btnW - labelW) / 2;
    int16_t labelY = y + (btnH - 8) / 2;
    tft.setCursor(labelX, labelY);
    tft.print(delLabels[i]);
  }
  
  Serial.println("顯示：Setting 主頁面");
}

int8_t Screen::getSettingMenuPress(int16_t x, int16_t y) {
  const int16_t btnW = 95;
  const int16_t btnH = 35;
  const int16_t gapX = 10;
  const int16_t gapY = 8;
  
  // 檢查新增區塊 (y: 85-163)
  const int16_t addStartY = 85;
  if (y >= addStartY && y <= addStartY + 2 * btnH + gapY) {
    for (uint8_t i = 0; i < 4; i++) {
      int16_t col = i % 2;
      int16_t row = i / 2;
      int16_t btnX = 20 + col * (btnW + gapX);
      int16_t btnY = addStartY + row * (btnH + gapY);
      
      if (x >= btnX && x <= btnX + btnW && y >= btnY && y <= btnY + btnH) {
        return i; // 返回 0-3
      }
    }
  }
  
  // 檢查刪除區塊 (y: 215-293)
  const int16_t delStartY = 215;
  if (y >= delStartY && y <= delStartY + 2 * btnH + gapY) {
    for (uint8_t i = 0; i < 4; i++) {
      int16_t col = i % 2;
      int16_t row = i / 2;
      int16_t btnX = 20 + col * (btnW + gapX);
      int16_t btnY = delStartY + row * (btnH + gapY);
      
      if (x >= btnX && x <= btnX + btnW && y >= btnY && y <= btnY + btnH) {
        return i + 4; // 返回 4-7
      }
    }
  }
  
  return -1; // 沒有按到
}

void Screen::showAddFingerprint() {
  tft.fillScreen(BG_DARK);
  drawHeader("Add Finger", true, false);
  
  // 主內容卡片
  tft.fillRoundRect(20, 70, 200, 160, 16, PASTEL_PINK);
  
  // 大圖標
  tft.setTextSize(5);
  tft.setTextColor(TEXT_DARK);
  tft.setCursor(90, 100);
  tft.print("F");
  
  // 提示文字
  tft.setTextSize(2);
  tft.setTextColor(TEXT_DARK);
  tft.setCursor(30, 160);
  tft.print("Place your");
  tft.setCursor(40, 185);
  tft.print("finger on");
  tft.setCursor(50, 210);
  tft.print("sensor");
  
  Serial.println("顯示：新增指紋");
}

void Screen::showEnrollStep(uint8_t step) {
  tft.fillScreen(BG_DARK);
  drawHeader("Add Finger", true, false);
  
  // 主內容卡片
  tft.fillRoundRect(20, 70, 200, 160, 16, PASTEL_PINK);
  
  // 大圖標
  tft.setTextSize(5);
  tft.setTextColor(TEXT_DARK);
  tft.setCursor(90, 100);
  tft.print("F");
  
  // 根據步驟顯示不同提示
  tft.setTextSize(2);
  tft.setTextColor(TEXT_DARK);
  
  if (step == 1) {
    // 第一次按壓
    tft.setCursor(30, 160);
    tft.print("Press your");
    tft.setCursor(40, 185);
    tft.print("finger");
    tft.setCursor(50, 210);
    tft.print("(1/2)");
  } else if (step == 2) {
    // 移開手指
    tft.setCursor(35, 160);
    tft.print("Remove");
    tft.setCursor(40, 185);
    tft.print("finger");
  } else if (step == 3) {
    // 第二次按壓
    tft.setCursor(30, 160);
    tft.print("Press your");
    tft.setCursor(40, 185);
    tft.print("finger");
    tft.setCursor(50, 210);
    tft.print("(2/2)");
  }
  
  Serial.printf("顯示：註冊步驟 %d\n", step);
}

void Screen::showAddFace() {
  tft.fillScreen(BG_DARK);
  drawHeader("Add Face", true, false);
  
  // 主內容卡片
  tft.fillRoundRect(20, 70, 200, 160, 16, PASTEL_GREEN);
  
  // 大圖標
  tft.setTextSize(5);
  tft.setTextColor(TEXT_DARK);
  tft.setCursor(90, 100);
  tft.print("A");
  
  // 提示文字
  tft.setTextSize(2);
  tft.setTextColor(TEXT_DARK);
  tft.setCursor(35, 160);
  tft.print("Look at the");
  tft.setCursor(50, 185);
  tft.print("camera");
  
  Serial.println("顯示：新增人臉");
}

void Screen::showAddRFID() {
  tft.fillScreen(BG_DARK);
  drawHeader("Add RFID", true, false);
  
  // 主內容卡片
  tft.fillRoundRect(20, 70, 200, 160, 16, PASTEL_ORANGE);
  
  // 大圖標
  tft.setTextSize(5);
  tft.setTextColor(TEXT_DARK);
  tft.setCursor(90, 100);
  tft.print("R");
  
  // 提示文字
  tft.setTextSize(2);
  tft.setTextColor(TEXT_DARK);
  tft.setCursor(50, 160);
  tft.print("Present");
  tft.setCursor(45, 185);
  tft.print("your card");
  tft.setCursor(45, 210);
  tft.print("to reader");
  
  Serial.println("顯示：新增 RFID 卡");
}

void Screen::showAddPassword() {
  tft.fillScreen(BG_DARK);
  drawHeader("Add Password", true, false);
  
  // 主內容卡片
  tft.fillRoundRect(20, 70, 200, 100, 16, PASTEL_BLUE);
  
  // 提示文字
  tft.setTextSize(2);
  tft.setTextColor(TEXT_DARK);
  tft.setCursor(30, 90);
  tft.print("Enter new");
  tft.setCursor(40, 115);
  tft.print("password");
  tft.setCursor(50, 140);
  tft.print("below:");
  
  // 密碼輸入區域（簡化版鍵盤）
  const char* keys[4][3] = {
    {"1", "2", "3"},
    {"4", "5", "6"},
    {"7", "8", "9"},
    {"C", "0", "OK"}
  };
  
  for (int row = 0; row < 4; row++) {
    for (int col = 0; col < 3; col++) {
      int16_t x = 25 + col * 65;
      int16_t y = 185 + row * 30;
      
      uint16_t color = PASTEL_BLUE;
      if (row == 3 && col == 0) color = FAIL_PINK;
      if (row == 3 && col == 2) color = SUCCESS_GREEN;
      
      tft.fillRoundRect(x, y, 60, 25, 6, color);
      
      tft.setTextSize(2);
      tft.setTextColor(TEXT_DARK);
      int16_t textW = strlen(keys[row][col]) * 12;
      int16_t textX = x + (60 - textW) / 2;
      int16_t textY = y + 5;
      tft.setCursor(textX, textY);
      tft.print(keys[row][col]);
    }
  }
  
  Serial.println("顯示：新增密碼");
}

void Screen::showRemoveMenu(uint8_t type) {
  tft.fillScreen(BG_DARK);
  
  const char* titles[4] = { "Remove Finger", "Remove Face", "Remove RFID", "Remove Pass" };
  const uint16_t colors[4] = { PASTEL_PINK, PASTEL_GREEN, PASTEL_ORANGE, PASTEL_BLUE };
  const char* icons[4] = { "F", "A", "R", "P" };
  
  if (type > 3) type = 0;
  
  drawHeader(titles[type], true, false);
  
  // 主內容卡片
  tft.fillRoundRect(20, 70, 200, 180, 16, colors[type]);
  
  // 大圖標
  tft.setTextSize(5);
  tft.setTextColor(TEXT_DARK);
  tft.setCursor(90, 90);
  tft.print(icons[type]);
  
  // 提示文字
  tft.setTextSize(2);
  tft.setTextColor(TEXT_DARK);
  tft.setCursor(40, 150);
  tft.print("Select ID");
  tft.setCursor(45, 175);
  tft.print("to remove:");
  
  // 示例列表（簡化）
  tft.setTextSize(1);
  tft.setCursor(30, 205);
  tft.print("ID 1  ID 2  ID 3");
  tft.setCursor(30, 225);
  tft.print("[Tap to remove]");
  
  Serial.printf("顯示：刪除選單 (type=%d)\n", type);
}


void Screen::showPasswordInput() {
  tft.fillScreen(BG_DARK);
  drawHeader("Password", true, false);

  // 密碼顯示區域
  tft.fillRoundRect(20, 50, 200, 30, 8, HEADER_BG);
  tft.setTextSize(2);
  tft.setTextColor(TEXT_DARK);
  tft.setCursor(25, 58);
  tft.print("Enter Password");

  const char* keys[4][3] = {
    {"1", "2", "3"},
    {"4", "5", "6"},
    {"7", "8", "9"},
    {"*", "0", "#"}
  };

  for (int row = 0; row < 4; row++) {
    for (int col = 0; col < 3; col++) {
      int16_t x = 10 + col * 75;
      int16_t y = 95 + row * 55;

      uint16_t color = PASTEL_BLUE;
      if (row == 3 && col == 0) color = SUCCESS_GREEN;  // *
      if (row == 3 && col == 2) color = FAIL_PINK;      // #

      // 陰影
      tft.fillRoundRect(x + 2, y + 2, 70, 50, 10, SHADOW_LIGHT);
      // 按鈕
      tft.fillRoundRect(x, y, 70, 50, 10, color);
      
      // 文字
      tft.setTextSize(3);
      tft.setTextColor(TEXT_DARK);
      int16_t textW = strlen(keys[row][col]) * 18;
      int16_t textX = x + (70 - textW) / 2;
      int16_t textY = y + 15;
      tft.setCursor(textX, textY);
      tft.print(keys[row][col]);
    }
  }

  Serial.println("顯示：密碼輸入介面");
}

Screen::Rect Screen::getMenuButtonRect(uint8_t index) {
  Rect r{0,0,0,0};
  if (index > 3) return r;  // 現在只有 4 個按鈕 (0-3)

  int16_t W = tft.width();   // 240
  int16_t H = tft.height();  // 320

  int16_t marginX, topY, gapX, gapY, btnW, btnH;
  computeMenuLayout(W, H, marginX, topY, gapX, gapY, btnW, btnH);

  const int16_t cols = 2;
  int16_t row = index / cols; // 0..1 (2 行)
  int16_t col = index % cols; // 0..1

  r.x = marginX + col * (btnW + gapX);
  r.y = topY    + row * (btnH + gapY);
  r.w = btnW;
  r.h = btnH;
  return r;
}
void Screen::updatePasswordDisplay(String maskedPW) {
  // 清除密碼顯示區域
  tft.fillRoundRect(20, 50, 200, 30, 8, HEADER_BG);

  tft.setTextSize(3);
  tft.setTextColor(TEXT_DARK);
  
  // 計算置中位置
  int16_t numStars = maskedPW.length();
  int16_t totalWidth = numStars * 18; // 每個 * 約 18px
  int16_t startX = 120 - totalWidth / 2; // 在 120px 處置中
  
  tft.setCursor(startX, 55);
  for (unsigned int i = 0; i < maskedPW.length(); i++) {
    tft.print("*");
  }

  Serial.printf("更新密碼顯示：%d 位\n", maskedPW.length());
}

void Screen::showWaitingForFinger() {
  tft.fillScreen(BG_DARK);
  drawHeader("Fingerprint", true, false);

  // 主內容卡片（使用 pastel 粉紅）
  tft.fillRoundRect(22, 112, 200, 100, 16, SHADOW_LIGHT); // 陰影
  tft.fillRoundRect(20, 110, 200, 100, 16, PASTEL_PINK);

  tft.setTextSize(2);
  tft.setTextColor(TEXT_DARK);
  tft.setCursor(30, 135);
  tft.print("Please place");
  tft.setCursor(40, 165);
  tft.print("your finger");

  Serial.println("顯示：等待指紋");
}

void Screen::showSuccess() {
  tft.fillScreen(BG_DARK);

  // 大圓角卡片帶陰影
  tft.fillRoundRect(22, 112, 200, 100, 20, SHADOW_LIGHT); // 陰影
  tft.fillRoundRect(20, 110, 200, 100, 20, SUCCESS_GREEN);

  tft.setTextSize(3);
  tft.setTextColor(TEXT_DARK);
  tft.setCursor(52, 135);
  tft.print("Access");
  tft.setCursor(40, 170);
  tft.print("Granted!");

  Serial.println("顯示：驗證成功");
}

void Screen::showFailed() {
  tft.fillScreen(BG_DARK);

  // 大圓角卡片帶陰影
  tft.fillRoundRect(22, 112, 200, 100, 20, SHADOW_LIGHT); // 陰影
  tft.fillRoundRect(20, 110, 200, 100, 20, FAIL_PINK);

  tft.setTextSize(3);
  tft.setTextColor(TEXT_DARK);
  tft.setCursor(52, 135);
  tft.print("Access");
  tft.setTextSize(3);
  tft.setCursor(50, 170);
  tft.print("Denied!");

  Serial.println("顯示：驗證失敗");
}

// ========================================
// 觸控函數
// ========================================
bool Screen::isTouched() {
  return ts.touched();
}

void Screen::getTouchPoint(int16_t &x, int16_t &y) {
  x = -1;
  y = -1;
  if (!ts.touched()) return;

  TS_Point p = ts.getPoint();

  const int16_t W = 240;  // ST7789 實際寬
  const int16_t H = 320;  // ST7789 實際高

  // 防呆（raw 超界）
  int16_t rx = constrain(p.x, TS_MINX, TS_MAXX);
  int16_t ry = constrain(p.y, TS_MINY, TS_MAXY);

  // raw → 螢幕座標（左上原點）
  x = map(rx, TS_MINX, TS_MAXX, 0, W);
  y = map(ry, TS_MINY, TS_MAXY, 0, H);
}



void Screen::printTouchDebug() {
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    int16_t x, y;
    getTouchPoint(x, y);
    Serial.printf("原始: X=%d Y=%d | 映射: X=%d Y=%d\n", p.x, p.y, x, y);
  }
}

int8_t Screen::getKeypadPress(int16_t x, int16_t y) {
  if (y < 80 || y > 310 || x < 10 || x > 235) return -1;

  int col = (x - 10) / 75;
  int row = (y - 80) / 60;

  if (col < 0 || col > 2 || row < 0 || row > 3) return -1;

  if (row == 0) return col + 1;
  if (row == 1) return col + 4;
  if (row == 2) return col + 7;
  if (row == 3) {
    if (col == 0) return 11;
    if (col == 1) return 0;
    if (col == 2) return 10;
  }
  return -1;
}

// ========================================
// 輔助函數
// ========================================
bool Screen::isButtonPressed(int16_t x, int16_t y,
                             int16_t btnX, int16_t btnY,
                             int16_t btnW, int16_t btnH) {
return (x >= btnX && x <= btnX + btnW &&
        y >= btnY && y <= btnY + btnH);
}

// ========================================
// 座標映射函數（你原本的維持）
// ========================================
void Screen::mapPoint0ToPhys(int16_t x0, int16_t y0, int16_t &xp, int16_t &yp) {
  xp = y0;
  yp = (LOG_W - 1) - x0;
}

void Screen::mapRect0ToPhys(int16_t x0, int16_t y0, int16_t w0, int16_t h0,
                            int16_t &xp, int16_t &yp, int16_t &wp, int16_t &hp) {
  int16_t x1 = x0;
  int16_t y1 = y0;
  int16_t x2 = x0 + w0 - 1;
  int16_t y2 = y0 + h0 - 1;

  int16_t p1x, p1y, p2x, p2y, p3x, p3y, p4x, p4y;
  mapPoint0ToPhys(x1, y1, p1x, p1y);
  mapPoint0ToPhys(x2, y1, p2x, p2y);
  mapPoint0ToPhys(x1, y2, p3x, p3y);
  mapPoint0ToPhys(x2, y2, p4x, p4y);

  int16_t minX = min(min(p1x, p2x), min(p3x, p4x));
  int16_t maxX = max(max(p1x, p2x), max(p3x, p4x));
  int16_t minY = min(min(p1y, p2y), min(p3y, p4y));
  int16_t maxY = max(max(p1y, p2y), max(p3y, p4y));

  xp = minX;
  yp = minY;
  wp = maxX - minX + 1;
  hp = maxY - minY + 1;
}

// ========================================
// 「以 0 為基底」的繪圖 wrapper
// ========================================
void Screen::fillScreen0(uint16_t color) {
  tft.fillScreen(color);
}

void Screen::setCursor0(int16_t x0, int16_t y0) {
  int16_t xp, yp;
  mapPoint0ToPhys(x0, y0, xp, yp);
  tft.setCursor(xp, yp);
}

void Screen::fillRoundRect0(int16_t x0, int16_t y0, int16_t w0, int16_t h0, int16_t r, uint16_t color) {
  int16_t xp, yp, wp, hp;
  mapRect0ToPhys(x0, y0, w0, h0, xp, yp, wp, hp);
  tft.fillRoundRect(xp, yp, wp, hp, r, color);
}

void Screen::drawRoundRect0(int16_t x0, int16_t y0, int16_t w0, int16_t h0, int16_t r, uint16_t color) {
  int16_t xp, yp, wp, hp;
  mapRect0ToPhys(x0, y0, w0, h0, xp, yp, wp, hp);
  tft.drawRoundRect(xp, yp, wp, hp, r, color);
}

void Screen::drawButton0(int16_t x0, int16_t y0, int16_t w0, int16_t h0,
                         uint16_t color, const String &text, bool pressed,
                         uint8_t textSize) {
  const int16_t r = 10;
  uint16_t fill = pressed ? (color >> 2) : (color >> 3);

  fillRoundRect0(x0, y0, w0, h0, r, fill);
  drawRoundRect0(x0, y0, w0, h0, r, color);

  tft.setTextColor(0xFFFF);
  tft.setTextSize(textSize);

  int16_t textW = text.length() * (6 * textSize);
  int16_t textH = 8 * textSize;

  int16_t tx0 = x0 + (w0 - textW) / 2;
  int16_t ty0 = y0 + (h0 - textH) / 2;

  setCursor0(tx0, ty0);
  tft.print(text);
}

// ========================================
// 更多顯示函數
// ========================================
void Screen::showWaitingForCard() {
  tft.fillScreen(BG_DARK);
  drawHeader("RFID Card", true, false);

  // 主內容卡片（使用 pastel 橙色）
  tft.fillRoundRect(22, 112, 200, 120, 16, SHADOW_LIGHT); // 陰影
  tft.fillRoundRect(20, 110, 200, 120, 16, PASTEL_ORANGE);

  tft.setTextSize(2);
  tft.setTextColor(TEXT_DARK);
  tft.setCursor(40, 130);
  tft.print("Present");
  
  tft.setTextSize(3);
  tft.setCursor(25, 160);
  tft.print("RFID Card");
  
  tft.setTextSize(2);
  tft.setCursor(45, 195);
  tft.print("to reader");

  Serial.println("顯示：等待 RFID 卡片");
}

// ========================================
// 校準測試
// ========================================
void Screen::showCalibrationMarkers() {
  tft.fillScreen(BLACK);

  tft.drawFastHLine(10, 20, 20, RED);
  tft.drawFastVLine(20, 10, 20, RED);

  tft.drawFastHLine(210, 20, 20, RED);
  tft.drawFastVLine(220, 10, 20, RED);

  tft.drawFastHLine(10, 300, 20, RED);
  tft.drawFastVLine(20, 290, 20, RED);

  tft.drawFastHLine(210, 300, 20, RED);
  tft.drawFastVLine(220, 290, 20, RED);

  tft.fillCircle(120, 160, 5, GREEN);

  tft.setTextSize(1);
  tft.setTextColor(WHITE);
  tft.setCursor(40, 150);
  tft.print("Touch markers for calibration");

  Serial.println("顯示：校準標記點");
}