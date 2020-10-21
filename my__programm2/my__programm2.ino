int ledPin6 = 6;
int ledPin7 = 7;
int ledPin8 = 8;

int KolVoIt;
float averageT;

void setup() {
    // put your setup code here, to run once:
  pinMode(ledPin6,OUTPUT);
  pinMode(ledPin7,OUTPUT);
  pinMode(ledPin8,OUTPUT);
  
  digitalWrite(ledPin6,LOW);
  digitalWrite(ledPin7,LOW);
  digitalWrite(ledPin8,LOW);
  
}

int ReplacePin1(int Position1) {
  // put your main code here, to run repeatedly:
    switch (Position1) { 
      case 1: 
         digitalWrite(ledPin6,LOW);
      case 2: 
         digitalWrite(ledPin7,HIGH);
      case 3: 
         digitalWrite(ledPin7,LOW);
      case 4: 
         digitalWrite(ledPin6,HIGH);
     }
  return 1;
}

int ReplacePin2(int Position2) {  
    switch (Position2) { 
      case 1: 
         digitalWrite(ledPin8,LOW);
      case 2: 
         digitalWrite(ledPin8,HIGH);
     }
  return 1;
}

void loop() {

  int t_short = 4;
  int t_long = 30;
  float t_dead = 1.2;
  float PriceOfStep = 0;
  int T_big = 250000;
  int Position1 = 1;
  int Position2 = 1;
  unsigned int Counter = 0;
  unsigned long T1 = 0, T2 = 0, Tc = 0, TP1 = 0, TP2 = 0, Q = 10000;

  while (1) {
    Counter++;
    switch (Counter) { 
      case Q: 
        Counter = 0;
        T2 = micros(); 
      case 1: 
        T1 = micros();
      }
 
      if ((T1 < T2) && (Counter = 0)) {PriceOfStep = (T2 - T1)/Q;}

      if (PriceOfStep > 0) {Tc = Tc + PriceOfStep;}

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

      switch (Position2) {
        case 1:
          if (Tc - TP2 > T_big) {
            TP2 = Tc;
            ReplacePin2(Position2);
            Position2 = 2;
            }
        case 2:
          if (Tc - TP2 > T_big) {
            TP2 = Tc;
            ReplacePin2(Position2);
            Position2 = 1;
            }
      }
  }
}
