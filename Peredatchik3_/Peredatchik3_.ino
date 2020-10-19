
/*
  Добавляем необходимые библиотеки
*/
#include <SPI.h>
#include <RH_ASK.h>

#define SPEED         (uint16_t)2000
#define RX_PIN        (uint8_t)11
#define TX_PIN        (uint8_t)12 //это передача
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

void send_command(uint8_t msg) {
  digitalWrite(LED_BUILTIN, LOW);
  driver.send((uint8_t *)&msg,  sizeof(msg));
  //driver.send(&msg,  strlen(msg));
  driver.waitPacketSent();
  digitalWrite(LED_BUILTIN, HIGH);
}

void AddToArray(int (&masU)[30], int &u, byte &n, boolean &f) {
  masU[n] = u;
  n++;
  if (n > 29) {
    n = 0;
    f = true;
  }
}

void loop() {
  //uint8_t data[2] = {0, 0};
  //char msg = 110;//null
  static unsigned long tm = 0;
  static uint8_t old_msg = 110;
  uint8_t msg;
  const int porog = 25;
  const int porog2 = 500;
  static int arrayU[30];
  static byte curNum = 0;
  static boolean flagU = false;

  int U0 = analogRead(U0_PIN);
  int U1 = analogRead(U1_PIN);

  if (U0 < 10 && U1 < 10) {
    //нет напражения
    msg = 'z'; //zero
  } else if (U0 > 100 && U1 > 1020) {
    //похоже на короткое замыкание
    msg = 'b'; //полный назад
    if (flagU) {
      for (int i = 0; i <= 29; i++) {
        if (arrayU[i] < 1000) {
          msg = 'c';//не все значения критичные, можно и шаг назад сделать
          break;
        }
      }
    }
  } else if (U0 > 100 && U1  < porog) {
    //нет рабочего процесса
    unsigned long delta;
    unsigned long tm_curr;
    tm_curr = millis();
    delta = tm_curr - tm;
    if (delta >= 5000.00) {
      //Serial.println(delta);
      msg = 'm'; //move 109
    }
    //} else if ((U0 > 1020 && U1 > porog2)) {
    //слишком высокое напряжение. Шаг назад
  } else {
    //рабочий процесс. Запомним время. 5 сек, чтобы не двигаться вперед
    msg = 's'; //stop 115
    tm = millis();
  }

  if (msg == 's' || msg == 'c') {
    AddToArray(arrayU, U1, curNum, flagU);
    delay(100);
    if (flagU) {
      unsigned int SumDelta = 0;
 
      for (int i = 0; i <= 29; i++) {
        if (i == 29) {
          SumDelta = SumDelta + abs(arrayU[0] - arrayU[i]);
        } else {
          SumDelta = SumDelta + abs(arrayU[i + 1] - arrayU[i]);
        }
        /*
        Serial.print("i: ");
        Serial.print(i);
        Serial.print(" arrayU[i]: ");
        Serial.print(arrayU[i]);
        Serial.print(" SumDelta: ");
        Serial.println(SumDelta);
        */
      }

      if (SumDelta > 0 && SumDelta < porog*2) {
        Serial.print("SumDelta < porog*2: ");
        Serial.println(SumDelta);
        //delay(2000);
        msg = 'm';
      }
    }
  } else {
    flagU = false;
    curNum = 0;
  }

  if (old_msg != msg) {
    Serial.print("U0; ");
    Serial.print(U0);
    Serial.print(" U1 ");
    Serial.print(U1);
    Serial.print(" msg ");
    Serial.println(char(msg));
    old_msg = msg;

    send_command(msg);

    if (msg == 'b') {
      Serial.println("waiting");
      delay(16000);
      old_msg = 0;
    }
  } else if (msg == 'm') {
    send_command(msg);
    Serial.print("move U1:");
    Serial.println(U1);
    delay(1200);
  } else if (U1 > 150) {
    //Serial.print("U1 ");
    //Serial.println(U1);
  }

  //delay(3000);
}
