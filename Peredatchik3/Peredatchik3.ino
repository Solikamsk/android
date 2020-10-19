/*
  Добавляем необходимые библиотеки
*/
#include <SPI.h>
#include <RH_ASK.h>
 
#define SPEED         (uint16_t)2000
#define RX_PIN        (uint8_t)11
#define TX_PIN        (uint8_t)12 //это пере дача
#define PTT_PIN       (uint8_t)10
#define PTT_INVERTED  false

int U0_PIN = A0;
int U1_PIN = A1;

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
}
 
void loop() {
  //uint8_t data[2] = {0, 0};
  char msg = 'n';//null
  int U0, U1;
  U0, U1 = analogRead(U0_PIN), analogRead(U1_PIN);

  //для отладки
  U0 = 900; 
  U1 = 900;
  
  if (U0 < 10 && U1 < 10) {
    //нет напражения
    msg = 'z'; //zero
  } else if (U0 > 800 && U1 < 100) {
    //короткое замыкание
    msg = 'b'; //back
  } else if (U0 > 800 && U1  > 800) {
    //нет рабочего процесса
    msg = 'm'; //move
  } else {
    msg = 's'; //stop
  }
 
  digitalWrite(LED_BUILTIN, LOW);
  driver.send((uint8_t )msg,  strlen(msg));
  driver.waitPacketSent();
  digitalWrite(LED_BUILTIN, HIGH);
  delay(100);
}
