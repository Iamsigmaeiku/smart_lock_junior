/*
 * 指紋感應器模組處理器 - 實作檔案
 * 
 * 參考文檔:
 * - AS608 User Manual V2.0 Section 4: Communication Protocol
 * - Section 5: Instruction System
 * - Section 6: Application Notes
 * 
 * 本檔案包含AS608指紋感應器的所有函式實作骨架
 * 搭配詳細的datasheet章節標註與封包協定說明
 */

#include "fingerprint_handler.h"

// ============================================================================
// 建構函式與解構函式
// ============================================================================

/*
 * 建構函式
 * 
 * 初始化UART2 (Hardware Serial)與Adafruit_Fingerprint物件
 * 
 * ESP32 UART配置:
 * - UART0: USB Serial (Debug用，不可用於AS608)
 * - UART1: Flash (系統使用)
 * - UART2: 可自定義GPIO (用於AS608)
 */
FingerprintHandler::FingerprintHandler() 
    : finger(&Serial2),  // 使用UART2
      fpSerial(&Serial2),
      templateCount(0),
      librarySize(FP_DATABASE_SIZE),
      securityLevel(3)  // 預設安全等級3
{
    // 初始化成員變數
}

/*
 * 解構函式
 * 
 * 關閉UART連線
 */
FingerprintHandler::~FingerprintHandler() {
    // UART會自動釋放
}

// ============================================================================
// 公開函式實作
// ============================================================================

/*
 * 初始化AS608指紋感應器
 * 
 * User Manual參考:
 * - Section 4.1: UART Communication
 *   - 波特率: 9600/19200/38400/57600/115200 (預設57600)
 *   - 資料位元: 8 bits
 *   - 校驗位元: None
 *   - 停止位元: 1 bit
 *   - 模組地址: 0xFFFFFFFF (預設，可修改)
 * 
 * - Section 5.1: VfyPwd - Verify Password (密碼驗證)
 *   - 指令碼: 0x13
 *   - 參數: 4 bytes密碼
 *   - 確認碼: 0x00=成功, 0x13=密碼錯誤
 *   - 預設密碼: 0x00000000
 * 
 * - Section 5.15: ReadSysPara - Read System Parameters
 *   - 指令碼: 0x0F
 *   - 回傳16 bytes系統參數
 *   - 包含: 狀態暫存器、指紋庫容量、安全等級等
 * 
 * 初始化流程:
 * 1. 初始化UART2通訊 (57600 bps)
 * 2. 發送VfyPwd驗證密碼
 * 3. 若成功，讀取系統參數
 * 4. 獲取已註冊指紋數量
 * 5. 顯示模組資訊
 * 
 * @return true: 初始化成功 | false: 初始化失敗或密碼錯誤
 * 
 * TODO:
 * 1. 初始化UART2 (begin)
 * 2. 設定指紋感應器地址 (預設0xFFFFFFFF)
 * 3. 呼叫verifyPassword()驗證密碼
 * 4. 讀取系統參數
 * 5. 獲取已註冊指紋數量
 * 6. 加入錯誤處理
 */
