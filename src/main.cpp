#include <TFT_eSPI.h>
#include<Arduino.h>

TFT_eSPI tft;

void setup() {
  Serial.begin(9600);
  tft.init();

  uint32_t id = tft.readcommand32(0x04); 
  Serial.print("Display ID: 0x");
  Serial.println(id, HEX);
}

void loop() {}
