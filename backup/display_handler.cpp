/*
 * TFT顯示器控制器 - 實作檔案
 * 
 * 參考文檔:
 * - ST7789V2 Datasheet Section 8.4: SPI Interface
 * - ST7789V2 Section 9: Command Table
 * - ST7789V2 Section 13: Pixel Format
 * - TFT_eSPI Library: https://github.com/Bodmer/TFT_eSPI
 * 
 * 本檔案包含ILI9341/ST7789顯示器的所有函式實作骨架
 * 搭配詳細的datasheet章節標註與UI設計範例
 */

#include "display_handler.h"

// ============================================================================
// 建構函式與解構函式
// ============================================================================

/*
 * 建構函式
 * 
 * 初始化TFT_eSPI物件與成員變數
 * TFT_eSPI的配置已在platformio.ini的build_flags中定義
 */
DisplayHandler::DisplayHandler() 
    : currentBrightness(255),
      isSleeping(false)
{
    // TFT_eSPI物件初始化
    // 驅動設定已由build_flags指定，無需額外配置
}

/*
 * 解構函式
 * 
 * 釋放資源，關閉顯示器
 */
DisplayHandler::~DisplayHandler() {
    if (!isSleeping) {
        sleep();  // 關閉顯示節省電力
    }
}

// ============================================================================
// 公開函式實作
// ============================================================================

/*
 * 初始化TFT顯示器
 * 
 * ST7789V2 Datasheet參考:
 * - Section 8.5: Reset Timing
 *   - 硬體重置: RST Low ≥10μs -> RST High -> 等待5ms
 * - Section 9.1: SWRESET (0x01) - 軟體重置
 * - Section 9.2: SLPOUT (0x11) - 退出睡眠模式
 *   - 執行後需等待120ms
 * - Section 9.17: DISPON (0x29) - 啟動顯示
 * - Section 9.20: COLMOD (0x3A) - 設定像素格式
 *   - 0x55: RGB565 (16-bit/pixel, 65K colors)
 * 
 * TFT_eSPI初始化流程:
 * 1. tft.init(): 執行硬體重置與基本初始化
 * 2. tft.setRotation(): 設定顯示方向 (0~3)
 * 3. tft.fillScreen(): 清除螢幕內容
 * 
 * @return true: 初始化成功 | false: 初始化失敗
 * 
 * TODO:
 * 1. 呼叫tft.init()執行初始化
 * 2. 設定螢幕旋轉方向
 * 3. 清除螢幕為黑色
 * 4. 顯示測試圖案驗證功能(選用)
 * 5. 加入錯誤處理
 */
bool DisplayHandler::init() {
    Serial.println("[Display] 初始化TFT顯示器...");
    
    // TODO: 初始化TFT_eSPI
    // tft.init();
    
    // TODO: 設定螢幕旋轉
    // 0: 0度 (豎屏)
    // 1: 90度 (橫屏)
    // 2: 180度 (豎屏翻轉)
    // 3: 270度 (橫屏翻轉)
    // tft.setRotation(0);
    
    // TODO: 清除螢幕
    // tft.fillScreen(COLOR_BLACK);
    
    // TODO: 顯示初始化完成訊息 (選用)
    // tft.setTextColor(COLOR_WHITE);
    // tft.setTextSize(2);
    // tft.setCursor(10, 10);
    // tft.println("Display Ready");
    
    isSleeping = false;
    Serial.println("[Display] 初始化完成");
    
    return true;  // TODO: 根據實際結果返回
}

/*
 * 清除螢幕
 * 
 * ST7789V2 Section 13.5: Frame Memory Write (RAMWR, 0x2C)
 * 
 * RGB565色彩編碼:
 * - 16位元色彩: RRRRRGGGGGGBBBBB
 * - 紅色5位元: 0~31
 * - 綠色6位元: 0~63 (人眼對綠色敏感)
 * - 藍色5位元: 0~31
 * - 總共65536種顏色
 * 
 * 常用顏色值:
 * - 黑色: 0x0000
 * - 白色: 0xFFFF
 * - 紅色: 0xF800
 * - 綠色: 0x07E0
 * - 藍色: 0x001F
 * 
 * @param color 填充顏色 (RGB565格式)
 * 
 * TODO:
 * 1. 呼叫tft.fillScreen(color)
 * 2. 可選: 加入填充動畫效果
 */
