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
  tft.fillRoundRect(x, y, w, h, 8, color);
  tft.drawRoundRect(x, y, w, h, 8, WHITE);

  tft.setTextSize(textSize);
  int16_t textWidth = strlen(label) * 6 * textSize;
  int16_t textHeight = 8 * textSize;
  int16_t textX = x + (w - textWidth) / 2;
  int16_t textY = y + (h - textHeight) / 2;

  tft.setCursor(textX, textY);
  tft.setTextColor(WHITE);
  tft.print(label);
}

// ========================================
// 顯示函數
// ========================================
void Screen::showWelcome() {
  tft.fillScreen(BLACK);

  tft.fillRoundRect(20, 100, 200, 70, 10, BLUE);

  tft.setTextSize(3);
  tft.setTextColor(WHITE);
  tft.setCursor(35, 125);
  tft.print("Smart Lock");

  tft.setTextSize(2);
  tft.setTextColor(GREEN);
  tft.setCursor(25, 220);
  tft.print("Touch sensor");
  tft.setCursor(50, 245);
  tft.print("to unlock");

  Serial.println("顯示：歡迎畫面");
}

static void computeMenuLayout(int16_t W, int16_t H,
                              int16_t &marginX, int16_t &topY,
                              int16_t &gapX, int16_t &gapY,
                              int16_t &btnW, int16_t &btnH) {
  // 視覺調過：240x320 很舒服
  marginX = 12;
  topY    = 52;   // 標題下方開始
  gapX    = 12;
  gapY    = 12;

  const int16_t cols = 2;
  const int16_t rows = 3;
  const int16_t bottomMargin = 14;

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
  tft.fillScreen(BLACK);

  int16_t W = tft.width();   // 240
  int16_t H = tft.height();  // 320

  // Title：真正置中
  drawCenteredText(tft, "Smart Lock", W/2, 22, 2, WHITE);

  // Layout
  int16_t marginX, topY, gapX, gapY, btnW, btnH;
  computeMenuLayout(W, H, marginX, topY, gapX, gapY, btnW, btnH);

  // Colors + labels by index 0..5
  const uint16_t colors[6] = { RED, ORANGE, GREEN, CYAN, GRAY, GRAY };
  const char*    labels[6] = { "Finger", "RFID", "Password", "Face", "Enroll", "Setting" };
  const uint8_t  sizes[6]  = { 2, 2, 1, 2, 2, 2 };

  for (uint8_t i = 0; i < 6; i++) {
    Rect r = getMenuButtonRect(i);
    drawButton(r.x, r.y, r.w, r.h, colors[i], labels[i], sizes[i]);
  }

  Serial.println("顯示：主選單 (auto 3x2 layout)");
}


void Screen::showPasswordInput() {
  tft.fillScreen(BLACK);

  tft.setTextSize(2);
  tft.setTextColor(WHITE);
  tft.setCursor(30, 10);
  tft.print("Enter Password");

  tft.drawLine(20, 55, 220, 55, WHITE);

  const char* keys[4][3] = {
    {"1", "2", "3"},
    {"4", "5", "6"},
    {"7", "8", "9"},
    {"*", "0", "#"}
  };

  for (int row = 0; row < 4; row++) {
    for (int col = 0; col < 3; col++) {
      int16_t x = 10 + col * 75;
      int16_t y = 80 + row * 60;

      uint16_t color = BLUE;
      if (row == 3 && col == 0) color = GREEN;
      if (row == 3 && col == 2) color = RED;

      drawButton(x, y, 70, 50, color, keys[row][col], 3);
    }
  }

  Serial.println("顯示：密碼輸入介面");
}

void Screen::showChangePasswordInput(const char* title) {
  tft.fillScreen(BLACK);

  tft.setTextSize(2);
  tft.setTextColor(WHITE);
  tft.setCursor(10, 10);
  tft.print(title);

  tft.drawLine(20, 55, 220, 55, WHITE);

  const char* keys[4][3] = {
    {"1", "2", "3"},
    {"4", "5", "6"},
    {"7", "8", "9"},
    {"*", "0", "#"}
  };

  for (int row = 0; row < 4; row++) {
    for (int col = 0; col < 3; col++) {
      int16_t x = 10 + col * 75;
      int16_t y = 80 + row * 60;

      uint16_t color = BLUE;
      if (row == 3 && col == 0) color = GREEN;
      if (row == 3 && col == 2) color = RED;

      drawButton(x, y, 70, 50, color, keys[row][col], 3);
    }
  }

  Serial.print("顯示：");
  Serial.println(title);
}

Screen::Rect Screen::getMenuButtonRect(uint8_t index) {
  Rect r{0,0,0,0};
  if (index > 5) return r;

  int16_t W = tft.width();   // 240
  int16_t H = tft.height();  // 320

  int16_t marginX, topY, gapX, gapY, btnW, btnH;
  computeMenuLayout(W, H, marginX, topY, gapX, gapY, btnW, btnH);

  const int16_t cols = 2;
  int16_t row = index / cols; // 0..2
  int16_t col = index % cols; // 0..1

  r.x = marginX + col * (btnW + gapX);
  r.y = topY    + row * (btnH + gapY);
  r.w = btnW;
  r.h = btnH;
  return r;
}
void Screen::updatePasswordDisplay(String maskedPW) {
  tft.fillRect(20, 30, 200, 20, BLACK);

  tft.setTextSize(3);
  tft.setTextColor(YELLOW);
  tft.setCursor(30, 30);

  for (unsigned int i = 0; i < maskedPW.length(); i++) {
    tft.print("*");
  }

  Serial.printf("更新密碼顯示：%d 位\n", maskedPW.length());
}

void Screen::showWaitingForFinger() {
  tft.fillScreen(BLACK);

  tft.fillRoundRect(20, 110, 200, 100, 10, YELLOW);

  tft.setTextSize(2);
  tft.setTextColor(BLACK);
  tft.setCursor(30, 135);
  tft.print("Please place");
  tft.setCursor(40, 165);
  tft.print("your finger");

  Serial.println("顯示：等待指紋");
}

void Screen::showSuccess() {
  tft.fillScreen(BLACK);

  tft.fillRoundRect(20, 110, 200, 100, 10, GREEN);

  tft.setTextSize(3);
  tft.setTextColor(WHITE);
  tft.setCursor(52, 135);
  tft.print("Access");
  tft.setCursor(40, 170);
  tft.print("Granted!");

  Serial.println("顯示：驗證成功");
}

void Screen::showFailed() {
  tft.fillScreen(BLACK);

  tft.fillRoundRect(20, 110, 200, 100, 10, RED);

  tft.setTextSize(3);
  tft.setTextColor(WHITE);
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
  fillScreen0(0x0000);
  delay(10);

  tft.setTextColor(0xFFFF);
  tft.setTextSize(2);

  tft.setCursor(20, 80);
  tft.print("Enroll new card");

  tft.setTextColor(0x07FF);
  tft.setTextSize(3);

  tft.setCursor(20, 140);
  tft.print("RFID CARD");

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