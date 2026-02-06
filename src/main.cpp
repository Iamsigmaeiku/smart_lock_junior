// main.cpp
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

// 如果你沒在 config.h 定義，就用預設 800ms
#ifndef UI_RESULT_HOLD_MS
  #define UI_RESULT_HOLD_MS 800
#endif

uint32_t unlockStartMs = 0;
bool isUnlocking = false;

Fingerprint fingerSensor;
Screen display;
Motor doorMotor;
RFID rfidReader;
HuskyLens aiCamera;
wifi_comm wifiModule;
Password pwManager;

// =====================
// LED + Buzzer (non-blocking)
// =====================
enum UiSignal {
  SIG_IDLE,
  SIG_WAIT,      // 等待輸入
  SIG_VERIFY,    // 驗證中
  SIG_SUCCESS,   // 成功
  SIG_FAIL,      // 失敗
  SIG_LOCKED     // 密碼鎖定
};

UiSignal uiSig = SIG_IDLE;

struct BlinkJob {
  bool enabled = false;
  uint32_t nextMs = 0;
  uint16_t periodMs = 500;
  bool on = false;
};

struct BeepJob {
  bool enabled = false;
  uint32_t nextMs = 0;
  uint32_t endMs = 0;
  uint16_t periodMs = 120;   // 0 代表只叫一次
  bool on = false;
};

BlinkJob ledBlink;
BeepJob buzJob;

uint32_t uiSigExpireMs = 0;    // 0 = 不自動回復
UiSignal uiSigAfter = SIG_IDLE;

static inline void ledSet(bool r, bool g) {
  digitalWrite(LED_R_PIN, r ? HIGH : LOW);
  digitalWrite(LED_G_PIN, g ? HIGH : LOW);
}

static inline void buzzerOn() {
#if BUZZ_ACTIVE
  digitalWrite(BUZZ_PIN, HIGH);
#else
  ledcWriteTone(0, 2000);
  ledcWrite(0, 128);
#endif
}

static inline void buzzerOff() {
#if BUZZ_ACTIVE
  digitalWrite(BUZZ_PIN, LOW);
#else
  ledcWriteTone(0, 0);
  ledcWrite(0, 0);
#endif
}

// 單次短叫
static inline void beepOnce(uint16_t ms) {
  buzJob.enabled = true;
  buzJob.periodMs = 0;
  buzJob.on = true;
  buzJob.nextMs = millis() + ms;
  buzJob.endMs = buzJob.nextMs;
  buzzerOn();
}

// 閃叫 pattern（duration 期間每 period 交替）
static inline void beepPattern(uint16_t periodMs, uint16_t durationMs) {
  buzJob.enabled = true;
  buzJob.periodMs = periodMs;
  buzJob.on = true;
  buzJob.nextMs = millis() + periodMs;
  buzJob.endMs = millis() + durationMs;
  buzzerOn();
}

static inline void stopBeep() {
  buzJob.enabled = false;
  buzzerOff();
}

static inline void updateBeep() {
  if (!buzJob.enabled) return;
  uint32_t now = millis();

  // 單次叫
  if (buzJob.periodMs == 0) {
    if (now >= buzJob.nextMs) stopBeep();
    return;
  }

  // pattern
  if (now >= buzJob.endMs) {
    stopBeep();
    return;
  }

  if (now >= buzJob.nextMs) {
    buzJob.nextMs = now + buzJob.periodMs;
    buzJob.on = !buzJob.on;
    if (buzJob.on) buzzerOn();
    else buzzerOff();
  }
}