void DisplayHandler::clear(uint16_t color) {
    // TODO: 清除螢幕
    // tft.fillScreen(color);
}

/*
 * 顯示歡迎畫面
 * 
 * UI設計建議:
 * - 標題: "智慧門鎖系統" (置中，大字型)
 * - 副標題: "Smart Lock v1.0" (小字型)
 * - 圖示: 鎖頭圖示
 * - 背景: 深色背景 + 淺色文字 (護眼)
 * - 版面配置: 標題在上、圖示居中、副標題在下
 * 
 * TFT_eSPI文字函式:
 * - setTextColor(fg, bg): 設定文字與背景顏色
 * - setTextSize(size): 設定文字大小 (1~7)
 * - setTextDatum(datum): 設定對齊方式
 *   - TL_DATUM: 左上對齊
 *   - TC_DATUM: 頂部置中
 *   - MC_DATUM: 中央置中
 * - drawString(text, x, y): 繪製字串
 * 
 * TFT_eSPI圖形函式:
 * - drawRect(x, y, w, h, color): 繪製矩形
 * - fillRect(x, y, w, h, color): 填充矩形
 * - drawCircle(x, y, r, color): 繪製圓形
 * - fillCircle(x, y, r, color): 填充圓形
 * 
 * TODO:
 * 1. 清除螢幕為深色背景
 * 2. 繪製標題文字 (置中對齊)
 * 3. 繪製鎖頭圖示 (使用幾何圖形)
 * 4. 繪製副標題與版本資訊
 * 5. 可選: 加入淡入動畫效果
 */
void DisplayHandler::showWelcome() {
    // TODO: 清除螢幕
    // clear(COLOR_BLACK);
    
    // TODO: 繪製標題
    // tft.setTextColor(COLOR_WHITE);
    // tft.setTextDatum(TC_DATUM);  // 頂部置中
    // tft.setTextSize(3);
    // tft.drawString("智慧門鎖系統", TFT_WIDTH/2, 40);
    
    // TODO: 繪製鎖頭圖示
    // uint16_t iconX = TFT_WIDTH / 2;
    // uint16_t iconY = TFT_HEIGHT / 2;
    // drawLockIcon(iconX, iconY, 50, true);
    
    // TODO: 繪製副標題
    // tft.setTextSize(2);
    // tft.setTextColor(COLOR_GRAY);
    // tft.drawString("Smart Lock v1.0", TFT_WIDTH/2, TFT_HEIGHT - 60);
    
    Serial.println("[Display] 顯示歡迎畫面");
}

/*
 * 顯示RFID掃描畫面
 * 
 * UI元素:
 * - 提示文字: "請感應卡片" (大字)
 * - RFID圖示: 卡片圖形 + 無線訊號波紋
 * - 狀態提示: "等待中..." (小字、閃爍)
 * 
 * 色彩配置:
 * - 主色調: 藍色 (0x001F) 或青色 (0x07FF)
 * - 背景: 深灰色 (0x4208)
 * - 文字: 白色或淺灰
 * 
 * 動畫效果 (進階):
 * - 掃描線由上至下移動
 * - 卡片圖示脈衝縮放
 * - 文字閃爍提示
 * 
 * TODO:
 * 1. 清除螢幕
 * 2. 繪製RFID卡片圖示
 * 3. 顯示提示文字
 * 4. 可選: 加入動畫效果
 */
