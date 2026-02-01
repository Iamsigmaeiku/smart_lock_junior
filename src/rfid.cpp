#include "rfid.h"
#include "config.h"
#include <SPI.h>
#include <EEPROM.h>

// 建構函數：初始化 MFRC522 物件
RFID::RFID() : mfrc522(RFID_CS, RFID_RST) {
  currentUIDLength = 0;
  memset(currentUID, 0, sizeof(currentUID));
}

void RFID::init() {
  Serial.println("初始化 RFID 模組...");

  // 1. 初始化 SPI (與螢幕共用，可能已經初始化過了)
  SPI.begin();

  // 2. 初始化 MFRC522
  mfrc522.PCD_Init();

  // 3. 初始化 EEPROM
  EEPROM.begin(EEPROM_SIZE);

  // 4. 驗證卡片數量合理性
  uint8_t count = getCardCount();
  if (count > MAX_CARDS || count == 0xFF) {
    Serial.println("EEPROM 資料異常，重置為 0");
    saveCardCount(0);
    EEPROM.commit();
  } else {
    Serial.print("已註冊卡片數量: ");
    Serial.println(count);
  }

  // 5. 顯示 MFRC522 版本資訊
  mfrc522.PCD_DumpVersionToSerial();

  Serial.println("RFID 模組初始化完成！");
}

bool RFID::detectCard() {
  // 檢測是否有新卡片靠近
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return false;
  }

  // 讀取卡片序號
  if (!mfrc522.PICC_ReadCardSerial()) {
    return false;
  }

  // 儲存當前卡片的 UID
  currentUIDLength = mfrc522.uid.size;
  for (uint8_t i = 0; i < currentUIDLength && i < UID_SIZE; i++) {
    currentUID[i] = mfrc522.uid.uidByte[i];
  }

  return true;
}

bool RFID::readCardUID(uint8_t* uid, uint8_t* uidLength) {
  // 從當前讀取的卡片複製 UID
  if (currentUIDLength == 0) {
    return false;
  }

  *uidLength = currentUIDLength;
  for (uint8_t i = 0; i < currentUIDLength && i < UID_SIZE; i++) {
    uid[i] = currentUID[i];
  }

  // 輸出 UID 到 Serial (hex format)
  Serial.print("讀取卡片 UID: ");
  for (uint8_t i = 0; i < currentUIDLength; i++) {
    if (currentUID[i] < 0x10) Serial.print("0");
    Serial.print(currentUID[i], HEX);
    if (i < currentUIDLength - 1) Serial.print(" ");
  }
  Serial.println();

  // 停止與卡片通訊
  mfrc522.PICC_HaltA();

  return true;
}

bool RFID::verifyCard() {
  // 如果還沒有讀取卡片，先檢測並讀取
  if (currentUIDLength == 0) {
    if (!detectCard()) {
      return false;
    }
  }

  // 讀取卡片 UID
  uint8_t uid[4];
  uint8_t uidLength;
  if (!readCardUID(uid, &uidLength)) {
    Serial.println("讀取卡片失敗");
    return false;
  }

  // 檢查是否已註冊
  if (isCardRegistered(uid)) {
    Serial.println("✓ 卡片驗證成功！");
    return true;
  } else {
    Serial.println("✗ 卡片未註冊");
    return false;
  }
}

bool RFID::enrollCard() {
  // 檢查已註冊卡片數量
  uint8_t count = getCardCount();
  if (count >= MAX_CARDS) {
    Serial.println("✗ 卡片容量已滿，無法註冊新卡片");
    return false;
  }

  // 如果還沒有讀取卡片，先檢測並讀取
  if (currentUIDLength == 0) {
    if (!detectCard()) {
      return false;
    }
  }

  // 讀取卡片 UID
  uint8_t uid[4];
  uint8_t uidLength;
  if (!readCardUID(uid, &uidLength)) {
    Serial.println("讀取卡片失敗");
    return false;
  }

  // 檢查是否已註冊
  if (isCardRegistered(uid)) {
    Serial.println("✗ 卡片已註冊，無需重複註冊");
    return false;
  }

  // 將 UID 寫入 EEPROM
  saveCardUID(count, uid);
  count++;
  saveCardCount(count);
  EEPROM.commit();

  Serial.print("✓ 卡片註冊成功！目前已註冊 ");
  Serial.print(count);
  Serial.println(" 張卡片");

  return true;
}

// ========================================
// 內部輔助函數實作
// ========================================

uint8_t RFID::getCardCount() {
  return EEPROM.read(EEPROM_ADDR_COUNT);
}

void RFID::saveCardCount(uint8_t count) {
  EEPROM.write(EEPROM_ADDR_COUNT, count);
}

bool RFID::getCardUID(uint8_t index, uint8_t* uid) {
  if (index >= MAX_CARDS) {
    return false;
  }

  uint16_t addr = EEPROM_ADDR_CARDS + (index * UID_SIZE);
  for (uint8_t i = 0; i < UID_SIZE; i++) {
    uid[i] = EEPROM.read(addr + i);
  }

  return true;
}

void RFID::saveCardUID(uint8_t index, const uint8_t* uid) {
  if (index >= MAX_CARDS) {
    return;
  }

  uint16_t addr = EEPROM_ADDR_CARDS + (index * UID_SIZE);
  for (uint8_t i = 0; i < UID_SIZE; i++) {
    EEPROM.write(addr + i, uid[i]);
  }
}

bool RFID::isCardRegistered(const uint8_t* uid) {
  uint8_t count = getCardCount();
  uint8_t storedUID[4];

  for (uint8_t i = 0; i < count; i++) {
    getCardUID(i, storedUID);
    if (memcmp(uid, storedUID, UID_SIZE) == 0) {
      return true;
    }
  }

  return false;
}
