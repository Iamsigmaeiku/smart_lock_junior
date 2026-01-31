/*
 * ESP32智慧門鎖主程式
 * 
 * 功能描述:
 * 整合RFID卡片辨識與指紋辨識，實現雙重認證智慧門鎖
 * 使用TFT顯示器提供視覺回饋，伺服馬達控制門鎖開關
 * 
 * 硬體組成:
 * - ESP32-WROOM-32: 主控制器
 * - MFRC522: RFID讀卡器
 * - AS608: 指紋感應器
 * - SG90: 伺服馬達
 * - ILI9341/ST7789: TFT顯示器
 * 
 * 學習架構:
 * - setup(): 系統初始化 (只執行一次)
 * - loop(): 主迴圈 (持續執行)
 * - 狀態機設計: 使用enum管理系統狀態
 * 
 * 參考文獻:
 * - ESP32 Arduino Core文檔
 * - FreeRTOS任務排程 (進階)
 */

#include <Arduino.h>
#include <SPI.h>
#include "config.h"
#include "rfid_handler.h"
#include "fingerprint_handler.h"
#include "servo_handler.h"
#include "display_handler.h"

// ============================================================================
// 全域物件實例
// ============================================================================

RFIDHandler rfid;               // RFID模組處理器
FingerprintHandler fingerprint; // 指紋模組處理器
ServoHandler servo;             // 伺服馬達控制器
DisplayHandler display;         // 顯示器控制器

// ============================================================================
// 系統狀態變數
// ============================================================================

SystemState currentState = STATE_INIT;  // 當前系統狀態
SystemState previousState = STATE_INIT; // 前一個狀態 (用於偵測狀態變化)

// 計時器變數
unsigned long lastUpdateTime = 0;      // 上次更新時間
unsigned long unlockStartTime = 0;     // 解鎖開始時間
bool isUnlocked = false;               // 解鎖狀態旗標

// 認證資料暫存
uint8_t detectedUID[10];               // 偵測到的RFID UID
uint8_t detectedUIDSize = 0;           // UID長度
uint16_t detectedFingerprintID = 0;    // 偵測到的指紋ID
uint16_t fingerprintConfidence = 0;    // 指紋信心分數

// ============================================================================
// 函式宣告 (Forward Declaration)
// ============================================================================

void initializeSystem();               // 系統初始化
void handleStateTransition();          // 處理狀態轉換
void updateDisplay();                  // 更新顯示器
void checkRFID();                      // 檢查RFID卡片
void checkFingerprint();               // 檢查指紋
void processUnlock();                  // 處理解鎖邏輯
void processLock();                    // 處理上鎖邏輯
void handleError(uint8_t errorCode);   // 處理錯誤

// ============================================================================
// Arduino Setup - 系統初始化
// ============================================================================
/*
 * setup()函式
 * 
 * Arduino架構特性:
 * - 在系統啟動時執行一次
 * - 用於初始化硬體、設定參數、配置通訊
 * 
 * ESP32啟動流程:
 * 1. 一級引導程式 (ROM Bootloader)
 * 2. 二級引導程式 (Second Stage Bootloader)
 * 3. Arduino Core初始化
 * 4. setup()執行
 * 5. loop()開始循環
 * 
 * ESP32 Technical Reference Manual:
 * - Section 2.3: System Reset (系統重置流程)
 * - Section 5.2: Reset and Clock Control
 */