void DisplayHandler::showRFIDScanning() {
    // TODO: 清除螢幕
    // clear(COLOR_DARKGRAY);
    
    // TODO: 繪製RFID卡片圖示
    // drawRFIDIcon(TFT_WIDTH/2, TFT_HEIGHT/2 - 40, 60);
    
    // TODO: 顯示提示文字
    // tft.setTextColor(COLOR_CYAN);
    // tft.setTextDatum(MC_DATUM);
    // tft.setTextSize(3);
    // tft.drawString("請感應卡片", TFT_WIDTH/2, TFT_HEIGHT/2 + 60);
    
    // TODO: 顯示狀態
    // tft.setTextSize(2);
    // tft.setTextColor(COLOR_GRAY);
    // tft.drawString("等待中...", TFT_WIDTH/2, TFT_HEIGHT - 40);
    
    Serial.println("[Display] 顯示RFID掃描畫面");
}

/*
 * 顯示指紋掃描畫面
 * 
 * UI元素:
 * - 提示文字: "請按壓指紋"
 * - 指紋圖示: 指紋紋路圖形
 * - 進度提示: "偵測中..." (閃爍)
 * 
 * 色彩配置:
 * - 主色調: 綠色 (0x07E0) 或黃色 (0xFFE0)
 * - 背景: 深色
 * - 指紋圖示: 使用同心圓或橢圓模擬紋路
 * 
 * TODO:
 * 1. 清除螢幕
 * 2. 繪製指紋圖示
 * 3. 顯示提示文字
 * 4. 可選: 加入掃描動畫
 */
void DisplayHandler::showFingerprintScanning() {
    // TODO: 清除螢幕
    // clear(COLOR_DARKGRAY);
    
    // TODO: 繪製指紋圖示
    // drawFingerprintIcon(TFT_WIDTH/2, TFT_HEIGHT/2 - 40, 60);
    
    // TODO: 顯示提示文字
    // tft.setTextColor(COLOR_YELLOW);
    // tft.setTextDatum(MC_DATUM);
    // tft.setTextSize(3);
    // tft.drawString("請按壓指紋", TFT_WIDTH/2, TFT_HEIGHT/2 + 60);
    
    // TODO: 顯示狀態
    // tft.setTextSize(2);
    // tft.setTextColor(COLOR_GRAY);
    // tft.drawString("偵測中...", TFT_WIDTH/2, TFT_HEIGHT - 40);
    
    Serial.println("[Display] 顯示指紋掃描畫面");
}

/*
 * 顯示驗證成功畫面
 * 
 * UI元素:
 * - 大型勾勾圖示 ✓ (綠色)
 * - 主文字: "驗證成功" (綠色、大字)
 * - 副文字: "門鎖已開啟" (白色)
 * - 倒數計時: "5秒後自動上鎖" (灰色、小字)
 * - 使用者名稱: 顯示授權使用者 (選用)
 * 
 * 色彩配置:
 * - 主色調: 綠色 (0x07E0)
 * - 背景: 白色或淺綠色
 * - 圖示: 粗線條勾勾
 * 
 * 動畫效果 (進階):
 * - 勾勾圖示由小變大 (縮放動畫)
 * - 文字淡入效果
 * - 倒數計時數字更新
 * 
 * @param userName 使用者名稱 (選用，nullptr表示不顯示)
 * @param remainTime 剩餘開鎖時間 (秒)
 * 
 * TODO:
 * 1. 清除螢幕為淺色背景
 * 2. 繪製大型綠色勾勾
 * 3. 顯示成功訊息
 * 4. 顯示倒數計時
 * 5. 可選: 顯示使用者名稱
 * 6. 可選: 加入動畫效果
 */
