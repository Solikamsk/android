// include the library code:
#include <CyberLib.h>
#include <LiquidCrystal.h>

boolean stable = LOW;

int ReplacePin1(int Position1) {
  switch (Position1) {
    case 1:
      D6_Low;
      break;
    case 2:
      D7_High;
      break;
    case 3:
      D7_Low;
      break;
    case 4:
      D6_High;
      break;
  }
  return 1;
}

void setup() {
  // put your setup code here, to run once:
  D6_Out;
  D7_Out;

  D6_Low;
  D7_Low;

}

void loop() {
  while (1) {
    if (stable == HIGH) {
        D7_High;
        D6_Low;
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