void setup() {
    /*
     * 串列通訊初始化
     * 
     * 用途: Debug輸出、系統日誌
     * 波特率: 115200 bps (標準速率)
     * 
     * ESP32 UART0:
     * - TX: GPIO1 (連接USB-UART晶片)
     * - RX: GPIO3
     * 
     * TODO: 開啟Serial並輸出啟動訊息
     */
    Serial.begin(SERIAL_BAUD_RATE);
    delay(100);  // 等待Serial穩定
    
    Serial.println("\n=================================");
    Serial.println("  ESP32 智慧門鎖系統啟動");
    Serial.println("  Smart Lock System v1.0");
    Serial.println("=================================\n");
    
    // 顯示系統資訊
    Serial.print("ESP32 Chip ID: ");
    Serial.println((uint32_t)ESP.getEfuseMac(), HEX);
    Serial.print("CPU Frequency: ");
    Serial.print(ESP.getCpuFreqMHz());
    Serial.println(" MHz");
    Serial.print("Flash Size: ");
    Serial.print(ESP.getFlashChipSize() / 1024 / 1024);
    Serial.println(" MB");
    Serial.println();
    
    /*
     * SPI匯流排初始化
     * 
     * ESP32 VSPI (預設SPI):
     * - MOSI: GPIO23
     * - MISO: GPIO19
     * - SCK:  GPIO18
     * - SS:   任意GPIO (每個從設備不同)
     * 
     * SPI參數:
     * - 最大頻率: 取決於從設備 (MFRC522: 10MHz, TFT: 27MHz)
     * - 模式: SPI_MODE0 (MFRC522), SPI_MODE3 (部分TFT)
     * - 位元順序: MSBFIRST
     * 
     * Arduino SPI庫會自動配置VSPI
     * 
     * TODO: 初始化SPI (通常由各模組的init()內部處理)
     */
    Serial.println("[1/5] 初始化SPI匯流排...");
    SPI.begin(SPI_SCK_PIN, SPI_MISO_PIN, SPI_MOSI_PIN);
    Serial.println("      ✓ SPI初始化完成");
    
    /*
     * 呼叫系統初始化函式
     * 
     * 初始化順序很重要:
     * 1. 顯示器 (最先初始化，提供視覺回饋)
     * 2. RFID模組
     * 3. 指紋模組
     * 4. 伺服馬達
     */
    initializeSystem();
    
    /*
     * 設定初始狀態
     */
    currentState = STATE_IDLE;
    previousState = STATE_INIT;
    
    Serial.println("\n=================================");
    Serial.println("  系統初始化完成，進入待機模式");
    Serial.println("=================================\n");
}

// ============================================================================
// Arduino Loop - 主迴圈
// ============================================================================
/*
 * loop()函式
 * 
 * Arduino架構特性:
 * - 在setup()完成後，無限循環執行
 * - 每次循環稱為一個"迭代" (Iteration)
 * - 需注意執行時間，避免阻塞
 * 
 * FreeRTOS背景知識:
 * ESP32實際上運行FreeRTOS即時作業系統
 * loop()函式在預設Task中執行 (loopTask)
 * 優先權: 1 (較低)
 * 堆疊大小: 8KB
 * 
 * 非阻塞設計原則:
 * - 避免使用delay()，改用millis()計時
 * - 使用狀態機管理複雜流程
 * - 每個模組的檢查應該快速返回
 * 
 * TODO: 實作主迴圈邏輯
 */

