/*
 * SG90伺服馬達控制器 - 實作檔案
 * 
 * 參考文檔:
 * - SG90 Servo Datasheet Section 3: Control Signal
 * - ESP32 Technical Reference Manual Section 14: LED PWM Controller
 * 
 * 本檔案包含SG90伺服馬達的所有函式實作骨架
 * 搭配詳細的datasheet章節標註，供學習使用
 */

#include "servo_handler.h"

// ============================================================================
// 建構函式與解構函式
// ============================================================================

/*
 * 建構函式
 * 
 * 初始化成員變數的預設值
 * 此時尚未配置硬體資源，需呼叫init()完成初始化
 */
ServoHandler::ServoHandler() 
    : currentAngle(0),
      initialized(false),
      speed(3)  // 預設中等速度
{
    // 建構函式只初始化成員變數
    // 實際硬體初始化在init()中進行
}

/*
 * 解構函式
 * 
 * 釋放資源，停止PWM輸出
 */
ServoHandler::~ServoHandler() {
    if (initialized) {
        servo.detach();  // 解除GPIO綁定，停止PWM
    }
}

// ============================================================================
// 公開函式實作
// ============================================================================

/*
 * 初始化伺服馬達
 * 
 * ESP32 TRM參考:
 * - Section 14.3: PWM Timer Configuration (PWM定時器配置)
 * - Section 14.4: PWM Duty Cycle (PWM工作週期計算)
 * 
 * SG90 Datasheet參考:
 * - Section 3: Control Signal
 *   - PWM頻率: 50Hz (週期20ms)
 *   - 脈衝寬度: 1ms(0°) ~ 1.5ms(90°) ~ 2ms(180°)
 * 
 * ESP32Servo庫配置:
 * - 自動分配LEDC通道
 * - 自動計算定時器參數
 * - 支援多個伺服馬達同時控制
 * 
 * @return true: 初始化成功 | false: 初始化失敗
 * 
 * TODO: 
 * 1. 呼叫servo.attach()綁定GPIO
 * 2. 設定PWM頻率範圍 (可選：servo.setPeriodHertz(50))
 * 3. 測試移動到中心位置驗證功能
 * 4. 加入錯誤處理（檢查GPIO是否已被占用）
 */
bool ServoHandler::init() {
    Serial.println("[Servo] 初始化伺服馬達...");
    
    /*
     * ESP32Servo.attach() 函式
     * 
     * 原型: attach(pin, minUs, maxUs, channel)
     * - pin: GPIO腳位
     * - minUs: 最小脈衝寬度(微秒) - 對應0度
     * - maxUs: 最大脈衝寬度(微秒) - 對應180度
     * - channel: LEDC通道 (選用，-1自動分配)
     * 
     * SG90標準值:
     * - min: 1000μs (0度)
     * - max: 2000μs (180度)
     * 
     * 部分伺服馬達可能需要微調這些值以達到精確角度
     */
    
    // TODO: 實作attach函式呼叫
    // servo.attach(SERVO_PIN, SERVO_PULSE_MIN, SERVO_PULSE_MAX);
    
    // TODO: 測試伺服馬達是否正常工作
    // 移動到初始位置(0度 - 上鎖)
    // setAngle(SERVO_LOCK_ANGLE);
    
    // TODO: 加入初始化驗證
    // if (!servo.attached()) {
    //     Serial.println("[Servo] 錯誤: GPIO綁定失敗");
    //     return false;
    // }
    
    initialized = true;
    Serial.println("[Servo] 初始化完成");
    
    return true;  // TODO: 根據實際初始化結果返回
}

/*
 * 設定伺服馬達角度
 * 
 * SG90規格:
 * - 工作角度: 0° ~ 180°
 * - 轉速: 0.12sec/60° @ 4.8V
 * - 死區: 5μs (控制訊號容忍度)
 * 
 * 角度與脈衝寬度關係:
 * pulseWidth(μs) = 1000 + (angle / 180.0) * 1000
 * 
 * 範例計算:
 * - 0°:   1000 + (0/180)   * 1000 = 1000μs
 * - 45°:  1000 + (45/180)  * 1000 = 1250μs
 * - 90°:  1000 + (90/180)  * 1000 = 1500μs
 * - 135°: 1000 + (135/180) * 1000 = 1750μs
 * - 180°: 1000 + (180/180) * 1000 = 2000μs
 * 
 * ESP32Servo庫會自動處理角度到脈衝寬度的轉換
 * 
 * @param angle 目標角度 (0~180)
 * 
 * TODO:
 * 1. 限制角度範圍 (0~180)
 * 2. 呼叫servo.write(angle)設定角度
 * 3. 更新currentAngle成員變數
 * 4. 加入Serial除錯輸出
 */
