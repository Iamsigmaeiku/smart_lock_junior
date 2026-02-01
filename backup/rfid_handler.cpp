/*
 * RFID模組處理器 - 實作檔案
 * 
 * 參考文檔:
 * - MFRC522 Datasheet Section 8.1: SPI Interface
 * - Section 9.3: Card Detection and Anti-collision
 * - Section 10: PICC Command Set
 * 
 * 本檔案包含MFRC522 RFID讀卡器的所有函式實作骨架
 * 搭配詳細的datasheet章節標註與ISO14443A協定說明
 */

#include "rfid_handler.h"

// ============================================================================
// 建構函式與解構函式
// ============================================================================

/*
 * 建構函式
 * 
 * 初始化MFRC522物件與成員變數
 * 
 * MFRC522建構函式參數:
 * - SS Pin: Slave Select腳位 (片選)
 * - RST Pin: Reset腳位 (重置)
 */
RFIDHandler::RFIDHandler() 
    : mfrc522(RFID_SS_PIN, RFID_RST_PIN),
      lastUIDSize(0),
      authorizedCardCount(0)
{
    // 初始化UID緩衝區
    memset(lastUID, 0, sizeof(lastUID));
    
    // 初始化白名單
    memset(authorizedUIDs, 0, sizeof(authorizedUIDs));
    memset(authorizedUIDSizes, 0, sizeof(authorizedUIDSizes));
}

/*
 * 解構函式
 * 
 * 停止天線，釋放資源
 */
RFIDHandler::~RFIDHandler() {
    // 停止天線以節省電力
    // mfrc522.PCD_AntennaOff();
}

// ============================================================================
// 公開函式實作
// ============================================================================

/*
 * 初始化MFRC522模組
 * 
 * Datasheet參考:
 * - Section 8.1.1: SPI Mode (SPI模式配置)
 *   - SPI Mode 0: CPOL=0, CPHA=0
 *   - 最大時鐘頻率: 10MHz
 *   - 位元順序: MSB First
 * 
 * - Section 8.6.1: Power-On Reset (上電重置時序)
 *   - 上電後約37ms進入Idle狀態
 *   - 建議執行軟體重置確保穩定
 * 
 * - Section 9.1: Internal States (內部狀態機)
 *   - PowerDown -> Idle -> Transmit/Receive
 * 
 * - Section 9.2.2.3: Soft Reset Command
 *   - CommandReg寫入0x0F執行軟體重置
 * 
 * 初始化流程:
 * 1. SPI匯流排初始化 (由Arduino完成)
 * 2. MFRC522物件初始化 (PCD_Init)
 * 3. 讀取Version Register驗證通訊
 * 4. 啟動天線 (PCD_AntennaOn)
 * 5. 設定天線增益 (可選)
 * 
 * @return true: 初始化成功 | false: 初始化失敗
 * 
 * TODO:
 * 1. 呼叫mfrc522.PCD_Init()初始化
 * 2. 讀取Version Register (0x37)驗證連線
 * 3. 啟動天線
 * 4. 可選: 設定天線增益
 * 5. 加入錯誤處理
 */
