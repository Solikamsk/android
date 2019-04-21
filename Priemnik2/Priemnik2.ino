/*
  Добавляем необходимые библиотеки
*/
#include <SPI.h>
#include <RH_ASK.h>
 
#define SPEED         (uint16_t)2000
#define RX_PIN        (uint8_t)11
#define TX_PIN        (uint8_t)12
#define PTT_PIN       (uint8_t)10
#define PTT_INVERTED  false
 
/*
  Создаём экземпляр класса RH_ASK приёмника
*/
RH_ASK driver(SPEED, RX_PIN, TX_PIN, PTT_PIN, PTT_INVERTED);
 
void setup() {
  /*
    задаем скорость общения с компьютером
  */
  Serial.begin(115200);
  /*
    Инициализируем передатчик
  */
  if (! driver.init()) {
    Serial.println(F("RF init failed!"));
    while (true) {
      delay(1);
    }
  }
  /*
    Настройка встроенного светодиода
  */
  pinMode(LED_BUILTIN, OUTPUT);
}
 
void loop() {
  /*
     Буфер полученных данных
  */
  uint8_t data[2];
  /*
     Размер полученных данных
  */
  uint8_t buflen = sizeof(data);
  uint8_t i;
  /*
     Проверяем наличие новых данных
  */
  if (driver.recv(data, &buflen)) {
    /*
      Гасим светодиод
    */
    digitalWrite(LED_BUILTIN, LOW);
    /*
      Выводим в терминал полученные данные
    */
    Serial.print("Size: ");
    Serial.print(buflen);
    Serial.println();
    Serial.print("Data: ");
    for (i = 0; i < buflen; i++) {
      //Serial.print((char)data[i]);
      Serial.print(data[i]);
      Serial.print(' ');
    }
    Serial.println();
    /*
      Включаем светодиод
    */
    digitalWrite(LED_BUILTIN, HIGH);
  }
}
