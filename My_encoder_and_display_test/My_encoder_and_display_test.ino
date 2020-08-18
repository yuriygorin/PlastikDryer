
#include <GyverTimer1.h>

#include "GyverEncoder.h"
#define CLK 8
#define DT 7
#define SW 6
Encoder enc1(CLK, DT, SW);  // для работы c кнопкой

// include the library code:
#include <LiquidCrystal.h>

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

#define DS_PIN 9  // пин датчика

// Целевая температура, будет зависеть от типа пластика примерно от 50 для ПЛА до 90 для нейлона
#define TargetTemperature 25 
// окно гистерезиса, чтоб не постоянно щелкать релюхой, а только иногда. Диапазон допустимых температур от TargetTemperature - TemperatureWindow до TargetTemperature + TemperatureWindow
#define TemperatureWindow 2
#define TemperatureMaxWorkRange 100
#define TemperatureMinWorkRange 5

//Defines for power Rele States
unsigned char PowerRele_State;
#define ON 8
#define OFF 9
#define PowerRele_pin 10

#define buzzer 13

const char *Menus[]  = {
  "PlasticDryer 02b",     // 01
  "yuriygorin@ya.ru",     // 02
  
  " Press and hold ",
  "encoder to start",
  
  " Click to setup ",
  " type and time  ",
  
  " Select plastik ",
  "PLA ABS PETG NYL",
  
  "Dry plastic for ",
  "HH hours MM min ",
  
  "  Waiting GGGoC ",
  "  Left    HH:MM ",
  
  "  Drying  GGGoC ",
  "  Left    HH:MM ",
  
  "    Success     ",
  "  Drying ended  ",
  
  " Press and hold ",
  "encoder to stop ",

  "TempSensor error",
  " Drying stoped  "

};

void setup() {
  Serial.begin(9600); // Настройка последовательного порта для отправки отладочной информации на ПК
  
  enc1.setType(TYPE2);    // Настройка Энкодера
  attachInterrupt(0, isrCLK, CHANGE);    // прерывание на 2 пине! CLK у энка
  attachInterrupt(1, isrDT,  CHANGE);    // прерывание на 3 пине! DT у энка

  lcd.begin(16, 2);       // Настройка дисплея
  lcd.noCursor();
  dallas_begin(DS_PIN); // инициализация  датчика температуры
  
  PowerRele_State = OFF;                //По подаче питания надо отключить реле.
  pinMode(PowerRele_pin, OUTPUT);       //
  digitalWrite(PowerRele_pin, HIGH);   // turn Rele Off

//  lcd.setCursor(0, 0);  
//  lcd.print("Rele Off        "); 

  lcd.setCursor(0, 0);  
  lcd.print(Menus[0]); 
  lcd.setCursor(0, 1);  
  lcd.print(Menus[1]); 
  


  pinMode(buzzer, OUTPUT); // Set buzzer - pin 13 as an output

  timer1_setFrequency(1000);    // ставим 10 герца
  timer1_ISR(handler_10Hz);    // подключить прерывание
  timer1_start();         // запустить таймер
}




char InSecondCounter = 0;
unsigned char LCDScreenIndex = 0;
float Current_tempr_value;
unsigned char Mode = 1;
unsigned char CurrentScreen = 0;
unsigned int InfoScreen_TimeOut = 0;

#define IntroMode     1
#define SelectMode    2
#define TimeSetMode   3
#define WaitMode      4
#define DryMode       5
#define DoneMode      6

#define Screen_1      0
#define Screen_2      1
#define Screen_3      2
#define Screen_4      3
#define Screen_5      4
#define Screen_6      5
#define Screen_7      6
#define Screen_8      7
#define Screen_9      8




void isrCLK() {
  enc1.tick();  // отработка в прерывании
}
void isrDT() {
  enc1.tick();  // отработка в прерывании
}