void loop() {
    /*
     * 獲取當前時間
     * 
     * millis()函式:
     * - 返回系統啟動後的毫秒數 (unsigned long)
     * - 精度: 1ms
     * - 約49.7天後溢位 (重新從0開始)
     * 
     * 溢位處理:
     * 使用差值計算自動處理溢位
     * 例如: (currentMillis - lastUpdateTime) >= INTERVAL
     */
    unsigned long currentMillis = millis();
    
    /*
     * 狀態轉換處理
     * 
     * 當狀態改變時，執行對應的進入動作
     * 例如: 進入STATE_IDLE時，顯示歡迎畫面
     */
    if (currentState != previousState) {
        handleStateTransition();
        previousState = currentState;
    }
    
    /*
     * 根據當前狀態執行對應邏輯
     * 
     * 狀態機設計模式:
     * 使用switch-case根據currentState執行不同程式碼
     * 每個state負責特定的功能
     */
    switch (currentState) {
        case STATE_INIT:
            /*
             * 初始化狀態
             * 
             * 通常在setup()完成後立即轉換為STATE_IDLE
             * 若初始化失敗，可停留在此狀態等待重試
             */
            // TODO: 處理初始化後續工作
            currentState = STATE_IDLE;
            break;
            
        case STATE_IDLE:
            /*
             * 待機狀態
             * 
             * 功能:
             * 1. 持續檢測RFID卡片
             * 2. 持續檢測指紋
             * 3. 更新顯示器 (動畫效果)
             * 
             * 非阻塞設計:
             * - checkRFID()應快速返回 (≤10ms)
             * - checkFingerprint()應快速返回 (≤50ms)
             * - 使用計時器控制檢查頻率
             */
            
            // 限制更新頻率 (避免過度消耗CPU)
            if (currentMillis - lastUpdateTime >= SYSTEM_UPDATE_INTERVAL) {
                checkRFID();         // TODO: 實作RFID檢查
                checkFingerprint();  // TODO: 實作指紋檢查
                // updateDisplay();  // TODO: 更新顯示器動畫 (選用)
                
                lastUpdateTime = currentMillis;
            }
            break;
            
        case STATE_RFID_DETECTED:
            /*
             * RFID卡片偵測狀態
             * 
             * 流程:
             * 1. 讀取完整的UID
             * 2. 驗證UID是否在白名單
             * 3. 根據驗證結果切換狀態
             */
            Serial.println("[RFID] 偵測到卡片，讀取UID...");
            
            // TODO: 讀取UID並驗證
            // if (rfid.readCardUID(detectedUID, &detectedUIDSize)) {
            //     if (rfid.verifyCard(detectedUID, detectedUIDSize)) {
            //         currentState = STATE_ACCESS_GRANTED;
            //     } else {
            //         currentState = STATE_ACCESS_DENIED;
            //     }
            // } else {
            //     currentState = STATE_IDLE;  // 讀取失敗，返回待機
            // }
            
            // 暫時直接進入驗證通過 (示範用)
            currentState = STATE_ACCESS_GRANTED;
            break;
            
        case STATE_FP_DETECTED:
            /*
             * 指紋偵測狀態
             * 
             * 流程:
             * 1. 採集指紋圖像
             * 2. 提取特徵
             * 3. 搜尋指紋庫
             * 4. 根據信心分數決定是否通過
             */
            Serial.println("[指紋] 偵測到手指，開始辨識...");
            
            // TODO: 採集並驗證指紋
            // if (fingerprint.captureFingerprint(1)) {
            //     if (fingerprint.verifyFingerprint(detectedFingerprintID, fingerprintConfidence)) {
            //         if (fingerprintConfidence >= FP_CONFIDENCE_THRESHOLD) {
            //             currentState = STATE_ACCESS_GRANTED;
            //         } else {
            //             currentState = STATE_ACCESS_DENIED;
            //         }
            //     } else {
            //         currentState = STATE_ACCESS_DENIED;
            //     }
            // } else {
            //     currentState = STATE_IDLE;  // 採集失敗，返回待機
            // }
            
            // 暫時直接進入驗證通過 (示範用)
            currentState = STATE_ACCESS_GRANTED;
            break;
            
        case STATE_VERIFYING:
            /*
             * 驗證中狀態
             * 
             * 顯示驗證進度 (選用)
             * 某些情況下驗證可能需要較長時間
             */
            // TODO: 顯示驗證進度
            break;
            
        case STATE_ACCESS_GRANTED:
            /*
             * 驗證通過狀態
             * 
             * 動作:
             * 1. 顯示成功畫面
             * 2. 解鎖門鎖 (伺服馬達轉動)
             * 3. 啟動自動上鎖計時器
             */
            Serial.println("[系統] 驗證成功！解鎖門鎖...");
            
            processUnlock();  // TODO: 實作解鎖邏輯
            
            // 啟動解鎖計時器
            unlockStartTime = currentMillis;
            isUnlocked = true;
            
            // 返回待機狀態 (但保持解鎖)
            currentState = STATE_IDLE;
            break;
            
        case STATE_ACCESS_DENIED:
            /*
             * 驗證失敗狀態
             * 
             * 動作:
             * 1. 顯示失敗畫面
             * 2. 播放警告音 (若有蜂鳴器)
             * 3. 記錄失敗事件 (安全日誌)
             * 4. 延遲後返回待機
             */
            Serial.println("[系統] 驗證失敗！拒絕存取。");
            
            // TODO: 顯示失敗畫面
            // display.showAccessDenied();
            
            delay(2000);  // 顯示失敗畫面2秒
            
            // 返回待機狀態
            currentState = STATE_IDLE;
            break;
            
        case STATE_ERROR:
            /*
             * 錯誤狀態
             * 
             * 處理系統錯誤
             * 可能需要重新初始化或重啟
             */
            Serial.println("[錯誤] 系統錯誤，請檢查硬體連接。");
            
            // TODO: 顯示錯誤畫面
            // display.showError(1, "System Error");
            
            delay(5000);
            
            // 嘗試重新初始化
            initializeSystem();
            currentState = STATE_IDLE;
            break;
    }
    
    /*
     * 自動上鎖檢查
     * 
     * 當門鎖解鎖超過指定時間，自動上鎖
     */
    if (isUnlocked) {
        unsigned long unlockDuration = currentMillis - unlockStartTime;
        
        // 顯示剩餘時間 (選用)
        if (unlockDuration % 1000 == 0) {  // 每秒更新一次
            uint16_t remainingSeconds = (UNLOCK_DURATION - unlockDuration) / 1000;
            // TODO: display.updateCountdown(remainingSeconds);
            
            Serial.print("[門鎖] 剩餘解鎖時間: ");
            Serial.print(remainingSeconds);
            Serial.println(" 秒");
        }
        
        // 檢查是否超過解鎖時間
        if (unlockDuration >= UNLOCK_DURATION) {
            Serial.println("[門鎖] 自動上鎖...");
            processLock();  // TODO: 實作上鎖邏輯
            isUnlocked = false;
        }
    }
    
    /*
     * 讓出CPU時間給其他任務
     * 
     * FreeRTOS任務排程:
     * delay(1)會讓當前任務進入阻塞狀態1ms
     * 允許其他低優先權任務執行 (WiFi、BLE等)
     * 
     * 若不呼叫delay，看門狗定時器可能觸發重啟
     * ESP32 Watchdog: 預設5秒無餵狗會重啟
     * 
     * 替代方案:
     * - vTaskDelay(1 / portTICK_PERIOD_MS)
     * - yield()
     */
    delay(1);
}

