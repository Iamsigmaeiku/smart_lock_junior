#include <Arduino.h>
#include <ESP32Servo.h>
#include "motor.h"

Servo myservo;

void Motor::init(uint8_t pin) {
  _pin = pin;
  myservo.attach(_pin);
  myservo.write(0);
  delay(1000);
  myservo.detach();
  Serial.println("初始化舵機...");

}

void Motor::unlock() {
  myservo.attach(_pin);
  myservo.write(0);
  delay(50);
  _servo.write(0);
  delay(1000);
  Serial.println("開鎖");
}

void Motor::lock() {
  myservo.attach(_pin);
  myservo.write(90);
  _servo.write(90);
  delay(1000);
  Serial.println("上鎖");
}