// программа управления шаговым двигателем с помощью библиотеки StepDirDriver
// двигатель делает 2 оборота по часовой срелке , пауза 1 сек, 
// 2 оборота против часовой стрелки, пауза 1 сек, и так в бесконечном цикле
#include <TimerOne.h>
#include <StepDirDriver.h>

StepDirDriver myMotor(9, 8, 12);  // создаем объект типа StepDirDriver, задаем выводы для сигналов

unsigned int timeCounter; // счетчик времени
byte md;  // режим: 0 - вращение против ч.с., 1 - пауза, 2 - вращение против ч.с., 3 - пауза

void setup() {
  Timer1.initialize(250);  // инициализация таймера 1, период 250 мкс
  Timer1.attachInterrupt(timerInterrupt, 250);  // задаем обработчик прерываний
  myMotor.setMode(0, false);  // шаговый режим, без фиксации при остановке
  myMotor.setDivider(5);     // делитель частоты 10 (1 оборот в сек) чем меньше, тем выше скорость.
  md= 0;  // начальный режим
  myMotor.step(0);  // начальный запуск
}

void loop() {

  // управление вращением двигателя
  if (md == 0)  {
    // 2 оборота по часовой стрелке
    if (myMotor.readSteps() == 0) { md=1; timeCounter=0; }
  }
  else if (md == 1) {
    // пауза 1 сек
    if (timeCounter >= 1000)  { md=2; myMotor.step(-800); }
  }
  else if (md == 2) {
    // 2 оборота против часовой стрелки
    if (myMotor.readSteps() == 0) { md=3; timeCounter=0; }    
  }
  else {
    // пауза 1 сек
    if (timeCounter >= 1000)  { md=0; myMotor.step(800); }    
  }
}

//-------------------------------------- обработчик прерывания 0,25 мс
void  timerInterrupt() {
  myMotor.control(); // управвление двигателем
  timeCounter++; // счетчик времени
}