// ============================================================================
// 系統初始化函式
// ============================================================================
/*
 * 初始化所有硬體模組
 * 
 * 錯誤處理:
 * - 若關鍵模組初始化失敗，進入STATE_ERROR
 * - 非關鍵模組失敗，記錄警告但繼續運行
 */

void initializeSystem() {
    Serial.println("\n開始初始化硬體模組...\n");
    
    /*
     * 初始化顯示器
     * 
     * 優先順序: 最高
     * 原因: 提供視覺回饋，讓使用者知道系統運作中
     */
    Serial.println("[2/5] 初始化TFT顯示器...");
    if (display.init()) {
        Serial.println("      ✓ 顯示器初始化成功");
        display.showWelcome();  // TODO: 實作歡迎畫面
    } else {
        Serial.println("      ✗ 顯示器初始化失敗！");
        // 顯示器失敗不影響核心功能，繼續運行
    }
    delay(500);
    
    /*
     * 初始化RFID模組
     * 
     * 檢查項目:
     * 1. SPI通訊是否正常
     * 2. MFRC522版本暫存器讀取 (應為0x92)
     * 3. 天線是否啟動
     */
    Serial.println("[3/5] 初始化MFRC522 RFID模組...");
    if (rfid.init()) {
        Serial.println("      ✓ RFID模組初始化成功");
    } else {
        Serial.println("      ✗ RFID模組初始化失敗！");
        Serial.println("      請檢查接線:");
        Serial.println("        - SS: GPIO21");
        Serial.println("        - RST: GPIO22");
        Serial.println("        - MOSI: GPIO23");
        Serial.println("        - MISO: GPIO19");
        Serial.println("        - SCK: GPIO18");
        currentState = STATE_ERROR;
    }
    delay(500);
    
    /*
     * 初始化指紋模組
     * 
     * 檢查項目:
     * 1. UART通訊是否正常
     * 2. 密碼驗證 (預設0x00000000)
     * 3. 讀取系統參數
     */
    Serial.println("[4/5] 初始化AS608指紋感應器...");
    if (fingerprint.init()) {
        Serial.println("      ✓ 指紋模組初始化成功");
        
        // 顯示指紋庫資訊
        uint16_t enrolledCount = fingerprint.getEnrolledCount();
        Serial.print("      已註冊指紋數量: ");
        Serial.print(enrolledCount);
        Serial.print(" / ");
        Serial.println(FP_DATABASE_SIZE);
    } else {
        Serial.println("      ✗ 指紋模組初始化失敗！");
        Serial.println("      請檢查接線:");
        Serial.println("        - TX (ESP32): GPIO17 -> RX (AS608)");
        Serial.println("        - RX (ESP32): GPIO16 -> TX (AS608)");
        currentState = STATE_ERROR;
    }
    delay(500);
    
    /*
     * 初始化伺服馬達
     * 
     * 動作:
     * 1. 配置PWM通道
     * 2. 設定為上鎖位置 (0度)
     */
    Serial.println("[5/5] 初始化SG90伺服馬達...");
    if (servo.init()) {
        Serial.println("      ✓ 伺服馬達初始化成功");
        
        // 確保門鎖處於上鎖狀態
        servo.lock();
        Serial.println("      門鎖已上鎖 (0度位置)");
    } else {
        Serial.println("      ✗ 伺服馬達初始化失敗！");
        Serial.println("      請檢查接線:");
        Serial.println("        - Signal: GPIO13");
        Serial.println("        - VCC: 5V (建議外部供電)");
        Serial.println("        - GND: 共地");
        currentState = STATE_ERROR;
    }
    delay(500);
    
    Serial.println("\n硬體模組初始化完成！\n");
}

