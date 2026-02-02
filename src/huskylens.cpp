#include "husky_lens.h"
#include "config.h"

#include "huskylens.h"
#include "config.h"
#include <Wire.h>

#include "huskylens.h"
#include "config.h"
#include <Wire.h>

void HuskyLens::init() {
<<<<<<< Updated upstream
<<<<<<< Updated upstream
  // TODO: 初始化 HUSKYLENS AI 辨識鏡頭

  // 1. 初始化 I2C 通訊
  // Wire.begin(HUSKYLENS_SDA, HUSKYLENS_SCL);

  // 2. 初始化 HUSKYLENS 物件（如果使用函式庫）
  // huskylens.begin(Wire);

  // 3. 等待 HUSKYLENS 啟動
  // delay(100);

  // 4. 設定演算法為人臉辨識模式
  // huskylens.writeAlgorithm(ALGORITHM_FACE_RECOGNITION);

  // 5. 檢查連線狀態
  // if (!huskylens.isConnected()) {
  //   Serial.println("HUSKYLENS 連線失敗！");
  //   return;
  // }

  Serial.println("初始化 HUSKYLENS 模組...");

  // 初始化成員變數
  isInitialized = false;
  lastRecognizedID = -1;

  // 初始化 UART2 (GPIO9=RX, GPIO10=TX, 波特率 9600)
  serial = &Serial1;
  serial->begin(9600, SERIAL_8N1, HUSKYLENS_RX, HUSKYLENS_TX);

  delay(100);  // 等待串列埠穩定

  // 初始化 HUSKYLENS 物件
  while (!huskylens.begin(*serial)) {
    Serial.println("HUSKYLENS 連線失敗！請檢查接線");
    delay(1000);
  }

  Serial.println("HUSKYLENS 連線成功！");

  // 設定為人臉辨識模式
  if (!huskylens.writeAlgorithm(ALGORITHM_FACE_RECOGNITION)) {
    Serial.println("切換人臉辨識模式失敗！");
  } else {
    Serial.println("已切換到人臉辨識模式");
  }

  delay(100);
  isInitialized = true;
  Serial.println("HUSKYLENS 初始化完成！");
=======
  // 暫時禁用 HUSKYLENS 功能
  Serial.println("HUSKYLENS 模組已禁用");
  isInitialized = false;
>>>>>>> Stashed changes
=======
  // 暫時禁用 HUSKYLENS 功能
  Serial.println("HUSKYLENS 模組已禁用");
  isInitialized = false;
>>>>>>> Stashed changes
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
<<<<<<< Updated upstream
<<<<<<< Updated upstream
  // TODO: 驗證人臉是否為已註冊使用者

  int faceID = recognizeFace();

  // 如果 ID > 0，代表是已註冊的人臉
  if (faceID > 0) {
    Serial.printf("✓ 驗證成功！使用者 ID: %d\n", faceID);
    return true;
  }

  Serial.println("驗證失敗！未辨識到已註冊人臉");
=======
  // 暫時禁用
>>>>>>> Stashed changes
=======
  // 暫時禁用
>>>>>>> Stashed changes
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

