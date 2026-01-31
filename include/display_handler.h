/*
 * TFT顯示器控制器 - ILI9341 / ST7789
 * 
 * 參考文檔:
 * 1. ST7789V2 Datasheet
 *    - Section 8.4: 4-Line Serial Interface (SPI通訊)
 *    - Section 9: Command Table (命令表)
 *    - Section 13: Pixel Format (像素格式)
 * 2. TFT_eSPI Library Documentation
 *    - https://github.com/Bodmer/TFT_eSPI
 * 
 * 學習目標:
 * 1. 理解TFT LCD的SPI通訊協定
 * 2. 學習圖形渲染與色彩編碼 (RGB565)
 * 3. 掌握UI設計原則與顯示優化
 */

#ifndef DISPLAY_HANDLER_H
#define DISPLAY_HANDLER_H

#include <Arduino.h>
#include <TFT_eSPI.h>
#include "config.h"

class DisplayHandler {
public:
    // 建構函式
    DisplayHandler();
    
    // 解構函式
    ~DisplayHandler();
    
    /*
     * 初始化TFT顯示器
     * 
     * ST7789V2 Datasheet參考:
     * - Section 8.5: Reset Timing (重置時序)
     * - Section 9.1: Software Reset (SWRESET, 0x01)
     * - Section 9.2: Sleep Out (SLPOUT, 0x11)
     * - Section 9.17: Display On (DISPON, 0x29)
     * 
     * 初始化流程:
     * 1. 硬體重置 (RST腳位拉低->拉高)
     *    時序: RST Low ≥10μs -> RST High -> 等待5ms
     * 2. 發送軟體重置命令 (SWRESET)
     * 3. 退出睡眠模式 (SLPOUT)
     * 4. 設定像素格式 (COLMOD, 0x3A)
     *    - 0x55: RGB565 (16-bit/pixel)
     * 5. 設定顯示方向 (MADCTL, 0x36)
     *    - MY, MX, MV位元控制鏡像與旋轉
     * 6. 啟動顯示 (DISPON)
     * 
     * TFT_eSPI配置:
     * - 驅動已在platformio.ini的build_flags設定
     * - 接腳定義已在config.h定義
     * - TFT_eSPI會自動執行初始化流程
     * 
     * @return true: 初始化成功 | false: 初始化失敗
     * 
     * TODO: 初始化TFT_eSPI物件，測試顯示功能
     *       可顯示測試圖案確認接線正確
     */
    bool init();
    
    /*
     * 清除螢幕
     * 
     * ST7789V2 Section 13.5: Frame Memory Write (RAMWR, 0x2C)
     * 
     * 方法:
     * 1. 設定顯示窗口 (Column/Page Address Set)
     * 2. 填充單色像素資料
     * 
     * RGB565顏色編碼:
     * - 16位元: RRRRR GGGGGG BBBBB
     * - 紅色5位元: 0~31 (0x00~0x1F)
     * - 綠色6位元: 0~63 (0x00~0x3F)
     * - 藍色5位元: 0~31 (0x00~0x1F)
     * 
     * 範例顏色值:
     * - 黑色: 0x0000 (0b0000000000000000)
     * - 白色: 0xFFFF (0b1111111111111111)
     * - 紅色: 0xF800 (0b1111100000000000)
     * - 綠色: 0x07E0 (0b0000011111100000)
     * - 藍色: 0x001F (0b0000000000011111)
     * 
     * @param color 填充顏色 (RGB565格式)
     * 
     * TODO: 實作fillScreen()函式
     */
    void clear(uint16_t color = COLOR_BLACK);
    
    /*
     * 顯示歡迎畫面
     * 
     * UI設計建議:
     * - 標題: "智慧門鎖系統" (置中，大字型)
     * - 副標題: "Smart Lock v1.0"
     * - 圖示: 鎖頭圖示 (可用簡單幾何圖形繪製)
     * - 背景: 深色背景 + 淺色文字 (護眼)
     * 
     * TFT_eSPI文字函式:
     * - setTextColor(fg, bg): 設定文字顏色
     * - setTextSize(size): 設定文字大小 (1~7)
     * - setTextDatum(datum): 設定文字對齊方式
     *   - TC_DATUM: 頂部置中
     *   - MC_DATUM: 中央置中
     * - drawString(text, x, y): 繪製字串
     * 
     * TODO: 設計美觀的歡迎畫面
     *       使用drawString, drawRect, fillCircle等函式
     */
    void showWelcome();
    
