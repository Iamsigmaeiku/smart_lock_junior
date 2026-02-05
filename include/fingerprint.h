#ifndef FINGERPRINT_H
#define FINGERPRINT_H

#include <Arduino.h>

class Screen;  // 前向聲明
class Adafruit_Fingerprint;
// AS608 指紋傳感器控制類
class Fingerprint {
public:
  // 初始化
  void init();

  // 檢測指紋
  bool detectFinger();

  // 驗證指紋
  bool verifyFinger();
  // 註冊新指紋
  bool enrollFinger(uint8_t id);
  // 獲取下一個可用 ID
  uint8_t getNextAvailableID();
  // 設置 Screen 對象用於 UI 更新
  void setDisplay(Screen* disp);

private:
  HardwareSerial*fpSerial;
  uint32_t baudRate;
  Adafruit_Fingerprint* finger;
  Screen* display;
  int rx;
  int tx;
//statue
bool initialized;
uint16_t lastmatchID;
uint16_t lastConfidence;
uint8_t nextID;
bool ensureInit();
};

#endif
