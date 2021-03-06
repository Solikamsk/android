/*
 This is the base example sketch for using the LeoPhi. It can output via USB serial and Serial1(hardware USART)
 just use which one you need. Also there is an I2C slave enabled version of this for an example how to use via
 I2C. We will be using timer1 for an ISR to take a fixed frequency sample (to oversample) adding
 16 consecutive samples together. We will then decimate this number back down to a 12bit number.
 Since our pH readings will change relatively slowly we can use this method to gain the added resolution.
 There are a number of ways to calculate pH based on the E(electromagnetic potentional) reading from our circuit.
 We will use a simple method, but recommend using temperature compensation maths for a much better result.
 A rolling average method can also be applied to filter the reading before display. Dont forget there are many pins
 broken out on the headers(I2C,Serial1,PWM,AnanlogIn). Both the green and blue leds are on PWM(though the pins sink current)
 and Red is digital. By default the green led will fade in and out to indicate on and working status
 
 Usage is simple as passing in commands to read pH, set calibration points, read with temp, etc...
 Feel free to adjust per usage, and please share so others can learn from any additions too!!
 LeoPhi can operate from (VCC) 2.65 to 5 so remember to set your reference voltage in math to this VCC.
 Sparky's Widgets 2012
 http://www.sparkyswidgets.com/Projects/LeoPhi.aspx
 
 */
#include <LiquidCrystal.h>
LiquidCrystal lcd(13, 5, 2, 3, 1, 0);
#include <avr/eeprom.h> //we'll use this to write out data structure to eeprom
//Our pin definitions
int PHIN = A0;
int HIN = A9;
int TIN = A10;
int GREENLED = 6; //on PWM line :note all these led pins sink not source
int BLUELED = 11; //on PWM line
int REDLED = 12; //normal digital
 
//LED fade effects
int brightness = 0;
int fadeAmount = 5;
 
//EEPROM trigger check
#define Write_Check 0x1234
#define VERSION 0x0001
 
//Oversampling Globals
#define BUFFER_SIZE 16 // For 12 Bit ADC data
volatile uint32_t result[BUFFER_SIZE];
volatile int i = 0;
volatile uint32_t sum=0;
 
//Rolling average this should act as a digital filter (IE smoothing)
const int numPasses = 20; //Larger number = slower response but greater smoothin effect
 
int passes[numPasses]; //Our storage array
int passIndex = 0; //what pass are we on? this one
long total = 0; //running total
int pHSmooth = 0; //Our smoothed pHRaw number
//pH calc globals
int pHRaw,tRaw;
float temp, miliVolts, pH,Temp,Hum; //using floats will transmit as 4 bytes over I2C
 
//Continous reading flag
bool continousFlag,statusGFlag;
 
//Our parameter, for ease of use and eeprom access lets use a struct
struct parameters_T
{
 unsigned int WriteCheck;
 int pH7Cal, pH4Cal,pH10Cal;
 bool continous,statusLEDG;
 float pHStep;
}
params;
 
void setup()
{
 pinMode(GREENLED, OUTPUT);
 setupADC(0,100); //Setup our ISR sampling routine analog pin 0 100hz
 //Serial1.begin(57600); //Enable basic sesrial commands in base version
 eeprom_read_block(¶ms, (void *)0, sizeof(params));
 continousFlag = params.continous;
 statusGFlag = params.statusLEDG;
 if (params.WriteCheck != Write_Check){
 reset_Params();
 }
 // initialize smoothing variables to 0:
 for (int thisPass = 0; thisPass < numPasses; thisPass++)
 passes[thisPass] = 0;
 
 lcd.begin(20,4);
 lcd.print("pH:");
 lcd.setCursor(0,1);
 lcd.print("Temp:");
 lcd.setCursor(0,2);
 lcd.print("Humidity:");
}
 