    /*
     * 顯示RFID掃描畫面
     * 
     * UI元素:
     * - 提示文字: "請感應卡片"
     * - RFID圖示: 卡片圖形
     * - 動畫效果: 掃描線或等待動畫 (選用)
     * 
     * 色彩配置:
     * - 主色調: 藍色 (0x001F) 或青色 (0x07FF)
     * - 背景: 深灰色 (0x4208)
     * 
     * TODO: 設計RFID等待畫面
     *       考慮加入簡單動畫提升使用者體驗
     */
    void showRFIDScanning();
    
    /*
     * 顯示指紋掃描畫面
     * 
     * UI元素:
     * - 提示文字: "請按壓指紋"
     * - 指紋圖示: 指紋紋路圖形
     * - 進度提示: "偵測中..." (閃爍效果)
     * 
     * 色彩配置:
     * - 主色調: 綠色 (0x07E0) 或黃色 (0xFFE0)
     * 
     * TODO: 設計指紋等待畫面
     */
    void showFingerprintScanning();
    
    /*
     * 顯示驗證成功畫面
     * 
     * UI元素:
     * - 大型勾勾圖示 ✓
     * - 文字: "驗證成功" (綠色)
     * - 副文字: "門鎖已開啟"
     * - 倒數計時: "5秒後自動上鎖"
     * 
     * 色彩配置:
     * - 主色調: 綠色 (0x07E0)
     * - 背景: 白色或淺綠背景
     * 
     * 動畫效果:
     * - 勾勾圖示由小變大 (縮放動畫)
     * - 文字淡入效果
     * 
     * @param userName 使用者名稱 (選用)
     * @param remainTime 剩餘開鎖時間 (秒)
     * 
     * TODO: 設計成功畫面，加入視覺回饋
     */
    void showUnlocked(const char* userName = nullptr, uint16_t remainTime = 0);
    
    /*
     * 顯示驗證失敗畫面
     * 
     * UI元素:
     * - 大型叉叉圖示 ✗
     * - 文字: "驗證失敗" (紅色)
     * - 副文字: "無效的卡片/指紋"
     * 
     * 色彩配置:
     * - 主色調: 紅色 (0xF800)
     * - 背景: 白色或淺紅背景
     * 
     * 動畫效果:
     * - 螢幕晃動效果 (左右偏移)
     * - 紅色閃爍警告
     * 
     * TODO: 設計失敗畫面，提供清晰的錯誤提示
     */
    void showAccessDenied();
    
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
     * TODO: 設計錯誤畫面
     */
    void showError(uint8_t errorCode, const char* errorMsg);
    
    /*
     * 顯示系統狀態 (選用，用於除錯)
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
     * TODO: 設計狀態顯示畫面
     */
    void showSystemStatus(bool rfidStatus, bool fpStatus, uint16_t fpCount);
    
    /*
     * 更新倒數計時顯示
     * 
     * 用途: 在解鎖後顯示剩餘時間
     * 
     * @param seconds 剩餘秒數
     * 
     * TODO: 實作倒數計時更新，避免整個畫面重繪
     *       只更新數字區域提升效能
     */
    void updateCountdown(uint16_t seconds);
    
    /*
     * 繪製圖示 - 鎖頭
     * 
     * 使用TFT_eSPI幾何繪圖函式:
     * - drawRect(): 繪製矩形 (鎖身)
     * - drawCircle(): 繪製圓形 (鎖扣)
     * - fillRect(): 填充矩形
     * 
     * @param x X座標
     * @param y Y座標
     * @param size 圖示大小
     * @param locked true: 上鎖圖示 | false: 解鎖圖示
     * 
     * TODO: 設計鎖頭圖示
     */
    void drawLockIcon(int16_t x, int16_t y, uint16_t size, bool locked = true);
    