void ServoHandler::setAngle(uint8_t angle) {
    // TODO: 限制角度範圍
    // angle = constrainAngle(angle);
    
    // TODO: 設定馬達角度
    // servo.write(angle);
    
    // TODO: 更新當前角度
    // currentAngle = angle;
    
    // TODO: 除錯輸出
    // Serial.print("[Servo] 設定角度: ");
    // Serial.print(angle);
    // Serial.println("°");
}

/*
 * 門鎖上鎖
 * 
 * 功能: 將伺服馬達轉至0度位置
 * 
 * 機械設計考量:
 * - 0度位置應對應門栓完全伸出狀態
 * - 確保門栓能穩固卡入門框
 * - 可搭配限位開關確認上鎖到位
 * 
 * 安全設計:
 * - Fail-Safe: 預設狀態為上鎖
 * - 斷電時門鎖保持上鎖(機械彈簧)
 * - 防止暴力破解時馬達過載損壞
 * 
 * TODO:
 * 1. 呼叫setAngle(SERVO_LOCK_ANGLE)
 * 2. 延遲等待馬達到位 (根據SG90轉速計算)
 * 3. 可選: 加入音效或LED指示
 * 4. 可選: 記錄上鎖時間戳記
 */
void ServoHandler::lock() {
    Serial.println("[Servo] 執行上鎖動作...");
    
    // TODO: 設定為上鎖角度
    // setAngle(SERVO_LOCK_ANGLE);
    
    // TODO: 等待馬達移動完成
    // SG90轉速: 0.12秒/60度
    // 從90度轉到0度約需: (90/60) * 0.12 = 0.18秒 = 180ms
    // delay(200);  // 預留安全時間
    
    Serial.println("[Servo] 上鎖完成");
}

/*
 * 門鎖解鎖
 * 
 * 功能: 將伺服馬達轉至90度位置
 * 
 * 機械設計考量:
 * - 90度位置應對應門栓完全縮回狀態
 * - 確保門能順利推開
 * - 解鎖後應在指定時間內自動上鎖
 * 
 * 使用者體驗:
 * - 解鎖時顯示器顯示成功訊息
 * - 播放確認音 (若有蜂鳴器)
 * - LED顯示綠燈
 * - 開始倒數計時自動上鎖
 * 
 * TODO:
 * 1. 呼叫setAngle(SERVO_UNLOCK_ANGLE)
 * 2. 延遲等待馬達到位
 * 3. 可選: 觸發解鎖事件回調
 * 4. 可選: 記錄解鎖日誌(使用者、時間)
 */
void ServoHandler::unlock() {
    Serial.println("[Servo] 執行解鎖動作...");
    
    // TODO: 設定為解鎖角度
    // setAngle(SERVO_UNLOCK_ANGLE);
    
    // TODO: 等待馬達移動完成
    // 從0度轉到90度約需: (90/60) * 0.12 = 0.18秒 = 180ms
    // delay(200);
    
    Serial.println("[Servo] 解鎖完成");
}

/*
 * 獲取當前角度
 * 
 * @return 當前馬達角度 (0~180)
 */
uint8_t ServoHandler::getCurrentAngle() const {
    return currentAngle;
}

/*
 * 檢查是否為上鎖狀態
 * 
 * @return true: 上鎖 | false: 解鎖或中間狀態
 */
bool ServoHandler::isLocked() const {
    // 考慮馬達誤差，允許±5度的容差
    return (currentAngle <= SERVO_LOCK_ANGLE + 5);
}

/*
 * 平滑移動到目標角度
 * 
 * 功能: 透過小步進移動實現平滑轉動效果
 * 
 * 好處:
 * - 減少機械衝擊，延長馬達壽命
 * - 降低電流突波
 * - 降低噪音
 * - 更好的使用者體驗
 * 
 * 實作原理:
 * 將大角度移動分解為多個小步驟
 * 每步之間加入短暫延遲
 * 
 * 範例: 從0°移動到90°
 * Step 1: 0° -> 5°  (延遲20ms)
 * Step 2: 5° -> 10° (延遲20ms)
 * ...
 * Step 18: 85° -> 90° (延遲20ms)
 * 總時間: 18 * 20ms = 360ms
 * 
 * 緩動函式 (Easing Function):
 * - Linear: 勻速移動
 * - EaseInOut: 先加速後減速 (更自然)
 * - EaseOut: 先快後慢
 * 
 * 參考: https://easings.net/
 * 
 * @param targetAngle 目標角度 (0~180)
 * @param stepDelay 每步延遲時間(毫秒)
 * 
 * TODO:
 * 1. 計算當前角度到目標角度的差值
 * 2. 決定步進大小 (建議5度/步)
 * 3. 使用easeInOutQuad緩動函式計算每步角度
 * 4. 逐步移動並延遲
 * 5. 可選: 加入中斷機制 (緊急停止)
 */
