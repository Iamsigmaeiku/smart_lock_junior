#include "husky_lens.h"
#include "config.h"

void HuskyLens::init() {
  Serial.println("初始化 HUSKYLENS 模組...");
  
  // 初始化成員變數
  isInitialized = false;
  lastRecognizedID = -1;
  
  // 初始化 UART2 (GPIO9=RX, GPIO10=TX, 波特率 9600)
  serial = &Serial2;
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
}

bool HuskyLens::detectFace() {
  if (!isInitialized) return false;
  
  // 請求 HUSKYLENS 讀取資料
  if (!huskylens.request()) {
    return false;
  }
  
  // 檢查是否有偵測到物件（人臉）
  if (huskylens.available()) {
    int count = huskylens.count();
    if (count > 0) {
      Serial.printf("偵測到 %d 個人臉\n", count);
      return true;
    }
  }
  
  return false;
}

int HuskyLens::recognizeFace() {
  if (!isInitialized) return -1;
  
  // 請求 HUSKYLENS 讀取資料
  if (!huskylens.request()) {
    return -1;
  }
  
  // 如果有辨識到人臉
  if (huskylens.available()) {
    // 讀取第一個辨識結果
    HUSKYLENSResult result = huskylens.read();
    
    // 檢查是否為已學習的人臉（ID > 0）
    // ID = 0 代表未學習的人臉
    // ID > 0 代表已學習的人臉
    if (result.ID > 0) {
      lastRecognizedID = result.ID;
      Serial.printf("辨識到人臉 ID: %d\n", result.ID);
      return result.ID;
    } else {
      Serial.println("偵測到未學習的人臉");
    }
  }
  
  return -1; // 未辨識到或陌生人
}

bool HuskyLens::verifyFace() {
  if (!isInitialized) {
    Serial.println("HUSKYLENS 未初始化！");
    return false;
  }
  
  // 辨識人臉並取得 ID
  int faceID = recognizeFace();
  
  // 如果 ID > 0，代表是已註冊的人臉
  if (faceID > 0) {
    Serial.printf("✓ 驗證成功！使用者 ID: %d\n", faceID);
    return true;
  }
  
  Serial.println("✗ 驗證失敗！未辨識到已註冊人臉");
  return false;
}

bool HuskyLens::learnFace(uint8_t faceID) {
  if (!isInitialized) {
    Serial.println("HUSKYLENS 未初始化！");
    return false;
  }
  
  Serial.printf("準備學習人臉 ID: %d\n", faceID);
  Serial.println("請將臉對準鏡頭...");
  
  // 等待偵測到人臉
  unsigned long startTime = millis();
  while (millis() - startTime < 10000) {  // 10秒超時
    if (huskylens.request() && huskylens.available()) {
      // 使用 writeLearn 指令學習當前人臉
      if (huskylens.writeLearn(faceID)) {
        Serial.printf("✓ 學習成功！已註冊人臉 ID: %d\n", faceID);
        delay(100);
        return true;
      }
    }
    delay(100);
  }
  
  Serial.println("✗ 學習失敗！請確保有偵測到人臉");
  return false;
}

void HuskyLens::setAlgorithm(protocolAlgorithm algorithm) {
  if (!isInitialized) {
    Serial.println("HUSKYLENS 未初始化！");
    return;
  }
  
  Serial.printf("切換演算法模式: %d\n", (int)algorithm);
  
  if (huskylens.writeAlgorithm(algorithm)) {
    Serial.println("✓ 切換成功");
    delay(100);  // 等待切換完成
  } else {
    Serial.println("✗ 切換失敗");
  }
}

int HuskyLens::getObjectCount() {
  if (!isInitialized) return 0;
  
  // 請求最新資料
  if (!huskylens.request()) {
    return 0;
  }
  
  // 回傳物件數量
  return huskylens.count();
}
