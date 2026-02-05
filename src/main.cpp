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
  WAITING_INPUT,
  PASSWORD_INPUT,
  VERIFYING,
  UNLOCKING,
  LOCKED,
  ENROLLING,
  CHANGE_PASSWORD
};

SystemState currentState = IDLE;

enum ChangePasswordStep {
  ENTER_OLD_PASSWORD,
  ENTER_NEW_PASSWORD,
  CONFIRM_NEW_PASSWORD
};

ChangePasswordStep changePasswordStep = ENTER_OLD_PASSWORD;

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

  fingerSensor.init();
  display.init();
  display.initTouch();
  doorMotor.init(MOTOR_PIN);
  rfidReader.init();
  aiCamera.init();
  wifiModule.init();
  pwManager.init();

  display.showWelcome();
  delay(2000);

  currentState = MENU;
  display.showMainMenu();

  Serial.println("系統準備就緒！");
}

void loop() {
  wifiModule.update();

  switch (currentState) {

  case MENU: {
  if (display.isTouched()) {
    int16_t x, y;
    display.getTouchPoint(x, y);

#if DEBUG_TOUCH_DOT
    // 在你「程式認為的座標」畫點，幫你確認對不對
    display.display().fillCircle(x, y, 3, 0xFFFF);
    Serial.printf("[MENU TOUCH] x=%d y=%d\n", x, y);
    delay(80);
#endif

    // === 用 config.h 的座標，跟 Screen::showMainMenu() 畫的位置 100% 同步 ===
    int pressed = -1;

    // Row 1: Finger / RFID
    if (display.isButtonPressed(x, y, MENU_BTN_LEFT_X,  MENU_BTN_ROW1_Y, MENU_BTN_WIDTH, MENU_BTN_HEIGHT))  pressed = 0;
    else if (display.isButtonPressed(x, y, MENU_BTN_RIGHT_X, MENU_BTN_ROW1_Y, MENU_BTN_WIDTH, MENU_BTN_HEIGHT)) pressed = 1;

    // Row 2: Password / Face
    else if (display.isButtonPressed(x, y, MENU_BTN_LEFT_X,  MENU_BTN_ROW2_Y, MENU_BTN_WIDTH, MENU_BTN_HEIGHT))  pressed = 2;
    else if (display.isButtonPressed(x, y, MENU_BTN_RIGHT_X, MENU_BTN_ROW2_Y, MENU_BTN_WIDTH, MENU_BTN_HEIGHT)) pressed = 3;

    // Row 3: Enroll / Setting
    else if (display.isButtonPressed(x, y, MENU_BTN_LEFT_X,  MENU_BTN_ROW3_Y, MENU_BTN_WIDTH, MENU_BTN_HEIGHT))  pressed = 4;
    else if (display.isButtonPressed(x, y, MENU_BTN_RIGHT_X, MENU_BTN_ROW3_Y, MENU_BTN_WIDTH, MENU_BTN_HEIGHT)) pressed = 5;

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
          display.showWaitingForFinger(); // 你可改 showWaitingForFace()
          break;

        case 4: // Enroll
          Serial.println("選擇：註冊卡片");
          currentState = ENROLLING;
          display.showWaitingForCard();
          break;

        case 5: // Setting
          Serial.println("選擇：修改密碼");
          currentState = CHANGE_PASSWORD;
          changePasswordStep = ENTER_OLD_PASSWORD;
          display.showChangePasswordInput("Enter Old Password");
          break;
      }

      delay(300); // 防連點
    }
  }
  break;
}


    case WAITING_INPUT:
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

    case ENROLLING:
      if (rfidReader.detectCard()) {
        if (rfidReader.enrollCard()) display.showSuccess();
        else display.showFailed();
        delay(2000);
        currentState = MENU;
        display.showMainMenu();
      }
      break;

    case CHANGE_PASSWORD: {
      static String enteredPW = "";
      static String oldPW = "";
      static String newPW = "";
      static unsigned long lastTouchTime = 0;

      if (display.isTouched()) {
        if (millis() - lastTouchTime < 200) break;
        lastTouchTime = millis();

        int16_t x, y;
        display.getTouchPoint(x, y);
        int8_t key = display.getKeypadPress(x, y);

        // 數字鍵 0-9：累加輸入
        if (key >= 0 && key <= 9) {
          if (enteredPW.length() < 8) {
            enteredPW += String(key);
            display.updatePasswordDisplay(enteredPW);
          }
        }
        // * 鍵（key=10）：清除輸入
        else if (key == 10) {
          enteredPW = "";
          display.updatePasswordDisplay(enteredPW);
        }
        // # 鍵（key=11）：確認並進入下一步
        else if (key == 11) {
          // 檢查長度
          if (enteredPW.length() < 4) {
            display.showFailed();
            delay(1500);
            enteredPW = "";
            if (changePasswordStep == ENTER_OLD_PASSWORD) {
              display.showChangePasswordInput("Enter Old Password");
            } else if (changePasswordStep == ENTER_NEW_PASSWORD) {
              display.showChangePasswordInput("Enter New Password");
            } else {
              display.showChangePasswordInput("Confirm New Password");
            }
          } else {
            // 根據當前步驟處理
            if (changePasswordStep == ENTER_OLD_PASSWORD) {
              // 驗證舊密碼
              if (pwManager.verifyPassword(enteredPW)) {
                oldPW = enteredPW;
                enteredPW = "";
                changePasswordStep = ENTER_NEW_PASSWORD;
                display.showChangePasswordInput("Enter New Password");
              } else {
                display.showFailed();
                delay(2000);
                enteredPW = "";
                display.showChangePasswordInput("Enter Old Password");
              }
            } else if (changePasswordStep == ENTER_NEW_PASSWORD) {
              // 儲存新密碼待確認
              newPW = enteredPW;
              enteredPW = "";
              changePasswordStep = CONFIRM_NEW_PASSWORD;
              display.showChangePasswordInput("Confirm New Password");
            } else if (changePasswordStep == CONFIRM_NEW_PASSWORD) {
              // 確認密碼是否一致
              if (enteredPW == newPW) {
                // 執行密碼修改
                if (pwManager.changePassword(oldPW, newPW)) {
                  display.showSuccess();
                  delay(2000);
                  // 重置變數
                  enteredPW = "";
                  oldPW = "";
                  newPW = "";
                  changePasswordStep = ENTER_OLD_PASSWORD;
                  currentState = MENU;
                  display.showMainMenu();
                } else {
                  display.showFailed();
                  delay(2000);
                  enteredPW = "";
                  oldPW = "";
                  newPW = "";
                  changePasswordStep = ENTER_OLD_PASSWORD;
                  currentState = MENU;
                  display.showMainMenu();
                }
              } else {
                // 兩次輸入不一致
                display.showFailed();
                delay(2000);
                enteredPW = "";
                newPW = "";
                changePasswordStep = ENTER_NEW_PASSWORD;
                display.showChangePasswordInput("Enter New Password");
              }
            }
          }
        }
      }
      break;
    }

    default:
      break;
  }
}