// ============================================================================
// 狀態轉換處理函式
// ============================================================================
/*
 * 處理狀態改變時的進入動作
 * 
 * 狀態機設計模式:
 * - onEnter: 進入狀態時執行一次
 * - onUpdate: 在狀態中持續執行
 * - onExit: 離開狀態時執行一次
 * 
 * 此函式處理onEnter邏輯
 */

void handleStateTransition() {
    Serial.print("[狀態] ");
    Serial.print(previousState);
    Serial.print(" -> ");
    Serial.println(currentState);
    
    // TODO: 根據新狀態更新顯示器
    switch (currentState) {
        case STATE_IDLE:
            // display.showWelcome();
            break;
        case STATE_RFID_DETECTED:
            // display.showRFIDScanning();
            break;
        case STATE_FP_DETECTED:
            // display.showFingerprintScanning();
            break;
        case STATE_ACCESS_GRANTED:
            // display.showUnlocked();
            break;
        case STATE_ACCESS_DENIED:
            // display.showAccessDenied();
            break;
        default:
            break;
    }
}

// ============================================================================
// RFID檢查函式
// ============================================================================
/*
 * 非阻塞式RFID卡片檢測
 * 
 * 流程:
 * 1. 檢查是否有卡片靠近
 * 2. 若偵測到卡片，切換狀態為STATE_RFID_DETECTED
 * 
 * 效能考量:
 * - 此函式應在10ms內返回
 * - 使用MFRC522的非阻塞API
 */

