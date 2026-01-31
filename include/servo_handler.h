/*
 * 伺服馬達控制器 - SG90
 * 
 * 參考文檔:
 * 1. SG90 Servo Datasheet
 *    - Section 3: Control Signal (控制訊號規格)
 * 2. ESP32 Technical Reference Manual
 *    - Section 14: LED PWM (LEDC) Controller (PWM控制器)
 * 
 * 學習目標:
 * 1. 理解PWM (Pulse Width Modulation) 脈衝寬度調變原理
 * 2. 學習伺服馬達的控制訊號時序
 * 3. 掌握ESP32的LEDC外設配置
 */

#ifndef SERVO_HANDLER_H
#define SERVO_HANDLER_H

#include <Arduino.h>
#include <ESP32Servo.h>
#include "config.h"

class ServoHandler {
public:
    // 建構函式
    ServoHandler();
    
    // 解構函式
    ~ServoHandler();
    
    /*
     * 初始化伺服馬達
     * 
     * ESP32 TRM參考:
     * - Section 14.3: PWM Timer Configuration (PWM定時器配置)
     * - Section 14.4: PWM Duty Cycle (PWM工作週期)
     * 
     * SG90 Datasheet參考:
     * - Control Signal: PWM週期20ms (50Hz)
     * - Pulse Width: 1ms~2ms
     *   - 1.0ms: 0度
     *   - 1.5ms: 90度 (中心位置)
     *   - 2.0ms: 180度
     * 
     * PWM原理:
     * 伺服馬達透過PWM訊號的脈衝寬度判斷目標角度
     * 控制電路會根據脈衝寬度調整內部電機位置
     * 
     * ┌─────┐     ┌─────┐     ┌─────┐
     * │     │     │     │     │     │
     * │     │     │     │     │     │
     * ┘     └─────┘     └─────┘     └─────
     * │<-1ms>│<--19ms-->│  (0度)
     * 
     * ┌──────────┐┌──────────┐┌──────────┐
     * │          ││          ││          │
     * │          ││          ││          │
     * ┘          └┘          └┘          └
     * │<--1.5ms->│<-18.5ms->│  (90度)
     * 
     * ┌────────────────┐    ┌────────────────┐
     * │                │    │                │
     * │                │    │                │
     * ┘                └────┘                └
     * │<-----2ms------>│<--18ms-->│  (180度)
     * 
     * ESP32 LEDC配置:
     * 1. 選擇LEDC通道 (0~15，共16個通道)
     * 2. 設定PWM頻率: 50Hz
     * 3. 設定解析度: 16位元 (0~65535)
     * 4. 將GPIO綁定到LEDC通道
     * 5. 啟動LEDC定時器
     * 
     * 解析度計算:
     * - 週期: 20ms = 20,000μs
     * - 16位元解析度: 2^16 = 65536 steps
     * - 每步時間: 20,000 / 65536 ≈ 0.305μs
     * - 1ms脈衝: 1000 / 0.305 ≈ 3277 steps
     * - 2ms脈衝: 2000 / 0.305 ≈ 6554 steps
     * 
     * @return true: 初始化成功 | false: 初始化失敗
     * 
     * TODO: 實作LEDC通道配置與GPIO綁定
     *       使用ESP32Servo庫簡化配置流程
     */
    bool init();
    
    /*
     * 設定伺服馬達角度
     * 
     * SG90規格:
     * - 工作角度: 0° ~ 180°
     * - 轉速: 0.12sec/60° (4.8V)
     * - 扭力: 1.8kg·cm (4.8V)
     * - 死區: 5μs (訊號誤差容忍度)
     * 
     * 角度對應脈衝寬度:
     * angle(°) = (pulseWidth - 1000) / 1000 * 180
     * pulseWidth(μs) = 1000 + angle / 180 * 1000
     * 
     * 範例:
     * - 0°:   1000μs
     * - 45°:  1250μs
     * - 90°:  1500μs
     * - 135°: 1750μs
     * - 180°: 2000μs
     * 
     * 注意事項:
     * 1. 避免超出0~180°範圍，可能損壞馬達
     * 2. 馬達移動需要時間，不要頻繁切換
     * 3. 建議加入緩動 (Easing) 使移動更平滑
     * 
     * @param angle 目標角度 (0~180)
     * 
     * TODO: 實作角度限制與脈衝寬度轉換
     *       加入角度範圍檢查: if (angle > 180) angle = 180;
     */
    void setAngle(uint8_t angle);
    
