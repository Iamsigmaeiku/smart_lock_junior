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