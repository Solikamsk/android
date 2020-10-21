#include <LiquidCrystal.h>

LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

// Install Pin change interrupt for a pin, can be called multiple times
void KontrolShield() //обработчик прерывания
{
  int x, curPeriodMin;

  x = analogRead (0);


  lcd.setCursor(3, 1);
  //lcd.print ("t");
  lcd.print (x);

  //lcd.setCursor(5, 1);

  if (x < 60) {
    //PeriodMin = PeriodMin + 10;
    lcd.print ("Right");
    //lcd.print (x);
  }
  else if (x < 200) {
    //PeriodMin = PeriodMin + 1;
    lcd.print ("Up");
    //lcd.print (x);
  }
  else if (x < 400) {
    //PeriodMin = PeriodMin - 1;
    lcd.print ("Down");
    //lcd.print (x);
  }
  else if (x < 600) {
    //PeriodMin = PeriodMin - 10;
    lcd.print ("Left");
    //lcd.print (x);
  }
  else if (x < 800) {
    lcd.print ("Select");
  }
}

void pciSetup(byte pin) 
{
    *digitalPinToPCMSK(pin) |= bit (digitalPinToPCMSKbit(pin));  // enable pin
    PCIFR  |= bit (digitalPinToPCICRbit(pin)); // clear any outstanding interrupt
    PCICR  |= bit (digitalPinToPCICRbit(pin)); // enable interrupt for the group 
}

// Use one Routine to handle each group

ISR (PCINT1_vect) // handle pin change interrupt for A0 to A5 here
 {
     KontrolShield();
     //digitalWrite(13,digitalRead(A0));
 }  

void setup() {   
int i; 

// set pullups, if necessary
//  for (i=0; i<=12; i++) 
//      digitalWrite(i,HIGH);  // pinMode( ,INPUT) is default
//
  for (i=A0; i<=A5; i++) 
      digitalWrite(i,HIGH); 
//
//  pinMode(13,OUTPUT);  // LED

// enable interrupt for pin...
  pciSetup(A0);
}


void loop() {
  // Nothing needed
}

