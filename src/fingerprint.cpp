#include "fingerprint.h"
#include "screen.h"
#include <Adafruit_Fingerprint.h>



bool Fingerprint::ensureInit() {
  return initialized && (fpSerial != nullptr) && (finger != nullptr);
}
void Fingerprint::init() {
  fpSerial = &Serial2;
  baudRate = 57600;
  tx = 17;
  rx = 16;
  nextID = 1;
  display = nullptr;
  fpSerial->begin(baudRate, SERIAL_8N1, rx, tx);
  Serial.println("初始化指紋傳感器...");
  delay(100);
  finger = new Adafruit_Fingerprint(fpSerial);
  if (finger->verifyPassword()) {
    Serial.println("AS608 connect success!");
    initialized = true;
  } else {
    Serial.println("AS608 connect failed!");
    initialized = false;
  }
}


bool Fingerprint::detectFinger() {
  if(!ensureInit()) {
    return false;
  }
  return (finger->getImage() == FINGERPRINT_OK);
}

bool Fingerprint::verifyFinger() {
  if(!ensureInit()) {
    return false;
  }
  lastmatchID = 0xFFFF;
  lastConfidence = 0;
  if(finger->getImage() != FINGERPRINT_OK) {
    return false;
  }
  if(finger->image2Tz(1) != FINGERPRINT_OK) {
    return false;
  }
  if(finger->fingerSearch() != FINGERPRINT_OK) {
    return false;
  }
  lastmatchID = finger->fingerID;
  lastConfidence = finger->confidence;
  return true;
}

bool Fingerprint::enrollFinger(uint8_t id) {
  if(!ensureInit()) {
    return false;
  }
  int p = -1;
  //第一次
  Serial.println("please press your finger (1/2).");
  if (display) display->showEnrollStep(1);
  
  while (p != FINGERPRINT_OK) {
    p = finger->getImage();
    if (p == FINGERPRINT_NOFINGER) delay(80);
  }
  if (finger->image2Tz(1) != FINGERPRINT_OK) {
     Serial.println("first press fail.");
    return false;
  }
  
  //移開手指
  Serial.println("remove finger.");
  if (display) display->showEnrollStep(2);
  delay(800);
  while (finger->getImage() != FINGERPRINT_NOFINGER) delay(80);
  
  //第二次
  p = -1;
  Serial.println("please press your finger (2/2).");
  if (display) display->showEnrollStep(3);
  
  while (p != FINGERPRINT_OK) {
    p = finger->getImage();
    if (p == FINGERPRINT_NOFINGER) delay(80);
  }
  if (finger->image2Tz(2) != FINGERPRINT_OK) {
    Serial.println("second press fail.");
    return false;
  }

  if (finger->createModel() != FINGERPRINT_OK) return false;
  return (finger->storeModel(id) == FINGERPRINT_OK);
}

uint8_t Fingerprint::getNextAvailableID() {
  if(!ensureInit()) {
    return 1;
  }
  return nextID++;
}

void Fingerprint::setDisplay(Screen* disp) {
  display = disp;
}

bool Fingerprint::deleteFinger(uint8_t id) {
  if(!ensureInit()) {
    Serial.println("指紋模組未初始化！");
    return false;
  }
  
  Serial.printf("刪除指紋 ID: %d\n", id);
  uint8_t result = finger->deleteModel(id);
  
  if (result == FINGERPRINT_OK) {
    Serial.printf("✓ 成功刪除指紋 ID: %d\n", id);
    return true;
  } else {
    Serial.printf("✗ 刪除失敗 (錯誤碼: %d)\n", result);
    return false;
  }
}

uint8_t Fingerprint::getStoredCount() {
  if(!ensureInit()) {
    return 0;
  }
  
  // AS608 使用 templateCount 來獲取已存儲的指紋數量
  uint8_t count = 0;
  
  // 掃描所有可能的 ID (1-127)
  for (uint8_t id = 1; id <= 127; id++) {
    if (finger->loadModel(id) == FINGERPRINT_OK) {
      count++;
    }
  }
  
  Serial.printf("已存儲指紋數量: %d\n", count);
  return count;
}

bool Fingerprint::isFingerStored(uint8_t id) {
  if(!ensureInit()) {
    return false;
  }
  
  // 嘗試加載指定 ID 的模型來檢查是否存在
  return (finger->loadModel(id) == FINGERPRINT_OK);
}