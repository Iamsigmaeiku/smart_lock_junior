#ifndef PASSWORD_H
#define PASSWORD_H

#include <Arduino.h>

// 密碼管理類（使用ESP32 Preferences存儲）
class Password {
public:
  // 初始化並從EEPROM載入密碼
  void init();

  // 驗證輸入的密碼
  bool verifyPassword(String input);

  // 修改密碼（需要提供舊密碼）
  bool changePassword(String oldPW, String newPW);

  // 重置為預設密碼
  void resetToDefault();

  // 獲取當前錯誤嘗試次數
  uint8_t getFailedAttempts();

  // 檢查是否被鎖定
  bool isLocked();

  // 清除錯誤計數
  void clearFailedAttempts();

private:
  String currentPassword;
  uint8_t failedAttempts;
  unsigned long lockUntil;  // 鎖定到此時間戳

  const String DEFAULT_PASSWORD = "1234";
  const uint8_t MAX_ATTEMPTS = 3;
  const unsigned long LOCK_DURATION = 30000;  // 30秒

  // 從Preferences載入密碼
  void loadPassword();

  // 保存密碼到Preferences
  void savePassword();

  // 驗證密碼格式
  bool isValidPassword(String pw);
};

#endif