    /*
     * 門鎖上鎖
     * 
     * 功能: 將伺服馬達轉至0度位置 (鎖定)
     * 
     * 機械設計建議:
     * - 0度位置: 門栓完全伸出，阻擋門框
     * - 可使用3D列印或金屬零件製作門栓機構
     * - 確保門栓長度足夠，能穩固鎖住門框
     * 
     * 安全考量:
     * - 預設狀態應為上鎖 (Fail-Safe設計)
     * - 斷電時門鎖應保持上鎖狀態
     * - 可搭配機械彈簧實現自動上鎖
     * 
     * TODO: 實作上鎖動作，設定為SERVO_LOCK_ANGLE (0度)
     *       可加入音效或LED指示
     */
    void lock();
    
    /*
     * 門鎖解鎖
     * 
     * 功能: 將伺服馬達轉至90度位置 (解鎖)
     * 
     * 機械設計建議:
     * - 90度位置: 門栓完全縮回，釋放門框
     * - 解鎖時間建議: 5~10秒後自動上鎖
     * - 可加入手動開門偵測 (霍爾感測器/磁簧開關)
     * 
     * 使用者體驗:
     * - 解鎖時顯示器顯示 "Access Granted"
     * - 播放提示音 (若有蜂鳴器)
     * - LED顯示綠燈
     * 
     * TODO: 實作解鎖動作，設定為SERVO_UNLOCK_ANGLE (90度)
     *       考慮加入解鎖計時器，自動上鎖
     */
    void unlock();
    
    /*
     * 獲取當前角度
     * 
     * @return 當前伺服馬達角度 (0~180)
     */
    uint8_t getCurrentAngle() const;
    
    /*
     * 檢查是否為上鎖狀態
     * 
     * @return true: 上鎖 | false: 解鎖
     */
    bool isLocked() const;
    
    /*
     * 平滑移動到目標角度 (進階功能)
     * 
     * 功能: 透過小步進移動，實現平滑轉動
     * 
     * 好處:
     * - 減少機械衝擊，延長馬達壽命
     * - 降低噪音
     * - 更好的使用者體驗
     * 
     * 實作方式:
     * 將大角度移動分解為多個小步驟
     * 每步之間加入短暫延遲 (10~50ms)
     * 
     * 範例: 從0°移動到90°
     * 0° -> 10° -> 20° -> ... -> 80° -> 90°
     * 
     * 緩動函式 (Easing Function):
     * - Linear: 線性移動 (勻速)
     * - EaseInOut: 先加速後減速 (更自然)
     * - EaseOut: 先快後慢 (緊急煞車效果)
     * 
     * @param targetAngle 目標角度
     * @param stepDelay 每步延遲時間 (毫秒)
     * 
     * TODO: 實作平滑移動演算法
     *       參考: https://easings.net/
     */
    void smoothMove(uint8_t targetAngle, uint16_t stepDelay = 20);
    
    /*
     * 設定伺服馬達速度 (透過步進控制)
     * 
     * 注意: SG90是標準伺服馬達，轉速固定
     * 此函式透過控制步進間隔模擬速度調整
     * 
     * @param speed 速度等級 (1=慢, 5=快)
     */
    void setSpeed(uint8_t speed);
    
    /*
     * 停止伺服馬達 (進入空閒模式)
     * 
     * 功能: 停止發送PWM訊號，馬達進入無力模式
     * 
     * 用途:
     * - 降低功耗
     * - 減少馬達抖動/噪音
     * - 允許手動調整角度 (測試用)
     * 
     * 注意: 停止後馬達失去保持力，可被外力轉動
     * 
     * ESP32 LEDC參考:
     * - 呼叫 ledcWrite(channel, 0) 停止PWM輸出
     * - 或使用 ledcDetachPin() 解除GPIO綁定
     * 
     * TODO: 實作PWM輸出停止
     */
    void stop();
    
    /*
     * 恢復伺服馬達 (重新啟用PWM)
     * 
     * 功能: 恢復PWM訊號，馬達回到上次設定的角度
     * 
     * TODO: 實作PWM輸出恢復
     */
    void resume();

private:
    // ESP32Servo物件
    Servo servo;
    
    // 當前角度
    uint8_t currentAngle;
    
    // 是否已初始化
    bool initialized;
    
    // 速度設定 (用於smoothMove)
    uint8_t speed;
    
