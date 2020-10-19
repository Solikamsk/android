#include "I2C_Anything.h"

#include <TimerOne.h>
#include <LiquidCrystal_I2C.h>
#include <StepDirDriver.h>
//#include <cmath.h>

#define PIN_R 2
#define PIN_U0 5
#define MAX_U1 10000
#define U_BACK 3500
//для 50 кГц MAX_U1 6000 U_BACK 4000 porog = 1500

LiquidCrystal_I2C lcd(0x3F,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display

int porog = 10;//в милливольтах

byte SlowSpeed = 10;
byte MiddleSpeed = 6;
byte FastSpeed = 2;

StepDirDriver myMotor(9, 8, 12);  // создаем объект типа StepDirDriver, задаем выводы для сигналов

unsigned int timeCounter; // счетчик времени

volatile boolean haveData = false;
volatile unsigned int int_tic;
boolean hD = false;
unsigned int lostWire = 0;


void setup() {

  Wire.begin();
  
  Timer1.initialize(250);  // инициализация таймера 1, период 250 мкс
  Timer1.attachInterrupt(timerInterrupt, 250);  // задаем обработчик прерываний

  myMotor.setMode(2, false);  // междушаговый режим, без фиксации при остановке
  myMotor.setDivider(SlowSpeed);     // делитель частоты 10 (1 оборот в сек) чем меньше, тем выше скорость.
  myMotor.step(0);  // начальный запуск

  pinMode(PIN_R, INPUT);
  pinMode(PIN_U0, INPUT);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("Starting");

  Serial.begin(115200);
  Serial.println("Starting");
}

unsigned int ReturnU1() {
  int k = 0;
  
    Wire.requestFrom(0x0F, 2);    // запросить 2 байта от ведомого устройства #0x0F
    delay(20);
    
    if (Wire.available() == 2)    // ведомое устройство может послать меньше, чем запрошено
    { 
      I2C_readAnything(int_tic);
     haveData = true;
    } else {
      lostWire++;
    }

    hD = haveData;

    if (haveData) {
      haveData = false;
      float sumU = ((float)int_tic)/3215*5000;
      return (unsigned int) sumU;
    } else {
      return 0;
    }
}

String transformK(unsigned int val) {
  if (val > 999) {
    return String(round((double) val/1000)) + "k";
  } else {
    return String(val);
  }
}

void loop() {
  char St;  // режим: g - щель для промывки, F - широкий поиск, f - узкий поиск, b - отход от замыкания, s - шаг назад, w - в рабочем процессе
  static char PrevSt = 'B';// = 
  static unsigned long tm = millis();
  static unsigned long tmSec = millis();
  static unsigned long tmSlowSec = millis();
  static boolean first = true;
  static int steps = 0;
  
  int U0 = 0;//напряжение базовое в милливольтах
  unsigned int U1 = 0;//напряжение рабочее в милливольтах
  static unsigned int MaxU1 = 0;
  static unsigned int MinU1 = 40000;
  byte pump;
  static byte schBack = 0;
  static String History;

  U1 = ReturnU1();

  if (U1 > MaxU1) {
    MaxU1 = U1;
  }

  if (U1 < MinU1) {
    MinU1 = U1;
  }
  
  if(digitalRead(PIN_U0) == 1) {
    U0 = 5000;
  }

  if (hD == false) {
    St = 's';//нет ответа о напряжении, откат назад
  }else if (U0 > 10 && U1 >= 0 && U1 < porog) {
    //нет процесса
    if (PrevSt == 'b' || PrevSt == 'f' || PrevSt == 's' || PrevSt == 'g' || PrevSt == 'w') {
      St = 'f';
    } else {
      St = 'F';
    }
  } else if (U1 >= MAX_U1) {
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
  
  pump = digitalRead(PIN_R);
  if (pump == 0) {
    St = 'b';
  }
 
  unsigned long tm_curr = millis();
  //пять минут стоим
  if (St == 'b') {
    tm = tm_curr;
  } else if ((tm_curr - tm) >= 120000 && St == 'w') {
    St = 'g'; 
    tm = tm_curr;
  } 
  /*
    Serial.print("St: ");
    Serial.println(St);
     Serial.print("PrevSt: ");
    Serial.println(PrevSt);
    Serial.print("History: ");
    Serial.println(History);
    */
    
    if (St != PrevSt) {
      History += St;
      while (History.length() > 7) {
        History = History.substring(1);
        //History.remove(6);
      }
    } 
  
  if ((tm_curr - tmSec) >= 1000) {
    String S1;
    tmSec = tm_curr;
    
    if (U0 > 0) {
      S1 = "W";  
    } else {
      S1 = "_";
    }
    
    if (hD) {
      S1 += "" + transformK(U1) + "    ";
      //S1 += " F" + transformK(int_tic) + "   ";  
    } else {
      S1 += "___ ";
    }
    S1.remove(5);

    int t_curr = ((int)(tm_curr/1000))%100;
    S1 += "t" + String(t_curr);
    S1 += " " + History;
    
    lcd.setCursor(0, 0);
    lcd.print(S1);
  }

  if ((tm_curr - tmSlowSec) >= 5000) {
    String S2;
    tmSlowSec = tm_curr;

    S2 = "" + transformK(MaxU1) + "    ";
    S2.remove(3);

    S2 += "/" + transformK(MinU1) + "    ";
    S2.remove(7);
    MaxU1 = 0; MinU1 = 40000;
    
    S2 += " m" + String(steps) + "      ";
    S2.remove(16);
    //S2 += " L" + String(lostWire) + "  ";
     
    lcd.setCursor(0, 1);
    lcd.print(S2);
  }
    
  if (St == 'F') {
    ChangeSpeed(MiddleSpeed);
    myMotor.step(-50);
    steps -= -50;
  } else if (St == 'w') {
    myMotor.step(0);
     if (first) {
      steps = 0;
      first = false;
    }
 } else if (St == 'g') {
    ChangeSpeed(FastSpeed);
    TudaSuda(9000, -8990);
    steps -= 10;     
    //TudaSuda(5950, -5950);     
  } else if (St == 'b') {
    myMotor.step(0);
    ChangeSpeed(FastSpeed);
    if (schBack < 3) {
      TudaSuda(6400, -6350);
      schBack++;
    } else {
      TudaSuda(6400*2, -(6400*2- 50));//6400 полный оборот 1 мм
      schBack = 0;
    }
    steps -= 50;
    //myMotor.step(300);
  } else if (St == 'f') {
    ChangeSpeed(SlowSpeed);
    myMotor.step(-1);
    steps -= - 1;
  } else if (St == 's') {
    ChangeSpeed(SlowSpeed);
    myMotor.step(10);
    steps -= 10;   
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

  if (St == 'f') {
    delay(100);
  } else {
    delay(200);
  }
}

void TudaSuda(int s1, int s2) {
    //lcd.setCursor(0, 0);
    //lcd.print("ts");  

    myMotor.step(s1);
    while (myMotor.readSteps() > 0) {
      /*lcd.setCursor(8, 1);
      lcd.print(myMotor.readSteps());
      lcd.print("    ");*/
      delay(250);
    }
    myMotor.step(s2);
    while (myMotor.readSteps() < 0) {
      /*lcd.setCursor(8, 1);
      lcd.print(myMotor.readSteps());
      lcd.print("    ");*/
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
