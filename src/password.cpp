#include "password.h"
#include <Preferences.h>

Preferences prefs;

void Password::init() {
  failedAttempts = 0;
  lockUntil = 0;
  
  // 初始化Preferences
  prefs.begin("smartlock", false);  // false = read/write
  
  // 載入密碼
  loadPassword();
  
  Serial.println("密碼管理模組初始化完成");
}

void Password::loadPassword() {
  // 從Preferences讀取密碼，若無則使用預設值
  currentPassword = prefs.getString("password", DEFAULT_PASSWORD);
  
  // 如果是首次使用，保存預設密碼
  if (!prefs.isKey("password")) {
    savePassword();
    Serial.println("首次使用，設定預設密碼: " + DEFAULT_PASSWORD);
  } else {
    Serial.println("載入已存儲的密碼");
  }
}

void Password::savePassword() {
  prefs.putString("password", currentPassword);
  Serial.println("密碼已保存到EEPROM");
}

bool Password::verifyPassword(String input) {
  // 檢查是否被鎖定
  if (isLocked()) {
    unsigned long remainingTime = (lockUntil - millis()) / 1000;
    Serial.printf("系統已鎖定，剩餘 %lu 秒\n", remainingTime);
    return false;
  }
  
  // 驗證密碼
  if (input == currentPassword) {
    Serial.println("密碼驗證成功");
    clearFailedAttempts();
    return true;
  } else {
    failedAttempts++;
    Serial.printf("密碼錯誤（錯誤次數: %d/%d）\n", failedAttempts, MAX_ATTEMPTS);
    
    // 達到最大錯誤次數，鎖定系統
    if (failedAttempts >= MAX_ATTEMPTS) {
      lockUntil = millis() + LOCK_DURATION;
      Serial.println("錯誤次數過多，系統鎖定30秒");
    }
    
    return false;
  }
}

bool Password::changePassword(String oldPW, String newPW) {
  // 驗證舊密碼
  if (oldPW != currentPassword) {
    Serial.println("舊密碼錯誤，無法修改");
    return false;
  }
  
  // 驗證新密碼格式
  if (!isValidPassword(newPW)) {
    Serial.println("新密碼格式無效（需4-8位數字）");
    return false;
  }
  
  // 更新密碼
  currentPassword = newPW;
  savePassword();
  Serial.println("密碼已成功修改");
  
  return true;
}

void Password::resetToDefault() {
  currentPassword = DEFAULT_PASSWORD;
  savePassword();
  clearFailedAttempts();
  Serial.println("密碼已重置為預設值");
}

uint8_t Password::getFailedAttempts() {
  return failedAttempts;
}

bool Password::isLocked() {
  if (lockUntil > 0 && millis() < lockUntil) {
    return true;
  }
  
  // 鎖定時間已過，解除鎖定
  if (lockUntil > 0 && millis() >= lockUntil) {
    lockUntil = 0;
    clearFailedAttempts();
  }
  
  return false;
}

void Password::clearFailedAttempts() {
  failedAttempts = 0;
}

bool Password::isValidPassword(String pw) {
  // 檢查長度（4-8位）
  if (pw.length() < 4 || pw.length() > 8) {
    return false;
  }
  
  // 檢查是否全為數字
  for (int i = 0; i < pw.length(); i++) {
    if (!isDigit(pw.charAt(i))) {
      return false;
    }
  }
  
  return true;
}
