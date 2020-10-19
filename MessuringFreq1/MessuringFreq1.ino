
volatile unsigned int int_tic=0; 
volatile unsigned long tic; 
void setup() {
  Serial.begin(115200);
  TCCR1A=0; TIMSK1 = 1<<TOIE1; //прерывание по переполнению
}

ISR (TIMER1_OVF_vect){ int_tic++; }

void loop() {

  pinMode (5,INPUT); // вход сигнала T1 (only для atmega328)
  TCCR1B = (1<<CS10)|(1<<CS11)|(1<<CS12);//тактировани от входа Т1
  delay(1000);
  TCCR1B=0;
  tic= ((uint32_t)int_tic<<16) | TCNT1; //сложить что натикало
  int_tic=0; TCNT1 = 0; 
  Serial.print(tic);
  Serial.println(" Hz  ");


}
