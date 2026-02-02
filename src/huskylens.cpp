#include "huskylens.h"
#include "config.h"
#include <Wire.h>

#include "huskylens.h"
#include "config.h"
#include <Wire.h>

void HuskyLens::init() {
  // 暫時禁用 HUSKYLENS 功能
  Serial.println("HUSKYLENS 模組已禁用");
  isInitialized = false;
}

bool HuskyLens::detectFace() {
  // 暫時禁用
  return false;
}

int HuskyLens::recognizeFace() {
  // 暫時禁用
  return -1;
}

bool HuskyLens::verifyFace() {
  // 暫時禁用
  return false;
}

bool HuskyLens::learnFace(uint8_t faceID) {
  // 暫時禁用
  Serial.print("HUSKYLENS 學習功能已禁用，ID: ");
  Serial.println(faceID);
  return false;
}

void HuskyLens::setAlgorithm(uint8_t algorithm) {
  // 暫時禁用
  Serial.print("HUSKYLENS 演算法切換已禁用: ");
  Serial.println(algorithm);
}

int HuskyLens::getObjectCount() {
  // 暫時禁用
  return 0;
}
