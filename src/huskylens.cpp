#include "huskylens.h"
#include "config.h"
#include <Wire.h>

void HuskyLens::init() {
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
}

bool HuskyLens::detectFace() {
  // TODO: 檢測是否有人臉出現在畫面中

  // 1. 請求 HUSKYLENS 讀取資料
  // huskylens.request();

  // 2. 檢查是否有偵測到物件（人臉）
  // if (huskylens.available()) {
  //   return huskylens.count() > 0;
  // }

  return false;
}

int HuskyLens::recognizeFace() {
  // TODO: 辨識人臉並返回已學習的 ID

  // 1. 請求 HUSKYLENS 讀取資料
  // huskylens.request();

  // 2. 如果有辨識到人臉
  // if (huskylens.available()) {
  //   HUSKYLENSResult result = huskylens.read();
  //
  //   // 檢查是否為已學習的人臉（ID > 0）
  //   if (result.ID > 0) {
  //     Serial.print("辨識到人臉 ID: ");
  //     Serial.println(result.ID);
  //     return result.ID;
  //   }
  // }

  return -1; // 未辨識到或陌生人
}

bool HuskyLens::verifyFace() {
  // TODO: 驗證人臉是否為已註冊使用者

  int faceID = recognizeFace();

  // 如果 ID > 0，代表是已註冊的人臉
  if (faceID > 0) {
    Serial.print("驗證成功！使用者 ID: ");
    Serial.println(faceID);
    return true;
  }

  Serial.println("驗證失敗！未辨識到已註冊人臉");
  return false;
}

bool HuskyLens::learnFace(uint8_t faceID) {
  // TODO: 學習新人臉（註冊新使用者）

  // 學習模式通常需要：
  // 1. 切換到學習模式（長按 HUSKYLENS 的學習按鈕，或透過指令）
  // 2. 等待使用者將臉對準鏡頭
  // 3. HUSKYLENS 會自動拍攝並學習該人臉
  // 4. 學習完成後會分配一個 ID

  // 注意：學習功能可能需要透過 HUSKYLENS 的按鈕操作
  // 部分函式庫可能不支援程式化學習

  Serial.print("請在 HUSKYLENS 前擺好姿勢，準備學習人臉 ID: ");
  Serial.println(faceID);

  return false;
}

void HuskyLens::setAlgorithm(uint8_t algorithm) {
  // TODO: 切換 HUSKYLENS 的演算法模式

  // huskylens.writeAlgorithm(algorithm);

  Serial.print("切換演算法模式: ");
  Serial.println(algorithm);
}

int HuskyLens::getObjectCount() {
  // TODO: 獲取當前辨識到的物件數量

  // huskylens.request();
  // return huskylens.count();

  return 0;
}
