#ifndef RFID_H
#define RFID_H

#include <Arduino.h>
#include <MFRC522.h>

// RFID RC522 控制類
class RFID {
public:
  // 建構函數
  RFID();

  // 初始化
  void init();

  // 檢測卡片
  bool detectCard();

  // 讀取卡片 UID
  bool readCardUID(uint8_t* uid, uint8_t* uidLength);

  // 驗證卡片
  bool verifyCard();

  // 註冊新卡片
  bool enrollCard();

  // 刪除卡片
  bool deleteCard(uint8_t index);

  // 獲取已註冊卡片數量
  uint8_t getCardCount();

private:
  MFRC522 mfrc522;
  uint8_t currentUID[4];
  uint8_t currentUIDLength;

  static const uint8_t MAX_CARDS = 10;
  static const uint8_t UID_SIZE = 4;
  static const uint16_t EEPROM_SIZE = 64;
  static const uint16_t EEPROM_ADDR_COUNT = 0;
  static const uint16_t EEPROM_ADDR_CARDS = 1;

  // 內部輔助函數
  void saveCardCount(uint8_t count);
  bool getCardUID(uint8_t index, uint8_t* uid);
  void saveCardUID(uint8_t index, const uint8_t* uid);
  bool isCardRegistered(const uint8_t* uid);
};

#endif
