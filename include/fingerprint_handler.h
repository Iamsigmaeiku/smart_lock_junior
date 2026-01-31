/*
 * 指紋感應器模組處理器 - AS608
 * 
 * 參考文檔: AS608 User Manual V2.0
 * 主要章節:
 * - Section 4: Communication Protocol (通訊協定)
 * - Section 5: Instruction System (指令系統)
 * - Section 6: Application Notes (應用注意事項)
 * 
 * 學習目標:
 * 1. 理解UART非同步串列通訊
 * 2. 學習封包協定設計 (Header + Data + Checksum)
 * 3. 掌握指紋辨識流程 (圖像採集 -> 特徵提取 -> 比對)
 */

#ifndef FINGERPRINT_HANDLER_H
#define FINGERPRINT_HANDLER_H

#include <Arduino.h>
#include <Adafruit_Fingerprint.h>
#include "config.h"

class FingerprintHandler {
public:
    // 建構函式
    FingerprintHandler();
    
    // 解構函式
    ~FingerprintHandler();
    
    /*
     * 初始化AS608指紋感應器
     * 
     * User Manual參考:
     * - Section 4.1: UART Communication (UART通訊設定)
     * - Section 5.1: VfyPwd - Verify Password (密碼驗證)
     * - Table 1: Default Parameters (預設參數)
     * 
     * 通訊參數:
     * - 波特率: 57600 bps (預設，可配置9600~115200)
     * - 資料位元: 8 bits
     * - 校驗位元: None
     * - 停止位元: 1 bit
     * - 模組地址: 0xFFFFFFFF (預設)
     * - 密碼: 0x00000000 (預設)
     * 
     * 初始化流程:
     * 1. 初始化UART2 (Hardware Serial)
     * 2. 發送VfyPwd命令驗證模組密碼
     * 3. 讀取系統參數 (ReadSysPara)
     * 4. 確認指紋庫容量與安全等級
     * 
     * @return true: 初始化成功 | false: 初始化失敗或密碼錯誤
     * 
     * TODO: 實作UART連線測試與密碼驗證
     *       錯誤碼: 0x00=成功, 0x01=收包錯誤, 0x13=密碼錯誤
     */
    bool init();
    
    /*
     * 檢測是否有手指放置在感應器上
     * 
     * User Manual參考:
     * - Section 5.2: GenImg - Generate Image (採集指紋圖像)
     * - Section 6.1: Fingerprint Registration (指紋註冊流程)
     * 
     * 工作原理:
     * AS608使用光學或半導體感應器偵測手指
     * 當手指覆蓋感應區域，內部ADC會產生指紋圖像
     * 
     * 指令流程:
     * 1. 發送GenImg命令 (0x01)
     * 2. 等待確認碼回應
     *    - 0x00: 採集成功
     *    - 0x02: 無法偵測手指
     *    - 0x03: 指紋圖像不清晰
     * 
     * 非阻塞設計: 
     * 此函式應快速檢測並返回，避免阻塞主迴圈
     * 建議搭配狀態機實作非阻塞式採集
     * 
     * @return true: 偵測到手指 | false: 無手指
     * 
     * TODO: 實作GenImg命令發送與確認碼判斷
     *       注意: 採集圖像需要時間，不要頻繁呼叫 (建議>100ms間隔)
     */
    bool detectFinger();
    
    /*
     * 採集並處理指紋圖像
     * 
     * User Manual參考:
     * - Section 5.3: Img2Tz - Image to Template (圖像轉特徵)
     * - Section 6.2: Feature Extraction (特徵提取)
     * 
     * 指紋辨識原理:
     * AS608不儲存指紋圖像，而是儲存特徵模板 (Template)
     * 特徵模板提取指紋的關鍵特徵點 (Minutiae):
     * - 端點 (Ending)
     * - 分叉點 (Bifurcation)
     * - 短紋 (Short Ridge)
     * 
     * 指令流程:
     * 1. GenImg: 採集指紋圖像 -> ImageBuffer
     * 2. Img2Tz: 提取特徵 -> CharBuffer1 或 CharBuffer2
     * 
     * CharBuffer用途:
     * - CharBuffer1: 存放第一次採集的特徵
     * - CharBuffer2: 存放第二次採集的特徵 (用於比對或合併)
     * 
     * @param bufferID 特徵緩衝區編號 (1或2)
     * @return true: 採集成功 | false: 採集失敗
     * 
     * TODO: 實作Img2Tz命令，處理圖像品質判斷
     *       確認碼: 0x00=成功, 0x06=特徵點過少, 0x07=圖像混亂
     */
    bool captureFingerprint(uint8_t bufferID = 1);
    