void loop()
{
 //Our smoothing portion
 //subtract the last pass
 total = total - passes[passIndex];
 //grab our pHRaw this should pretty much always be updated due to our Oversample ISR
 //and place it in our passes array this mimics an analogRead on a pin
 passes[passIndex] = pHRaw;
 total = total + passes[passIndex];
 passIndex = passIndex + 1;
 //Now handle end of array and make our rolling average portion
 if(passIndex >= numPasses)
 passIndex = 0;
 pHSmooth = total/numPasses;
 if(statusGFlag)
 {
 analogWrite(GREENLED, brightness);
 // change the brightness for next time through the loop:
 brightness = brightness + fadeAmount;
 // reverse the direction of the fading at the ends of the fade:
 if (brightness == 0 || brightness == 255) {
 fadeAmount = -fadeAmount ;
 }
 }
 if(Serial.available() )
 {
 String msgIN = "";
 char c;
 while(Serial.available())
 {
 c = Serial.read();
 msgIN += c;
 }
 processMessage(msgIN);
 }
calcpH();
tRaw = LM335ATempConvert(analogRead(TIN),'F');
// The max voltage value drops down 0.00372549 for each degree F over 32F. The voltage at 32F is 3.27 (corrected for zero precent voltage)
float max_voltage = (3.27-(0.00372549*tRaw));
Hum = (((((float)analogRead(HIN)/1023)*5)-.8)/max_voltage)*100;
lcd.setCursor(4,0);
lcd.print(pH);
lcd.print(" ");
lcd.setCursor(6,1);
lcd.print(tRaw);
lcd.print((char)223);
lcd.print('F');
lcd.print(" ");
lcd.setCursor(10,2);
lcd.print(Hum);
if(continousFlag)
{
 Serial.print("pHRaw: ");
 Serial.print(pHSmooth);
 Serial.print(" | ");
 Serial.print("pH10bit: ");
 Serial.print(analogRead(A0));
 Serial.print(" | ");
 Serial.print("Milivolts: ");
 Serial.println(miliVolts);
 Serial.print(" | ");
 Serial.print("pH: ");
 Serial.println(pH);
 Serial.print("Hum: ");
 Serial.println(Hum);
 delay(1000);
}
 delay(30);
}
 
void calcpH()
{
 miliVolts = (((float)pHSmooth/4096)*5)*1000;
 temp = ((((5*(float)params.pH7Cal)/4096)*1000)- miliVolts)/5.25; //5.25 is the gain of our amp stage we need to remove it
 pH = 7-(temp/params.pHStep);
}
 
void processMessage(String msg)
{
 if(msg.startsWith("L"))
 {
 if (msg.substring(2,1) == "0")
 {
 //Status led visual indication of a working unit on powerup 0 means off
 statusGFlag = false;
 digitalWrite(GREENLED, HIGH);
 Serial.println("Status led off");
 params.statusLEDG = statusGFlag;
 eeprom_write_block(¶ms, (void *)0, sizeof(params));
 }
 if (msg.substring(2,1) == "1")
 {
 //Status led visual indication of a working unit on powerup 0 means off
 statusGFlag = true;
 Serial.println("Status led on");
 params.statusLEDG = statusGFlag;
 eeprom_write_block(¶ms, (void *)0, sizeof(params));
 }
 
 }
 if(msg.startsWith("R"))
 {
 //take a pH reading
 calcpH();
 Serial.println(pH);
 }
 if(msg.startsWith("C"))
 {
 continousFlag = true;
 Serial.println("Continous Reading On");
 params.continous = continousFlag;
 eeprom_write_block(¶ms, (void *)0, sizeof(params));
 }
 if(msg.startsWith("E"))
 {
 //exit continous reading mode
 continousFlag = false;
 Serial.println("Continous Reading Off");
 params.continous = continousFlag;
 eeprom_write_block(¶ms, (void *)0, sizeof(params));
 }
 if(msg.startsWith("S"))
 {
 //Calibrate to pH7 solution, center on this for zero
 Serial.println("Calibrate 7");
 params.pH7Cal = pHSmooth;
 eeprom_write_block(¶ms, (void *)0, sizeof(params));
 }
 if(msg.startsWith("F"))
 {
 //calibrate to pH4 solution, recalculate our slope to account for probe
 Serial.println("Calibrate 4");
 params.pH4Cal = pHSmooth;
 //RefVoltage * our deltaRawpH / 12bit steps *mV in V / OP-Amp gain /pH step difference 7-4
 params.pHStep = ((((5*(float)(params.pH7Cal - params.pH4Cal))/4096)*1000)/5.25)/3;
 eeprom_write_block(¶ms, (void *)0, sizeof(params));
 }
 if(msg.startsWith("T"))
 {
 //calibrate to pH10 solution, recalculate our slope to account for probe
 Serial.println("Calibrate 10");
 params.pH10Cal = pHSmooth;
 //RefVoltage * our deltaRawpH / 12bit steps *mV in V / OP-Amp gain /pH step difference 10-7
 params.pHStep = ((((5*(float)(params.pH10Cal - params.pH7Cal))/4096)*1000)/5.25)/3;
 eeprom_write_block(¶ms, (void *)0, sizeof(params));
 }
 if(msg.startsWith("I"))
 {
 //Lets read in our parameters and spit out the info!
 eeprom_read_block(¶ms, (void *)0, sizeof(params));
 Serial.print("LeoPhi Info: Firmware Ver ");
 Serial.println(VERSION);
 Serial.print("pH 7 cal: ");
 Serial.print(params.pH7Cal);
 Serial.print(" | ");
 Serial.print("pH 4 cal: ");
 Serial.print(params.pH4Cal);
 Serial.print(" | ");
 Serial.print("pH probe slope: ");
 Serial.println(params.pHStep);
 }
 if(msg.startsWith("X"))
 {
 //restore to default settings
 Serial.println("Reseting to default settings");
 reset_Params();
 }
}
 