void checkRFID() {
    // TODO: 實作RFID檢查邏輯
    // if (rfid.isCardPresent()) {
    //     currentState = STATE_RFID_DETECTED;
    // }
}

// ============================================================================
// 指紋檢查函式
// ============================================================================
/*
 * 非阻塞式指紋檢測
 * 
 * 流程:
 * 1. 檢查是否有手指放置
 * 2. 若偵測到手指，切換狀態為STATE_FP_DETECTED
 * 
 * 效能考量:
 * - 此函式應在50ms內返回
 * - 使用AS608的GenImg命令快速檢測
 */

void checkFingerprint() {
    // TODO: 實作指紋檢查邏輯
    // if (fingerprint.detectFinger()) {
    //     currentState = STATE_FP_DETECTED;
    // }
}

// ============================================================================
// 解鎖處理函式
// ============================================================================
/*
 * 執行解鎖動作
 * 
 * 動作流程:
 * 1. 伺服馬達轉至解鎖位置 (90度)
 * 2. 更新顯示器 (顯示解鎖成功)
 * 3. 記錄解鎖事件 (日誌)
 */

void processUnlock() {
    Serial.println("[門鎖] 執行解鎖動作...");
    
    // TODO: 伺服馬達解鎖
    // servo.unlock();
    
    // TODO: 更新顯示器
    // display.showUnlocked(nullptr, UNLOCK_DURATION / 1000);
    
    Serial.println("[門鎖] 解鎖完成");
}

// ============================================================================
// 上鎖處理函式
// ============================================================================
/*
 * 執行上鎖動作
 * 
 * 動作流程:
 * 1. 伺服馬達轉至上鎖位置 (0度)
 * 2. 更新顯示器 (返回待機畫面)
 * 3. 記錄上鎖事件 (日誌)
 */

void processLock() {
    Serial.println("[門鎖] 執行上鎖動作...");
    
    // TODO: 伺服馬達上鎖
    // servo.lock();
    
    // TODO: 更新顯示器
    // display.showWelcome();
    
    Serial.println("[門鎖] 上鎖完成");
}

// ============================================================================
// 錯誤處理函式
// ============================================================================
/*
 * 處理系統錯誤
 * 
 * @param errorCode 錯誤代碼
 */

void handleError(uint8_t errorCode) {
    Serial.print("[錯誤] 錯誤代碼: ");
    Serial.println(errorCode);
    
    // TODO: 顯示錯誤訊息
    // display.showError(errorCode, "System Error");
    
    currentState = STATE_ERROR;
}

/*
 * ============================================================================
 * 進階功能擴展建議
 * ============================================================================
 * 
 * 1. Wi-Fi連接與遠端控制:
 *    - 使用ESP32的WiFi功能
 *    - 整合MQTT或HTTP API
 *    - 遠端查看門鎖狀態與日誌
 * 
 * 2. 網頁介面管理:
 *    - ESP32作為Web Server
 *    - 管理授權RFID卡片與指紋
 *    - 查看存取記錄
 * 
 * 3. 時間戳記與日誌:
 *    - 整合RTC (DS3231) 或NTP
 *    - 記錄每次開門時間與使用者
 *    - 儲存到SD卡或雲端
 * 
 * 4. 電池電量監控:
 *    - 使用ADC監測電池電壓
 *    - 低電量警告
 * 
 * 5. 防撬警報:
 *    - 加速度感應器 (MPU6050)
 *    - 偵測異常震動
 *    - 觸發警報或通知
 * 
 * 6. 多重認證模式:
 *    - RFID + 指紋雙重認證 (更安全)
 *    - 藍牙手機解鎖
 *    - 密碼鍵盤輸入
 * 
 * 7. 省電模式:
 *    - 使用Deep Sleep降低功耗
 *    - 外部中斷喚醒 (RFID或PIR感應器)
 * 
 * 8. 音效回饋:
 *    - 加入蜂鳴器 (Passive Buzzer)
 *    - 不同音效對應不同狀態
 * 
 * ============================================================================
 */
