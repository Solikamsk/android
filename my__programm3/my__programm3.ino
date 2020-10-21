// include the library code:
#include <LiquidCrystal.h>
#include <MsTimer2.h>
#define Offset 0.00            //deviation compensate

int arrayMeasure[20];

LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

int KatodPin6 = 6;
int AnodPin7 = 7;

// это для насоса
int PumpPin2 = 2;
int PeriodMin = 30;//раз в полчаса
unsigned int CurForPump = 0;
//\

int ph_pin = A5; //This is the pin number connected to Po

void PhMeter() //обработчик прерывания
{
  static float pHValue, Po;
  int measure = analogRead(ph_pin);

  int i; 
  int AllMeasure = measure;

  for (i = 19; i > 0; i = i - 1) {
    arrayMeasure[i] = arrayMeasure[i - 1];
    AllMeasure = AllMeasure + arrayMeasure[i];
  Serial.println("i: ");
  Serial.println(i);
  Serial.println(arrayMeasure[i]);
  }
  arrayMeasure[0] = measure;
  Serial.println(AllMeasure);

  lcd.setCursor(0, 0);
  lcd.print("Ph:");

  Serial.print("Measure: ");
  Serial.println(measure);

  double voltage = 5 / 1023.0 * AllMeasure/10; //classic digital to voltage conversion
  //double voltage = 4.9 / 1024.0 * measure; //classic digital to voltage conversion
  
  // PH_step = (voltage@PH7 - voltage@PH4) / (PH7 - PH4)
  // PH_probe = PH7 - ((voltage@PH7 - voltage@probe) / PH_step)
  //Po = 6.86 + ((2.5 - voltage) / 0.208);
  Po = 6.86 + 5 * (511 - AllMeasure/20) / 1023.0 / 0.193;
  
  
  lcd.print(Po, 2);
  lcd.print(";");
  lcd.print(measure);
  lcd.print("; ");
  //http://www.dfrobot.com/wiki/index.php/PH_meter_V1.1_SKU:SEN0161

  
 
}

void KontrolShield() //обработчик прерывания
{
  int x, curPeriodMin;

  x = analogRead (0);

  curPeriodMin = CurForPump / 2 / 60;

  lcd.setCursor(5, 1);
  //lcd.print ("t");
  lcd.print (curPeriodMin);
  lcd.print (";");

  //lcd.setCursor(5, 1);

  if (x < 60) {
    PeriodMin = PeriodMin + 10;
    //lcd.print ("Right " + PeriodMin);
    lcd.print (PeriodMin);
  }
  else if (x < 200) {
    PeriodMin = PeriodMin + 1;
    //lcd.print ("Up    " + PeriodMin);
    lcd.print (PeriodMin);
  }
  else if (x < 400) {
    PeriodMin = PeriodMin - 1;
    //lcd.print ("Down  " + PeriodMin);
    lcd.print (PeriodMin);
  }
  else if (x < 600) {
    PeriodMin = PeriodMin - 10;
    //lcd.print ("Left  " + PeriodMin);
    lcd.print (PeriodMin);
  }
  else if (x < 800) {
    lcd.print ("Select");
  }

  CurForPump++;

  //дернем один раз насос
  if (CurForPump == 2 * 60 * PeriodMin) {
    digitalWrite(PumpPin2, HIGH);
    delay(1500);
    digitalWrite(PumpPin2, LOW);
    CurForPump = 0;
  }
  else if (CurForPump <= 0) {
    digitalWrite(PumpPin2, HIGH);
    delay(1500);
    digitalWrite(PumpPin2, LOW);
    delay(1000);
    CurForPump = 0;
  }
}


void setup() {
  // put your setup code here, to run once:
  pinMode(PumpPin2, OUTPUT);
  pinMode(AnodPin7, OUTPUT);
  pinMode(KatodPin6, OUTPUT);

  digitalWrite(PumpPin2, LOW);
  digitalWrite(AnodPin7, LOW);
  digitalWrite(KatodPin6, LOW);

  Serial.begin(9600);      // открываем последовательное соединение

  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.setCursor(0, 0);
  lcd.print("LCD Key Shield");
  lcd.setCursor(0, 1);
  lcd.print("Time:");

  MsTimer2::set(100, PhMeter); // 3s период
  //MsTimer2::start();  //включить таймер

  //MsTimer2::set(500, KontrolShield); // 500ms период
  MsTimer2::start();  //включить таймер

}

int ReplacePin1(int Position1) {
  switch (Position1) {
    case 1:
      digitalWrite(KatodPin6, LOW);
    case 2:
      digitalWrite(AnodPin7, HIGH);
    case 3:
      digitalWrite(AnodPin7, LOW);
    case 4:
      digitalWrite(KatodPin6, HIGH);
  }
  return 1;
}

void loop() {

  int t_short = 4;
  int t_long = 30;
  float t_dead = 1.2;
  float PriceOfStep = 0;
  int Position1 = 1;
  unsigned int Counter = 0;
  unsigned long T1 = 0, T2 = 0, TP1 = 0, TP2 = 0, Tc = 0;
  const int Q = 10000;

  while (1) {
    Counter++;
    switch (Counter) {
      case Q: //после 10 тысячи зануляем Q
        Counter = 0;
        T2 = micros();
      case 1:
        T1 = micros();
    }

    if ((T1 < T2) && (Counter = 0)) {
      PriceOfStep = (T2 - T1) / Q;
    }

    if (PriceOfStep > 0) {
      Tc = Tc + PriceOfStep;
    }

    switch (Position1) {
      case 1:
        if (Tc - TP1 > t_dead) {
          TP1 = Tc;
          ReplacePin1(Position1);
          Position1 = 2;
        }
      case 2:
        if (Tc - TP1 > t_long) {
          TP1 = Tc;
          ReplacePin1(Position1);
          Position1 = 3;
        }
      case 3:
        if (Tc - TP1 > t_dead) {
          TP1 = Tc;
          ReplacePin1(Position1);
          Position1 = 4;
        }
      case 4:
        if (Tc - TP1 > t_short) {
          TP1 = Tc;
          ReplacePin1(Position1);
          Position1 = 1;
        }
    }
  }
}
