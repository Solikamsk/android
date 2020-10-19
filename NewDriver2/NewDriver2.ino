#include <TimerOne.h>
#include <StepDirDriver.h>
#include <LiquidCrystal_I2C.h>
//#include <cmath.h>

#define PIN_U0 5
#define PIN_U1 6
#define MAX_U1 6000
#define U_BACK 3000
//для 50 кГц MAX_U1 6000 U_BACK 4000 porog = 1500

LiquidCrystal_I2C lcd(0x3F,17,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display

int porog = 100;//в милливольтах
//int MAX_U1 = 5000;//в милливольтах

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

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("Starting");

  Serial.begin(115200);
  //Serial.println("Starting");
}

int ReturnU1() {
  //делаем три измерения, берем среднее
  float sumU = 0;
  byte ss = 0;
  for (int i = 0; i <= 3; i++) {//3
    float Ttime1 = 0.0;
    float Ttime2 = 0.0;
    
    Ttime1 = pulseIn(PIN_U1, HIGH, 40000);//25000
    if (Ttime1 > 0.0) {
        Ttime2 = pulseIn(PIN_U1, LOW, 40000);//, 35000
    }
   
   if (Ttime1 > 0.0 && Ttime2 > 0.0) {
      sumU = sumU + (1000000/(Ttime1 + Ttime2) - 45)*5000/3215;
      ss++;
    }
  }
  
  if (ss == 0) {
    return 0;//-1
  } else {
    return (int) sumU/ss;
  } 
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
    
  U1 = ReturnU1();
  if (U1 >= MAX_U1) {
    //перепроверям два раза, может просто выброс
    for (int i = 0; i <= 2; i++) {
      int u = ReturnU1();
     if (u < MAX_U1 && u > 0) {
        U1 = u;
        break;
      }
    }
  }
  
  if(digitalRead(PIN_U0) == 1) {
    U0 = 5000;
  }

  if (U0 > 10 && U1 >= 0 && U1 < porog) {
    //нет процесса
    if (PrevSt == 'b' || PrevSt == 'f' || PrevSt == 's' || PrevSt == 'g' || PrevSt == 'w') {
      St = 'f';
    } else {
      St = 'F';
    }
  } else if (U0 > 10 && U1 >= MAX_U1) {
    //замыкание
    St = 'b';
  } else if (U0 > 10 && U1 > 0 && U1 < MAX_U1) {
     //в рабочем процессе
     St = 'w';
     if (U1 > U_BACK) {
      St = 's';
     }
  } else {
    //нет напряжения
    St = 's';
  }
  
  if (St == 'w' && U1 > 0) {
    AddToArray(arrayU, U1, curNum, flagU);
    delay(10);
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
        St = 's';
      }
    }
  } else {
    flagU = false;
    curNum = 0;
  }

  unsigned long tm_curr = millis();
  
  //пять минут стоим
  if ((tm_curr - tm) >= 240000) {//
    St = 'g'; 
    tm = tm_curr;
  }
  
  lcd.setCursor(0, 0);
  if (U0 > 0) {
    lcd.print("W");  
  } else {
    lcd.print("_");
  }
  lcd.setCursor(1, 0);
  lcd.print(" U"); 
  lcd.print(U1); //
  lcd.print("    ");
  lcd.setCursor(7, 0);
  lcd.print(" Pr="); 
  lcd.print(PrevSt);
  lcd.print(" C=");
  lcd.print(St);
   
  lcd.setCursor(0, 1);
  lcd.print("t=");
  lcd.print((tm_curr/1000)%1000);
  lcd.print("    ");
  lcd.setCursor(6, 1);
  lcd.print("m=");
  lcd.print(myMotor.readSteps());
  lcd.print("     ");
           
  if (St == 'F') {
    ChangeSpeed(MiddleSpeed);
    myMotor.step(-50);
  } else if (St == 'w') {
    myMotor.step(0);
    
  } else if (St == 'g') {
    ChangeSpeed(FastSpeed);
    TudaSuda(9000, -9000);     
    TudaSuda(5950, -5950);     
  } else if (St == 'b') {
    myMotor.step(0);
    ChangeSpeed(FastSpeed);
    TudaSuda(6000, -5925);
    //myMotor.step(300);
  } else if (St == 'f') {
    ChangeSpeed(SlowSpeed);
    myMotor.step(-1);
  } else if (St == 's') {
    ChangeSpeed(SlowSpeed);
    myMotor.step(1);   
  }

/*  if (PrevSt != St) {
    Serial.print(" St: ");
    Serial.print(St);
    Serial.print(" PrevSt: ");
    Serial.print(PrevSt);
    Serial.print(" U0: ");
    Serial.print(U0);
    Serial.print(" U1: ");
    Serial.println(U1);
  } 
*/
  PrevSt = St;
  delay(50);
}

void TudaSuda(int s1, int s2) {
    myMotor.step(s1);
    while (myMotor.readSteps() > 0) {
      lcd.setCursor(8, 1);
      lcd.print(myMotor.readSteps());
      lcd.print("    ");
      delay(250);
    }
    myMotor.step(s2);
    while (myMotor.readSteps() < 0) {
      lcd.setCursor(8, 1);
      lcd.print(myMotor.readSteps());
      lcd.print("    ");
      delay(250);
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