void DisplayHandler::showUnlocked(const char* userName, uint16_t remainTime) {
    // TODO: 清除螢幕為淺色
    // clear(COLOR_WHITE);
    
    // TODO: 繪製勾勾圖示
    // drawCheckmark(TFT_WIDTH/2, TFT_HEIGHT/2 - 60, 80, COLOR_GREEN);
    
    // TODO: 顯示成功文字
    // tft.setTextColor(COLOR_GREEN);
    // tft.setTextDatum(MC_DATUM);
    // tft.setTextSize(4);
    // tft.drawString("驗證成功", TFT_WIDTH/2, TFT_HEIGHT/2 + 40);
    
    // TODO: 顯示副標題
    // tft.setTextSize(2);
    // tft.setTextColor(COLOR_BLACK);
    // tft.drawString("門鎖已開啟", TFT_WIDTH/2, TFT_HEIGHT/2 + 80);
    
    // TODO: 顯示倒數計時
    // if (remainTime > 0) {
    //     String countdownText = String(remainTime) + "秒後自動上鎖";
    //     tft.setTextSize(2);
    //     tft.setTextColor(COLOR_GRAY);
    //     tft.drawString(countdownText, TFT_WIDTH/2, TFT_HEIGHT - 40);
    // }
    
    // TODO: 顯示使用者名稱 (選用)
    // if (userName != nullptr) {
    //     tft.setTextSize(2);
    //     tft.setTextColor(COLOR_GRAY);
    //     tft.drawString(userName, TFT_WIDTH/2, 30);
    // }
    
    Serial.println("[Display] 顯示驗證成功畫面");
}

/*
 * 顯示驗證失敗畫面
 * 
 * UI元素:
 * - 大型叉叉圖示 ✗ (紅色)
 * - 主文字: "驗證失敗" (紅色、大字)
 * - 副文字: "無效的卡片/指紋" (白色)
 * 
 * 色彩配置:
 * - 主色調: 紅色 (0xF800)
 * - 背景: 白色或淺紅色
 * - 警告感: 粗線條、高對比
 * 
 * 動畫效果 (進階):
 * - 螢幕晃動效果 (左右偏移)
 * - 紅色閃爍警告
 * - 叉叉圖示脈衝
 * 
 * TODO:
 * 1. 清除螢幕為淺色
 * 2. 繪製紅色叉叉
 * 3. 顯示失敗訊息
 * 4. 可選: 加入警告動畫
 * 5. 2秒後自動返回待機畫面
 */
void DisplayHandler::showAccessDenied() {
    // TODO: 清除螢幕
    // clear(COLOR_WHITE);
    
    // TODO: 繪製叉叉圖示
    // drawCross(TFT_WIDTH/2, TFT_HEIGHT/2 - 60, 80, COLOR_RED);
    
    // TODO: 顯示失敗文字
    // tft.setTextColor(COLOR_RED);
    // tft.setTextDatum(MC_DATUM);
    // tft.setTextSize(4);
    // tft.drawString("驗證失敗", TFT_WIDTH/2, TFT_HEIGHT/2 + 40);
    
    // TODO: 顯示副標題
    // tft.setTextSize(2);
    // tft.setTextColor(COLOR_BLACK);
    // tft.drawString("無效的卡片/指紋", TFT_WIDTH/2, TFT_HEIGHT/2 + 80);
    
    Serial.println("[Display] 顯示驗證失敗畫面");
}

/*
 * 顯示系統錯誤畫面
 * 
 * UI元素:
 * - 錯誤代碼
 * - 錯誤訊息
 * - 重啟提示
 * 
 * @param errorCode 錯誤代碼
 * @param errorMsg 錯誤訊息
 * 
 * TODO:
 * 1. 清除螢幕為紅色背景
 * 2. 顯示錯誤資訊
 * 3. 提供重啟或除錯提示
 */
void DisplayHandler::showError(uint8_t errorCode, const char* errorMsg) {
    // TODO: 顯示錯誤畫面
    // clear(COLOR_RED);
    // tft.setTextColor(COLOR_WHITE);
    // tft.setTextDatum(MC_DATUM);
    // tft.setTextSize(3);
    // tft.drawString("系統錯誤", TFT_WIDTH/2, TFT_HEIGHT/2 - 40);
    
    // TODO: 顯示錯誤代碼
    // String errCode = "Code: " + String(errorCode);
    // tft.setTextSize(2);
    // tft.drawString(errCode, TFT_WIDTH/2, TFT_HEIGHT/2);
    
    // TODO: 顯示錯誤訊息
    // if (errorMsg != nullptr) {
    //     tft.drawString(errorMsg, TFT_WIDTH/2, TFT_HEIGHT/2 + 40);
    // }
    
    Serial.print("[Display] 錯誤: ");
    Serial.print(errorCode);
    Serial.print(" - ");
    Serial.println(errorMsg);
}