void ServoHandler::smoothMove(uint8_t targetAngle, uint16_t stepDelay) {
    // TODO: 限制目標角度範圍
    // targetAngle = constrainAngle(targetAngle);
    
    // TODO: 計算角度差
    // int16_t angleDiff = targetAngle - currentAngle;
    // if (angleDiff == 0) return;  // 已在目標位置
    
    // TODO: 計算總步數
    // uint16_t totalSteps = abs(angleDiff) / 5;  // 每5度一步
    // if (totalSteps == 0) totalSteps = 1;
    
    // TODO: 逐步移動
    // for (uint16_t step = 0; step <= totalSteps; step++) {
    //     // 計算進度 (0.0 ~ 1.0)
    //     float progress = (float)step / totalSteps;
    //     
    //     // 應用緩動函式
    //     float easedProgress = easeInOutQuad(progress);
    //     
    //     // 計算當前目標角度
    //     uint8_t stepAngle = currentAngle + (angleDiff * easedProgress);
    //     
    //     // 移動到該角度
    //     setAngle(stepAngle);
    //     
    //     // 延遲
    //     delay(stepDelay);
    // }
    
    // TODO: 確保最終到達精確的目標角度
    // setAngle(targetAngle);
}

/*
 * 設定馬達速度等級
 * 
 * 注意: SG90是標準伺服馬達，轉速由內部電路決定
 * 此函式透過調整smoothMove的步進間隔來模擬速度控制
 * 
 * @param speed 速度等級 (1=慢, 5=快)
 * 
 * TODO:
 * 1. 限制速度範圍 (1~5)
 * 2. 儲存速度設定
 * 3. 在smoothMove中使用此速度計算延遲時間
 */
void ServoHandler::setSpeed(uint8_t speed) {
    // TODO: 限制速度範圍
    // if (speed < 1) speed = 1;
    // if (speed > 5) speed = 5;
    
    // this->speed = speed;
}

/*
 * 停止伺服馬達 (進入空閒模式)
 * 
 * 功能: 停止PWM訊號輸出
 * 
 * 效果:
 * - 馬達失去保持力，可被手動轉動
 * - 降低功耗 (空載電流約5mA)
 * - 減少馬達發熱
 * - 消除馬達抖動/嗡嗡聲
 * 
 * 使用場景:
 * - 長時間待機時節省電力
 * - 測試時需要手動調整角度
 * - 馬達過熱保護
 * 
 * 注意: 停止後馬達無法保持位置，門鎖可能被外力推開
 * 
 * ESP32Servo參考:
 * - detach(): 解除GPIO綁定，停止PWM
 * 
 * TODO:
 * 1. 呼叫servo.detach()停止PWM
 * 2. 更新initialized狀態
 * 3. 記錄停止前的角度
 */
void ServoHandler::stop() {
    // TODO: 停止PWM輸出
    // if (initialized && servo.attached()) {
    //     servo.detach();
    //     Serial.println("[Servo] PWM已停止");
    // }
}

/*
 * 恢復伺服馬達 (重新啟用PWM)
 * 
 * 功能: 重新綁定GPIO，恢復PWM輸出
 * 
 * TODO:
 * 1. 重新呼叫attach綁定GPIO
 * 2. 恢復到停止前的角度
 * 3. 更新initialized狀態
 */
void ServoHandler::resume() {
    // TODO: 恢復PWM輸出
    // if (!servo.attached()) {
    //     servo.attach(SERVO_PIN, SERVO_PULSE_MIN, SERVO_PULSE_MAX);
    //     setAngle(currentAngle);  // 恢復到原角度
    //     Serial.println("[Servo] PWM已恢復");
    // }
}

// ============================================================================
// 私有函式實作
// ============================================================================