bool RFIDHandler::init() {
    Serial.println("[RFID] 初始化MFRC522模組...");
    
    /*
     * PCD_Init() 函式
     * 
     * 功能:
     * - 執行軟體重置
     * - 設定Timer Mode
     * - 配置Tx/Rx參數
     * - 啟動CRC協處理器
     * 
     * 內部流程:
     * 1. 發送Soft Reset命令
     * 2. 等待PowerDown位元清除
     * 3. 設定TModeReg、TPrescalerReg (定時器)
     * 4. 設定TxAutoReg (自動傳輸)
     * 5. 設定ModeReg (CRC初始值)
     */
    
    // TODO: 初始化MFRC522
    // mfrc522.PCD_Init();
    
    /*
     * 讀取Version Register驗證通訊
     * 
     * Datasheet Section 9.3.4.8: Version Register (0x37)
     * - 位址: 0x37 (只讀)
     * - MFRC522 v2.0: 返回 0x92
     * - MFRC522 v1.0: 返回 0x91
     * 
     * 用途: 驗證SPI通訊是否正常
     */
    
    // TODO: 讀取版本暫存器
    // byte version = mfrc522.PCD_ReadRegister(mfrc522.VersionReg);
    // Serial.print("[RFID] 版本: 0x");
    // Serial.println(version, HEX);
    // 
    // if (version != 0x92 && version != 0x91) {
    //     Serial.println("[RFID] 錯誤: 無法偵測MFRC522模組");
    //     return false;
    // }
    
    /*
     * 啟動天線
     * 
     * Datasheet Section 9.3.1.7: TxControlReg (0x14)
     * - Bit 0: Tx1RFEn - 啟用天線輸出TX1
     * - Bit 1: Tx2RFEn - 啟用天線輸出TX2
     * - Bit 5: InvTx2RFOn - TX2訊號反相
     * 
     * 天線必須啟動才能進行RFID通訊
     */
    
    // TODO: 啟動天線
    // mfrc522.PCD_AntennaOn();
    // Serial.println("[RFID] 天線已啟動");
    
    /*
     * 設定天線增益 (選用)
     * 
     * Datasheet Section 9.3.3.5: RFCfgReg (0x26)
     * - Bit 6-4: RxGain - 接收增益
     *   - 000: 18 dB (最小)
     *   - 111: 48 dB (最大)
     * 
     * 預設值: 48 dB (最大增益)
     * 高增益可提升讀取距離，但可能增加誤判
     */
    
    // TODO: 設定天線增益 (選用)
    // mfrc522.PCD_SetAntennaGain(mfrc522.RxGain_max);
    
    Serial.println("[RFID] 初始化完成");
    return true;  // TODO: 根據實際結果返回
}

/*
 * 檢測是否有卡片靠近
 * 
 * Datasheet參考:
 * - Section 9.3.3: REQA and WUPA Commands
 *   - REQA (0x26): Request Type A - 喚醒Idle狀態的卡片
 *   - WUPA (0x52): Wake-Up Type A - 喚醒Halt狀態的卡片
 * 
 * - Section 10.1: Request Command (ISO14443A)
 *   - 短幀格式: 7位元命令
 *   - ATQA回應: 2 bytes (Answer To Request Type A)
 * 
 * ATQA格式 (Table 149):
 * - Byte 1: UID size, bit frame anticollision
 * - Byte 2: Platform information
 * 
 * MFRC522庫函式:
 * - PICC_IsNewCardPresent(): 偵測新卡片
 * - PICC_RequestA(): 發送REQA命令
 * 
 * 非阻塞設計:
 * 此函式應快速返回，避免阻塞主迴圈
 * 通常在10ms內完成
 * 
 * @return true: 偵測到卡片 | false: 無卡片
 * 
 * TODO:
 * 1. 呼叫PICC_IsNewCardPresent()檢測卡片
 * 2. 若偵測到，呼叫PICC_ReadCardSerial()準備讀取
 * 3. 加入防抖動機制 (避免重複偵測)
 */
bool RFIDHandler::isCardPresent() {
    // TODO: 檢測卡片是否存在
    // if (!mfrc522.PICC_IsNewCardPresent()) {
    //     return false;  // 無卡片
    // }
    
    // TODO: 準備讀取卡片序號
    // if (!mfrc522.PICC_ReadCardSerial()) {
    //     return false;  // 讀取失敗
    // }
    
    // 偵測到卡片
    return false;  // TODO: 根據實際結果返回
}