/*
 * 顯示系統狀態 (除錯用)
 * 
 * 顯示內容:
 * - RFID狀態: 已連接/未連接
 * - 指紋感應器狀態
 * - 已註冊指紋數量
 * - Wi-Fi狀態 (若啟用)
 * - 系統運行時間
 * 
 * @param rfidStatus RFID狀態
 * @param fpStatus 指紋感應器狀態
 * @param fpCount 已註冊指紋數量
 * 
 * TODO:
 * 1. 清除螢幕
 * 2. 顯示各模組狀態
 * 3. 使用顏色區分狀態 (綠=正常, 紅=異常)
 */
void DisplayHandler::showSystemStatus(bool rfidStatus, bool fpStatus, uint16_t fpCount) {
    // TODO: 實作系統狀態顯示
    Serial.println("[Display] 顯示系統狀態");
}

/*
 * 更新倒數計時顯示
 * 
 * 優化: 只更新數字區域，避免整個畫面重繪
 * 
 * @param seconds 剩餘秒數
 * 
 * TODO:
 * 1. 計算數字顯示區域
 * 2. 清除舊數字
 * 3. 繪製新數字
 * 4. 可選: 加入進度條視覺化
 */
void DisplayHandler::updateCountdown(uint16_t seconds) {
    // TODO: 更新倒數計時
    // String countdownText = String(seconds) + "秒";
    // 
    // // 清除舊文字區域
    // tft.fillRect(TFT_WIDTH/2 - 50, TFT_HEIGHT - 50, 100, 30, COLOR_WHITE);
    // 
    // // 繪製新文字
    // tft.setTextColor(COLOR_GRAY);
    // tft.setTextDatum(MC_DATUM);
    // tft.setTextSize(2);
    // tft.drawString(countdownText, TFT_WIDTH/2, TFT_HEIGHT - 40);
}

// ============================================================================
// 圖示繪製函式
// ============================================================================

/*
 * 繪製鎖頭圖示
 * 
 * 設計:
 * - 鎖身: 矩形
 * - 鎖扣: 半圓形 (上鎖) 或開啟 (解鎖)
 * - 鎖孔: 小圓形或鑰匙孔形狀
 * 
 * @param x X座標 (圖示中心)
 * @param y Y座標 (圖示中心)
 * @param size 圖示大小
 * @param locked true: 上鎖圖示 | false: 解鎖圖示
 * 
 * TODO:
 * 1. 繪製鎖身 (fillRect)
 * 2. 繪製鎖扣 (drawArc 或 drawCircle)
 * 3. 繪製鎖孔 (fillCircle)
 * 4. 根據locked狀態調整鎖扣位置
 */
void DisplayHandler::drawLockIcon(int16_t x, int16_t y, uint16_t size, bool locked) {
    // TODO: 繪製鎖頭圖示
    // uint16_t color = locked ? COLOR_RED : COLOR_GREEN;
    // 
    // // 繪製鎖身
    // tft.fillRect(x - size/2, y, size, size, color);
    // 
    // // 繪製鎖扣
    // if (locked) {
    //     tft.drawArc(x, y - size/4, size/2, size/3, 0, 180, color, color);
    // } else {
    //     tft.drawArc(x - size/2, y - size/4, size/2, size/3, 0, 180, color, color);
    // }
    // 
    // // 繪製鎖孔
    // tft.fillCircle(x, y + size/4, size/8, COLOR_BLACK);
}

