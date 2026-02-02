#ifndef HUSKYLENS_WRAPPER_H
#define HUSKYLENS_WRAPPER_H

#include <Arduino.h>
#include <Wire.h>
#include <HUSKYLENS.h>

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
  void setAlgorithm(protocolAlgorithm algorithm);
  
  // 獲取辨識到的物件數量
  int getObjectCount();

private:
  HUSKYLENS huskylens;                // 官方函式庫物件
  HardwareSerial *serial;             // UART 串列物件指標
  bool isInitialized;                 // 初始化狀態
  int lastRecognizedID;               // 最後辨識的人臉 ID
};

// HUSKYLENS 演算法常數已由官方庫的 protocolAlgorithm enum 提供：
// ALGORITHM_FACE_RECOGNITION, ALGORITHM_OBJECT_TRACKING,
// ALGORITHM_OBJECT_RECOGNITION, ALGORITHM_LINE_TRACKING,
// ALGORITHM_COLOR_RECOGNITION, ALGORITHM_TAG_RECOGNITION,
// ALGORITHM_OBJECT_CLASSIFICATION

#endif  // HUSKYLENS_WRAPPER_H
