// include the library code:
#include <LiquidCrystal.h>
#include <MsTimer2.h>
#include <CyberLib.h>

int arrayMeasure[20];

LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

//int KatodPin6 = 12;
//int AnodPin7 = 13;

boolean stable = HIGH;

int ph_pin = A5; //This is the pin number connected to Po

void PhMeter() //кислотность
{
  static float pHValue, Po;
  int measure = analogRead(ph_pin);

  int i; 
  int AllMeasure = measure;

  for (i = 19; i > 0; i = i - 1) {
    arrayMeasure[i] = arrayMeasure[i - 1];
    AllMeasure = AllMeasure + arrayMeasure[i];
  }
  arrayMeasure[0] = measure;

  double voltage = 5 / 1023.0 * AllMeasure/10; //classic digital to voltage conversion
  
  Po = 6.86 + 5 * (511 - AllMeasure/20) / 1023.0 / 0.193;
  
  lcd.setCursor(0, 0);
  lcd.print("Ph:");

  lcd.print(Po, 2);
  lcd.print(";    ");
}

void KontrolShield() //обработчик прерывания
{
  int x;

  x = analogRead (0);

  lcd.setCursor(0, 1);
  //lcd.print ("t");
  lcd.print ("Stable:");

  //lcd.setCursor(5, 1);

  if (x < 60) {
    lcd.print ("    ");
  }
  else if (x < 200) {
    stable = HIGH;
    lcd.print ("1  ");
    //lcd.print ("Up    ");
  }
  else if (x < 400) {
    stable = LOW;
    Serial.println("In control:");
    Serial.println(stable);
    lcd.print ("0  ");
    //lcd.print ("Down  ");
  }
  else if (x < 600) {
    //stable = LOW;
    lcd.print ("0  ");
    //lcd.print ("Left  ");
  }
  else if (x < 800) {
    //stable = HIGH;
    lcd.print ("1");
    //lcd.print ("Select");
  }
}

void pciSetup(byte pin) 
{
    *digitalPinToPCMSK(pin) |= bit (digitalPinToPCMSKbit(pin));  // enable pin
    PCIFR  |= bit (digitalPinToPCICRbit(pin)); // clear any outstanding interrupt
    PCICR  |= bit (digitalPinToPCICRbit(pin)); // enable interrupt for the group 
}

ISR (PCINT1_vect) // handle pin change interrupt for A0 to A5 here
 {
     KontrolShield();
 }  

void setup() {
  // put your setup code here, to run once:
  D6_Out;
  D7_Out;

  D6_Low;
  D7_High;
  
  //pinMode(AnodPin7, OUTPUT);
  //pinMode(KatodPin6, OUTPUT);

  //digitalWrite(AnodPin7, LOW);
  //digitalWrite(KatodPin6, LOW);

  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.setCursor(0, 0);
  lcd.print("I'm ready");
  lcd.setCursor(0, 1);
  lcd.print("Status: 0");

  MsTimer2::set(500, PhMeter); // 3s период
  MsTimer2::start();  //включить таймер
    
  digitalWrite(A0,HIGH);  // enable interrupt for pin...
  pciSetup(A0);

   Serial.begin(9600);      // открываем последовательное соединение

}

int ReplacePin1(int Position1) {
  switch (Position1) {
    case 1:
      D6_Low;
      //digitalWrite(KatodPin6, LOW);
    case 2:
      D7_High;
      //digitalWrite(AnodPin7, HIGH);
    case 3:
      D7_Low;
      //digitalWrite(AnodPin7, LOW);
    case 4:
      D6_High;
      //digitalWrite(KatodPin6, HIGH);
  }
  return 1;
}

void loop() {
  int t_short = 4;
  int t_long = 30;
  float t_dead = 1.0;
  float PriceOfStep = 0;
  int Position1 = 1;
  unsigned int Counter = 0;
  unsigned long T1 = 0, T2 = 0, TP1 = 0, TP2 = 0, Tc = 0;
  const int Q = 10000;

  while (1) {
    
    Counter++;
    switch (Counter) {
      case Q: //после 1 тысячи зануляем Counter
        Counter = 0;
        T2 = micros();
        break;
      case 1:
        T1 = micros();
        break;
      default: {} 
    }

    if (T1 < T2) {
      PriceOfStep = (T2 - T1) / Q;
    }

    if (PriceOfStep > 0) {
      Tc = Tc + PriceOfStep;
    }

    if (stable == HIGH) {
      D7_High;
      D6_Low;
      Position1 = 1;
    }
    else
    {
      switch (Position1) {
        case 1:
          if (Tc - TP1 > t_dead) {
            TP1 = Tc;
            ReplacePin1(Position1);
            Position1 = 2;
            break;
          }
        case 2:
          if (Tc - TP1 > t_long) {
            TP1 = Tc;
            ReplacePin1(Position1);
            Position1 = 3;
            break;
          }
        case 3:
          if (Tc - TP1 > t_dead) {
            TP1 = Tc;
            ReplacePin1(Position1);
            Position1 = 4;
            break;
          }
        case 4:
          if (Tc - TP1 > t_short) {
            TP1 = Tc;
            ReplacePin1(Position1);
            Position1 = 1;
            break;
          }
      }
    }
  }
}