/*
 * 繪製RFID卡片圖示
 * 
 * 設計:
 * - 卡片外框: 圓角矩形
 * - 晶片圖案: 小矩形網格
 * - 無線訊號: 波紋線條
 * 
 * @param x X座標
 * @param y Y座標
 * @param size 圖示大小
 * 
 * TODO:
 * 1. 繪製卡片外框 (drawRoundRect)
 * 2. 繪製晶片圖案
 * 3. 繪製無線訊號波紋
 */
void DisplayHandler::drawRFIDIcon(int16_t x, int16_t y, uint16_t size) {
    // TODO: 繪製RFID卡片圖示
    // // 卡片外框
    // tft.drawRoundRect(x - size/2, y - size/3, size, size*2/3, 5, COLOR_CYAN);
    // 
    // // 晶片圖案 (4x4網格)
    // uint16_t chipSize = size / 4;
    // for (int i = 0; i < 4; i++) {
    //     for (int j = 0; j < 4; j++) {
    //         tft.drawRect(x - chipSize + i*chipSize/2, 
    //                      y - chipSize + j*chipSize/2, 
    //                      chipSize/3, chipSize/3, COLOR_CYAN);
    //     }
    // }
}

/*
 * 繪製指紋圖示
 * 
 * 設計:
 * - 使用同心圓或橢圓模擬指紋紋路
 * - 3~5層紋路
 * - 不完全閉合，製造真實感
 * 
 * @param x X座標
 * @param y Y座標
 * @param size 圖示大小
 * 
 * TODO:
 * 1. 繪製多層同心圓或橢圓
 * 2. 使用drawArc繪製不完全閉合的弧線
 * 3. 調整線條粗細與間距
 */
void DisplayHandler::drawFingerprintIcon(int16_t x, int16_t y, uint16_t size) {
    // TODO: 繪製指紋圖示
    // uint16_t color = COLOR_YELLOW;
    // 
    // // 繪製多層同心圓 (模擬指紋紋路)
    // for (int i = 1; i <= 5; i++) {
    //     uint16_t radius = size * i / 10;
    //     // 繪製不完全閉合的圓弧
    //     tft.drawArc(x, y, radius, radius-2, 30, 330, color, color);
    // }
}

/*
 * 繪製勾勾圖示 ✓
 * 
 * 設計:
 * - 使用兩條直線組成
 * - 短邊向下，長邊向右上
 * - 線條粗細可調
 * 
 * @param x X座標 (勾勾中心)
 * @param y Y座標 (勾勾中心)
 * @param size 圖示大小
 * @param color 顏色
 * 
 * TODO:
 * 1. 計算勾勾兩條線的起終點
 * 2. 使用drawLine繪製
 * 3. 可選: 繪製多次增加線條粗細
 */
void DisplayHandler::drawCheckmark(int16_t x, int16_t y, uint16_t size, uint16_t color) {
    // TODO: 繪製勾勾
    // // 短邊 (左下到中間)
    // int16_t x1 = x - size/2;
    // int16_t y1 = y;
    // int16_t x2 = x - size/6;
    // int16_t y2 = y + size/2;
    // 
    // // 長邊 (中間到右上)
    // int16_t x3 = x2;
    // int16_t y3 = y2;
    // int16_t x4 = x + size/2;
    // int16_t y4 = y - size/2;
    // 
    // // 繪製粗線條 (多次繪製)
    // for (int i = -2; i <= 2; i++) {
    //     tft.drawLine(x1+i, y1, x2+i, y2, color);
    //     tft.drawLine(x3+i, y3, x4+i, y4, color);
    // }
}

/*
 * 繪製叉叉圖示 ✗
 * 
 * 設計:
 * - 兩條對角線交叉
 * - 線條粗細可調
 * 
 * @param x X座標 (叉叉中心)
 * @param y Y座標 (叉叉中心)
 * @param size 圖示大小
 * @param color 顏色
 * 
 * TODO:
 * 1. 計算兩條對角線的起終點
 * 2. 使用drawLine繪製
 * 3. 繪製多次增加線條粗細
 */
