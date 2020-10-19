#include "I2C_Anything.h"
#include <FreqMeasure.h>

//volatile unsigned int int_tic=0; 
//volatile unsigned long tic;
volatile unsigned int averageFrequency = 0;

void setup(){

  analogWrite(3,127);
  
  Serial.begin(115200);
  Wire.begin(0x0F);             // подключиться к i2c шине с адресом #0x0F
  Wire.onRequest(requestEvent); // зарегистрировать обработчик события

  FreqMeasure.begin();

}


//   функция, которая будет выполняться всякий раз, когда от мастера принимаются данные
//   данная функция регистрируется как обработчик события, смотрите setup()
void requestEvent(int howMany) 
{
    I2C_writeAnything(averageFrequency);
}

void loop(){
  static unsigned int arrayF[10];
  static byte curNum = 0;
  static boolean flagF = false;

  float fr0 = 0;
  if (FreqMeasure.available()) {
    // average several reading together
    double sum = FreqMeasure.read();
    fr0 = FreqMeasure.countToFrequency(sum);
    if (fr0 > 40000) {
      fr0 = 40000;
    }
  }

  unsigned int f = (unsigned int) fr0;
  AddToArray(f, arrayF, curNum, flagF);

  if (flagF) {
    double SumDelta = 0;
    
    for (int i = 0; i <= 9; i++) {
        SumDelta = SumDelta + arrayF[i];
    }
    averageFrequency = (unsigned int) (SumDelta/10);
  }
  
  delay(25);
}

void AddToArray(unsigned int &f, unsigned int (&masF)[10], byte &n, boolean &fl) {
  masF[n] = f;
  n++;
  if (n > 9) {
    n = 0;
    fl = true;
  }
}
