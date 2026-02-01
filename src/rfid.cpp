#include "rfid.h"
#include "config.h"
#include <SPI.h>

void RFID::init() {
  // TODO: 初始化 RC522 RFID 模組
  // 1. 初始化 SPI (與螢幕共用，可能已經初始化過了)
  // SPI.begin();
  
  // 2. 設定 CS 和 RST 腳位
  // pinMode(RFID_CS, OUTPUT);
  // pinMode(RFID_RST, OUTPUT);
  // digitalWrite(RFID_CS, HIGH);  // 預設不選中
  
  // 3. 重置 RC522
  // digitalWrite(RFID_RST, HIGH);
  // delay(50);
  // digitalWrite(RFID_RST, LOW);
  // delay(50);
  // digitalWrite(RFID_RST, HIGH);
  // delay(50);
  
  // 4. 初始化 RC522 暫存器（查閱 MFRC522 datasheet）
  
  Serial.println("初始化 RFID 模組...");
}

bool RFID::detectCard() {
  // TODO: 檢測是否有卡片靠近
  return false;
}

bool RFID::readCardUID(uint8_t* uid, uint8_t* uidLength) {
  // TODO: 讀取卡片的 UID
  return false;
}

bool RFID::verifyCard() {
  // TODO: 驗證卡片是否已註冊
  return false;
}

bool RFID::enrollCard() {
  // TODO: 註冊新卡片
  return false;
}