static inline void setUiSignal(UiSignal s, uint16_t holdMs = 0, UiSignal after = SIG_IDLE) {
  uiSig = s;

  // 設定自動回復
  if (holdMs > 0) {
    uiSigExpireMs = millis() + holdMs;
    uiSigAfter = after;
  } else {
    uiSigExpireMs = 0;
    uiSigAfter = SIG_IDLE;
  }

  // 每次切狀態先關閉閃爍
  ledBlink.enabled = false;

  switch (uiSig) {
    case SIG_IDLE:
      ledSet(false, false);
      break;

    case SIG_WAIT:
      // 等待輸入：綠燈常亮
      ledSet(false, false);
      break;

    case SIG_VERIFY:
      // 驗證中：綠燈閃爍
      ledSet(false, false);
      ledBlink.enabled = true;
      ledBlink.periodMs = 250;
      ledBlink.nextMs = millis() + ledBlink.periodMs;
      ledBlink.on = false;
      break;

    case SIG_SUCCESS:
      // 成功：綠燈常亮 + 嗶嗶
      ledSet(false, true);
      beepPattern(120, 300);
      break;

    case SIG_FAIL:
      // 失敗：紅燈常亮 + 嗶嗶
      ledSet(true, false);
      beepPattern(120, 600);
      break;

    case SIG_LOCKED:
      // 鎖定：紅燈慢閃 + 長叫
      ledSet(false, false);
      ledBlink.enabled = true;
      ledBlink.periodMs = 600;
      ledBlink.nextMs = millis() + ledBlink.periodMs;
      ledBlink.on = false;
      beepOnce(800);
      break;
  }
}

static inline void updateLedBlink() {
  if (!ledBlink.enabled) return;
  uint32_t now = millis();
  if (now < ledBlink.nextMs) return;

  ledBlink.nextMs = now + ledBlink.periodMs;
  ledBlink.on = !ledBlink.on;

  if (uiSig == SIG_VERIFY) {
    // 綠閃
    ledSet(false, ledBlink.on);
  } else if (uiSig == SIG_LOCKED) {
    // 紅閃
    ledSet(ledBlink.on, false);
  }
}

static inline void updateUiSignalTimeout() {
  if (uiSigExpireMs == 0) return;
  if (millis() >= uiSigExpireMs) {
    uiSigExpireMs = 0;
    setUiSignal(uiSigAfter);
  }
}

// =====================
// State machine
// =====================
enum SystemState {
  IDLE,
  MENU,
  SETTING_MENU,
  SETTING_ADD,
  SETTING_REMOVE,
  WAITING_INPUT,
  PASSWORD_INPUT,
  VERIFYING,
  UNLOCKING,
  LOCKED,
  ENROLLING,
  CHANGE_PASSWORD
};

SystemState currentState = IDLE;

uint8_t currentSettingType = 0; // 0=Finger, 1=Face, 2=RFID, 3=Password
uint8_t passwordChangeStep = 0; // 0=未開始, 1=輸入舊密碼, 2=輸入新密碼
String oldPasswordInput = "";
String newPasswordInput = "";

// 刪除功能相關變量
uint8_t removeItemIDs[10];
uint8_t removeItemCount = 0;

enum AuthMethod {
  NONE,
  FINGERPRINT,
  RFID_CARD,
  FACE_RECOGNITION,
  PASSWORD
};

AuthMethod lastAuthMethod = NONE;

void setup() {
  // ===== LED + Buzzer init =====
  pinMode(LED_R_PIN, OUTPUT);
  pinMode(LED_G_PIN, OUTPUT);
  digitalWrite(LED_R_PIN, LOW);
  digitalWrite(LED_G_PIN, LOW);

#if BUZZ_ACTIVE
  pinMode(BUZZ_PIN, OUTPUT);
  digitalWrite(BUZZ_PIN, LOW);
#else
  ledcAttachPin(BUZZ_PIN, 0);
  ledcWrite(0, 0);
#endif

  setUiSignal(SIG_IDLE);

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
  setUiSignal(SIG_IDLE);

  Serial.println("================================");
  Serial.println("✓ 系統準備就緒！");
  Serial.println("================================");
}