    /*
     * 搜尋並驗證指紋
     * 
     * User Manual參考:
     * - Section 5.4: Search - Search for Template (搜尋指紋庫)
     * - Section 5.5: Match - Match Two Templates (比對兩個特徵)
     * 
     * 搜尋流程:
     * 1. 採集當前指紋特徵 (存於CharBuffer1)
     * 2. 發送Search命令
     * 3. AS608在指紋庫中逐一比對
     * 4. 返回匹配的PageID與信心分數 (Score)
     * 
     * 搜尋參數:
     * - BufferID: 1 或 2 (指定要搜尋的特徵緩衝區)
     * - StartPage: 搜尋起始位置 (0~126)
     * - PageNum: 搜尋頁數
     * 
     * 信心分數 (Confidence Score):
     * - 範圍: 0~255
     * - 分數越高表示相似度越高
     * - 建議閾值: 50~100 (可調整安全等級)
     * 
     * @param id[out] 匹配的指紋ID (PageID)
     * @param confidence[out] 信心分數 (0~255)
     * @return true: 找到匹配指紋 | false: 無匹配或錯誤
     * 
     * TODO: 實作Search命令，處理搜尋範圍與閾值設定
     *       確認碼: 0x00=找到, 0x09=未找到匹配指紋
     */
    bool verifyFingerprint(uint16_t& id, uint16_t& confidence);
    
    /*
     * 註冊新指紋
     * 
     * User Manual參考:
     * - Section 5.6: RegModel - Generate Template (生成模板)
     * - Section 5.7: Store - Store Template (儲存模板)
     * - Section 6.1: Fingerprint Registration (註冊流程)
     * 
     * 註冊流程 (需採集2次):
     * 1. 採集第1次指紋 -> CharBuffer1
     * 2. 提示使用者抬起手指
     * 3. 採集第2次指紋 -> CharBuffer2
     * 4. RegModel: 合併CharBuffer1與CharBuffer2 -> Template
     * 5. Store: 將Template儲存到指定PageID
     * 
     * 為什麼需要採集2次?
     * - 提高準確性: 確保兩次採集的特徵一致
     * - 濾除雜訊: 兩次比對可排除偶然的誤差
     * - 生成更穩定的模板
     * 
     * @param id 指定儲存的位置 (PageID, 0~126)
     * @return true: 註冊成功 | false: 註冊失敗
     * 
     * TODO: 實作雙次採集邏輯，加入使用者提示 (顯示器/LED/蜂鳴器)
     *       RegModel確認碼: 0x00=合併成功, 0x0A=特徵不匹配
     */
    bool enrollFingerprint(uint16_t id);
    
    /*
     * 刪除指定指紋
     * 
     * User Manual參考:
     * - Section 5.8: DeletChar - Delete Template (刪除模板)
     * 
     * 刪除模式:
     * - 單筆刪除: 刪除指定PageID
     * - 批次刪除: 刪除指定範圍 (StartPage ~ StartPage+N)
     * 
     * @param id 要刪除的指紋ID (PageID)
     * @param count 刪除數量 (預設1)
     * @return true: 刪除成功 | false: 刪除失敗
     * 
     * TODO: 實作DeletChar命令
     */
    bool deleteFingerprint(uint16_t id, uint16_t count = 1);
    
    /*
     * 清空整個指紋資料庫
     * 
     * User Manual參考:
     * - Section 5.9: Empty - Clear Library (清空指紋庫)
     * 
     * 警告: 此操作不可逆，會刪除所有已註冊的指紋
     * 
     * @return true: 清空成功 | false: 清空失敗
     * 
     * TODO: 實作Empty命令，建議加入二次確認機制
     */
    bool emptyDatabase();
    
    /*
     * 獲取指紋資料庫已註冊數量
     * 
     * User Manual參考:
     * - Section 5.10: TemplateNum - Get Template Count (取得模板數量)
     * 
     * @return 已註冊的指紋數量 (0~127)
     * 
     * TODO: 實作TemplateNum命令
     */
    uint16_t getEnrolledCount();
    
    /*
     * 讀取系統參數
     * 
     * User Manual參考:
     * - Section 5.11: ReadSysPara - Read System Parameters (讀取系統參數)
     * - Table 2: System Parameters (系統參數表)
     * 
     * 系統參數包含:
     * - 狀態暫存器 (Status Register)
     * - 系統識別碼 (System Identifier)
     * - 指紋庫容量 (Library Size): 127
     * - 安全等級 (Security Level): 1~5 (等級越高越嚴格)
     * - 模組地址 (Device Address)
     * - 資料封包大小 (Data Package Size): 32/64/128/256 bytes
     * - 波特率倍數 (Baud Rate Setting)
     * 
     * TODO: 實作ReadSysPara命令，解析系統參數結構
     */
    bool readSystemParameters();
    
    /*
     * 設定安全等級
     * 
     * User Manual參考:
     * - Section 5.12: SetSysPara - Set System Parameters (設定系統參數)
     * - Parameter Number 5: Security Level
     * 
     * 安全等級說明:
     * - Level 1: FAR (False Acceptance Rate) ≈ 0.001% (最寬鬆)
     * - Level 2: FAR ≈ 0.0001%
     * - Level 3: FAR ≈ 0.00001% (平衡)
     * - Level 4: FAR ≈ 0.000001%
     * - Level 5: FAR ≈ 0.0000001% (最嚴格)
     * 
     * FAR vs FRR:
     * - FAR (False Acceptance Rate): 錯誤接受率
     * - FRR (False Rejection Rate): 錯誤拒絕率
     * - 等級越高，FAR越低但FRR越高 (更安全但更容易誤拒)
     * 
     * @param level 安全等級 (1~5)
     * @return true: 設定成功 | false: 設定失敗
     * 
     * TODO: 實作SetSysPara命令
     */
    bool setSecurityLevel(uint8_t level);
    
