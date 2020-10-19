// Install Pin change interrupt for a pin, can be called multiple times

void pciSetup(byte pin) 
{
    *digitalPinToPCMSK(pin) |= bit (digitalPinToPCMSKbit(pin));  // enable pin
    PCIFR  |= bit (digitalPinToPCICRbit(pin)); // clear any outstanding interrupt
    PCICR  |= bit (digitalPinToPCICRbit(pin)); // enable interrupt for the group 
}

// Use one Routine to handle each group

ISR (PCINT1_vect) // handle pin change interrupt for A0 to A5 here
 {
     Serial.println("asd");
     //digitalWrite(13,digitalRead(A0));
 }  

void setup() {   
int i; 

  Serial.begin(9600);
// set pullups, if necessary

  for (i=A0; i<=A0; i++) 
      digitalWrite(i,HIGH); 

  pinMode(13,OUTPUT);  // LED

// enable interrupt for pin...
  pciSetup(A0);
}


void loop() {
  // Nothing needed
}

