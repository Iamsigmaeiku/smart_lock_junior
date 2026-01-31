/*
 * RFID模組處理器 - MFRC522
 * 
 * 參考文檔: MFRC522 Datasheet
 * 主要章節:
 * - Section 8.1: SPI Interface (SPI通訊協定)
 * - Section 9.3: Card Detection and Anti-collision (卡片偵測與防碰撞)
 * - Section 10: MIFARE Classic Commands (MIFARE卡片命令)
 * 
 * 學習目標:
 * 1. 理解SPI通訊協定與時序
 * 2. 學習ISO/IEC 14443A標準的RFID通訊
 * 3. 掌握PICC (Proximity Integrated Circuit Card) 命令流程
 */

#ifndef RFID_HANDLER_H
#define RFID_HANDLER_H

#include <Arduino.h>
#include <MFRC522.h>
#include <SPI.h>
#include "config.h"

class RFIDHandler {
public:
    // 建構函式
    RFIDHandler();
    
    // 解構函式
    ~RFIDHandler();
    
    /*
     * 初始化MFRC522模組
     * 
     * Datasheet參考:
     * - Section 8.1.1: SPI Mode (SPI模式配置)
     * - Section 8.6.1: Power-On Reset (上電重置時序)
     * - Section 9.1: Internal States (內部狀態機)
     * 
     * 初始化流程:
     * 1. 配置SPI匯流排 (CPOL=0, CPHA=0, 最大10MHz)
     * 2. 執行Soft Reset (軟體重置)
     * 3. 設定Timer Mode (定時器模式)
     * 4. 啟動天線 (Antenna On)
     * 
     * @return true: 初始化成功 | false: 初始化失敗
     * 
     * TODO: 實作SPI通訊測試，讀取Version Register (0x37)驗證連線
     *       預期值: 0x92 (MFRC522 v2.0)
     */
    bool init();
    
    /*
     * 檢測是否有卡片靠近
     * 
     * Datasheet參考:
     * - Section 9.3.3: REQA and WUPA Commands (請求命令)
     * - Section 10.1: Request Command (0x26 = REQA)
     * - Table 149: REQA Response Format (回應格式)
     * 
     * 工作原理:
     * 1. 發送REQA命令 (0x26) 喚醒待機卡片
     * 2. 等待ATQA (Answer To Request) 回應
     * 3. ATQA包含卡片類型資訊 (2 bytes)
     * 
     * 非阻塞設計: 此函式應該快速返回，避免阻塞主迴圈
     * 
     * @return true: 偵測到卡片 | false: 無卡片
     * 
     * TODO: 實作REQA命令發送與ATQA接收
     *       注意時序: REQA後需等待ATQA (約4.7ms)
     */
    bool isCardPresent();
    
    /*
     * 讀取卡片UID (Unique Identifier)
     * 
     * Datasheet參考:
     * - Section 9.3.4: Anti-collision Loop (防碰撞迴圈)
     * - Section 10.2: Anti-collision Command (0x93 = SEL_CL1)
     * - Table 17: UID Size (UID長度: 4/7/10 bytes)
     * 
     * ISO/IEC 14443A標準:
     * - Single Size UID: 4 bytes (常見於MIFARE Classic 1K)
     * - Double Size UID: 7 bytes
     * - Triple Size UID: 10 bytes
     * 
     * 防碰撞演算法:
     * 當多張卡片同時進入感應範圍，MFRC522會執行位元級碰撞偵測
     * 透過遞迴選擇演算法，逐一選中每張卡片
     * 
     * @param uid[out] 存放讀取到的UID (需預先配置4-10 bytes空間)
     * @param uidSize[out] UID的實際長度
     * @return true: 讀取成功 | false: 讀取失敗
     * 
     * TODO: 實作Anti-collision演算法 (Section 9.3.5)
     *       處理位元碰撞 (Bit Collision Detection)
     */
    bool readCardUID(uint8_t* uid, uint8_t* uidSize);
    
    /*
     * 驗證卡片UID是否在白名單中
     * 
     * 安全性考量:
     * - UID可被複製，不應作為唯一的安全驗證
     * - 建議搭配Crypto1加密認證 (Datasheet Section 9.4)
     * - 或使用更安全的MIFARE DESFire / NTAG系列
     * 
     * 白名單管理:
     * - 可儲存在EEPROM或SPIFFS檔案系統
     * - 支援動態新增/刪除授權卡片
     * 
     * @param uid 待驗證的卡片UID
     * @param uidSize UID長度
     * @return true: 卡片已授權 | false: 卡片未授權
     * 
     * TODO: 實作白名單比對邏輯
     *       考慮使用EEPROM儲存授權UID列表
     */
    bool verifyCard(const uint8_t* uid, uint8_t uidSize);
    
