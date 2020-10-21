#include <LiquidCrystal.h>

volatile byte state = LOW;
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

void KontrolShield() //обработчик прерывания
{
  int x, curPeriodMin;

  x = analogRead (0);


  lcd.setCursor(3, 1);
  lcd.print ("       ");
  lcd.setCursor(3, 1);
  lcd.print (x);

  //lcd.setCursor(5, 1);

  if (x < 60) {
    //PeriodMin = PeriodMin + 10;
    //lcd.print ("Right  ");
    //lcd.print (x);
  }
  else if (x < 200) {
    //PeriodMin = PeriodMin + 1;
    //lcd.print ("Up    ");
    //lcd.print (x);
  }
  else if (x < 400) {
    //PeriodMin = PeriodMin - 1;
    //lcd.print ("Down  ");
    //lcd.print (x);
  }
  else if (x < 600) {
    //PeriodMin = PeriodMin - 10;
    //lcd.print ("Left  ");
    //lcd.print (x);
  }
  else if (x < 800) {
    //lcd.print ("Select");
  }
}

void setup(){
    //PORTB = 0b00111111;
    PORTC = 0b00111111;
    //PORTD = 0b11111111;
    PCICR = 0b00000111;
    //PCMSK2 = 0b11111111;
    PCMSK1 = 0b00111111;
    //PCMSK0 = 0b00111111; 

  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.setCursor(0, 0);
  lcd.print("LCD Key Shield");
  lcd.setCursor(0, 1);
  lcd.print("Time:");


  Serial.begin(9600);

}
 
void loop() {
int x;
    x = analogRead (0);
  Serial.println(x);

  if (state == HIGH) {
    KontrolShield();
    state = LOW;
    Serial.println(state);
    }
    delay(1000);
  } 
 
ISR(PCINT1_vect) {
    if (!(PINC & (1 << PC0))) {state = HIGH;}
//     if (!(PINC & (1 << PC1))) {/* Arduino pin A1 interrupt*/}
//     if (!(PINC & (1 << PC2))) {/* Arduino pin A2 interrupt*/}
//     if (!(PINC & (1 << PC3))) {/* Arduino pin A3 interrupt*/}
//     if (!(PINC & (1 << PC4))) {/* Arduino pin A4 interrupt*/}
//     if (!(PINC & (1 << PC5))) {/* Arduino pin A5 interrupt*/}
}

