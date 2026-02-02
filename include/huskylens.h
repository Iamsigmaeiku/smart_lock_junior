#ifndef HUSKYLENS_H
#define HUSKYLENS_H

#include <Arduino.h>

// HUSKYLENS AI 辨識鏡頭控制類
class HuskyLens {
public:
  // 初始化
  void init();
  
  // 檢測是否有人臉
  bool detectFace();
  
  // 學習新人臉（註冊使用者）
  bool learnFace(uint8_t faceID);
  
  // 辨識人臉並返回 ID
  int recognizeFace();
  
  // 驗證人臉是否已註冊
  bool verifyFace();
  
  // 切換演算法模式（人臉辨識、物體追蹤、顏色辨識等）
  void setAlgorithm(uint8_t algorithm);
  
  // 獲取辨識到的物件數量
  int getObjectCount();

private:
  // 暫時禁用 HUSKYLENS 功能
  // I2C 地址（預設 0x32）
  uint8_t i2cAddress = 0x32;
  
  // 當前辨識到的人臉 ID
  int lastRecognizedID = -1;
  
  // 初始化狀態
  bool isInitialized = false;
};

#endif