void DisplayHandler::drawCross(int16_t x, int16_t y, uint16_t size, uint16_t color) {
    // TODO: 繪製叉叉
    // // 左上到右下
    // int16_t x1 = x - size/2;
    // int16_t y1 = y - size/2;
    // int16_t x2 = x + size/2;
    // int16_t y2 = y + size/2;
    // 
    // // 右上到左下
    // int16_t x3 = x + size/2;
    // int16_t y3 = y - size/2;
    // int16_t x4 = x - size/2;
    // int16_t y4 = y + size/2;
    // 
    // // 繪製粗線條
    // for (int i = -2; i <= 2; i++) {
    //     tft.drawLine(x1+i, y1, x2+i, y2, color);
    //     tft.drawLine(x3+i, y3, x4+i, y4, color);
    // }
}

// ============================================================================
// 進階功能
// ============================================================================

/*
 * 設定螢幕亮度
 * 
 * 方法: 使用PWM控制背光腳位
 * 
 * 注意: 需要TFT_BL腳位定義為有效GPIO (不是-1)
 * 若背光直接接3.3V，此功能無效
 * 
 * @param brightness 亮度 (0~255)
 * 
 * TODO:
 * 1. 檢查TFT_BL是否定義
 * 2. 配置PWM通道
 * 3. 設定PWM duty cycle
 */
void DisplayHandler::setBrightness(uint8_t brightness) {
    // TODO: 實作背光PWM控制
    // if (TFT_BL != -1) {
    //     ledcSetup(TFT_BL_CHANNEL, 5000, 8);  // 5kHz, 8位元解析度
    //     ledcAttachPin(TFT_BL, TFT_BL_CHANNEL);
    //     ledcWrite(TFT_BL_CHANNEL, brightness);
    //     currentBrightness = brightness;
    // }
}

/*
 * 螢幕休眠
 * 
 * ST7789V2 Section 9.3: Display OFF (DISPOFF, 0x28)
 * ST7789V2 Section 9.4: Sleep In (SLPIN, 0x10)
 * 
 * 省電模式:
 * - 關閉背光
 * - 發送DISPOFF命令
 * - 發送SLPIN進入睡眠
 * 
 * TODO:
 * 1. 關閉背光 (setBrightness(0))
 * 2. 發送DISPOFF命令 (TFT_eSPI可能沒有直接API)
 * 3. 更新isSleeping狀態
 */
void DisplayHandler::sleep() {
    // TODO: 進入休眠模式
    // setBrightness(0);
    // // tft.writecommand(0x28);  // DISPOFF
    // // tft.writecommand(0x10);  // SLPIN
    // isSleeping = true;
    // Serial.println("[Display] 進入休眠模式");
}

/*
 * 螢幕喚醒
 * 
 * ST7789V2 Section 9.2: Sleep Out (SLPOUT, 0x11)
 * - 需等待120ms喚醒
 * ST7789V2 Section 9.17: Display ON (DISPON, 0x29)
 * 
 * TODO:
 * 1. 發送SLPOUT命令
 * 2. 延遲120ms
 * 3. 發送DISPON命令
 * 4. 恢復背光亮度
 */
void DisplayHandler::wakeup() {
    // TODO: 喚醒顯示器
    // // tft.writecommand(0x11);  // SLPOUT
    // delay(120);
    // // tft.writecommand(0x29);  // DISPON
    // setBrightness(currentBrightness);
    // isSleeping = false;
    // Serial.println("[Display] 喚醒顯示器");
}

/*
 * 獲取螢幕寬度
 */
uint16_t DisplayHandler::getWidth() const {
    return TFT_WIDTH;
}

/*
 * 獲取螢幕高度
 */
uint16_t DisplayHandler::getHeight() const {
    return TFT_HEIGHT;
}

// ============================================================================
// 私有函式實作
// ============================================================================