bool FingerprintHandler::init() {
    Serial.println("[指紋] 初始化AS608指紋感應器...");
    
    /*
     * ESP32 UART2 GPIO配置
     * 
     * begin()函式原型:
     * begin(baudrate, config, rx_pin, tx_pin)
     * 
     * SERIAL_8N1: 8資料位元, 無校驗, 1停止位元
     */
    
    // TODO: 初始化UART2
    // fpSerial->begin(FP_BAUD_RATE, SERIAL_8N1, FP_RX_PIN, FP_TX_PIN);
    // Serial.print("[指紋] UART2已啟動, 波特率: ");
    // Serial.println(FP_BAUD_RATE);
    
    /*
     * Adafruit_Fingerprint庫初始化
     * 
     * begin()函式會:
     * 1. 設定模組地址 (預設0xFFFFFFFF)
     * 2. 不自動驗證密碼，需手動呼叫verifyPassword()
     */
    
    // TODO: 初始化指紋感應器物件
    // finger.begin(FP_BAUD_RATE);
    
    /*
     * 密碼驗證
     * 
     * User Manual Section 5.1: VfyPwd命令
     * 
     * 封包格式:
     * Header(2) + Address(4) + PID(1) + Length(2) + Data(N) + Checksum(2)
     * 0xEF01    + 0xFFFFFFFF + 0x01   + 0x0007    + 0x13+PWD  + CHK
     * 
     * 確認碼:
     * - 0x00: 密碼正確
     * - 0x01: 收包錯誤
     * - 0x13: 密碼錯誤
     */
    
    // TODO: 驗證密碼
    // uint8_t confirmCode = finger.verifyPassword();
    // if (confirmCode != FINGERPRINT_OK) {
    //     Serial.print("[指紋] 密碼驗證失敗, 確認碼: 0x");
    //     Serial.println(confirmCode, HEX);
    //     return false;
    // }
    // Serial.println("[指紋] 密碼驗證成功");
    
    /*
     * 讀取系統參數
     * 
     * User Manual Section 5.15: ReadSysPara命令
     * 
     * 系統參數包含 (16 bytes):
     * - Status Register (2 bytes): 狀態暫存器
     * - System Identifier (2 bytes): 系統識別碼 (0x0009)
     * - Library Size (2 bytes): 指紋庫容量 (127)
     * - Security Level (2 bytes): 安全等級 (1~5)
     * - Device Address (4 bytes): 模組地址
     * - Data Package Size (2 bytes): 資料封包大小
     * - Baud Rate (2 bytes): 波特率設定
     */
    
    // TODO: 讀取系統參數
    // finger.getParameters();
    // 
    // Serial.println("[指紋] 系統參數:");
    // Serial.print("  - 指紋庫容量: ");
    // Serial.println(finger.capacity);
    // Serial.print("  - 安全等級: ");
    // Serial.println(finger.securityLevel);
    // Serial.print("  - 封包大小: ");
    // Serial.println(finger.packetLen);
    
    /*
     * 獲取已註冊指紋數量
     * 
     * User Manual Section 5.10: TemplateNum命令
     * - 指令碼: 0x1D
     * - 回傳已儲存的模板數量
     */
    
    // TODO: 獲取已註冊數量
    // templateCount = getEnrolledCount();
    // Serial.print("[指紋] 已註冊指紋: ");
    // Serial.print(templateCount);
    // Serial.print(" / ");
    // Serial.println(librarySize);
    
    Serial.println("[指紋] 初始化完成");
    return true;  // TODO: 根據實際結果返回
}

/*
 * 檢測是否有手指放置在感應器上
 * 
 * User Manual參考:
 * - Section 5.2: GenImg - Generate Image (採集指紋圖像)
 *   - 指令碼: 0x01
 *   - 無參數
 *   - 確認碼:
 *     0x00: 採集成功
 *     0x01: 收包錯誤
 *     0x02: 無法偵測手指
 *     0x03: 指紋圖像不清晰
 * 
 * 工作原理:
 * AS608使用光學或半導體感應器偵測手指
 * - 光學式: LED照射 + CMOS感應器
 * - 半導體式: 電容感應
 * 
 * GenImg命令會:
 * 1. 啟動感應器
 * 2. 偵測手指是否放置
 * 3. 採集指紋圖像到ImageBuffer
 * 4. 返回確認碼
 * 
 * 注意事項:
 * - 此函式應快速返回 (非阻塞)
 * - 建議檢查間隔 >100ms
 * - 手指需要完全覆蓋感應區域
 * - 手指過乾或過濕會影響偵測
 * 
 * @return true: 偵測到手指 | false: 無手指或圖像不清晰
 * 
 * TODO:
 * 1. 呼叫getImage()採集指紋圖像
 * 2. 檢查確認碼
 * 3. 根據確認碼返回結果
 * 4. 可選: 加入重試機制 (圖像不清晰時)
 */
bool FingerprintHandler::detectFinger() {
    // TODO: 採集指紋圖像
    // uint8_t result = finger.getImage();
    // 
    // if (result == FINGERPRINT_OK) {
    //     Serial.println("[指紋] 偵測到手指");
    //     return true;
    // } else if (result == FINGERPRINT_NOFINGER) {
    //     // 無手指，正常情況
    //     return false;
    // } else if (result == FINGERPRINT_IMAGEFAIL) {
    //     Serial.println("[指紋] 警告: 圖像不清晰");
    //     return false;
    // } else {
    //     Serial.print("[指紋] 錯誤: 0x");
    //     Serial.println(result, HEX);
    //     return false;
    // }
    
    return false;  // TODO: 實際偵測邏輯
}