void loop() {
  wifiModule.update();

  // ===== LED/Buzzer update (non-blocking) =====
  updateLedBlink();
  updateBeep();
  updateUiSignalTimeout();

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
          setUiSignal(SIG_IDLE);
          delay(300);
          break;
        }

        // 檢查主選單按鈕（2x2）
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
              setUiSignal(SIG_WAIT);
              break;

            case 1: // RFID
              Serial.println("選擇：RFID驗證");
              lastAuthMethod = RFID_CARD;
              currentState = WAITING_INPUT;
              display.showWaitingForCard();
              setUiSignal(SIG_WAIT);
              break;

            case 2: // Password
              Serial.println("選擇：密碼驗證");
              lastAuthMethod = PASSWORD;
              currentState = PASSWORD_INPUT;
              display.showPasswordKeypad("Password", "Enter Password");
              display.updatePasswordDisplay("");
              setUiSignal(SIG_WAIT);
              break;

            case 3: // Face
              Serial.println("選擇：人臉驗證");
              lastAuthMethod = FACE_RECOGNITION;
              currentState = WAITING_INPUT;
              display.showWaitingForFinger();
              setUiSignal(SIG_WAIT);
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
          setUiSignal(SIG_IDLE);
          delay(300);
          break;
        }
      }

      // 檢測感應器輸入 -> VERIFYING
      if (fingerSensor.detectFinger()) {
        lastAuthMethod = FINGERPRINT;
        currentState = VERIFYING;
        setUiSignal(SIG_VERIFY);
      }
      if (rfidReader.detectCard()) {
        lastAuthMethod = RFID_CARD;
        currentState = VERIFYING;
        setUiSignal(SIG_VERIFY);
      }
      if (aiCamera.detectFace()) {
        lastAuthMethod = FACE_RECOGNITION;
        currentState = VERIFYING;
        setUiSignal(SIG_VERIFY);
      }
      break;
    }

    case PASSWORD_INPUT: {
      static String enteredPW = "";
      static unsigned long lastTouchTime = 0;

      if (pwManager.isLocked()) {
        setUiSignal(SIG_LOCKED, UI_RESULT_HOLD_MS, SIG_IDLE);
        display.showFailed();
        delay(800);
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

        // header back
        Screen::HeaderTouch headerPress = display.getHeaderTouch(x, y);
        if (headerPress == Screen::HEADER_BACK) {
          Serial.println("返回主選單");
          enteredPW = "";
          currentState = MENU;
          display.showMainMenu();
          setUiSignal(SIG_IDLE);
          delay(300);
          break;
        }

        int8_t key = display.getKeypadPress(x, y);

        if (key >= 0 && key <= 9) {
          if (enteredPW.length() < 8) {
            enteredPW += String(key);
            display.updatePasswordDisplay(enteredPW);
          }
        } else if (key == 10) {  // C
          enteredPW = "";
          display.updatePasswordDisplay(enteredPW);
        } else if (key == 11) {  // OK
          setUiSignal(SIG_VERIFY);
          if (enteredPW.length() < 4) {
            setUiSignal(SIG_FAIL, UI_RESULT_HOLD_MS, SIG_WAIT);
            display.showFailed();
            delay(600);
            enteredPW = "";
            display.showPasswordKeypad("Password", "Enter Password");
            display.updatePasswordDisplay("");
            setUiSignal(SIG_WAIT);
          } else {
            bool verified = pwManager.verifyPassword(enteredPW);
            if (verified) {
              setUiSignal(SIG_SUCCESS, UI_RESULT_HOLD_MS, SIG_IDLE);
              display.showSuccess();
              doorMotor.unlock();

              // Discord 推播（密碼成功）
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
              setUiSignal(SIG_FAIL, UI_RESULT_HOLD_MS, SIG_WAIT);
              display.showFailed();
              delay(800);
              enteredPW = "";
              if (pwManager.isLocked()) {
                setUiSignal(SIG_LOCKED, UI_RESULT_HOLD_MS, SIG_IDLE);
                currentState = MENU;
                display.showMainMenu();
                setUiSignal(SIG_IDLE);
              } else {
                display.showPasswordKeypad("Password", "Enter Password");
                display.updatePasswordDisplay("");
                setUiSignal(SIG_WAIT);
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
        setUiSignal(SIG_SUCCESS, UI_RESULT_HOLD_MS, SIG_IDLE);
        display.showSuccess();
        doorMotor.unlock();

        // Discord 推播（指紋/RFID/人臉成功）
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
        setUiSignal(SIG_FAIL, UI_RESULT_HOLD_MS, SIG_IDLE);
        display.showFailed();
        delay(3000);
        lastAuthMethod = NONE;
        currentState = MENU;
        display.showMainMenu();
        setUiSignal(SIG_IDLE);
      }
      break;
    }

    case UNLOCKING: {
      if (isUnlocking && (millis() - unlockStartMs >= UNLOCK_DURATION)) {
        doorMotor.lock();
        isUnlocking = false;
        lastAuthMethod = NONE;
        currentState = MENU;
        display.showMainMenu();
        setUiSignal(SIG_IDLE);
      }
      break;
    }

    case SETTING_MENU: {
      if (display.isTouched()) {
        int16_t x, y;
        display.getTouchPoint(x, y);

        // header back
        Screen::HeaderTouch headerPress = display.getHeaderTouch(x, y);
        if (headerPress == Screen::HEADER_BACK) {
          Serial.println("返回主選單");
          currentState = MENU;
          display.showMainMenu();
          setUiSignal(SIG_IDLE);
          delay(300);
          break;
        }

        int8_t btnPress = display.getSettingMenuPress(x, y);

        if (btnPress >= 0 && btnPress <= 3) {
          // Add
          currentSettingType = btnPress;
          currentState = SETTING_ADD;

          if (btnPress == 0) display.showAddFingerprint();
          else if (btnPress == 1) display.showAddFace();
          else if (btnPress == 2) display.showAddRFID();
          else if (btnPress == 3) {
            display.showPasswordKeypad("Change Pass", "Enter OLD");
            display.updatePasswordDisplay("");
            passwordChangeStep = 1;
            oldPasswordInput = "";
            newPasswordInput = "";
          }

          Serial.printf("新增類型: %d\n", btnPress);
          setUiSignal(SIG_WAIT);
          delay(300);
        } else if (btnPress >= 4 && btnPress <= 7) {
          // Remove
          currentSettingType = btnPress - 4;
          currentState = SETTING_REMOVE;

          removeItemCount = 0;
          if (currentSettingType == 0) {
            for (uint8_t id = 1; id <= 10 && removeItemCount < 10; id++) {
              if (fingerSensor.isFingerStored(id)) removeItemIDs[removeItemCount++] = id;
            }
          } else if (currentSettingType == 1) {
            for (uint8_t id = 1; id <= 10; id++) removeItemIDs[removeItemCount++] = id;
          } else if (currentSettingType == 2) {
            removeItemCount = rfidReader.getCardCount();
            for (uint8_t i = 0; i < removeItemCount && i < 10; i++) removeItemIDs[i] = i;
          } else if (currentSettingType == 3) {
            removeItemCount = 1;
            removeItemIDs[0] = 0;
          }

          display.showRemoveMenu(currentSettingType, removeItemIDs, removeItemCount);
          Serial.printf("刪除類型: %d, 項目數: %d\n", currentSettingType, removeItemCount);
          setUiSignal(SIG_IDLE);
          delay(300);
        }
      }
      break;
    }

    case SETTING_ADD: {
      static unsigned long lastTouchTime = 0;

      if (display.isTouched()) {
        int16_t x, y;
        display.getTouchPoint(x, y);

        // header back
        Screen::HeaderTouch headerPress = display.getHeaderTouch(x, y);
        if (headerPress == Screen::HEADER_BACK) {
          Serial.println("返回 Setting 選單");
          passwordChangeStep = 0;
          oldPasswordInput = "";
          newPasswordInput = "";
          currentState = SETTING_MENU;
          display.showSettingMenu();
          setUiSignal(SIG_IDLE);
          delay(300);
          break;
        }

        // 密碼修改鍵盤
        if (currentSettingType == 3) {
          if (millis() - lastTouchTime < 200) break;
          lastTouchTime = millis();

          int8_t key = display.getKeypadPress(x, y);

          if (key >= 0 && key <= 9) {
            if (passwordChangeStep == 1) {
              if (oldPasswordInput.length() < 8) {
                oldPasswordInput += String(key);
                display.updatePasswordDisplay(oldPasswordInput);
              }
            } else if (passwordChangeStep == 2) {
              if (newPasswordInput.length() < 8) {
                newPasswordInput += String(key);
                display.updatePasswordDisplay(newPasswordInput);
              }
            }
          } else if (key == 10) { // C
            if (passwordChangeStep == 1) oldPasswordInput = "";
            else if (passwordChangeStep == 2) newPasswordInput = "";
            display.updatePasswordDisplay((passwordChangeStep == 1) ? oldPasswordInput : newPasswordInput);
          } else if (key == 11) { // OK
            setUiSignal(SIG_VERIFY);

            if (passwordChangeStep == 1) {
              if (oldPasswordInput.length() < 4) {
                setUiSignal(SIG_FAIL, UI_RESULT_HOLD_MS, SIG_WAIT);
                display.showFailed();
                delay(600);
                oldPasswordInput = "";
                display.showPasswordKeypad("Change Pass", "Enter OLD");
                display.updatePasswordDisplay("");
                setUiSignal(SIG_WAIT);
              } else {
                bool verified = pwManager.verifyPassword(oldPasswordInput);
                if (verified) {
                  passwordChangeStep = 2;
                  newPasswordInput = "";
                  setUiSignal(SIG_SUCCESS, UI_RESULT_HOLD_MS, SIG_WAIT);
                  display.showPasswordKeypad("Set New Password", "Enter NEW");
                  display.updatePasswordDisplay("");
                  setUiSignal(SIG_WAIT);
                  delay(300);
                } else {
                  setUiSignal(SIG_FAIL, UI_RESULT_HOLD_MS, SIG_IDLE);
                  display.showFailed();
                  delay(800);
                  passwordChangeStep = 0;
                  oldPasswordInput = "";
                  currentState = SETTING_MENU;
                  display.showSettingMenu();
                  setUiSignal(SIG_IDLE);
                }
              }
            } else if (passwordChangeStep == 2) {
              if (newPasswordInput.length() < 4) {
                setUiSignal(SIG_FAIL, UI_RESULT_HOLD_MS, SIG_WAIT);
                display.showFailed();
                delay(600);
                newPasswordInput = "";
                display.showPasswordKeypad("Set New Password", "Enter NEW");
                display.updatePasswordDisplay("");
                setUiSignal(SIG_WAIT);
              } else {
                bool success = pwManager.changePassword(oldPasswordInput, newPasswordInput);
                if (success) {
                  setUiSignal(SIG_SUCCESS, UI_RESULT_HOLD_MS, SIG_IDLE);
                  display.showSuccess();
                  delay(800);
                } else {
                  setUiSignal(SIG_FAIL, UI_RESULT_HOLD_MS, SIG_IDLE);
                  display.showFailed();
                  delay(800);
                }
                passwordChangeStep = 0;
                oldPasswordInput = "";
                newPasswordInput = "";
                currentState = SETTING_MENU;
                display.showSettingMenu();
                setUiSignal(SIG_IDLE);
              }
            }
          }
        }
      }

      // 非密碼：新增
      if (currentSettingType == 0) {
        static bool enrollStarted = false;
        if (!enrollStarted) {
          enrollStarted = true;
          uint8_t targetID = fingerSensor.getNextAvailableID();
          Serial.printf("開始註冊指紋，ID: %d\n", targetID);

          setUiSignal(SIG_VERIFY);
          bool success = fingerSensor.enrollFinger(targetID);

          if (success) {
            setUiSignal(SIG_SUCCESS, UI_RESULT_HOLD_MS, SIG_IDLE);
            display.showSuccess();
          } else {
            setUiSignal(SIG_FAIL, UI_RESULT_HOLD_MS, SIG_IDLE);
            display.showFailed();
          }

          delay(800);
          enrollStarted = false;
          currentState = SETTING_MENU;
          display.showSettingMenu();
          setUiSignal(SIG_IDLE);
        }
      } else if (currentSettingType == 2) {
        if (rfidReader.detectCard()) {
          setUiSignal(SIG_VERIFY);
          if (rfidReader.enrollCard()) {
            setUiSignal(SIG_SUCCESS, UI_RESULT_HOLD_MS, SIG_IDLE);
            display.showSuccess();
          } else {
            setUiSignal(SIG_FAIL, UI_RESULT_HOLD_MS, SIG_IDLE);
            display.showFailed();
          }
          delay(800);
          currentState = SETTING_MENU;
          display.showSettingMenu();
          setUiSignal(SIG_IDLE);
        }
      } else if (currentSettingType == 1) {
        if (aiCamera.detectFace()) {
          setUiSignal(SIG_SUCCESS, UI_RESULT_HOLD_MS, SIG_IDLE);
          display.showSuccess();
          delay(800);
          currentState = SETTING_MENU;
          display.showSettingMenu();
          setUiSignal(SIG_IDLE);
        }
      }
      break;
    }

    case SETTING_REMOVE: {
      if (display.isTouched()) {
        int16_t x, y;
        display.getTouchPoint(x, y);

        // header back
        Screen::HeaderTouch headerPress = display.getHeaderTouch(x, y);
        if (headerPress == Screen::HEADER_BACK) {
          Serial.println("返回 Setting 選單");
          currentState = SETTING_MENU;
          display.showSettingMenu();
          setUiSignal(SIG_IDLE);
          delay(300);
          break;
        }

        int8_t selectedIndex = display.getRemoveMenuPress(x, y, removeItemCount);
        if (selectedIndex >= 0) {
          Serial.printf("選擇刪除項目: %d\n", selectedIndex);
          bool deleteSuccess = false;

          setUiSignal(SIG_VERIFY);

          if (currentSettingType == 0) {
            uint8_t fingerId = removeItemIDs[selectedIndex];
            deleteSuccess = fingerSensor.deleteFinger(fingerId);
          } else if (currentSettingType == 1) {
            uint8_t faceId = removeItemIDs[selectedIndex];
            deleteSuccess = aiCamera.forgetFace(faceId);
          } else if (currentSettingType == 2) {
            deleteSuccess = rfidReader.deleteCard(selectedIndex);
          } else if (currentSettingType == 3) {
            pwManager.resetToDefault();
            deleteSuccess = true;
          }

          if (deleteSuccess) {
            setUiSignal(SIG_SUCCESS, UI_RESULT_HOLD_MS, SIG_IDLE);
            display.showSuccess();
          } else {
            setUiSignal(SIG_FAIL, UI_RESULT_HOLD_MS, SIG_IDLE);
            display.showFailed();
          }
          delay(800);

          // 重新載入列表
          removeItemCount = 0;
          if (currentSettingType == 0) {
            for (uint8_t id = 1; id <= 10 && removeItemCount < 10; id++) {
              if (fingerSensor.isFingerStored(id)) removeItemIDs[removeItemCount++] = id;
            }
          } else if (currentSettingType == 1) {
            for (uint8_t id = 1; id <= 10; id++) removeItemIDs[removeItemCount++] = id;
          } else if (currentSettingType == 2) {
            removeItemCount = rfidReader.getCardCount();
            for (uint8_t i = 0; i < removeItemCount && i < 10; i++) removeItemIDs[i] = i;
          } else if (currentSettingType == 3) {
            removeItemCount = 1;
            removeItemIDs[0] = 0;
          }

          display.showRemoveMenu(currentSettingType, removeItemIDs, removeItemCount);
          setUiSignal(SIG_IDLE);
          delay(300);
        }
      }
      break;
    }

    case ENROLLING: {
      if (rfidReader.detectCard()) {
        setUiSignal(SIG_VERIFY);
        if (rfidReader.enrollCard()) {
          setUiSignal(SIG_SUCCESS, UI_RESULT_HOLD_MS, SIG_IDLE);
          display.showSuccess();
        } else {
          setUiSignal(SIG_FAIL, UI_RESULT_HOLD_MS, SIG_IDLE);
          display.showFailed();
        }
        delay(800);
        currentState = MENU;
        display.showMainMenu();
        setUiSignal(SIG_IDLE);
      }
      break;
    }

    default:
      break;
  }
}
