#include <Arduino.h>
#include "config.h"
#include "fingerprint.h"
#include "screen.h"
#include "motor.h"
#include "rfid.h"
#include "husky_lens.h"
#include "wifi_comm.h"
#include "password.h"
#define DEBUG_TOUCH_DOT  0   // 改 0 就關掉畫點除錯

uint32_t unlockStartMs = 0;
bool isUnlocking = false;

Fingerprint fingerSensor;
Screen display;
Motor doorMotor;
RFID rfidReader;
HuskyLens aiCamera;
wifi_comm wifiModule;
Password pwManager;

enum SystemState {
  IDLE,
  MENU,
  SETTING_MENU,        // 新增：Setting 主頁面
  SETTING_ADD,         // 新增：Setting 新增子頁面
  SETTING_REMOVE,      // 新增：Setting 刪除子頁面
  WAITING_INPUT,
  PASSWORD_INPUT,
  VERIFYING,
  UNLOCKING,
  LOCKED,
  ENROLLING
};

SystemState currentState = IDLE;
uint8_t currentSettingType = 0; // 用於記錄當前選擇的設定類型 (0=Finger, 1=Face, 2=RFID, 3=Password)
uint8_t passwordChangeStep = 0; // 0=未開始, 1=輸入舊密碼, 2=輸入新密碼
String oldPasswordInput = "";
String newPasswordInput = "";

// 刪除功能相關變量
uint8_t removeItemIDs[10]; // 存儲當前要刪除的項目 ID 列表（最多10個）
uint8_t removeItemCount = 0; // 當前列表中的項目數量

enum AuthMethod {
  NONE,
  FINGERPRINT,
  RFID_CARD,
  FACE_RECOGNITION,
  PASSWORD
};

AuthMethod lastAuthMethod = NONE;

void setup() {
  Serial.begin(SERIAL_BAUD);
  Serial.println("================================");
  Serial.println("智慧門鎖系統啟動中...");
  Serial.println("================================");

  Serial.println("[1/8] 初始化指紋模組...");
  fingerSensor.init();
  
  Serial.println("[2/8] 初始化螢幕...");
  display.init();
  display.initTouch();
  
  // 連接指紋模組和螢幕以顯示 UI
  fingerSensor.setDisplay(&display);
  
  Serial.println("[3/8] 初始化馬達...");
  doorMotor.init(MOTOR_PIN);
  
  Serial.println("[4/8] 初始化RFID讀卡器...");
  rfidReader.init();
  
  Serial.println("[5/8] 初始化HuskyLens...");
  aiCamera.init();
  
  Serial.println("[6/8] 初始化WiFi模組...");
  wifiModule.init();
  
  Serial.println("[7/8] 初始化密碼管理...");
  pwManager.init();

  Serial.println("[8/8] 顯示歡迎畫面...");
  display.showWelcome();
  delay(2000);

  currentState = MENU;
  display.showMainMenu();

  Serial.println("================================");
  Serial.println("✓ 系統準備就緒！");
  Serial.println("================================");
}