/*
 * 採集並處理指紋圖像
 * 
 * User Manual參考:
 * - Section 5.3: Img2Tz - Image to Template (圖像轉特徵)
 *   - 指令碼: 0x02
 *   - 參數: BufferID (1 或 2)
 *   - 確認碼:
 *     0x00: 轉換成功
 *     0x01: 收包錯誤
 *     0x06: 特徵點過少，無法生成特徵文件
 *     0x07: 圖像太亂，無法生成特徵文件
 *     0x15: ImageBuffer中沒有有效圖像
 * 
 * 指紋辨識原理:
 * AS608不儲存指紋圖像，而是儲存特徵模板
 * 
 * 特徵提取過程:
 * 1. 圖像預處理 (增強對比、去噪)
 * 2. 特徵點檢測 (Minutiae Detection)
 *    - 端點 (Ridge Ending)
 *    - 分叉點 (Ridge Bifurcation)
 * 3. 特徵編碼 (Feature Encoding)
 * 4. 儲存到CharBuffer1或CharBuffer2
 * 
 * CharBuffer用途:
 * - CharBuffer1: 第一次採集的特徵
 * - CharBuffer2: 第二次採集的特徵
 * - 用於註冊時比對兩次採集是否一致
 * - 用於驗證時搜尋指紋庫
 * 
 * 完整流程:
 * 1. GenImg: 採集圖像 -> ImageBuffer
 * 2. Img2Tz: 提取特徵 -> CharBuffer
 * 
 * @param bufferID 特徵緩衝區編號 (1或2)
 * @return true: 採集成功 | false: 採集失敗
 * 
 * TODO:
 * 1. 確認已偵測到手指 (GenImg執行過)
 * 2. 呼叫image2Tz(bufferID)轉換特徵
 * 3. 檢查確認碼
 * 4. 處理特徵點過少或圖像混亂的情況
 */
bool FingerprintHandler::captureFingerprint(uint8_t bufferID) {
    // TODO: 先採集圖像
    // if (!detectFinger()) {
    //     return false;
    // }
    
    // TODO: 轉換圖像為特徵
    // uint8_t result = finger.image2Tz(bufferID);
    // 
    // if (result == FINGERPRINT_OK) {
    //     Serial.print("[指紋] 特徵提取成功 (Buffer ");
    //     Serial.print(bufferID);
    //     Serial.println(")");
    //     return true;
    // } else if (result == FINGERPRINT_IMAGEMESS) {
    //     Serial.println("[指紋] 錯誤: 圖像太亂");
    //     return false;
    // } else if (result == FINGERPRINT_FEATUREFAIL) {
    //     Serial.println("[指紋] 錯誤: 特徵點過少");
    //     return false;
    // } else {
    //     Serial.print("[指紋] 錯誤: 0x");
    //     Serial.println(result, HEX);
    //     return false;
    // }
    
    return false;  // TODO: 實際採集邏輯
}

/*
 * 搜尋並驗證指紋
 * 
 * User Manual參考:
 * - Section 5.4: Search - Search for Template (搜尋指紋庫)
 *   - 指令碼: 0x04
 *   - 參數: 
 *     - BufferID (1 byte): 1或2
 *     - StartPage (2 bytes): 搜尋起始位置
 *     - PageNum (2 bytes): 搜尋頁數
 *   - 回傳:
 *     - 確認碼
 *     - PageID (2 bytes): 匹配的指紋ID
 *     - MatchScore (2 bytes): 信心分數
 * 
 * - Section 5.5: Match - Match Two Templates (比對兩個特徵)
 *   - 指令碼: 0x03
 *   - 比對CharBuffer1與CharBuffer2
 *   - 回傳匹配分數
 * 
 * 搜尋流程:
 * 1. 採集當前指紋 -> CharBuffer1
 * 2. 發送Search命令
 * 3. AS608逐一比對指紋庫中的模板
 * 4. 返回最匹配的PageID與分數
 * 
 * 信心分數 (Match Score):
 * - 範圍: 0~255
 * - 分數越高，相似度越高
 * - 閾值建議:
 *   - 寬鬆: 30~50 (便利性優先)
 *   - 平衡: 50~100 (推薦)
 *   - 嚴格: 100~150 (安全性優先)
 * 
 * 確認碼:
 * - 0x00: 找到匹配指紋
 * - 0x01: 收包錯誤
 * - 0x09: 沒有找到匹配指紋
 * 
 * @param id[out] 匹配的指紋ID (PageID, 0~126)
 * @param confidence[out] 信心分數 (0~255)
 * @return true: 找到匹配指紋 | false: 無匹配或錯誤
 * 
 * TODO:
 * 1. 確認CharBuffer1中有有效特徵
 * 2. 呼叫fingerSearch()搜尋指紋庫
 * 3. 檢查確認碼
 * 4. 獲取PageID與MatchScore
 * 5. 比對信心分數閾值
 * 6. 返回結果
 */