/*
 * 讀取卡片UID (Unique Identifier)
 * 
 * Datasheet參考:
 * - Section 9.3.4: Anti-collision Loop (防碰撞迴圈)
 * - Section 10.2: Anti-collision and Select (防碰撞與選擇)
 * 
 * ISO/IEC 14443-3 Type A標準:
 * - Single Size UID: 4 bytes (最常見，如MIFARE Classic 1K)
 * - Double Size UID: 7 bytes
 * - Triple Size UID: 10 bytes
 * 
 * 防碰撞演算法 (Section 9.3.5):
 * 當多張卡片同時進入感應範圍時，會發生位元碰撞
 * MFRC522使用位元級防碰撞演算法逐一識別每張卡片
 * 
 * 防碰撞流程:
 * 1. 發送SEL_CL1命令 (0x93) + NVB (Number of Valid Bits)
 * 2. 卡片回應部分UID
 * 3. 若發生碰撞，記錄碰撞位置
 * 4. 重複步驟1-3，逐位元確定UID
 * 5. 完整UID接收後，發送SELECT命令選中卡片
 * 
 * CollReg暫存器 (Section 9.3.1.9):
 * - Bit 7: ValuesAfterColl - 碰撞後的位元值
 * - Bit 6-0: CollPos - 碰撞位置
 * 
 * MFRC522庫簡化:
 * PICC_ReadCardSerial()已內建完整防碰撞演算法
 * UID儲存在mfrc522.uid結構中
 * 
 * @param uid[out] 存放讀取到的UID
 * @param uidSize[out] UID的實際長度
 * @return true: 讀取成功 | false: 讀取失敗
 * 
 * TODO:
 * 1. 確認卡片已被選中 (PICC_ReadCardSerial已執行)
 * 2. 從mfrc522.uid複製UID到緩衝區
 * 3. 複製UID長度
 * 4. 加入資料驗證 (CRC check)
 */
bool RFIDHandler::readCardUID(uint8_t* uid, uint8_t* uidSize) {
    // TODO: 檢查是否已偵測到卡片
    // if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
    //     return false;
    // }
    
    // TODO: 複製UID資料
    // *uidSize = mfrc522.uid.size;
    // memcpy(uid, mfrc522.uid.uidByte, *uidSize);
    // 
    // // 儲存到lastUID
    // memcpy(lastUID, uid, *uidSize);
    // lastUIDSize = *uidSize;
    
    // TODO: 列印UID (除錯用)
    // Serial.print("[RFID] UID: ");
    // printUID(uid, *uidSize);
    
    return false;  // TODO: 根據實際結果返回
}

/*
 * 驗證卡片UID是否在白名單中
 * 
 * 安全性考量:
 * 
 * 1. UID可被複製 (Clone Attack)
 *    - UID儲存在卡片晶片的唯讀區域
 *    - 但可購買可寫UID的空白卡片
 *    - 使用RFID讀寫器即可複製UID
 *    → 單純依賴UID驗證不夠安全
 * 
 * 2. 建議安全措施:
 *    - 使用Crypto1加密認證 (MIFARE Classic)
 *      Datasheet Section 9.4: Authenticate Command
 *      需要密鑰進行挑戰-回應認證
 * 
 *    - 使用更安全的卡片:
 *      - MIFARE DESFire (3DES/AES加密)
 *      - MIFARE Plus (AES-128)
 *      - NTAG系列 (數位簽章)
 * 
 *    - 搭配其他認證方式:
 *      - RFID + 指紋雙因子認證
 *      - RFID + PIN碼
 * 
 * 白名單管理:
 * - 儲存方式: EEPROM、SPIFFS、SD卡
 * - 動態管理: 新增、刪除、查詢
 * - 加密儲存: 防止記憶體讀取攻擊
 * 
 * @param uid 待驗證的卡片UID
 * @param uidSize UID長度
 * @return true: 卡片已授權 | false: 卡片未授權
 * 
 * TODO:
 * 1. 遍歷白名單比對UID
 * 2. 使用compareUID()函式比對
 * 3. 記錄驗證結果日誌
 * 4. 可選: 實作Crypto1認證
 */
bool RFIDHandler::verifyCard(const uint8_t* uid, uint8_t uidSize) {
    // TODO: 遍歷白名單
    // for (uint8_t i = 0; i < authorizedCardCount; i++) {
    //     if (authorizedUIDSizes[i] == uidSize) {
    //         if (compareUID(uid, authorizedUIDs[i], uidSize)) {
    //             Serial.println("[RFID] 卡片驗證通過");
    //             return true;
    //         }
    //     }
    // }
    
    Serial.println("[RFID] 卡片未授權");
    return false;  // TODO: 實際驗證邏輯
}

