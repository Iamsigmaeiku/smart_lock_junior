#ifndef MOTOR_H
#define MOTOR_H

#include <Arduino.h>
#include <ESP32Servo.h>

// SG90 舵機控制類
// 接腳: VCC->5V, GND->GND, Signal->GPIO13
class Motor {
public:
  // 初始化
  void init(uint8_t pin);

  // 開鎖
  void unlock();

  // 上鎖
  void lock();

private:
  uint8_t _pin;
  Servo _servo;
};

#endif