bool FingerprintHandler::verifyFingerprint(uint16_t& id, uint16_t& confidence) {
    // TODO: 確認已採集指紋
    // if (!captureFingerprint(1)) {
    //     return false;
    // }
    
    // TODO: 搜尋指紋庫
    // uint8_t result = finger.fingerSearch();
    // 
    // if (result == FINGERPRINT_OK) {
    //     id = finger.fingerID;
    //     confidence = finger.confidence;
    //     
    //     Serial.print("[指紋] 找到匹配: ID=");
    //     Serial.print(id);
    //     Serial.print(", 信心分數=");
    //     Serial.println(confidence);
    //     
    //     // 檢查信心分數閾值
    //     if (confidence >= FP_CONFIDENCE_THRESHOLD) {
    //         Serial.println("[指紋] 驗證通過");
    //         return true;
    //     } else {
    //         Serial.println("[指紋] 驗證失敗: 信心分數過低");
    //         return false;
    //     }
    // } else if (result == FINGERPRINT_NOTFOUND) {
    //     Serial.println("[指紋] 未找到匹配指紋");
    //     return false;
    // } else {
    //     Serial.print("[指紋] 搜尋錯誤: 0x");
    //     Serial.println(result, HEX);
    //     return false;
    // }
    
    return false;  // TODO: 實際搜尋邏輯
}

/*
 * 註冊新指紋
 * 
 * User Manual參考:
 * - Section 6.1: Fingerprint Registration (指紋註冊流程)
 * - Section 5.6: RegModel - Generate Template (生成模板)
 *   - 指令碼: 0x05
 *   - 合併CharBuffer1與CharBuffer2生成模板
 *   - 確認碼:
 *     0x00: 合併成功
 *     0x01: 收包錯誤
 *     0x0A: 兩次採集的特徵不匹配
 * 
 * - Section 5.7: Store - Store Template (儲存模板)
 *   - 指令碼: 0x06
 *   - 參數:
 *     - BufferID (1 byte): 1或2
 *     - PageID (2 bytes): 儲存位置 (0~126)
 *   - 確認碼:
 *     0x00: 儲存成功
 *     0x0B: PageID超出範圍
 *     0x18: Flash寫入錯誤
 * 
 * 註冊流程 (需採集2次):
 * 
 * 1. 第一次採集:
 *    - GenImg: 採集指紋圖像
 *    - Img2Tz(1): 提取特徵到CharBuffer1
 *    - 提示使用者抬起手指
 * 
 * 2. 第二次採集:
 *    - 等待手指移除
 *    - GenImg: 再次採集指紋圖像
 *    - Img2Tz(2): 提取特徵到CharBuffer2
 * 
 * 3. 生成模板:
 *    - RegModel: 合併CharBuffer1與CharBuffer2
 *    - 若兩次特徵不匹配，返回錯誤
 * 
 * 4. 儲存模板:
 *    - Store: 將模板儲存到指定PageID
 * 
 * 為什麼需要採集2次?
 * - 提高準確性: 確保兩次採集的特徵一致
 * - 濾除雜訊: 兩次比對可排除偶然的誤差
 * - 生成更穩定的模板: 綜合兩次採集的特徵點
 * 
 * @param id 指定儲存的位置 (PageID, 0~126)
 * @return true: 註冊成功 | false: 註冊失敗
 * 
 * TODO:
 * 1. 第一次採集 -> CharBuffer1
 * 2. 提示使用者抬起手指
 * 3. 等待手指移除
 * 4. 第二次採集 -> CharBuffer2
 * 5. 呼叫createModel()合併特徵
 * 6. 呼叫storeModel(id)儲存模板
 * 7. 更新templateCount
 * 8. 加入UI回饋與錯誤處理
 */
