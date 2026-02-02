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
  // TODO: 添加必要的成員變數
  // 例如:
  // - HUSKYLENS 物件（如果使用函式庫）
  // - I2C 地址（預設 0x32）
  // - 當前辨識到的人臉 ID
  // - 辨識結果緩衝區

  // 範例骨架：
  // uint8_t i2cAddress = 0x32;
  // int lastRecognizedID = -1;
  // bool isInitialized = false;
};

// HUSKYLENS 演算法常數（參考 HUSKYLENS 函式庫定義）
#define ALGORITHM_FACE_RECOGNITION  0
#define ALGORITHM_OBJECT_TRACKING   1
#define ALGORITHM_OBJECT_RECOGNITION 2
#define ALGORITHM_LINE_TRACKING     3
#define ALGORITHM_COLOR_RECOGNITION 4
#define ALGORITHM_TAG_RECOGNITION   5

#endif
