int ledPin8 = 8;
int ledPin7 = 7;
int ledPin6 = 6;

byte t_long = 50;
byte t_short = 7;
byte t_dead = 2;

unsigned long Counter = 0;
unsigned long MaxCounter = 150000;
byte Position = 1;

void setup() {
  // put your setup code here, to run once:
pinMode(ledPin8,OUTPUT);
pinMode(ledPin7,OUTPUT);
pinMode(ledPin6,OUTPUT);

digitalWrite(ledPin8,LOW);
digitalWrite(ledPin7,LOW);
digitalWrite(ledPin6,LOW);

}


void loop() {
  
  
  // put your main code here, to run repeatedly:
  digitalWrite(ledPin7,LOW);
  delayMicroseconds(t_dead);
  
  digitalWrite(ledPin6,HIGH);
  delayMicroseconds(t_short);
  
  digitalWrite(ledPin6,LOW);
  delayMicroseconds(t_dead);
  
  digitalWrite(ledPin7,HIGH);
  delayMicroseconds(t_long);

  Counter = Counter + t_long + t_short + t_dead*2;

  if (Counter > MaxCounter) {
    Counter = 0;
      switch (Position) {
        case 1:
          digitalWrite(ledPin8,HIGH);
          Position = 2;
          MaxCounter = 150000;
          break;
        case 2:
          digitalWrite(ledPin8,LOW);
          Position = 1;
          MaxCounter = 850000;
          break;
      }
    }
}