/*
 * 角度轉換為脈衝寬度
 * 
 * SG90脈衝寬度計算公式:
 * pulseWidth = minPulse + (angle / maxAngle) * (maxPulse - minPulse)
 * 
 * 代入SG90參數:
 * pulseWidth = 1000 + (angle / 180) * (2000 - 1000)
 *            = 1000 + angle * (1000/180)
 *            = 1000 + angle * 5.556
 * 
 * 範例驗證:
 * - angle=0:   1000 + 0*5.556   = 1000μs ✓
 * - angle=90:  1000 + 90*5.556  = 1500μs ✓
 * - angle=180: 1000 + 180*5.556 = 2000μs ✓
 * 
 * @param angle 角度 (0~180)
 * @return 脈衝寬度 (微秒)
 * 
 * 注意: ESP32Servo庫已內建此轉換，此函式供學習用
 */
uint16_t ServoHandler::angleToPulseWidth(uint8_t angle) {
    // TODO: 實作角度到脈衝寬度的轉換
    // return SERVO_PULSE_MIN + (angle * (SERVO_PULSE_MAX - SERVO_PULSE_MIN) / 180);
    
    return 1500;  // 暫時返回中心位置脈衝寬度
}

/*
 * 限制角度範圍
 * 
 * 使用Arduino的constrain()函式限制數值範圍
 * 
 * @param angle 輸入角度
 * @return 限制後的角度 (0~180)
 */
uint8_t ServoHandler::constrainAngle(uint8_t angle) {
    // Arduino constrain(x, a, b): 限制x在a~b範圍內
    // 若x < a, 返回a
    // 若x > b, 返回b
    // 否則返回x
    
    // TODO: 實作角度限制
    // return constrain(angle, 0, 180);
    
    if (angle > 180) return 180;
    return angle;
}

/*
 * 緩動函式: EaseInOutQuad (二次方緩動)
 * 
 * 數學公式:
 * - 前半段 (t < 0.5): y = 2 * t^2
 * - 後半段 (t ≥ 0.5): y = 1 - 2 * (1-t)^2
 * 
 * 特性:
 * - 開始緩慢加速
 * - 中間勻速
 * - 結束緩慢減速
 * - 曲線對稱
 * 
 * 應用:
 * 讓馬達移動更自然，避免突然啟動/停止
 * 
 * @param t 輸入進度 (0.0~1.0)
 * @return 緩動後的進度 (0.0~1.0)
 * 
 * 範例:
 * - t=0.0  -> 0.000 (起點)
 * - t=0.25 -> 0.125 (前期加速慢)
 * - t=0.5  -> 0.500 (中間勻速)
 * - t=0.75 -> 0.875 (後期減速)
 * - t=1.0  -> 1.000 (終點)
 * 
 * 參考: https://easings.net/#easeInOutQuad
 */
float ServoHandler::easeInOutQuad(float t) {
    // TODO: 實作緩動函式
    // if (t < 0.5) {
    //     return 2 * t * t;
    // } else {
    //     float temp = 1 - t;
    //     return 1 - 2 * temp * temp;
    // }
    
    return t;  // 暫時返回線性進度
}

/*
 * ============================================================================
 * 學習重點總結
 * ============================================================================
 * 
 * 1. PWM控制原理
 *    - 週期固定 (20ms = 50Hz)
 *    - 透過改變高電位時間 (脈衝寬度) 控制角度
 *    - SG90: 1ms=0°, 1.5ms=90°, 2ms=180°
 * 
 * 2. ESP32 LEDC硬體
 *    - 16個獨立PWM通道
 *    - 支援多種解析度 (1~16位元)
 *    - 自動計算定時器參數
 * 
 * 3. ESP32Servo庫優勢
 *    - 簡化PWM配置流程
 *    - 自動角度轉換
 *    - 支援多個馬達
 *    - 避免手動計算定時器參數
 * 
 * 4. 機械設計考量
 *    - 確保門栓長度足夠
 *    - 加入限位開關確認到位
 *    - 考慮馬達扭力是否足夠
 *    - 設計Fail-Safe機制
 * 
 * 5. 電源設計
 *    - SG90堵轉電流可達1A
 *    - 不可直接使用ESP32的3.3V/5V輸出
 *    - 建議外部5V電源 + 共地
 *    - 加入100μF去耦電容濾波
 * 
 * 6. 進階優化
 *    - 使用smoothMove實現平滑移動
 *    - 加入緩動函式改善使用者體驗
 *    - 實作中斷機制處理緊急情況
 *    - 記錄使用次數監控馬達壽命
 * 
 * ============================================================================
 */
