#include <AccelStepper.h>
#define pot0 A0
#define pot1 A1

//A+ red; A- green; B+ yellow; B- blue
//AccelStepper stepper; // Defaults to AccelStepper::FULL4WIRE (4 pins) on 2, 3, 4, 5
AccelStepper mystepper(1, 9, 8); //1, step, direction
unsigned long tm = millis();
unsigned long tm_curr;
unsigned long PosZero; 

void setup()
{  
    mystepper.setMaxSpeed(5000);
    mystepper.setSpeed(1000); 
    mystepper.setAcceleration(300);
    mystepper.setCurrentPosition(0);
    Serial.begin(9600); 
}

void loop()
{  
  int Vist = analogRead(pot0);//макс. 1023
  int Vgape = analogRead(pot1);
  int dir = 1;

  Vgape = 102; Vist = 1023;

  PosZero = mystepper.currentPosition();
  //Serial.println(PosZero);

  if (Vist < 15 && Vgape < 15) { 
    //напряжения нет.Ничего не делаем
  } else if (Vist > 1000 && Vgape > 1000) { 
    //напряжение есть. Надо сводить электроды
    PosZero = mystepper.currentPosition();
    
    mystepper.move(dir);
    mystepper.runToPosition();
  } else if (Vist > 1000 && Vgape < 20) {
    //напряжение есть. Но на зазоре короткое замыкание. Полный назад
    mystepper.setCurrentPosition(0);
    mystepper.move(-dir*1000);
    mystepper.runToPosition();
    mystepper.move(dir*950);
    mystepper.runToPosition();

  } else if (Vist > 1000 && Vgape < 200) {
    //идет процесс эрозии
    tm_curr = millis();
    unsigned long delta = tm_curr - tm;
    Serial.println("Delta " + String(delta) + " tm_curr " + tm_curr + " tm " + tm);
    if (delta >= 90000.00) {
      //долго стоим, назад надо отвести
          Serial.println("zzz");
         tm = tm_curr;
         mystepper.setCurrentPosition(0);
         mystepper.move(-dir*5000);
         mystepper.runToPosition();       
         mystepper.move(dir*4950);
         mystepper.runToPosition();       
     }
  } else {
    
  }
  
  
  //mystepper.moveTo(Vist);
  //mystepper.setSpeed(1000);
  //mystepper.runSpeedToPosition();
  
  //while (!mystepper.run());
  //while (!mystepper.runSpeedToPosition());

/*  
  mystepper.runToPosition(); 
  Serial.println(mystepper.distanceToGo());
 */
 
  delay(3000);
/*  
  while (mystepper.distanceToGo()!=0)
    {
      mystepper.run();
      //mystepper.runSpeedToPosition();
      Serial.println(mystepper.distanceToGo());
    }
 */    

}
