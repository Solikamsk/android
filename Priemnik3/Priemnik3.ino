/*
  Добавляем необходимые библиотеки
*/
#include <SPI.h>
#include <RH_ASK.h>
 
#define SPEED         (uint16_t)2000
#define RX_PIN        (uint8_t)11 //это прием
#define TX_PIN        (uint8_t)12
#define PTT_PIN       (uint8_t)10
#define PTT_INVERTED  false
 
/*
  Создаём экземпляр класса RH_ASK приёмника
*/
RH_ASK driver(SPEED, RX_PIN, TX_PIN, PTT_PIN, PTT_INVERTED);
 
void setup() {

  Serial.begin(115200);
  if (! driver.init()) {
    Serial.println(F("RF init failed!"));
    while (true) {
      delay(1);
    }
  }
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.println("Starting");
}
 
void loop() {

  uint8_t buf[RH_ASK_MAX_MESSAGE_LEN];
  uint8_t buflen = sizeof(buf);
  
  uint8_t i;
  if (driver.recv(buf, &buflen)) {
    digitalWrite(LED_BUILTIN, LOW);
    Serial.print("Size: ");
    Serial.print(buflen);
    Serial.println();
    Serial.print("Data: ");
    for (i = 0; i < buflen; i++) {
      Serial.print((char)buf[i]);
      //Serial.print(buf[i]);
      Serial.print(' ');
    }
    Serial.println();
    driver.printBuffer("Got:", buf, buflen);
    digitalWrite(LED_BUILTIN, HIGH);
  }
}
