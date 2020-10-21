#include "I2C_Anything.h"

volatile unsigned int int_tic=0; 
volatile unsigned long tic; 
int int_tic_for_send = 0;

void setup(){

  analogWrite(3,127);
  
  Serial.begin(115200);
  Wire.begin(0x0F);             // подключиться к i2c шине с адресом #0x0F
  Wire.onRequest(requestEvent); // зарегистрировать обработчик события

  TCCR1A=0; TIMSK1 = 1<<TOIE1; //прерывание по переполнению
  
  Serial.println("Start");
}

ISR (TIMER1_OVF_vect){ int_tic++; }

//   функция, которая будет выполняться всякий раз, когда от мастера принимаются данные
//   данная функция регистрируется как обработчик события, смотрите setup()
void requestEvent(int howMany) 
{
   I2C_writeAnything(int_tic_for_send);
}

void loop(){
  
  pinMode (5,INPUT); // вход сигнала T1 (only для atmega328)
  TCCR1B = (1<<CS10)|(1<<CS11)|(1<<CS12);//тактировани от входа Т1
  delay(50);
  TCCR1B=0;
  tic = ((uint32_t)int_tic<<16) | TCNT1; //сложить что натикало
  int_tic_for_send = (int) tic*2;//это частота за период
  int_tic = 0; TCNT1 = 0;

  Serial.print("Start");
  Serial.println(tic);
}
