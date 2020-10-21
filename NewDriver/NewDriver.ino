#include <TimerOne.h>
#include <StepDirDriver.h>

int U0_PIN = A0;
int U1_PIN = A1;
int porog = 10;
byte SlowSpeed = 10;
byte MiddleSpeed = 6;
byte FastSpeed = 2;

StepDirDriver myMotor(9, 8, 12);  // создаем объект типа StepDirDriver, задаем выводы для сигналов

unsigned int timeCounter; // счетчик времени

void setup() {
  Timer1.initialize(250);  // инициализация таймера 1, период 250 мкс
  Timer1.attachInterrupt(timerInterrupt, 250);  // задаем обработчик прерываний

  myMotor.setMode(2, false);  // междушаговый режим, без фиксации при остановке
  myMotor.setDivider(SlowSpeed);     // делитель частоты 10 (1 оборот в сек) чем меньше, тем выше скорость.
  myMotor.step(0);  // начальный запуск

  Serial.begin(115200);
  Serial.println("Starting");
}

void loop() {
  //byte ind = 0;
  char St;  // режим: g - щель для промывки, F - широкий поиск, f - узкий поиск, b - отход от замыкания, s - шаг назад
  static char PrevSt = 'F';
  static unsigned long tm = millis();
  static int arrayU[30];
  static byte curNum = 0;
  static boolean flagU = false;
  
  int U0 = analogRead(U0_PIN);
  int U1 = analogRead(U1_PIN);

  if (U0 > 10 && U1 < porog) {
    //нет процесса
    if (PrevSt == 'b' || PrevSt == 'f' || PrevSt == 's' || PrevSt == 'g' || PrevSt == 'w') {
      St = 'f';
    } else {
      St = 'F';
    }
  } else if (U0 > 10 && U1 > 1020) {
    //замыкание
    St = 'b';
  } else if (U0 > 10 && U1 <= 1020) {
     //в рабочем процессе
     St = 'w';
     if (U1 > 800) {
      St = 's';
     }
  } else {
    //нет напряжения
    
  }
  
  if (St == 'w') {
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
        St = 'f';
      }
    }
  } else {
    flagU = false;
    curNum = 0;
  }

  unsigned long tm_curr = millis();
  //пять минут стоим
  if ((unsigned int)tm_curr - tm >= 240000) {
    Serial.print(" tm_curr - tm: ");
    Serial.println(tm_curr - tm);
    St = 'g'; 
    tm = tm_curr;
  }
  
  /*Serial.print("tm: ");
  Serial.print(tm);
  Serial.print(" tm_curr - tm: ");
  Serial.println(tm_curr - tm);*/
    
  if (St == 'F') {
    ChangeSpeed(MiddleSpeed);
    myMotor.step(-50);
  } else if (St == 'w') {
    myMotor.step(0);
    
  } else if (St == 'g') {
    ChangeSpeed(FastSpeed);
    TudaSuda(6000, 6000);     
    TudaSuda(-5950, -5950);     
  } else if (St == 'b') {
    myMotor.step(0);
    ChangeSpeed(FastSpeed);
    TudaSuda(3000, -2900);
    //myMotor.step(300);
  } else if (St == 'f') {
    ChangeSpeed(SlowSpeed);
    myMotor.step(-1);
  } else if (St == 's') {
    ChangeSpeed(SlowSpeed);
    myMotor.step(1);   
  }

  //if (PrevSt != St) {
    Serial.print("St: ");
    Serial.print(St);
    Serial.print(" PrevSt: ");
    Serial.print(PrevSt);
    Serial.print(" U0: ");
    Serial.print(U0);
    Serial.print(" U1: ");
    Serial.println(U1);
  //}

  PrevSt = St;
  delay(500);
}


void TudaSuda(int s1, int s2) {
    myMotor.step(s1);
    while (myMotor.readSteps() != 0) {
      delay(500);
    }
    myMotor.step(s2);
    while (myMotor.readSteps() != 0) {
      delay(500);
    }
}

//
void ChangeSpeed(byte speed) {
  static byte PrevSpeed = SlowSpeed;
  if (speed != PrevSpeed) {
    myMotor.setDivider(speed);
  }
  PrevSpeed = speed;
}
//-------------------------------------- обработчик прерывания 0,25 мс
void  timerInterrupt() {
  myMotor.control(); // управление двигателем
  timeCounter++; // счетчик времени
}

void AddToArray(int (&masU)[30], int &u, byte &n, boolean &f) {
  masU[n] = u;
  n++;
  if (n > 29) {
    n = 0;
    f = true;
  }
}