void handler_10Hz() {
  enc1.tick();     // отработка теперь находится здесь
  InSecondCounter++;
  switch (InSecondCounter) {
    case 1: 
    {
      dallas_requestTemp(DS_PIN); // запрос на начало изменения температуры
      if ( 0 == InfoScreen_TimeOut ) 
      { 
        if (IntroMode == Mode )
        {
          LCDScreenIndex = Screen_1;
        }      
      }
      else 
      {
         InfoScreen_TimeOut--;
      }
    }
    break;
    case 2: 
    {
      enc1.tick();     // отработка теперь находится здесь
      if (enc1.isRight())
      {
        Serial.println("Right");         // если был поворот
        switch (Mode) {
          case IntroMode: 
          {
            LCDScreenIndex = Screen_3;
            InfoScreen_TimeOut = 100; // 10 секундный таймер переустановлен 
          }
          break;
          case SelectMode: 
          {
            
          }
          break;
          case TimeSetMode: 
          {
            
          }
          break;
          case WaitMode: 
          case DryMode: 
          case DoneMode: 
          {
            
          }
          break;                                                       
        }
      }
      if (enc1.isLeft())
      {
        Serial.println("Left");         // если был поворот
        switch (Mode) {
          case IntroMode: 
          {
            LCDScreenIndex = Screen_2;
            InfoScreen_TimeOut = 100; // 10 секундный таймер переустановлен             
          }
          break;
          case SelectMode: 
          {
            
          }
          break;
          case TimeSetMode: 
          {
            
          }
          break;
          case WaitMode: 
          case DryMode: 
          case DoneMode: 
          {
            
          }
          break;                                                       
        }
      }
      if (enc1.isHolded())
      {
        Serial.println("Hold");         
        switch (Mode) {
          case IntroMode: 
          {
            Mode = WaitMode;
            LCDScreenIndex = Screen_6;

          }
          break;
          case SelectMode: 
          {
            
          }
          break;
          case TimeSetMode: 
          {
            
          }
          break;
          case WaitMode: 
          case DryMode: 
          case DoneMode: 
          {
            
          }
          break;                                                       
        }
      }      
      if (enc1.isClick())
      {
        Serial.println("Click");   
        switch (Mode) {
          case IntroMode: 
          {
            Mode = SelectMode;
            LCDScreenIndex = Screen_4;
          }
          break;
          case SelectMode: 
          {
            
          }
          break;
          case TimeSetMode: 
          {
            
          }
          break;
          case WaitMode: 
          case DryMode: 
          case DoneMode: 
          {
            
          }
          break;                                                       
        }
      } 
    }
      break;
    case 3: 
      break;
    case 4: 
      break;
    case 5: 
    {
      Current_tempr_value = dallas_getTemp(DS_PIN);
      if (  ( Current_tempr_value > TemperatureMaxWorkRange ) || (Current_tempr_value  < TemperatureMinWorkRange)  ) 
      {
        digitalWrite(PowerRele_pin, HIGH);   // turn Rele Off
        PowerRele_State = OFF;
        // todo Go to sensor Error Mode
      } 
      else if ( Mode == WaitMode ) 
      {        
        // todo Update temperature value on LCD string       
      }      
      else if ( Mode == DryMode ) 
      { 
        if ( (Current_tempr_value >= TargetTemperature + TemperatureWindow) &&  ( ON == PowerRele_State)   )
        {
          PowerRele_State = OFF;
          digitalWrite(PowerRele_pin, HIGH);   // turn Rele Off
        }
        else if ( (Current_tempr_value < TargetTemperature - TemperatureWindow) &&  ( OFF == PowerRele_State)   )
        {
          PowerRele_State = ON;
          digitalWrite(PowerRele_pin, LOW);   // turn Rele ON 
        }
        //todo Update temperature value on LCD string
      }
    }      
      break;
    case 6: 
      break;
    case 7: 
      break; 
    case 8: 
      break; 
    case 9: 
    {
      lcd.setCursor(0, 0);  
      lcd.print(Menus[LCDScreenIndex*2]); 
      lcd.setCursor(0, 1);  
      lcd.print(Menus[LCDScreenIndex*2+1]);
    }
      break;                             
    case 10:       
      InSecondCounter = 0;          
      break;
  }
}