/*
 * 繪製置中文字
 * 
 * @param text 文字內容
 * @param y Y座標
 * @param size 文字大小
 * @param color 文字顏色
 * 
 * TODO:
 * 1. 設定文字屬性
 * 2. 使用setTextDatum設定置中對齊
 * 3. 在螢幕中央X座標繪製
 */
void DisplayHandler::drawCenteredText(const char* text, int16_t y, uint8_t size, uint16_t color) {
    // TODO: 繪製置中文字
    // tft.setTextColor(color);
    // tft.setTextSize(size);
    // tft.setTextDatum(TC_DATUM);  // 頂部置中
    // tft.drawString(text, TFT_WIDTH/2, y);
}

/*
 * 繪製進度條
 * 
 * @param x X座標
 * @param y Y座標
 * @param width 寬度
 * @param height 高度
 * @param progress 進度 (0~100)
 * @param color 顏色
 * 
 * TODO:
 * 1. 繪製進度條外框
 * 2. 根據progress填充內部
 * 3. 可選: 加入漸層色效果
 */
void DisplayHandler::drawProgressBar(int16_t x, int16_t y, uint16_t width, uint16_t height, uint8_t progress, uint16_t color) {
    // TODO: 繪製進度條
    // // 外框
    // tft.drawRect(x, y, width, height, COLOR_GRAY);
    // 
    // // 內部填充
    // uint16_t fillWidth = (width - 4) * progress / 100;
    // tft.fillRect(x + 2, y + 2, fillWidth, height - 4, color);
}

/*
 * RGB888轉RGB565
 * 
 * RGB888: 24位元色彩 (每色8位元)
 * RGB565: 16位元色彩 (紅5位元、綠6位元、藍5位元)
 * 
 * 轉換公式:
 * R5 = R8 >> 3  (取高5位元)
 * G6 = G8 >> 2  (取高6位元)
 * B5 = B8 >> 3  (取高5位元)
 * RGB565 = (R5 << 11) | (G6 << 5) | B5
 * 
 * @param r 紅色 (0~255)
 * @param g 綠色 (0~255)
 * @param b 藍色 (0~255)
 * @return RGB565顏色值
 * 
 * TODO: 實作RGB888到RGB565的轉換
 */
uint16_t DisplayHandler::rgb888To565(uint8_t r, uint8_t g, uint8_t b) {
    // TODO: 實作RGB轉換
    // return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
    
    return 0x0000;  // 暫時返回黑色
}

/*
 * ============================================================================
 * 學習重點總結
 * ============================================================================
 * 
 * 1. TFT LCD顯示原理
 *    - 像素陣列: 240x320個獨立像素
 *    - RGB565色彩編碼: 16位元 = 65536色
 *    - Frame Buffer: 顯示記憶體儲存畫面資料
 * 
 * 2. SPI通訊
 *    - 4線SPI: MOSI, SCK, CS, DC
 *    - DC腳位: 0=命令, 1=資料
 *    - 高速傳輸: 最高27MHz
 * 
 * 3. TFT_eSPI庫優勢
 *    - 硬體加速 (DMA傳輸)
 *    - 豐富的繪圖函式
 *    - Sprite支援 (雙緩衝)
 *    - 多種字型選擇
 * 
 * 4. UI設計原則
 *    - 高對比度: 文字易讀
 *    - 大字型: 適合遠距離觀看
 *    - 視覺回饋: 即時顯示狀態
 *    - 色彩語言: 綠=成功, 紅=失敗, 藍=資訊
 * 
 * 5. 性能優化
 *    - 避免頻繁重繪整個螢幕
 *    - 只更新變化的區域
 *    - 使用Sprite實現複雜動畫
 *    - 預先計算圖形座標
 * 
 * 6. 記憶體管理
 *    - 240x320x2 = 153,600 bytes (全螢幕Buffer)
 *    - ESP32 SRAM有限，避免大型緩衝區
 *    - 分塊繪製或使用Sprite
 * 
 * ============================================================================
 */
