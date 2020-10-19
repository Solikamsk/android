#include <TimerOne.h>
#include <StepDirDriver.h>

#define PIN_U0 5
#define PIN_U1 6

int MAX_U1 = 5000;
int porog = 100;//в милливольтах
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

  pinMode(PIN_U0, INPUT);
  pinMode(PIN_U1, INPUT);

  Serial.begin(115200);
  Serial.println("Starting");
}

void loop() {
  //byte ind = 0;
  char St;  // режим: g - щель для промывки, F - широкий поиск, f - узкий поиск, b - отход от замыкания, s - шаг назад, w - в рабочем процессе
  static char PrevSt = 'F';
  static unsigned long tm = millis();
  static int arrayU[30];
  static byte curNum = 0;
  static boolean flagU = false;
  
  int U0 = 0;//напряжение базовое в милливольтах
  int U1 = 0;//напряжение рабочее в милливольтах
  
  /*int Htime;       // целочисленная переменная для хранения времени высокого логического уровня
  int Ltime;       // целочисленная переменная для хранения времени низкого логического уровня
  Htime=pulseIn(PIN_U1, HIGH);    // прочитать время высокого логического уровня
  Ltime=pulseIn(PIN_U1, LOW);     // прочитать время низкого логического уровня */

       // переменная для хранения общей длительности периода

  //делаем три измерения, берем среднее
  float sumU = 0;
  byte ss = 0;
  for (int i = 0; i <= 3; i++) {
    float Ttime1;
    float Ttime2;
    
    Ttime1 = pulseIn(PIN_U1, HIGH,10000);
    Ttime2 = pulseIn(PIN_U1, LOW,10000);
    if (Ttime1 > 50 && Ttime2 > 50) {
      float u = (1000000/(Ttime1 + Ttime2) - 45)*5000/3215;
      if (u > MAX_U1) {
        u = 5000.0;
      }
      sumU = sumU + u;
      ss++;
    }
  }

  if (ss > 0) {
    sumU = sumU/ss;
    U1 = (int)sumU;
  }
    
  if(digitalRead(PIN_U0) == 1) {
    U0 = 5000;
  }

  /*Serial.print("Ttime: ");
  Serial.print(Ttime);
  Serial.print(" U0: ");
  Serial.print(U0);
  Serial.print(" U1: ");
  Serial.println(U1);
  */      

  if (U0 > 10 && U1 >= 0 && U1 < porog) {
    //нет процесса
    if (PrevSt == 'b' || PrevSt == 'f' || PrevSt == 's' || PrevSt == 'g' || PrevSt == 'w') {
      St = 'f';
    } else {
      St = 'F';
    }
  } else if (U0 > 10 && U1 > 4950) {
    //замыкание
    St = 'b';
  } else if (U0 > 10 && U1 > 0 && U1 <= 4950) {
     //в рабочем процессе
     St = 'w';
     if (U1 > 3500) {
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
    Serial.print("sumU: ");
    Serial.print(sumU);
    Serial.print(" ss: ");
    Serial.print(ss);
    Serial.print(" St: ");
    Serial.print(St);
    Serial.print(" PrevSt: ");
    Serial.print(PrevSt);
    Serial.print(" U0: ");
    Serial.print(U0);
    Serial.print(" U1: ");
    Serial.println(U1);
  //} 

  PrevSt = St;
  delay(10);
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