/*
 * 停止當前卡片通訊
 * 
 * Datasheet參考:
 * - Section 9.3.7: HLTA Command (Halt Command)
 * - Command Code: 0x50
 * - ISO14443-3: HLTA命令格式
 * 
 * HLTA命令:
 * - 命令碼: 0x50
 * - 參數: 0x00
 * - CRC: 2 bytes
 * 
 * 功能:
 * 將選中的卡片設為Halt State (停止狀態)
 * 卡片進入低功耗模式，不再回應REQA命令
 * 只有WUPA (Wake-Up)命令才能喚醒
 * 
 * 用途:
 * - 防止重複讀取同一張卡片
 * - 降低卡片功耗
 * - 多卡環境下的卡片管理
 * 
 * MFRC522庫函式:
 * - PICC_HaltA(): 發送HLTA命令
 * 
 * TODO:
 * 1. 呼叫PICC_HaltA()停止卡片
 * 2. 可選: 驗證卡片是否真的進入Halt狀態
 */
void RFIDHandler::haltCard() {
    // TODO: 停止卡片通訊
    // mfrc522.PICC_HaltA();
    // Serial.println("[RFID] 卡片已停止");
}

/*
 * 獲取最後一次讀取的UID
 */
const uint8_t* RFIDHandler::getLastUID() const {
    return lastUID;
}

/*
 * 獲取最後一次讀取的UID長度
 */
uint8_t RFIDHandler::getLastUIDSize() const {
    return lastUIDSize;
}

/*
 * 列印UID到Serial (除錯用)
 * 
 * 格式: XX:XX:XX:XX (16進位)
 * 
 * @param uid UID陣列
 * @param uidSize UID長度
 */
void RFIDHandler::printUID(const uint8_t* uid, uint8_t uidSize) {
    for (uint8_t i = 0; i < uidSize; i++) {
        if (uid[i] < 0x10) {
            Serial.print("0");  // 補0對齊
        }
        Serial.print(uid[i], HEX);
        if (i < uidSize - 1) {
            Serial.print(":");
        }
    }
    Serial.println();
}

// ============================================================================
// 私有函式實作
// ============================================================================

/*
 * 新增授權卡片到白名單
 * 
 * 白名單管理:
 * - 記憶體中的白名單在斷電後會遺失
 * - 建議儲存到EEPROM或Flash
 * 
 * ESP32儲存方案:
 * 1. EEPROM Library (模擬EEPROM在Flash)
 * 2. Preferences Library (Key-Value儲存)
 * 3. SPIFFS (檔案系統)
 * 4. SD卡 (大容量儲存)
 * 
 * @param uid 卡片UID
 * @param uidSize UID長度
 * @return true: 新增成功 | false: 白名單已滿
 * 
 * TODO:
 * 1. 檢查白名單是否已滿
 * 2. 檢查UID是否已存在 (避免重複)
 * 3. 複製UID到白名單
 * 4. 可選: 儲存到EEPROM持久化
 */
bool RFIDHandler::addAuthorizedCard(const uint8_t* uid, uint8_t uidSize) {
    // TODO: 檢查白名單容量
    // if (authorizedCardCount >= MAX_AUTHORIZED_CARDS) {
    //     Serial.println("[RFID] 白名單已滿");
    //     return false;
    // }
    
    // TODO: 檢查是否已存在
    // for (uint8_t i = 0; i < authorizedCardCount; i++) {
    //     if (compareUID(uid, authorizedUIDs[i], uidSize)) {
    //         Serial.println("[RFID] 卡片已在白名單中");
    //         return false;
    //     }
    // }
    
    // TODO: 新增到白名單
    // memcpy(authorizedUIDs[authorizedCardCount], uid, uidSize);
    // authorizedUIDSizes[authorizedCardCount] = uidSize;
    // authorizedCardCount++;
    
    // TODO: 儲存到EEPROM (選用)
    // saveWhitelistToEEPROM();
    
    // Serial.println("[RFID] 卡片已新增到白名單");
    return false;  // TODO: 實際新增邏輯
}