    /*
     * LED控制 (部分AS608模組支援)
     * 
     * User Manual參考:
     * - Section 5.18: Control LED (控制LED)
     * 
     * 注意: 並非所有AS608模組都支援此功能
     * 
     * @param on true: 開啟LED | false: 關閉LED
     * 
     * TODO: 實作LED控制命令 (若模組支援)
     */
    void controlLED(bool on);

private:
    // Adafruit指紋感應器物件
    Adafruit_Fingerprint finger;
    
    // Hardware Serial物件 (UART2)
    HardwareSerial* fpSerial;
    
    // 系統參數快取
    uint16_t templateCount;    // 已註冊指紋數量
    uint16_t librarySize;      // 指紋庫容量
    uint8_t securityLevel;     // 當前安全等級
    
    /*
     * 等待手指移除
     * 
     * 用途: 在雙次採集時，確保手指已抬起
     * 避免重複採集同一次按壓
     * 
     * @param timeout 等待逾時 (毫秒)
     * @return true: 手指已移除 | false: 逾時
     * 
     * TODO: 實作手指移除偵測
     */
    bool waitForFingerRemoval(uint32_t timeout = 5000);
    
    /*
     * 發送封包並接收回應 (低階函式)
     * 
     * User Manual參考:
     * - Section 4.2: Package Protocol (封包協定)
     * 
     * 封包結構:
     * ┌────────────────────────────────────────────────────────┐
     * │ Header │ Address │ Package ID │ Length │ Data │ Checksum │
     * ├────────┼─────────┼────────────┼────────┼──────┼──────────┤
     * │ 2 bytes│ 4 bytes │  1 byte    │2 bytes │N bytes│ 2 bytes │
     * └────────────────────────────────────────────────────────┘
     * 
     * Header: 0xEF01 (固定值)
     * Address: 模組地址 (預設0xFFFFFFFF)
     * Package ID:
     *   - 0x01: Command Packet (命令封包)
     *   - 0x02: Data Packet (資料封包)
     *   - 0x07: Ack Packet (確認封包)
     *   - 0x08: End of Data Packet (資料結束封包)
     * Length: 資料長度 + 校驗和長度 (N+2)
     * Data: 指令碼 + 參數
     * Checksum: Package ID + Length + Data 的總和
     * 
     * TODO: 若使用自定義協定，需實作封包編碼/解碼
     *       (Adafruit庫已包含此功能)
     */
    bool sendPacket(uint8_t* data, uint16_t length);
};

/*
 * ============================================================================
 * AS608指令表 (Command Instruction)
 * ============================================================================
 * 
 * User Manual Section 5: Instruction System
 * 
 * 基本指令:
 * 0x01 - GenImg: 採集指紋圖像
 * 0x02 - Img2Tz: 生成特徵文件
 * 0x03 - Match: 精確比對兩個特徵文件
 * 0x04 - Search: 搜尋指紋庫
 * 0x05 - RegModel: 合併特徵文件並生成模板
 * 0x06 - Store: 儲存模板到Flash
 * 0x07 - LoadChar: 從Flash讀取模板到緩衝區
 * 0x0C - DeletChar: 刪除模板
 * 0x0D - Empty: 清空指紋庫
 * 0x0E - SetSysPara: 設定系統參數
 * 0x0F - ReadSysPara: 讀取系統參數
 * 0x11 - TemplateNum: 讀取已註冊指紋數量
 * 0x13 - VfyPwd: 驗證密碼
 * 
 * 進階指令 (部分模組支援):
 * 0x35 - Control: LED燈控制
 * 0x36 - ReadIndexTable: 讀取索引表
 * 
 * ============================================================================
 * 確認碼 (Confirmation Code)
 * ============================================================================
 * 
 * 0x00 - OK: 命令執行完成
 * 0x01 - 收包錯誤
 * 0x02 - 感應器上沒有手指
 * 0x03 - 錄入指紋圖像失敗
 * 0x06 - 特徵點太少，無法生成特徵文件
 * 0x07 - 圖像太亂，無法生成特徵文件
 * 0x08 - 特徵點太少，無法匹配
 * 0x09 - 沒有找到匹配的指紋
 * 0x0A - 特徵合併失敗
 * 0x0B - 訪問指紋庫地址超出範圍
 * 0x10 - 刪除模板失敗
 * 0x11 - 清空指紋庫失敗
 * 0x13 - 密碼錯誤
 * 0x15 - 緩衝區內沒有有效原始圖像
 * 0x18 - 讀寫Flash錯誤
 * 
 * ============================================================================
 */

#endif // FINGERPRINT_HANDLER_H