void loop() {
  enc1.tick();     // отработка теперь находится здесь
//  if (enc1.isRight()) Serial.println("Right");         // если был поворот
//  if (enc1.isLeft()) Serial.println("Left");
//  if (enc1.isHolded()) Serial.println("Holded");       // если была удержана и энк не поворачивался
//  if (enc1.isClick()) Serial.println("Click");         // одиночный клик
}
/*
 void loop() 
{

  dallas_requestTemp(DS_PIN); // запрос
  delay(900);
  tone(buzzer, 1000,100);
  float Current_tempr_value = dallas_getTemp(DS_PIN);
    
  if (  ( Current_tempr_value > TemperatureMaxWorkRange ) || (Current_tempr_value  < TemperatureMinWorkRange)  ) 
  {
    lcd.setCursor(0, 0);  
    lcd.print("TempSensor error");    
    digitalWrite(PowerRele_pin, HIGH);   // turn Rele Off
    PowerRele_State = OFF;
  }  
  else if ( (Current_tempr_value >= TargetTemperature + TemperatureWindow) &&  ( ON == PowerRele_State)   )
  {
    PowerRele_State = OFF;
    digitalWrite(PowerRele_pin, HIGH);   // turn Rele Off
    lcd.setCursor(0, 0);  
    lcd.print("Rele Off        ");   
  }
  else if ( (Current_tempr_value < TargetTemperature - TemperatureWindow) &&  ( OFF == PowerRele_State)   )
  {
    PowerRele_State = ON;
    digitalWrite(PowerRele_pin, LOW);   // turn Rele ON
    lcd.setCursor(0, 0);  
    lcd.print("Rele ON         ");  
  }

  lcd.setCursor(0, 1);  
  lcd.print(TargetTemperature - TemperatureWindow);
  lcd.print("C  ");    
  lcd.print(Current_tempr_value);
  lcd.print("C  "); 
  lcd.print(TargetTemperature + TemperatureWindow);  
  lcd.print("C");
}

*/
//Далее следует кусок чужого кода, который заявлен как самый легкий и быстрый для опроа датчика температуры
// ======= dallas =======
void dallas_begin(uint8_t pin) {
  pinMode(pin, INPUT);
  digitalWrite(pin, LOW);
}
void dallas_requestTemp(uint8_t pin) {
  if (oneWire_reset(pin)) return;
  oneWire_write(0xCC, pin);
  oneWire_write(0x44, pin);
}
float dallas_getTemp(uint8_t pin) {
  uint8_t data[2];
  if (oneWire_reset(pin)) return;
  oneWire_write(0xCC, pin);
  oneWire_write(0xBE, pin);
  data[0] = oneWire_read(pin);
  data[1] = oneWire_read(pin);
  float result = (float)((data[1] << 8) | data[0]) * 0.0625; //>
  return result;
}
// ======= 1wire =======
boolean oneWire_reset(byte pin) {
  pinMode(pin, 1);
  delayMicroseconds(640);
  pinMode(pin, 0);
  delayMicroseconds(2);
  for (uint8_t c = 80; c; c--) {
    if (!digitalRead(pin)) {
      uint32_t tmr = micros();
      while (!digitalRead(pin)) {
        if (micros() - tmr > 200) return false;
      }
      return false;
    }
    delayMicroseconds(1);
  }
  return true;
}
void oneWire_write(uint8_t data, byte pin) {
  for (uint8_t p = 8; p; p--) {
    pinMode(pin, 1);
    if (data & 1) {
      delayMicroseconds(5);
      pinMode(pin, 0);
      delayMicroseconds(90);
    } else {
      delayMicroseconds(90);
      pinMode(pin, 0);
      delayMicroseconds(5);
    }
    data >>= 1;
  }
}
uint8_t oneWire_read(byte pin) {
  uint8_t data = 0;
  for (uint8_t p = 8; p; p--) {
    data >>= 1;
    pinMode(pin, 1);
    delayMicroseconds(2);
    pinMode(pin, 0);
    delayMicroseconds(8);
    bool dataBit = digitalRead(pin);
    delayMicroseconds(80);
    if (dataBit) data |= 0x80;
  }
  return data;
}