bool FingerprintHandler::enrollFingerprint(uint16_t id) {
    Serial.print("[指紋] 開始註冊指紋, ID=");
    Serial.println(id);
    
    // TODO: 第一次採集
    // Serial.println("[指紋] 請按壓手指 (第1次)...");
    // while (!detectFinger()) {
    //     delay(50);
    // }
    // 
    // if (!captureFingerprint(1)) {
    //     Serial.println("[指紋] 第一次採集失敗");
    //     return false;
    // }
    
    // TODO: 提示抬起手指
    // Serial.println("[指紋] 請抬起手指...");
    // if (!waitForFingerRemoval(5000)) {
    //     Serial.println("[指紋] 逾時: 請抬起手指");
    //     return false;
    // }
    
    // TODO: 第二次採集
    // Serial.println("[指紋] 請再次按壓同一根手指 (第2次)...");
    // while (!detectFinger()) {
    //     delay(50);
    // }
    // 
    // if (!captureFingerprint(2)) {
    //     Serial.println("[指紋] 第二次採集失敗");
    //     return false;
    // }
    
    // TODO: 合併特徵生成模板
    // uint8_t result = finger.createModel();
    // if (result != FINGERPRINT_OK) {
    //     if (result == FINGERPRINT_ENROLLMISMATCH) {
    //         Serial.println("[指紋] 兩次採集的指紋不匹配");
    //     } else {
    //         Serial.print("[指紋] 生成模板失敗: 0x");
    //         Serial.println(result, HEX);
    //     }
    //     return false;
    // }
    
    // TODO: 儲存模板
    // result = finger.storeModel(id);
    // if (result != FINGERPRINT_OK) {
    //     Serial.print("[指紋] 儲存失敗: 0x");
    //     Serial.println(result, HEX);
    //     return false;
    // }
    
    // TODO: 更新已註冊數量
    // templateCount++;
    
    Serial.println("[指紋] 註冊成功");
    return false;  // TODO: 實際註冊邏輯
}

/*
 * 刪除指定指紋
 * 
 * User Manual參考:
 * - Section 5.8: DeletChar - Delete Template (刪除模板)
 *   - 指令碼: 0x0C
 *   - 參數:
 *     - PageID (2 bytes): 起始位置
 *     - N (2 bytes): 刪除數量
 *   - 確認碼:
 *     0x00: 刪除成功
 *     0x10: 刪除失敗
 * 
 * 刪除模式:
 * - 單筆刪除: N=1
 * - 批次刪除: N>1，刪除PageID ~ PageID+N-1
 * 
 * @param id 要刪除的指紋ID (PageID, 0~126)
 * @param count 刪除數量 (預設1)
 * @return true: 刪除成功 | false: 刪除失敗
 * 
 * TODO:
 * 1. 檢查id是否在有效範圍
 * 2. 呼叫deleteModel(id, count)
 * 3. 檢查確認碼
 * 4. 更新templateCount
 */
bool FingerprintHandler::deleteFingerprint(uint16_t id, uint16_t count) {
    // TODO: 刪除指紋
    // uint8_t result = finger.deleteModel(id, count);
    // 
    // if (result == FINGERPRINT_OK) {
    //     Serial.print("[指紋] 刪除成功, ID=");
    //     Serial.println(id);
    //     templateCount -= count;
    //     return true;
    // } else {
    //     Serial.print("[指紋] 刪除失敗: 0x");
    //     Serial.println(result, HEX);
    //     return false;
    // }
    
    return false;  // TODO: 實際刪除邏輯
}

/*
 * 清空整個指紋資料庫
 * 
 * User Manual參考:
 * - Section 5.9: Empty - Clear Library (清空指紋庫)
 *   - 指令碼: 0x0D
 *   - 無參數
 *   - 確認碼:
 *     0x00: 清空成功
 *     0x11: 清空失敗
 * 
 * 警告: 此操作不可逆，會刪除所有已註冊的指紋
 * 
 * @return true: 清空成功 | false: 清空失敗
 * 
 * TODO:
 * 1. 加入二次確認機制
 * 2. 呼叫emptyDatabase()
 * 3. 重置templateCount為0
 */
bool FingerprintHandler::emptyDatabase() {
    // TODO: 清空指紋庫
    // Serial.println("[指紋] 警告: 即將清空所有指紋");
    // 
    // uint8_t result = finger.emptyDatabase();
    // 
    // if (result == FINGERPRINT_OK) {
    //     Serial.println("[指紋] 指紋庫已清空");
    //     templateCount = 0;
    //     return true;
    // } else {
    //     Serial.print("[指紋] 清空失敗: 0x");
    //     Serial.println(result, HEX);
    //     return false;
    // }
    
    return false;  // TODO: 實際清空邏輯
}

