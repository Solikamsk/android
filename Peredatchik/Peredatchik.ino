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
  Создаём экземпляр класса RH_ASK передатчика
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
  if (!driver.init()) {
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
  static uint8_t data = 0;
 
  /*
     Гасим светодиод
  */
  digitalWrite(LED_BUILTIN, LOW);
  /*
     Передаём данные
  */
  driver.send((uint8_t*)&data, sizeof(data));
  /*
    Ждем пока передача будет окончена
  */
  driver.waitPacketSent();
  /*
     Выводим в терминал отправленные данные
  */
  Serial.print("TX: ");
  Serial.println(data);
  /*
      Инкрементируем значение
  */
  ++data;
  /*
    Включаем светодиод
  */
  digitalWrite(LED_BUILTIN, HIGH);
  /*
    Ждём
  */
  delay(100);
}