    /*
     * 繪製圖示 - RFID卡片
     * 
     * @param x X座標
     * @param y Y座標
     * @param size 圖示大小
     * 
     * TODO: 設計RFID卡片圖示
     */
    void drawRFIDIcon(int16_t x, int16_t y, uint16_t size);
    
    /*
     * 繪製圖示 - 指紋
     * 
     * @param x X座標
     * @param y Y座標
     * @param size 圖示大小
     * 
     * TODO: 設計指紋圖示
     *       可使用同心圓或橢圓模擬指紋紋路
     */
    void drawFingerprintIcon(int16_t x, int16_t y, uint16_t size);
    
    /*
     * 繪製圖示 - 勾勾 ✓
     * 
     * @param x X座標
     * @param y Y座標
     * @param size 圖示大小
     * @param color 顏色
     * 
     * TODO: 使用drawLine繪製勾勾
     */
    void drawCheckmark(int16_t x, int16_t y, uint16_t size, uint16_t color = COLOR_GREEN);
    
    /*
     * 繪製圖示 - 叉叉 ✗
     * 
     * @param x X座標
     * @param y Y座標
     * @param size 圖示大小
     * @param color 顏色
     * 
     * TODO: 使用drawLine繪製叉叉
     */
    void drawCross(int16_t x, int16_t y, uint16_t size, uint16_t color = COLOR_RED);
    
    /*
     * 設定螢幕亮度 (若支援)
     * 
     * 方法:
     * - 若TFT_BL接腳定義為GPIO，使用PWM調光
     * - ESP32 LEDC產生PWM訊號控制背光
     * 
     * @param brightness 亮度 (0~255)
     * 
     * TODO: 實作PWM背光控制
     *       config.h中TFT_BL=-1表示不使用，需修改
     */
    void setBrightness(uint8_t brightness);
    
    /*
     * 螢幕休眠 (關閉背光，節省電力)
     * 
     * ST7789V2 Section 9.3: Display OFF (DISPOFF, 0x28)
     * 
     * 省電模式:
     * - 關閉背光 (若支援PWM控制)
     * - 發送DISPOFF命令
     * - 或發送Sleep In (SLPIN, 0x10)
     * 
     * TODO: 實作螢幕休眠
     */
    void sleep();
    
    /*
     * 螢幕喚醒
     * 
     * ST7789V2 Section 9.2: Sleep Out (SLPOUT, 0x11)
     * 
     * 喚醒流程:
     * - 發送SLPOUT命令
     * - 等待120ms (喚醒時間)
     * - 發送DISPON命令
     * - 開啟背光
     * 
     * TODO: 實作螢幕喚醒
     */
    void wakeup();
    
    /*
     * 獲取螢幕寬度
     * 
     * @return 螢幕寬度 (像素)
     */
    uint16_t getWidth() const;
    
    /*
     * 獲取螢幕高度
     * 
     * @return 螢幕高度 (像素)
     */
    uint16_t getHeight() const;

private:
    // TFT_eSPI物件
    TFT_eSPI tft;
    
    // 當前亮度
    uint8_t currentBrightness;
    
    // 螢幕狀態
    bool isSleeping;
    
    /*
     * 繪製置中文字
     * 
     * 功能: 計算文字寬度，自動置中對齊
     * 
     * @param text 文字內容
     * @param y Y座標
     * @param size 文字大小
     * @param color 文字顏色
     * 
     * TODO: 實作文字置中繪製
     */
    void drawCenteredText(const char* text, int16_t y, uint8_t size, uint16_t color);
    
    /*
     * 繪製進度條 (選用)
     * 
     * 用途: 顯示載入進度或倒數時間
     * 
     * @param x X座標
     * @param y Y座標
     * @param width 寬度
     * @param height 高度
     * @param progress 進度 (0~100)
     * @param color 顏色
     * 
     * TODO: 實作進度條繪製
     */
    void drawProgressBar(int16_t x, int16_t y, uint16_t width, uint16_t height, uint8_t progress, uint16_t color);
    