/*
 * 比對兩個UID是否相同
 * 
 * 使用memcmp()進行位元組級比對
 * 
 * @param uid1 UID 1
 * @param uid2 UID 2
 * @param size UID長度
 * @return true: 相同 | false: 不同
 */
bool RFIDHandler::compareUID(const uint8_t* uid1, const uint8_t* uid2, uint8_t size) {
    // memcmp: 返回0表示相同
    return memcmp(uid1, uid2, size) == 0;
}

/*
 * ============================================================================
 * MFRC522關鍵暫存器詳解
 * ============================================================================
 * 
 * Datasheet Section 9.2: Register Description
 * 
 * 暫存器位址範圍: 0x00 ~ 0x3F (64個暫存器)
 * SPI讀取: 位址 << 1 | 0x80
 * SPI寫入: 位址 << 1
 * 
 * === 命令與控制暫存器 ===
 * 
 * 1. CommandReg (0x01) - 命令暫存器
 *    - 控制MFRC522執行的命令
 *    - 常用命令:
 *      0x00: Idle - 空閒
 *      0x0C: Transceive - 傳輸與接收
 *      0x0E: MFAuthent - MIFARE認證
 *      0x0F: SoftReset - 軟體重置
 * 
 * 2. ComIEnReg (0x02) - 中斷啟用暫存器
 *    - Bit 7: IRqInv - IRQ腳位反相
 *    - Bit 6: TxIEn - 傳輸完成中斷
 *    - Bit 5: RxIEn - 接收完成中斷
 *    - Bit 4: IdleIEn - 空閒中斷
 *    - Bit 1: ErrIEn - 錯誤中斷
 *    - Bit 0: TimerIEn - 定時器中斷
 * 
 * 3. ComIrqReg (0x04) - 中斷請求暫存器
 *    - 顯示當前中斷狀態
 *    - 寫入1清除對應中斷位元
 *    - Bit 5: RxIRq - 接收完成
 *    - Bit 4: IdleIRq - 命令執行完成
 * 
 * 4. ErrorReg (0x06) - 錯誤標誌暫存器
 *    - Bit 0: ProtocolErr - 協定錯誤
 *    - Bit 1: ParityErr - 校驗錯誤
 *    - Bit 2: CRCErr - CRC校驗錯誤
 *    - Bit 3: CollErr - 碰撞錯誤
 *    - Bit 4: BufferOvfl - FIFO溢位
 *    - Bit 5: TempErr - 溫度過高
 *    - Bit 6: WrErr - 寫入錯誤
 * 
 * === FIFO與資料處理 ===
 * 
 * 5. FIFODataReg (0x09) - FIFO資料暫存器
 *    - 64 bytes FIFO緩衝區
 *    - 讀寫資料通過此暫存器
 *    - 自動更新FIFO指標
 * 
 * 6. FIFOLevelReg (0x0A) - FIFO長度暫存器
 *    - Bit 6-0: FIFOLevel - FIFO中的資料位元組數
 *    - Bit 7: FlushBuffer - 寫入1清空FIFO
 * 
 * 7. ControlReg (0x0C) - 控制暫存器
 *    - Bit 7: TStopNow - 立即停止定時器
 *    - Bit 6: TStartNow - 立即啟動定時器
 * 
 * 8. BitFramingReg (0x0D) - 位元框架調整
 *    - Bit 6-4: RxAlign - 接收位元對齊
 *    - Bit 2-0: TxLastBits - 最後一個位元組的有效位元數
 *    - 用於防碰撞演算法的位元級傳輸
 * 
 * === 模式與配置 ===
 * 
 * 9. ModeReg (0x11) - 模式暫存器
 *    - Bit 7: MSBFirst - MSB優先傳輸
 *    - Bit 6-5: TxWaitRF - 傳輸等待RF穩定
 *    - Bit 3: PolMFin - MFIN腳位極性
 *    - Bit 1-0: CRCPreset - CRC初始值
 * 
 * 10. TxControlReg (0x14) - 天線驅動器控制
 *     - Bit 1: Tx2RFEn - 啟用TX2天線輸出
 *     - Bit 0: Tx1RFEn - 啟用TX1天線輸出
 *     - 兩者都需要啟用才能正常通訊
 * 
 * 11. TxASKReg (0x15) - ASK調變控制
 *     - ASK: Amplitude Shift Keying (振幅鍵移)
 *     - Bit 6: Force100ASK - 強制100% ASK調變
 * 
 * === RF配置 ===
 * 
 * 12. RFCfgReg (0x26) - RF配置暫存器
 *     - Bit 6-4: RxGain - 接收增益
 *       000: 18 dB
 *       001: 23 dB
 *       010: 18 dB
 *       011: 23 dB
 *       100: 33 dB
 *       101: 38 dB
 *       110: 43 dB
 *       111: 48 dB (最大)
 * 
 * === 定時器配置 ===
 * 
 * 13. TModeReg (0x2A) - 定時器模式
 *     - Bit 7: TAuto - 自動啟動定時器
 *     - Bit 3-0: TPrescaler_Hi - 預分頻器高4位元
 * 
 * 14. TPrescalerReg (0x2B) - 定時器預分頻
 *     - Bit 7-0: TPrescaler_Lo - 預分頻器低8位元
 *     - 定時器頻率 = 13.56MHz / (2*TPreScaler+1)
 * 
 * 15. TReloadRegH/L (0x2C/0x2D) - 定時器重載值
 *     - 16位元定時器重載值
 *     - 定時器減到0時觸發中斷並重載
 * 
 * === 版本與測試 ===
 * 
 * 16. VersionReg (0x37) - 版本暫存器 (只讀)
 *     - MFRC522 v2.0: 0x92
 *     - MFRC522 v1.0: 0x91
 *     - 用於驗證SPI通訊與晶片識別
 * 
 * 17. TestPinEnReg (0x36) - 測試腳位啟用
 *     - 用於測試模式
 *     - 正常使用不需要修改
 * 
 * ============================================================================
 * ISO/IEC 14443-3 Type A 協定
 * ============================================================================
 * 
 * 通訊流程:
 * 
 * 1. REQA/WUPA (Request/Wake-Up)
 *    - PCD發送: 0x26 (REQA) 或 0x52 (WUPA)
 *    - PICC回應: ATQA (Answer To Request, 2 bytes)
 * 
 * 2. Anti-collision (防碰撞)
 *    - PCD發送: SEL_CL1 (0x93) + NVB + partial UID
 *    - PICC回應: remaining UID + BCC
 *    - 重複直到完整UID接收
 * 
 * 3. SELECT (選擇)
 *    - PCD發送: SEL_CL1 + 0x70 + complete UID + CRC
 *    - PICC回應: SAK (Select Acknowledge)
 * 
 * 4. Authentication (認證) - MIFARE Classic
 *    - PCD發送: AUTH_A (0x60) 或 AUTH_B (0x61) + block + key
 *    - Crypto1加密認證流程
 * 
 * 5. Data Transfer (資料傳輸)
 *    - READ: 讀取資料塊
 *    - WRITE: 寫入資料塊
 *    - INCREMENT/DECREMENT: 值塊操作
 * 
 * 6. HLTA (Halt)
 *    - PCD發送: 0x50 + 0x00 + CRC
 *    - PICC進入Halt狀態
 * 
 * ============================================================================
 * 學習重點總結
 * ============================================================================
 * 
 * 1. SPI通訊
 *    - Mode 0: CPOL=0, CPHA=0
 *    - 最大10MHz時鐘頻率
 *    - 暫存器讀寫協定
 * 
 * 2. RFID工作原理
 *    - 13.56MHz載波
 *    - ASK調變 (振幅鍵移)
 *    - 電磁感應供電 (被動式卡片)
 * 
 * 3. 防碰撞演算法
 *    - 位元級碰撞偵測
 *    - 遞迴選擇演算法
 *    - CollReg暫存器碰撞位置
 * 
 * 4. 安全性考量
 *    - UID可被複製
 *    - 建議使用Crypto1認證
 *    - 或升級到更安全的卡片
 * 
 * 5. MFRC522庫簡化
 *    - 封裝底層暫存器操作
 *    - 提供高階API
 *    - 自動處理CRC、防碰撞
 * 
 * ============================================================================
 */