/*
 * 獲取指紋資料庫已註冊數量
 * 
 * User Manual參考:
 * - Section 5.10: TemplateNum - Get Template Count (取得模板數量)
 *   - 指令碼: 0x1D
 *   - 回傳TemplateNum (2 bytes)
 * 
 * @return 已註冊的指紋數量 (0~127)
 * 
 * TODO:
 * 1. 呼叫getTemplateCount()
 * 2. 更新templateCount成員變數
 * 3. 返回數量
 */
uint16_t FingerprintHandler::getEnrolledCount() {
    // TODO: 獲取已註冊數量
    // uint8_t result = finger.getTemplateCount();
    // 
    // if (result == FINGERPRINT_OK) {
    //     templateCount = finger.templateCount;
    //     return templateCount;
    // }
    
    return 0;  // TODO: 實際查詢邏輯
}

/*
 * 讀取系統參數
 * 
 * User Manual參考:
 * - Section 5.15: ReadSysPara - Read System Parameters
 * - Table 2: System Parameters Structure (16 bytes)
 * 
 * 系統參數結構:
 * 偏移  | 大小 | 名稱               | 說明
 * ------|------|--------------------|-----------------------
 * 0x00  | 2    | Status Register    | 狀態暫存器
 * 0x02  | 2    | System Identifier  | 系統識別碼 (0x0009)
 * 0x04  | 2    | Library Size       | 指紋庫容量 (127)
 * 0x06  | 2    | Security Level     | 安全等級 (1~5)
 * 0x08  | 4    | Device Address     | 模組地址 (0xFFFFFFFF)
 * 0x0C  | 2    | Data Package Size  | 封包大小 (32/64/128/256)
 * 0x0E  | 2    | Baud Rate Setting  | 波特率設定
 * 
 * @return true: 讀取成功 | false: 讀取失敗
 * 
 * TODO:
 * 1. 呼叫getParameters()
 * 2. 從finger物件讀取參數
 * 3. 更新成員變數
 * 4. 列印參數資訊
 */
bool FingerprintHandler::readSystemParameters() {
    // TODO: 讀取系統參數
    // uint8_t result = finger.getParameters();
    // 
    // if (result == FINGERPRINT_OK) {
    //     librarySize = finger.capacity;
    //     securityLevel = finger.securityLevel;
    //     
    //     Serial.println("[指紋] 系統參數:");
    //     Serial.print("  - 系統ID: 0x");
    //     Serial.println(finger.systemID, HEX);
    //     Serial.print("  - 指紋庫容量: ");
    //     Serial.println(librarySize);
    //     Serial.print("  - 安全等級: ");
    //     Serial.println(securityLevel);
    //     Serial.print("  - 封包大小: ");
    //     Serial.println(finger.packetLen);
    //     Serial.print("  - 波特率: ");
    //     Serial.println(finger.baudRate * 9600);
    //     
    //     return true;
    // }
    
    return false;  // TODO: 實際讀取邏輯
}

/*
 * 設定安全等級
 * 
 * User Manual參考:
 * - Section 5.12: SetSysPara - Set System Parameters (設定系統參數)
 *   - 指令碼: 0x0E
 *   - 參數:
 *     - Parameter Number (1 byte): 5=Security Level
 *     - Contents (1 byte): 1~5
 * 
 * 安全等級說明:
 * Level | FAR (False Acceptance Rate) | 說明
 * ------|-----------------------------|--------------
 * 1     | ≈ 0.001%                    | 最寬鬆，便利性高
 * 2     | ≈ 0.0001%                   | 寬鬆
 * 3     | ≈ 0.00001%                  | 平衡 (預設)
 * 4     | ≈ 0.000001%                 | 嚴格
 * 5     | ≈ 0.0000001%                | 最嚴格，安全性高
 * 
 * FAR vs FRR:
 * - FAR (False Acceptance Rate): 錯誤接受率
 *   - 將不同指紋誤判為相同的機率
 * - FRR (False Rejection Rate): 錯誤拒絕率
 *   - 將相同指紋誤判為不同的機率
 * 
 * 等級越高:
 * - FAR越低 (更安全，較少誤接受)
 * - FRR越高 (較不便利，較多誤拒絕)
 * 
 * 選擇建議:
 * - 一般應用: Level 3 (平衡)
 * - 高安全需求: Level 4~5
 * - 便利性優先: Level 1~2
 * 
 * @param level 安全等級 (1~5)
 * @return true: 設定成功 | false: 設定失敗
 * 
 * TODO:
 * 1. 檢查level範圍 (1~5)
 * 2. 呼叫setSystemParameter()
 * 3. 更新securityLevel成員變數
 */
