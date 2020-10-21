//#include "I2C_Anything.h"

volatile unsigned int int_tic=0; 
volatile unsigned long tic;

void setup(){
  pinMode(3, OUTPUT); // инициализируем наш порт на выход
  analogWrite(3, 100); 
  
  Serial.begin(115200);
  //Wire.begin(0x0F);             // подключиться к i2c шине с адресом #0x0F
  //Wire.onRequest(requestEvent); // зарегистрировать обработчик события
  pinMode(5,INPUT); // вход сигнала T1 (only для atmega328)
  TCCR2A=1<<WGM21; //CTC mode
  TIMSK2=1<<OCIE2A; OCR2A=124 ;//прерывание каждые 8мс
  TCCR2B=(1<<CS22)|(1<<CS21)|(1<<CS20); //делитель 1024
  TCCR1A=0; TIMSK1 = 1<<TOIE1; //прерывание по переполнению
  TCCR1B = (1<<CS10)|(1<<CS11)|(1<<CS12);//тактировани от входа Т1
}

ISR (TIMER1_OVF_vect){ int_tic++; }

ISR (TIMER2_COMPA_vect){
  static byte n=1;
  if (n==125){
           tic= ((uint32_t)int_tic<<16) | TCNT1; //сложить что натикало
           int_tic=0;
           TCNT1 = 0; n=0;
           }
      n++; 
}

//   функция, которая будет выполняться всякий раз, когда от мастера принимаются данные
//   данная функция регистрируется как обработчик события, смотрите setup()
/*
void requestEvent(int howMany) 
{
  Serial.print("int_tic ");
  Serial.println(int_tic);
  I2C_writeAnything(30);
  //I2C_writeAnything(int_tic);
}
*/

void loop(){
  Serial.print("int loop ");
  Serial.println(int_tic);
  //чтение на 5-м пине
  delay(500);
}