void loop() {
  wifiModule.update();

  switch (currentState) {

  case MENU: {
  if (display.isTouched()) {
    int16_t x, y;
    display.getTouchPoint(x, y);

#if DEBUG_TOUCH_DOT
    display.display().fillCircle(x, y, 3, 0xFFFF);
    Serial.printf("[MENU TOUCH] x=%d y=%d\n", x, y);
    delay(80);
#endif

    // 檢查 header 觸控
    Screen::HeaderTouch headerPress = display.getHeaderTouch(x, y);
    if (headerPress == Screen::HEADER_SETTING) {
      Serial.println("選擇：Setting");
      currentState = SETTING_MENU;
      display.showSettingMenu();
      delay(300);
      break;
    }

    // 檢查主選單按鈕（2x2 佈局，使用 getMenuButtonRect）
    int pressed = -1;
    for (uint8_t i = 0; i < 4; i++) {
      Screen::Rect r = display.getMenuButtonRect(i);
      if (display.isButtonPressed(x, y, r.x, r.y, r.w, r.h)) {
        pressed = i;
        break;
      }
    }

    if (pressed != -1) {
      switch (pressed) {
        case 0: // Finger
          Serial.println("選擇：指紋驗證");
          lastAuthMethod = FINGERPRINT;
          currentState = WAITING_INPUT;
          display.showWaitingForFinger();
          break;

        case 1: // RFID
          Serial.println("選擇：RFID驗證");
          lastAuthMethod = RFID_CARD;
          currentState = WAITING_INPUT;
          display.showWaitingForCard();
          break;

        case 2: // Password
          Serial.println("選擇：密碼驗證");
          lastAuthMethod = PASSWORD;
          currentState = PASSWORD_INPUT;
          display.showPasswordInput();
          break;

        case 3: // Face
          Serial.println("選擇：人臉驗證");
          lastAuthMethod = FACE_RECOGNITION;
          currentState = WAITING_INPUT;
          display.showWaitingForFinger();
          break;
      }

      delay(300); // 防連點
    }
  }
  break;
}


    case WAITING_INPUT: {
      // 先檢查back按鈕
      if (display.isTouched()) {
        int16_t x, y;
        display.getTouchPoint(x, y);
        
        Screen::HeaderTouch headerPress = display.getHeaderTouch(x, y);
        if (headerPress == Screen::HEADER_BACK) {
          Serial.println("返回主選單");
          lastAuthMethod = NONE;
          currentState = MENU;
          display.showMainMenu();
          delay(300);
          break;
        }
      }
      
      // 然後檢測感應器輸入
      if (fingerSensor.detectFinger()) {
        lastAuthMethod = FINGERPRINT;
        currentState = VERIFYING;
      }
      if (rfidReader.detectCard()) {
        lastAuthMethod = RFID_CARD;
        currentState = VERIFYING;
      }
      if (aiCamera.detectFace()) {
        lastAuthMethod = FACE_RECOGNITION;
        currentState = VERIFYING;
      }
      break;
    }

    case PASSWORD_INPUT: {
      static String enteredPW = "";
      static unsigned long lastTouchTime = 0;

      if (pwManager.isLocked()) {
        display.showFailed();
        delay(2000);
        enteredPW = "";
        currentState = MENU;
        display.showMainMenu();
        break;
      }

      if (display.isTouched()) {
        if (millis() - lastTouchTime < 200) break;
        lastTouchTime = millis();

        int16_t x, y;
        display.getTouchPoint(x, y);
        
        // 先檢查header back按鈕
        Screen::HeaderTouch headerPress = display.getHeaderTouch(x, y);
        if (headerPress == Screen::HEADER_BACK) {
          Serial.println("返回主選單");
          enteredPW = "";
          currentState = MENU;
          display.showMainMenu();
          delay(300);
          break;
        }
        
        int8_t key = display.getKeypadPress(x, y);

        if (key >= 0 && key <= 9) {
          if (enteredPW.length() < 8) {
            enteredPW += String(key);
            display.updatePasswordDisplay(enteredPW);
          }
        } else if (key == 10) {
          enteredPW = "";
          display.updatePasswordDisplay(enteredPW);
        } else if (key == 11) {
          if (enteredPW.length() < 4) {
            display.showFailed();
            delay(1500);
            enteredPW = "";
            display.showPasswordInput();
          } else {
            bool verified = pwManager.verifyPassword(enteredPW);
            if (verified) {
              display.showSuccess();
              doorMotor.unlock();
              // ✅ Discord 推播（密碼成功）
              if (wifiModule.isConnected()) {
                String msg =
                "🔓 **Smart Lock Unlocked**\n"
                "Method: Password";
              wifiModule.pushDiscord(msg);
            }
              unlockStartMs = millis();
              isUnlocking = true;
              currentState = UNLOCKING;
              enteredPW = "";
            } else {
              display.showFailed();
              delay(2000);
              enteredPW = "";
              if (pwManager.isLocked()) {
                currentState = MENU;
                display.showMainMenu();
              } else {
                display.showPasswordInput();
              }
            }
          }
        }
      }
      break;
    }

    case VERIFYING: {
      bool verified = false;
      if (lastAuthMethod == FINGERPRINT) verified = fingerSensor.verifyFinger();
      else if (lastAuthMethod == RFID_CARD) verified = rfidReader.verifyCard();
      else if (lastAuthMethod == FACE_RECOGNITION) verified = aiCamera.verifyFace();

      if (verified) {
        display.showSuccess();
        doorMotor.unlock();
        // ✅ Discord 推播（指紋/RFID/人臉成功）
      if (wifiModule.isConnected()) {
        String method =
        (lastAuthMethod == FINGERPRINT) ? "Fingerprint" :
        (lastAuthMethod == RFID_CARD) ? "RFID" :
        (lastAuthMethod == FACE_RECOGNITION) ? "Face" :
        (lastAuthMethod == PASSWORD) ? "Password" : "Unknown";

      String msg =
        "🔓 **Smart Lock Unlocked**\n"
        "Method: " + method;

      wifiModule.pushDiscord(msg);
  }
        unlockStartMs = millis();
        isUnlocking = true;
        currentState = UNLOCKING;
      } else {
        display.showFailed();
        delay(2000);
        lastAuthMethod = NONE;
        currentState = MENU;
        display.showMainMenu();
      }
      break;
    }

    case UNLOCKING:
      if (isUnlocking && (millis() - unlockStartMs >= UNLOCK_DURATION)) {
        doorMotor.lock();
        isUnlocking = false;
        lastAuthMethod = NONE;
        currentState = MENU;
        display.showMainMenu();
      }
      break;

    case SETTING_MENU: {
      if (display.isTouched()) {
        int16_t x, y;
        display.getTouchPoint(x, y);
        
        // 檢查 header back 按鈕
        Screen::HeaderTouch headerPress = display.getHeaderTouch(x, y);
        if (headerPress == Screen::HEADER_BACK) {
          Serial.println("返回主選單");
          currentState = MENU;
          display.showMainMenu();
          delay(300);
          break;
        }
        
        // 檢查 Setting 頁面按鈕
        int8_t btnPress = display.getSettingMenuPress(x, y);
        if (btnPress >= 0 && btnPress <= 3) {
          // Add 按鈕 (0=Finger, 1=Face, 2=RFID, 3=Password)
          currentSettingType = btnPress;
          currentState = SETTING_ADD;
          
          if (btnPress == 0) display.showAddFingerprint();
          else if (btnPress == 1) display.showAddFace();
          else if (btnPress == 2) display.showAddRFID();
          else if (btnPress == 3) {
            display.showAddPassword();
            passwordChangeStep = 1; // 開始第一階段：輸入舊密碼
            oldPasswordInput = "";
            newPasswordInput = "";
          }
          
          Serial.printf("新增類型: %d\n", btnPress);
          delay(300);
        } else if (btnPress >= 4 && btnPress <= 7) {
          // Remove 按鈕
          currentSettingType = btnPress - 4;
          currentState = SETTING_REMOVE;
          
          // 載入對應的列表
          removeItemCount = 0;
          if (currentSettingType == 0) {
            // Fingerprint: 掃描 ID 1-10
            for (uint8_t id = 1; id <= 10 && removeItemCount < 10; id++) {
              if (fingerSensor.isFingerStored(id)) {
                removeItemIDs[removeItemCount++] = id;
              }
            }
          } else if (currentSettingType == 1) {
            // Face: 假設 ID 1-10（簡化處理）
            for (uint8_t id = 1; id <= 10; id++) {
              removeItemIDs[removeItemCount++] = id;
            }
          } else if (currentSettingType == 2) {
            // RFID: 使用卡片數量
            removeItemCount = rfidReader.getCardCount();
            for (uint8_t i = 0; i < removeItemCount && i < 10; i++) {
              removeItemIDs[i] = i; // 索引即 ID
            }
          } else if (currentSettingType == 3) {
            // Password: 只有一個重置選項
            removeItemCount = 1;
            removeItemIDs[0] = 0;
          }
          
          display.showRemoveMenu(currentSettingType, removeItemIDs, removeItemCount);
          Serial.printf("刪除類型: %d, 項目數: %d\n", currentSettingType, removeItemCount);
          delay(300);
        }
      }
      break;
    }
    
    case SETTING_ADD: {
      static unsigned long lastTouchTime = 0;
      
      // 檢查 header back 按鈕
      if (display.isTouched()) {
        int16_t x, y;
        display.getTouchPoint(x, y);
        
        Screen::HeaderTouch headerPress = display.getHeaderTouch(x, y);
        if (headerPress == Screen::HEADER_BACK) {
          Serial.println("返回 Setting 選單");
          // 清理密碼相關狀態
          passwordChangeStep = 0;
          oldPasswordInput = "";
          newPasswordInput = "";
          currentState = SETTING_MENU;
          display.showSettingMenu();
          delay(300);
          break;
        }
        
        // 處理密碼修改的鍵盤輸入
        if (currentSettingType == 3) {
          if (millis() - lastTouchTime < 200) break;
          lastTouchTime = millis();
          
          int8_t key = display.getKeypadPress(x, y);
          
          // 數字鍵 0-9
          if (key >= 0 && key <= 9) {
            if (passwordChangeStep == 1) {
              // 第一階段：輸入舊密碼
              if (oldPasswordInput.length() < 8) {
                oldPasswordInput += String(key);
                display.updatePasswordDisplay(oldPasswordInput);
                Serial.printf("輸入舊密碼: %s\n", oldPasswordInput.c_str());
              }
            } else if (passwordChangeStep == 2) {
              // 第二階段：輸入新密碼
              if (newPasswordInput.length() < 8) {
                newPasswordInput += String(key);
                display.updatePasswordDisplay(newPasswordInput);
                Serial.printf("輸入新密碼: %s\n", newPasswordInput.c_str());
              }
            }
          }
          // C 鍵（清除）
          else if (key == 10) {
            if (passwordChangeStep == 1) {
              oldPasswordInput = "";
              display.updatePasswordDisplay(oldPasswordInput);
              Serial.println("清除舊密碼");
            } else if (passwordChangeStep == 2) {
              newPasswordInput = "";
              display.updatePasswordDisplay(newPasswordInput);
              Serial.println("清除新密碼");
            }
          }
          // OK 鍵（確認）
          else if (key == 11) {
            if (passwordChangeStep == 1) {
              // 第一階段：驗證舊密碼
              if (oldPasswordInput.length() < 4) {
                display.showFailed();
                delay(1500);
                oldPasswordInput = "";
                display.showAddPassword();
              } else {
                bool verified = pwManager.verifyPassword(oldPasswordInput);
                if (verified) {
                  Serial.println("舊密碼驗證成功，進入新密碼輸入");
                  passwordChangeStep = 2;
                  display.showAddPassword(); // 重新顯示鍵盤
                  display.updatePasswordDisplay(""); // 清空顯示
                  delay(300);
                } else {
                  Serial.println("舊密碼驗證失敗");
                  display.showFailed();
                  delay(2000);
                  oldPasswordInput = "";
                  passwordChangeStep = 0;
                  currentState = SETTING_MENU;
                  display.showSettingMenu();
                }
              }
            } else if (passwordChangeStep == 2) {
              // 第二階段：設定新密碼
              if (newPasswordInput.length() < 4) {
                display.showFailed();
                delay(1500);
                newPasswordInput = "";
                display.showAddPassword();
              } else {
                bool success = pwManager.changePassword(oldPasswordInput, newPasswordInput);
                if (success) {
                  Serial.println("密碼修改成功");
                  display.showSuccess();
                  delay(2000);
                } else {
                  Serial.println("密碼修改失敗");
                  display.showFailed();
                  delay(2000);
                }
                // 清理並返回 Setting 選單
                passwordChangeStep = 0;
                oldPasswordInput = "";
                newPasswordInput = "";
                currentState = SETTING_MENU;
                display.showSettingMenu();
              }
            }
          }
        }
      }
      
      // 根據類型執行新增操作（非密碼）
      if (currentSettingType == 0) {
        // 指紋新增
        static bool enrollStarted = false;
        
        if (!enrollStarted) {
          enrollStarted = true;
          uint8_t targetID = fingerSensor.getNextAvailableID();
          Serial.printf("開始註冊指紋，ID: %d\n", targetID);
          
          // 調用阻塞式註冊函數（會自動更新 UI）
          bool success = fingerSensor.enrollFinger(targetID);
          
          if (success) {
            Serial.println("指紋註冊成功！");
            display.showSuccess();
          } else {
            Serial.println("指紋註冊失敗！");
            display.showFailed();
          }
          
          delay(2000);
          enrollStarted = false;
          currentState = SETTING_MENU;
          display.showSettingMenu();
        }
      } else if (currentSettingType == 2) {
        // RFID 新增
        if (rfidReader.detectCard()) {
          if (rfidReader.enrollCard()) display.showSuccess();
          else display.showFailed();
          delay(2000);
          currentState = SETTING_MENU;
          display.showSettingMenu();
        }
      } else if (currentSettingType == 1) {
        // 人臉新增
        if (aiCamera.detectFace()) {
          // 這裡應該調用註冊人臉的函數
          display.showSuccess();
          delay(2000);
          currentState = SETTING_MENU;
          display.showSettingMenu();
        }
      }
      break;
    }
    
    case SETTING_REMOVE: {
      if (display.isTouched()) {
        int16_t x, y;
        display.getTouchPoint(x, y);
        
        // 檢查 header back 按鈕
        Screen::HeaderTouch headerPress = display.getHeaderTouch(x, y);
        if (headerPress == Screen::HEADER_BACK) {
          Serial.println("返回 Setting 選單");
          currentState = SETTING_MENU;
          display.showSettingMenu();
          delay(300);
          break;
        }
        
        // 檢查列表項目點擊
        int8_t selectedIndex = display.getRemoveMenuPress(x, y, removeItemCount);
        if (selectedIndex >= 0) {
          Serial.printf("選擇刪除項目: %d\n", selectedIndex);
          bool deleteSuccess = false;
          
          // 根據類型執行刪除操作
          if (currentSettingType == 0) {
            // Fingerprint
            uint8_t fingerId = removeItemIDs[selectedIndex];
            deleteSuccess = fingerSensor.deleteFinger(fingerId);
            Serial.printf("刪除指紋 ID %d: %s\n", fingerId, deleteSuccess ? "成功" : "失敗");
          } else if (currentSettingType == 1) {
            // Face
            uint8_t faceId = removeItemIDs[selectedIndex];
            deleteSuccess = aiCamera.forgetFace(faceId);
            Serial.printf("刪除人臉 ID %d: %s\n", faceId, deleteSuccess ? "成功" : "失敗");
          } else if (currentSettingType == 2) {
            // RFID
            deleteSuccess = rfidReader.deleteCard(selectedIndex);
            Serial.printf("刪除卡片索引 %d: %s\n", selectedIndex, deleteSuccess ? "成功" : "失敗");
          } else if (currentSettingType == 3) {
            // Password reset
            pwManager.resetToDefault();
            deleteSuccess = true;
            Serial.println("密碼已重置為預設值");
          }
          
          // 顯示結果
          if (deleteSuccess) {
            display.showSuccess();
          } else {
            display.showFailed();
          }
          delay(1500);
          
          // 重新載入列表
          removeItemCount = 0;
          if (currentSettingType == 0) {
            // Fingerprint: 重新掃描
            for (uint8_t id = 1; id <= 10 && removeItemCount < 10; id++) {
              if (fingerSensor.isFingerStored(id)) {
                removeItemIDs[removeItemCount++] = id;
              }
            }
          } else if (currentSettingType == 1) {
            // Face: 假設 ID 1-10
            for (uint8_t id = 1; id <= 10; id++) {
              removeItemIDs[removeItemCount++] = id;
            }
          } else if (currentSettingType == 2) {
            // RFID: 重新獲取數量
            removeItemCount = rfidReader.getCardCount();
            for (uint8_t i = 0; i < removeItemCount && i < 10; i++) {
              removeItemIDs[i] = i;
            }
          } else if (currentSettingType == 3) {
            // Password: 保持一個選項
            removeItemCount = 1;
            removeItemIDs[0] = 0;
          }
          
          display.showRemoveMenu(currentSettingType, removeItemIDs, removeItemCount);
          delay(300);
        }
      }
      break;
    }

    case ENROLLING:
      if (rfidReader.detectCard()) {
        if (rfidReader.enrollCard()) display.showSuccess();
        else display.showFailed();
        delay(2000);
        currentState = MENU;
        display.showMainMenu();
      }
      break;    default:
      break;
  }
}
