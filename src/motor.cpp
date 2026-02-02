#include <Arduino.h>//宣告Arduino.h
#include <ESP32Servo.h>//宣告ESP32Servo.h
#include "motor.h"//宣告motor.h

Servo myservo;//宣告Servo myservo; 語法 類型 變數名稱

void Motor::init(uint8_t pin) {//宣告void Motor::init(uint8_t pin) {語法 類型 函數名稱(參數) {
  _pin = pin;//宣告_pin = pin; 語法 類型 變數名稱 = 值;
  myservo.attach(_pin);//宣告myservo.attach(_pin); 語法 類型函數名稱(參數);
  myservo.write(0);//宣告myservo.write(0); 語法 類型.函數名稱(參數);
  delay(1000);//給馬達充足時間轉到指定位置
  myservo.detach();//讓馬達斷開，停止耗電。省電
  Serial.println("初始化舵機...");//印出初始化舵機...

}

void Motor::unlock() {//宣告void Motor::unlock() {語法 類型 函數名稱() {
  myservo.attach(_pin);//detach後再使用attach，重新連接馬達
  myservo.write(0);//寫入0度，開鎖
  delay(1000);//給馬達充足時間轉到指定位置
  Serial.println("開鎖");//印出開鎖
  myservo.detach();//讓馬達斷開，停止耗電。省電
}

void Motor::lock() {//宣告void Motor::lock() {語法 類型 函數名稱() {
  myservo.attach(_pin);//detach後再使用attach，重新連接馬達
  myservo.write(90);//寫入90度，上鎖
  delay(1000);//給馬達充足時間轉到指定位置
  Serial.println("上鎖");//印出上鎖
  myservo.detach();//讓馬達斷開，停止耗電。省電
}