bool FingerprintHandler::setSecurityLevel(uint8_t level) {
    // TODO: 檢查範圍
    // if (level < 1 || level > 5) {
    //     Serial.println("[指紋] 錯誤: 安全等級必須為1~5");
    //     return false;
    // }
    
    // TODO: 設定安全等級
    // uint8_t result = finger.setSystemParameter(5, level);
    // 
    // if (result == FINGERPRINT_OK) {
    //     securityLevel = level;
    //     Serial.print("[指紋] 安全等級已設為: ");
    //     Serial.println(level);
    //     return true;
    // } else {
    //     Serial.print("[指紋] 設定失敗: 0x");
    //     Serial.println(result, HEX);
    //     return false;
    // }
    
    return false;  // TODO: 實際設定邏輯
}

/*
 * LED控制 (部分AS608模組支援)
 * 
 * User Manual參考:
 * - Section 5.18: Control LED (控制LED)
 *   - 指令碼: 0x35 (部分版本)
 * 
 * 注意: 並非所有AS608模組都支援LED控制
 * 需查閱具體模組的datasheet
 * 
 * @param on true: 開啟LED | false: 關閉LED
 * 
 * TODO:
 * 1. 檢查模組是否支援LED控制
 * 2. 發送LED控制命令
 */
void FingerprintHandler::controlLED(bool on) {
    // TODO: 控制LED (若模組支援)
    // Serial.print("[指紋] LED: ");
    // Serial.println(on ? "ON" : "OFF");
}

// ============================================================================
// 私有函式實作
// ============================================================================

/*
 * 等待手指移除
 * 
 * 用途: 在雙次採集時，確保手指已抬起
 * 避免重複採集同一次按壓
 * 
 * 實作方式:
 * 持續呼叫getImage()檢測是否有手指
 * 當返回NOFINGER時表示手指已移除
 * 
 * @param timeout 等待逾時 (毫秒)
 * @return true: 手指已移除 | false: 逾時
 * 
 * TODO:
 * 1. 記錄開始時間
 * 2. 持續檢測手指
 * 3. 若逾時返回false
 * 4. 若手指移除返回true
 */
bool FingerprintHandler::waitForFingerRemoval(uint32_t timeout) {
    // TODO: 等待手指移除
    // unsigned long startTime = millis();
    // 
    // while ((millis() - startTime) < timeout) {
    //     uint8_t result = finger.getImage();
    //     if (result == FINGERPRINT_NOFINGER) {
    //         return true;  // 手指已移除
    //     }
    //     delay(100);
    // }
    
    return false;  // TODO: 實際等待邏輯
}

/*
 * 發送封包並接收回應 (低階函式)
 * 
 * User Manual參考:
 * - Section 4.2: Package Protocol (封包協定)
 * 
 * 封包結構 (9+N+2 bytes):
 * ┌────────┬─────────┬────────────┬────────┬──────┬──────────┐
 * │ Header │ Address │ Package ID │ Length │ Data │ Checksum │
 * ├────────┼─────────┼────────────┼────────┼──────┼──────────┤
 * │2 bytes │ 4 bytes │  1 byte    │2 bytes │N bytes│ 2 bytes │
 * └────────┴─────────┴────────────┴────────┴──────┴──────────┘
 * 
 * 欄位說明:
 * 
 * 1. Header (2 bytes): 0xEF01
 *    - 固定值，用於識別封包起始
 * 
 * 2. Address (4 bytes): 模組地址
 *    - 預設: 0xFFFFFFFF
 *    - 可修改以支援多個感應器
 * 
 * 3. Package ID (1 byte): 封包類型
 *    - 0x01: Command Packet (命令封包)
 *    - 0x02: Data Packet (資料封包)
 *    - 0x07: Ack Packet (確認封包)
 *    - 0x08: End of Data Packet (資料結束封包)
 * 
 * 4. Length (2 bytes): 資料長度 + 校驗和長度
 *    - Length = N + 2
 *    - 高位元組在前 (Big-Endian)
 * 
 * 5. Data (N bytes): 命令碼 + 參數
 *    - Byte 0: 命令碼 (Instruction Code)
 *    - Byte 1~N-1: 參數 (Parameters)
 * 
 * 6. Checksum (2 bytes): 校驗和
 *    - 計算方式: Sum = PID + Length + Data[0..N-1]
 *    - 取低16位元
 *    - 高位元組在前
 * 
 * 範例: VfyPwd命令 (驗證密碼0x12345678)
 * 
 * 封包內容:
 * EF 01                    // Header
 * FF FF FF FF              // Address
 * 01                       // Package ID (Command)
 * 00 07                    // Length (7 = 1+4+2)
 * 13                       // Instruction Code (VfyPwd)
 * 12 34 56 78              // Password
 * 00 9D                    // Checksum (0x01+0x07+0x13+0x12+0x34+0x56+0x78=0x019D)
 * 
 * Adafruit_Fingerprint庫已實作完整的封包協定
 * 此函式供學習參考，實際使用庫提供的API即可
 * 
 * @param data 資料陣列 (命令碼+參數)
 * @param length 資料長度
 * @return true: 發送並接收成功 | false: 失敗
 * 
 * TODO: (選用) 若需要自定義協定，實作封包編碼/解碼
 */
