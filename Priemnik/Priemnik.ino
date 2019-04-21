/*
  Добавляем необходимые библиотеки
*/
#include <SPI.h>
#include <RH_ASK.h>
 
#define SPEED         (uint16_t)1200
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
  Serial.println(F("start!"));
 
  /*
    Инициализируем приёмник
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
  static uint8_t estdata;
  uint8_t data;
  uint8_t buflen = sizeof(data);

/*  if (driver.available()) {
    Serial.println("available!");
  }
  else {
    Serial.println("no available!");
  }
*/  
  /*
     Проверяем наличие новых данных
  */
  if (driver.recv((uint8_t*)&data, &buflen)) {
    /*
      Гасим светодиод
    */
    digitalWrite(LED_BUILTIN, LOW);
    /*
      Выводим в терминал '*' если полученные данные не совпадают с ожидаемыми
    */
    if (data != estdata) {
      Serial.print('*');
    }
    /*
      Выводим в терминал полученные данные
    */
    Serial.print("RX: ");
    Serial.println(data);
    /*
      Инкрементируем значение
    */
    estdata = data + 1;
    /*
      Включаем светодиод
    */
    digitalWrite(LED_BUILTIN, HIGH);
  }
}