    /*
     * RGB888轉RGB565
     * 
     * 公式:
     * RGB565 = ((R & 0xF8) << 8) | ((G & 0xFC) << 3) | (B >> 3)
     * 
     * 解釋:
     * - 紅色: 取高5位 (右移3位捨棄低3位)
     * - 綠色: 取高6位 (右移2位捨棄低2位)
     * - 藍色: 取高5位 (右移3位捨棄低3位)
     * 
     * @param r 紅色 (0~255)
     * @param g 綠色 (0~255)
     * @param b 藍色 (0~255)
     * @return RGB565顏色值
     */
    uint16_t rgb888To565(uint8_t r, uint8_t g, uint8_t b);
};

/*
 * ============================================================================
 * ST7789V2 關鍵命令說明
 * ============================================================================
 * 
 * Datasheet Section 9: Command Table
 * 
 * 系統控制命令:
 * 0x01 - SWRESET: 軟體重置
 * 0x10 - SLPIN: 進入睡眠模式
 * 0x11 - SLPOUT: 退出睡眠模式
 * 0x28 - DISPOFF: 顯示關閉
 * 0x29 - DISPON: 顯示開啟
 * 0x38 - IDMOFF: 關閉Idle模式
 * 0x39 - IDMON: 開啟Idle模式
 * 
 * 顯示設定命令:
 * 0x2A - CASET: Column Address Set (設定X座標範圍)
 * 0x2B - RASET: Row Address Set (設定Y座標範圍)
 * 0x2C - RAMWR: Memory Write (寫入像素資料)
 * 0x36 - MADCTL: Memory Data Access Control (顯示方向控制)
 *        位元定義:
 *        - MY (bit 7): 垂直鏡像
 *        - MX (bit 6): 水平鏡像
 *        - MV (bit 5): 行列交換 (旋轉90°)
 *        - ML (bit 4): 垂直刷新順序
 *        - RGB (bit 3): RGB/BGR順序
 * 0x3A - COLMOD: Interface Pixel Format (像素格式)
 *        - 0x55: RGB565 (16-bit)
 *        - 0x66: RGB666 (18-bit)
 * 
 * 顏色設定命令:
 * 0xB1 - FRMCTR1: Frame Rate Control (Normal Mode)
 * 0xB2 - FRMCTR2: Frame Rate Control (Idle Mode)
 * 0xB3 - FRMCTR3: Frame Rate Control (Partial Mode)
 * 
 * ============================================================================
 * TFT_eSPI 常用API
 * ============================================================================
 * 
 * 初始化:
 * - tft.init(): 初始化顯示器
 * - tft.setRotation(r): 設定旋轉角度 (0~3)
 * 
 * 基本繪圖:
 * - tft.fillScreen(color): 填充整個螢幕
 * - tft.drawPixel(x, y, color): 繪製像素
 * - tft.drawLine(x0, y0, x1, y1, color): 繪製直線
 * - tft.drawRect(x, y, w, h, color): 繪製矩形
 * - tft.fillRect(x, y, w, h, color): 填充矩形
 * - tft.drawCircle(x, y, r, color): 繪製圓形
 * - tft.fillCircle(x, y, r, color): 填充圓形
 * - tft.drawTriangle(x0, y0, x1, y1, x2, y2, color): 繪製三角形
 * 
 * 文字處理:
 * - tft.setTextColor(fg [, bg]): 設定文字顏色
 * - tft.setTextSize(size): 設定文字大小
 * - tft.setCursor(x, y): 設定游標位置
 * - tft.print(text): 列印文字
 * - tft.println(text): 列印文字並換行
 * - tft.drawString(text, x, y): 繪製字串
 * - tft.setTextDatum(datum): 設定對齊方式
 *   - TL_DATUM: 左上對齊
 *   - TC_DATUM: 頂部置中
 *   - TR_DATUM: 右上對齊
 *   - MC_DATUM: 中央置中
 * 
 * 進階功能:
 * - tft.setSwapBytes(swap): 設定位元組順序
 * - tft.pushImage(x, y, w, h, data): 推送圖片資料
 * - tft.loadFont(fontName): 載入字型
 * 
 * 性能優化:
 * - 使用Sprite (精靈圖) 實現雙緩衝，消除閃爍
 * - 只更新變化的區域，避免全螢幕重繪
 * - 使用DMA傳輸加速大量資料寫入
 * 
 * ============================================================================
 */

#endif // DISPLAY_HANDLER_H