void reset_Params(void)
{
 //Restore to default set of parameters!
 params.WriteCheck = Write_Check;
 params.statusLEDG = true;
 params.continous = false; //turn off continous readings
 params.pH7Cal = 2048; //assume ideal probe and amp conditions 1/2 of 4096
 params.pH4Cal = 1286; //using ideal probe slope we end up this many 12bit units away on the 4 scale
 params.pH10Cal = 2810;//using ideal probe slope we end up this many 12bit units away on the 10 scale
 params.pHStep = 59.16;//ideal probe slope
 eeprom_write_block(¶ms, (void *)0, sizeof(params)); //write these settings back to eeprom
}
 
//LM335a temp conversion routine this just makes stuff a lot easier
int LM335ATempConvert(int tempIn, char unitSystem)
{
 int KelvinC=273;
 int KelvinTemp = (long(tempIn) * 5 * 100) / 1023; // convert
 int CelsiusTemp = KelvinTemp-KelvinC;
 int FahrenheitTemp = (CelsiusTemp)*(9/5)+32;
 int tempOut;
 
 switch(unitSystem){
 case 'K':
 tempOut = KelvinTemp;
 break;
 case 'C':
 tempOut = CelsiusTemp;
 break;
 case 'F':
 tempOut = FahrenheitTemp;
 break;
 }
 return tempOut;
}
 
//Our oversampling read functions we will access the hardware directly setting up a counter and a read frequency
//based on the default ADC clock of 250khz, this is all under an Interrupt Service Routine this means we need to keep
//everything contained within as fast as possible especially if we intend on using I2C (clock dragging)
void setupADC(uint8_t channel, int frequency)
{
 cli();
 ADMUX = channel | _BV(REFS0);
 ADCSRA |= _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0) | _BV(ADATE) | _BV(ADIE);
 ADCSRB |= _BV(ADTS2) | _BV(ADTS0); //Compare Match B on counter 1
 TCCR1A = _BV(COM1B1);
 TCCR1B = _BV(CS11)| _BV(CS10) | _BV(WGM12);
 uint32_t clock = 250000;
 uint16_t counts = clock/(BUFFER_SIZE*frequency);
 OCR1B = counts;
 
 TIMSK1 = _BV(OCIE1B);
 ADCSRA |= _BV(ADEN) | _BV(ADSC);
 sei();
}
 
ISR(ADC_vect)
{
 result[i] = ADC;
 i=++i&(BUFFER_SIZE-1);
 for(int j=0;j<BUFFER_SIZE;j++)
 {
 sum+=result[j];
 }
 if(i==0)
 {
 /****DEAL WITH DATA HERE*****/
 sum = sum>>2;
 //Serial.println(sum,DEC);
 //We will simply set a variable here and perform a rolling average on the pH.
 pHRaw = sum;
 }
 sum=0;
 TCNT1=0;
}
ISR(TIMER1_COMPB_vect)
{
}

