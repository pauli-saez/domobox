/* Create by IOXhop : www.ioxhop.com */

#include <i2c_keypad.h>

I2CKEYPAD key;
// I2CKEYPAD key(21, 22); // if you use ESP8266 or ESP32 you can change SDA and SCL pin

void setup() {
  Serial.begin(9600);
  Serial.println("Start Keypad");
    
  key.begin(0x20, 100); // void begin(char addr = 0x20, long interval = 200) ;
  key.on(PRESS, [](char key) { // void on(KEYPAD_EVENT event, KeypadEventCallback callback) ;
    Serial.print("PRESS: ");
    Serial.println(key);
  });
  /*
  PRESS: devuelve la tecla al presionar
  DO: repite la tecla mientras este presionada
  RELEASE: devuelve la tecla al soltar 
  
  key.on(DO, [](char key) {
    Serial.print("DO: ");
    Serial.println(key);
  });
  key.on(RELEASE, [](char key) {
    Serial.print("RELEASE: ");
    Serial.println(key);
  });
  */
}

void loop() {
  key.scand();
}