bool FingerprintHandler::sendPacket(uint8_t* data, uint16_t length) {
    // Adafruit庫已包含封包處理
    // 此函式僅供學習封包協定使用
    return false;
}

/*
 * ============================================================================
 * AS608確認碼說明
 * ============================================================================
 * 
 * User Manual Section 5: Instruction System
 * 
 * 每個命令執行後，AS608會返回確認碼 (Confirmation Code)
 * 確認碼位於回應封包的Data[0]
 * 
 * 通用確認碼:
 * 0x00 - OK: 命令執行完成，成功
 * 0x01 - PACKETRECIEVEERR: 收包錯誤
 * 0x0E - BADLOCATION: 無法找到指定位置
 * 0x0F - DBREADFAIL: 從Flash讀取模板失敗
 * 0x10 - DBRANGEFAIL: 刪除模板失敗
 * 0x11 - DBCLEARFAIL: 清空指紋庫失敗
 * 0x18 - FLASHERR: Flash讀寫錯誤
 * 0x1A - INVALIDREG: 無效的暫存器號碼
 * 
 * GenImg命令 (0x01):
 * 0x02 - NOFINGER: 感應器上沒有手指
 * 0x03 - IMAGEFAIL: 錄入指紋圖像失敗
 * 
 * Img2Tz命令 (0x02):
 * 0x06 - FEATUREFAIL: 特徵點太少，無法生成特徵文件
 * 0x07 - IMAGEMESS: 圖像太亂，無法生成特徵文件
 * 0x15 - INVALIDIMAGE: ImageBuffer中沒有有效原始圖像
 * 
 * Match命令 (0x03):
 * 0x08 - NOMATCH: 特徵點太少，無法匹配
 * 
 * Search命令 (0x04):
 * 0x09 - NOTFOUND: 沒有找到匹配的指紋
 * 
 * RegModel命令 (0x05):
 * 0x0A - ENROLLMISMATCH: 兩次採集的特徵文件不匹配，無法合併
 * 
 * Store命令 (0x06):
 * 0x0B - BADLOCATION: 訪問指紋庫地址超出範圍
 * 
 * VfyPwd命令 (0x13):
 * 0x13 - PASSFAIL: 密碼錯誤
 * 
 * ============================================================================
 * 學習重點總結
 * ============================================================================
 * 
 * 1. UART通訊
 *    - 非同步串列通訊
 *    - 8N1格式 (8資料位元，無校驗，1停止位元)
 *    - 波特率可配置 (9600~115200)
 * 
 * 2. 封包協定
 *    - Header識別封包起始
 *    - 校驗和確保資料完整性
 *    - 確認碼指示命令執行結果
 * 
 * 3. 指紋辨識流程
 *    - 採集圖像 (GenImg)
 *    - 特徵提取 (Img2Tz)
 *    - 模板比對 (Search/Match)
 * 
 * 4. 註冊流程
 *    - 雙次採集確保準確性
 *    - 特徵合併生成模板
 *    - 儲存到Flash持久化
 * 
 * 5. 安全等級
 *    - FAR vs FRR權衡
 *    - 根據應用場景選擇
 * 
 * 6. Adafruit庫優勢
 *    - 封裝底層協定
 *    - 簡化命令發送
 *    - 自動處理校驗和
 * 
 * ============================================================================
 */
