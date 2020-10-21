// include the library code:
#include <LiquidCrystal.h>
#include <MsTimer2.h>

int arrayMeasure[20];

LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

int KatodPin6 = 12;
int AnodPin7 = 13;

boolean stable = HIGH;

int ph_pin = A5; //This is the pin number connected to Po

void PhMeter() //кислотность
{
  static float Po, Pt;
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
  Pt = 6.86 + 5 * (511 - measure) / 1023.0 / 0.193;
  
  lcd.setCursor(0, 0);
  lcd.print("Ph:");

  lcd.print(Po, 2);
  lcd.print("; ");
  lcd.print(Pt, 2);
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
  pinMode(AnodPin7, OUTPUT);
  pinMode(KatodPin6, OUTPUT);

  digitalWrite(AnodPin7, HIGH);
  digitalWrite(KatodPin6, LOW);//

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

}

int ReplacePin1(int Position1) {
  switch (Position1) {
    case 1:
      digitalWrite(KatodPin6, LOW);
      break;
    case 2:
      digitalWrite(AnodPin7, HIGH);
      break;
    case 3:
      digitalWrite(AnodPin7, LOW);
      break;
    case 4:
      digitalWrite(KatodPin6, HIGH);
      break;
  }
  return 1;
}

void loop() {
  while (1) {
    if (stable == HIGH) {
        digitalWrite(AnodPin7, HIGH);
        digitalWrite(KatodPin6, LOW);//
      }
      else
      {
        ReplacePin1(3);
        ReplacePin1(4);
        delayMicroseconds(4);
        ReplacePin1(1);
        ReplacePin1(2);
        delayMicroseconds(30);
      }
    } 
}