    /*
     * 停止當前卡片通訊
     * 
     * Datasheet參考:
     * - Section 9.3.7: HLTA Command (Halt Command)
     * - Command Code: 0x50
     * 
     * 功能:
     * 將選中的卡片設為Halt State (停止狀態)
     * 卡片進入低功耗模式，不再回應REQA
     * 需要WUPA (0x52) 命令才能再次喚醒
     * 
     * TODO: 實作HLTA命令發送
     */
    void haltCard();
    
    /*
     * 獲取最後一次讀取的UID
     * 
     * @return 指向UID緩衝區的指標
     */
    const uint8_t* getLastUID() const;
    
    /*
     * 獲取最後一次讀取的UID長度
     * 
     * @return UID長度 (bytes)
     */
    uint8_t getLastUIDSize() const;
    
    /*
     * 列印UID到Serial (用於除錯)
     * 
     * 格式: XX:XX:XX:XX (16進位)
     * 
     * @param uid UID陣列
     * @param uidSize UID長度
     */
    void printUID(const uint8_t* uid, uint8_t uidSize);

private:
    // MFRC522物件實例
    MFRC522 mfrc522;
    
    // UID緩衝區
    uint8_t lastUID[10];      // 最大10 bytes (Triple Size UID)
    uint8_t lastUIDSize;      // 實際UID長度
    
    /*
     * 授權UID白名單 (示範用)
     * 
     * 實際應用建議:
     * - 儲存在非揮發性記憶體 (EEPROM/Flash)
     * - 支援動態管理 (新增/刪除)
     * - 加密儲存 (避免記憶體讀取攻擊)
     * 
     * TODO: 將白名單移至EEPROM或SPIFFS
     *       參考: ESP32 EEPROM Library或Preferences Library
     */
    static const uint8_t MAX_AUTHORIZED_CARDS = 10;
    uint8_t authorizedUIDs[MAX_AUTHORIZED_CARDS][10];
    uint8_t authorizedUIDSizes[MAX_AUTHORIZED_CARDS];
    uint8_t authorizedCardCount;
    
    /*
     * 新增授權卡片到白名單
     * 
     * @param uid 卡片UID
     * @param uidSize UID長度
     * @return true: 新增成功 | false: 白名單已滿
     * 
     * TODO: 實作白名單新增邏輯
     */
    bool addAuthorizedCard(const uint8_t* uid, uint8_t uidSize);
    
    /*
     * 比對兩個UID是否相同
     * 
     * @param uid1 UID 1
     * @param uid2 UID 2
     * @param size UID長度
     * @return true: 相同 | false: 不同
     */
    bool compareUID(const uint8_t* uid1, const uint8_t* uid2, uint8_t size);
};

/*
 * ============================================================================
 * MFRC522關鍵暫存器說明
 * ============================================================================
 * 
 * Datasheet Section 9.2: Register Description
 * 
 * 常用暫存器 (透過SPI讀寫):
 * 
 * 1. CommandReg (0x01) - 命令暫存器
 *    - 控制MFRC522執行的命令
 *    - 常用命令: Idle(0x00), Transceive(0x0C), Authenticate(0x0E)
 * 
 * 2. ComIEnReg (0x02) - 中斷啟用暫存器
 *    - 設定哪些事件會觸發IRQ中斷
 * 
 * 3. DivIEnReg (0x03) - 分頻器中斷啟用
 * 
 * 4. ComIrqReg (0x04) - 中斷請求位元
 *    - 顯示當前中斷狀態
 * 
 * 5. ErrorReg (0x06) - 錯誤標誌
 *    - 位元0: ProtocolErr (協定錯誤)
 *    - 位元1: ParityErr (校驗錯誤)
 *    - 位元2: CRCErr (CRC錯誤)
 *    - 位元4: CollErr (碰撞錯誤)
 * 
 * 6. FIFODataReg (0x09) - FIFO資料暫存器
 *    - 讀寫FIFO緩衝區 (64 bytes)
 * 
 * 7. FIFOLevelReg (0x0A) - FIFO長度暫存器
 *    - 顯示FIFO中的資料位元組數
 * 
 * 8. ControlReg (0x0C) - 控制暫存器
 * 
 * 9. BitFramingReg (0x0D) - 位元框架調整
 *    - 用於防碰撞演算法的位元級傳輸
 * 
 * 10. ModeReg (0x11) - 模式暫存器
 *     - 設定CRC、極性等參數
 * 
 * 11. TxControlReg (0x14) - 天線驅動器控制
 *     - 位元0/1: Tx1RFEn, Tx2RFEn (啟用天線)
 * 
 * 12. VersionReg (0x37) - 版本暫存器 (唯讀)
 *     - 讀取值: 0x92 (MFRC522 v2.0)
 *     - 用於驗證模組連線
 * 
 * ============================================================================
 */

#endif // RFID_HANDLER_H
