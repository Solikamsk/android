//программа STEP/DIR драйвера шагового двигателем с управлением от компьютера
 
#include <TimerOne.h>
#include <StepDirDriver.h>

#define TIME_OUT 120 // время тайм-аута между командами (* 0,25 мс)

StepDirDriver myMotor(10, 11, 12);  // создаем объект типа StepDirDriver, задаем выводы для сигналов

byte  timeOutCount; // счетчик времени между приемом данных
byte  prevDataNum;  // число данных порта в предыдущем цикле
byte  letterNum;    // число принятых данных
char  textBuf[10]; // тестовый буфер
int i, s, t;

void setup() {
  Timer1.initialize(250);  // инициализация таймера 1, период 250 мкс
  Timer1.attachInterrupt(timerInterrupt, 250);  // задаем обработчик прерываний
  myMotor.setMode(0, false);  // шаговый режим, без фиксации при остановке
  myMotor.setDivider(83);     // делитель частоты 83 
  Serial.begin(9600); // инициализируем порт, скорость 9600
  pinMode(13, OUTPUT);  // разрешаем работу светодиода на плате        
}

void loop() {
//  myMotor.step(1000);

// обмен с компьютером
  letterNum= Serial.available(); // чтение числа принятых символов

  if ( letterNum == 0 ) {
    // данных нет
    timeOutCount= 0;
  }
  else  {  
    // есть данные    
    if ( letterNum != prevDataNum ) timeOutCount= 0;  // новое данное    
    prevDataNum= letterNum; // перегрузка числа принятых символов
  }

  if ( timeOutCount > TIME_OUT )  {
    // пауза между данными больше тайм-аута
    
    // команда принята, расшифровка
      while (true) {

        // прием префикса AT
        if ( Serial.read() != 'A') break; // ошибка
        if ( Serial.read() != 'T') break; // ошибка
        // принято "AT"
        s= Serial.read(); // третий символ команды

        //------------------------- команда AT ("AT", 13, 10)
        if ( s == 13 ) {
          if ( Serial.read() != 10 ) break; // ошибка
          // принято 13, 10

          // передача ответа
          Serial.print("OK\r\n");
          break;          
        }
        
        //------------------------- команда ATS= steps  ( "ATS=", steps,13, 10 )
        else if (s == 'S') {
          if ( Serial.read() != '=') break; // ошибка

          // чтение steps в textBuf
          i= 0; while (i < 8) {
            textBuf[i] = Serial.read();
            if ( textBuf[i] == 13 ) break; // конец команды
            // проверка символов на цифры
            if ( ((textBuf[i] < '0') || (textBuf[i] > '9')) && ((textBuf[i] != '-') && (textBuf[i] != ' ')) ) break; 
            i++;
          }

          if ( (textBuf[i] != 13) || (i > 7) ) break; // ошибка  
          textBuf[i]= 0;  // признак конца строки
          // steps находится textBuf

          // проверка окончания команды
          if ( Serial.read() != 10 ) break; // ошибка
          // принято 13, 10

          myMotor.step(atoi(textBuf));  // преобразование в int и запуск шагов
 
          // передача ответа
          Serial.print("OK\r\n");
          break;                                                  
        }
        
        //------------------------- команда ATM= mode, fix  ( "ATM=", mode, fix, 13, 10 )
        else if (s == 'M') {
          if ( Serial.read() != '=') break; // ошибка

          // чтение mode в textBuf
          textBuf[0]= Serial.read();
          if ( (textBuf[0] < '0') || (textBuf[0] > '9') ) break; // ошибка
          textBuf[1]= 0; // признак конца строки
          if ( Serial.read() != ',' ) break; // ошибка, нет запятой

          // чтение fix в textBuf
          textBuf[2]= Serial.read();
          if ( (textBuf[2] < '0') || (textBuf[2] > '9') ) break; // ошибка
          textBuf[3]= 0; // признак конца строки

          // mode и fix в textBuf

          // проверка окончания команды
          if ( Serial.read() != 13 ) break; // ошибка          
          if ( Serial.read() != 10 ) break; // ошибка
          // принято 13, 10

          myMotor.setMode(atoi(textBuf), atoi(textBuf+2)); // преобразование в int и установка режима

          // передача ответа
          Serial.print("OK\r\n");
          break;                                                                        
        }

        //------------------------- команда ATD= divider  ( "ATD=", divider,13, 10 )
        else if (s == 'D') {
          if ( Serial.read() != '=') break; // ошибка

          // чтение divider в textBuf
          i= 0; while (i < 8) {
            textBuf[i] = Serial.read();
            if ( textBuf[i] == 13 ) break; // конец команды
            // проверка символов на цифры
            if ( ((textBuf[i] < '0') || (textBuf[i] > '9')) && ((textBuf[i] != '-') && (textBuf[i] != ' ')) ) break; 
            i++;
          }

          if ( (textBuf[i] != 13) || (i > 7) ) break; // ошибка  
          textBuf[i]= 0;  // признак конца строки
          // steps находится textBuf

          // проверка окончания команды
          if ( Serial.read() != 10 ) break; // ошибка
          // принято 13, 10

          myMotor.setDivider(atoi(textBuf));  // преобразование в int и установка делителя

          // передача ответа
          Serial.print("OK\r\n");
          break;                                                  
        }

        //------------------------- команда ATR? ( "ATR?", 13, 10 )
        else if (s == 'R') {
          if ( Serial.read() != '?') break; // ошибка
          if ( Serial.read() != 13 ) break; // ошибка
          if ( Serial.read() != 10 ) break; // ошибка
          // принято 13, 10

          // передача ответа
          Serial.print(myMotor.readSteps()); // передача числа шагов
          Serial.print("OK\r\n");
          break;                                                  
        }
        
      break;  
      }
      // сброс порта
      timeOutCount= 0;
      while (true) { if (Serial.read() == 0xffff) break;}
  }            
}

//-------------------------------------- обработчик прерывания 250 мкс
void  timerInterrupt() {
  myMotor.control(); // управвление двигателем
  timeOutCount++; // счетчик времени между приемом данных       
}
