#include "husky_lens.h"
#include "config.h"

void HuskyLens::init() {
<<<<<<< HEAD
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

=======
>>>>>>> bb5e69dcbe42aa418a3e417576bff5c70debf65c
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
}

bool HuskyLens::detectFace() {
<<<<<<< HEAD
  // TODO: 檢測是否有人臉出現在畫面中

  // 1. 請求 HUSKYLENS 讀取資料
  // huskylens.request();

  // 2. 檢查是否有偵測到物件（人臉）
  // if (huskylens.available()) {
  //   return huskylens.count() > 0;
  // }
=======
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
>>>>>>> bb5e69dcbe42aa418a3e417576bff5c70debf65c

  return false;
}

int HuskyLens::recognizeFace() {
<<<<<<< HEAD
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
=======
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
>>>>>>> bb5e69dcbe42aa418a3e417576bff5c70debf65c

  return -1; // 未辨識到或陌生人
}

bool HuskyLens::verifyFace() {
<<<<<<< HEAD
  // TODO: 驗證人臉是否為已註冊使用者

=======
  if (!isInitialized) {
    Serial.println("HUSKYLENS 未初始化！");
    return false;
  }

  // 辨識人臉並取得 ID
>>>>>>> bb5e69dcbe42aa418a3e417576bff5c70debf65c
  int faceID = recognizeFace();

  // 如果 ID > 0，代表是已註冊的人臉
  if (faceID > 0) {
    Serial.printf("✓ 驗證成功！使用者 ID: %d\n", faceID);
    return true;
  }

<<<<<<< HEAD
  Serial.println("驗證失敗！未辨識到已註冊人臉");
=======
  Serial.println("✗ 驗證失敗！未辨識到已註冊人臉");
>>>>>>> bb5e69dcbe42aa418a3e417576bff5c70debf65c
  return false;
}

bool HuskyLens::learnFace(uint8_t faceID) {
<<<<<<< HEAD
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
=======
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
>>>>>>> bb5e69dcbe42aa418a3e417576bff5c70debf65c
}