    /*
     * 角度轉換為脈衝寬度
     * 
     * SG90公式:
     * pulseWidth(μs) = SERVO_PULSE_MIN + (angle / 180.0) * (SERVO_PULSE_MAX - SERVO_PULSE_MIN)
     * 
     * 範例:
     * - 0°:   1000 + (0/180)   * 1000 = 1000μs
     * - 90°:  1000 + (90/180)  * 1000 = 1500μs
     * - 180°: 1000 + (180/180) * 1000 = 2000μs
     * 
     * @param angle 角度 (0~180)
     * @return 脈衝寬度 (微秒)
     */
    uint16_t angleToPulseWidth(uint8_t angle);
    
    /*
     * 限制角度範圍
     * 
     * @param angle 輸入角度
     * @return 限制後的角度 (0~180)
     */
    uint8_t constrainAngle(uint8_t angle);
    
    /*
     * 緩動函式: EaseInOutQuad
     * 
     * 數學公式:
     * t < 0.5: 2 * t^2
     * t ≥ 0.5: 1 - 2 * (1-t)^2
     * 
     * @param t 進度 (0.0~1.0)
     * @return 緩動後的進度 (0.0~1.0)
     */
    float easeInOutQuad(float t);
};

/*
 * ============================================================================
 * ESP32 LEDC (LED PWM Controller) 詳解
 * ============================================================================
 * 
 * ESP32 TRM Section 14: LED PWM Controller
 * 
 * LEDC特性:
 * - 16個獨立PWM通道 (0~15)
 * - 支援2組定時器 (High Speed / Low Speed)
 * - 解析度: 1~16位元可調
 * - 頻率範圍: 1Hz ~ 40MHz
 * - 支援硬體淡入淡出
 * 
 * 通道分組:
 * - High Speed 通道: 0~7 (80MHz時鐘源)
 * - Low Speed 通道: 8~15 (APB時鐘源，支援睡眠模式)
 * 
 * 頻率與解析度關係:
 * frequency = clock_source / (2^resolution)
 * 
 * 範例:
 * - 50Hz, 16位元: 80MHz / (2^16) ≈ 1220Hz (不適用)
 * - 50Hz, 13位元: 80MHz / (2^13) ≈ 9765Hz -> 除頻至50Hz
 * 
 * ESP32內部會自動計算最佳的除頻參數
 * 
 * ============================================================================
 * Arduino ESP32Servo庫 API
 * ============================================================================
 * 
 * 常用函式:
 * - attach(pin): 綁定伺服馬達到指定GPIO
 * - write(angle): 設定角度 (0~180)
 * - writeMicroseconds(us): 直接設定脈衝寬度 (μs)
 * - read(): 讀取當前角度
 * - attached(): 檢查是否已綁定
 * - detach(): 解除綁定，停止PWM輸出
 * 
 * 進階配置:
 * - attach(pin, min, max): 自訂脈衝寬度範圍
 *   範例: servo.attach(SERVO_PIN, 500, 2500); // 更大角度範圍
 * 
 * ============================================================================
 * SG90伺服馬達規格表
 * ============================================================================
 * 
 * 電氣特性:
 * - 工作電壓: 4.8V ~ 6V (建議5V)
 * - 空載電流: 100mA @ 5V
 * - 堵轉電流: 600mA ~ 1A @ 5V
 * - 待機電流: 5mA
 * 
 * 機械特性:
 * - 扭力: 1.8kg·cm @ 4.8V, 2.5kg·cm @ 6V
 * - 轉速: 0.12s/60° @ 4.8V, 0.10s/60° @ 6V
 * - 工作角度: 0° ~ 180° (±10°)
 * - 死區寬度: 5μs
 * 
 * 物理尺寸:
 * - 重量: 9g
 * - 尺寸: 22.5 x 11.8 x 27mm
 * - 線長: 約25cm
 * 
 * 接線顏色:
 * - 橙色/黃色: 訊號線 (Signal)
 * - 紅色: 電源正極 (VCC, 5V)
 * - 棕色/黑色: 電源負極 (GND)
 * 
 * 重要注意事項:
 * 1. 電源供應: SG90電流需求較大，不建議直接使用ESP32的3.3V或5V輸出
 *    建議使用外部5V電源供應器，並與ESP32共地(GND)
 * 2. 去耦電容: 在SG90的VCC與GND間並聯100μF電解電容，濾除電流突波
 * 3. 訊號線: 可直接連接ESP32的GPIO (3.3V邏輯，SG90相容)
 * 4. 保護電路: 建議加入二極體保護，防止反向電動勢損壞ESP32
 * 
 * ============================================================================
 */

#endif // SERVO_HANDLER_H
