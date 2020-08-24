// нехватает конденсаторов для корректной работы энкодера, надо на каждый пин по 0.1 и на вход 10мкФ

#include <GyverTimer1.h>
#include <GyverEncoder.h>

#define CLK 2
#define DT 3
#define SW 4
Encoder enc1(CLK, DT, SW);  // для работы c кнопкой



// include the library code:
#include <LiquidCrystal.h>

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 12, en = 11, d4 = 5, d5 = 8, d6 = 7, d7 = 6;
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
const char *PlasticSelect_Menu [] ={
  "PLA abs petg nyl",
  "pla ABS petg nyl",
  "pla abs PETG nyl",
  "pla abs petg NYL"
};


const char *Menus[]  = {
  "PlasticDryer 02b",     // 01
  "yuriygorin@ya.ru",     
  
  " Press and hold ",     // 02
  "encoder to start",
  
  " Click to setup ",     // 03
  " type and time  ",
  
  " Select plastik ",     // 04
  "PLA ABS PETG NYL",
  
  "Dry plastic for ",     // 05
  "HH hours MM min ",
  
  "  Waiting GGGoC ",     // 06
  "  Left    HH:MM ",
  
  "  Drying  GGGoC ",     // 07
  "  Left    HH:MM ",
  
  "    Success     ",     // 08
  "  Drying ended  ",
  
  "  Click to next ",     // 09
  "  Hold to start ",
  
  " Press and hold ",     // 10
  "encoder to stop ",

  "TempSensor error",     // 11
  " Drying stoped  "

};

void setup() {
  Serial.begin(9600); // Настройка последовательного порта для отправки отладочной информации на ПК
  
  enc1.setType(TYPE2);    // Настройка Энкодера
  enc1.setPinMode(LOW_PULL);
//  attachInterrupt(0, isrCLK, CHANGE);    // прерывание на 2 пине! CLK у энка
//  attachInterrupt(1, isrDT,  CHANGE);    // прерывание на 3 пине! DT у энка

//  // настроить PCINT
  attachPCINT(CLK);
  attachPCINT(DT);

  lcd.begin(16, 2);       // Настройка дисплея
  lcd.noCursor();
  dallas_begin(DS_PIN); // инициализация  датчика температуры
  
  PowerRele_State = OFF;                //По подаче питания надо отключить реле.
  pinMode(PowerRele_pin, OUTPUT);       //
  digitalWrite(PowerRele_pin, HIGH);   // turn Rele Off

  lcd.setCursor(0, 0);  
  lcd.print(Menus[0]); 
  lcd.setCursor(0, 1);  
  lcd.print(Menus[1]); 
  pinMode(buzzer, OUTPUT); // Set buzzer - pin 13 as an output

  timer1_setFrequency(10);    // ставим 10 герца
  timer1_ISR(handler_10Hz);    // подключить прерывание
  timer1_start();         // запустить таймер
  dallas_requestTemp(DS_PIN); // запрос на начало изменения температуры

}




unsigned char LCDScreenIndex = 0;
float Current_tempr_value;
unsigned char Mode = 1;
unsigned char CurrentScreen;
unsigned int InfoScreen_TimeOut = 0;
#define InfoScreen_TimeOut_recharge  10
unsigned int TimeBeforeHint=0;
#define TimeBeforeHint_max   20

#define IntroMode         1
#define SelectMode        2
#define TimeSetMode       3
#define WaitMode          4
#define DryMode           5
#define DoneMode          6
#define Intro_Hint    7
#define Select_Hint   8
#define TimeSet_Hint  9
#define Wait_Hint     10
#define Dry_Hint      11
#define Done_Hint     12


#define Screen_1      0
#define Screen_2      1
#define Screen_3      2
#define Screen_4      3
#define Screen_5      4
#define Screen_6      5
#define Screen_7      6
#define Screen_8      7
#define Screen_9      8
#define Screen_10     9

unsigned char PlasticType_index = 0;
#define PlasticType_index_max  3
#define PlasticType_index_min  0

unsigned int WaitTime_5min_step = 0;
#define WaitTime_5min_step_Max 288
unsigned int    TotalTime; 
const unsigned char DryTime[4] = {10, 20, 30, 40};
const unsigned char DryTemp[4] = {50, 60, 70, 80};
unsigned long WaitTime_01sec_step=0, DryTime_01sec_step = 0;

         
unsigned char       Hours,Minutes, Seconds;
volatile unsigned int TimerUpdated = false, InSecondCounter;

void handler_10Hz() {
  InSecondCounter++;
  TimerUpdated = true;
}

#define Time4CountTime     1
#define Time4TempMeasure   2
#define Time4LCDUpdate     5
#define Time4ResetTimerCounter     10

void loop() {
  enc1.tick();     // отработка теперь находится здесь
  if (enc1.isRight()) {
    switch (Mode) {
      case IntroMode: {
        LCDScreenIndex = Screen_3;
        InfoScreen_TimeOut = InfoScreen_TimeOut_recharge; // 10 секундный таймер переустановлен 
      }
      break;
      case SelectMode: {
        if ( PlasticType_index < PlasticType_index_max )
          PlasticType_index++;
        TimeBeforeHint = 0;
      }
      break;
      case TimeSetMode: {
        if ( WaitTime_5min_step < WaitTime_5min_step_Max )
          WaitTime_5min_step++;
        TimeBeforeHint=0;
      }
      break;
      case WaitMode: 
      case DryMode: 
      case DoneMode: {
        Mode = Dry_Hint;
        LCDScreenIndex = Screen_10;
        InfoScreen_TimeOut = InfoScreen_TimeOut_recharge;                                 
      }
      break;   
      case Intro_Hint:{
        //nothing to do
      }
      break; 
      case Select_Hint:{
        Mode = SelectMode;
        LCDScreenIndex = Screen_4;
      }
      break;
      case TimeSet_Hint:{
        Mode = TimeSetMode;
        LCDScreenIndex = Screen_5;            
      }
      break; 
      case Wait_Hint:{
        //nothing to do            
      }
      break;                                                                                                                       
      case Dry_Hint:{
        //nothing to do            
      }
      break;                                                                                                                       
      case Done_Hint:{
        //nothing to do            
      }
      break;                                                                                                                      
    }
  }
  if (enc1.isLeft()) {
    switch (Mode) {
      case IntroMode: {
        LCDScreenIndex = Screen_2;
        InfoScreen_TimeOut = InfoScreen_TimeOut_recharge;             
      }
      break;
      case SelectMode: {
        if ( PlasticType_index > PlasticType_index_min )
          PlasticType_index--;
        TimeBeforeHint=0;
      }
      break;
      case TimeSetMode:{
        if ( WaitTime_5min_step != 0 )
          WaitTime_5min_step--;
        TimeBeforeHint=0;
      }
      break;
      case WaitMode: 
      case DryMode: 
      case DoneMode: {
        Mode = Dry_Hint;
        LCDScreenIndex = Screen_10;
        InfoScreen_TimeOut = InfoScreen_TimeOut_recharge;                         
      }
      break;   
      case Intro_Hint:{
        //nothing to do
      }
      break; 
      case Select_Hint:{
        Mode = SelectMode;
        LCDScreenIndex = Screen_4;
      }
      break;
      case TimeSet_Hint:{
        Mode = TimeSetMode;
        LCDScreenIndex = Screen_5;            
      }
      break; 
      case Wait_Hint:{
        //nothing to do            
      }
      break;                                                                                                                       
      case Dry_Hint:{
        //nothing to do            
      }
      break;                                                                                                                       
      case Done_Hint:{
        //nothing to do            
      }
      break;                                                                                                                       
    }
  }
  if (enc1.isHolded()) {
    switch (Mode) {
      case IntroMode:{
        Mode = WaitMode;
        LCDScreenIndex = Screen_6;
      }
      break;
      case SelectMode: {
        Mode = WaitMode;
        LCDScreenIndex = Screen_6;
        TimeBeforeHint=0;            
      }
      break;
      case TimeSetMode:{
        Mode = WaitMode;
        LCDScreenIndex = Screen_6;
        TimeBeforeHint=0;                        
      }
      break;
      case WaitMode: 
      case DryMode: 
      case DoneMode:{
        Mode = SelectMode; 
        LCDScreenIndex = Screen_4;                                            
      }
      break; 
      case Intro_Hint:
      case Select_Hint:
      case TimeSet_Hint:
      {
        Mode = WaitMode;
        LCDScreenIndex = Screen_6;             
      }
      break; 
      case Wait_Hint:          
      case Dry_Hint:
      case Done_Hint:{
        Mode = SelectMode;
        LCDScreenIndex = Screen_4;            
      }
      break;                                                                                                                      
    }
  }      
  if (enc1.isClick()) {
    switch (Mode) {
      case IntroMode: {
        Mode = SelectMode;
        LCDScreenIndex = Screen_4;
      }
      break;
      case SelectMode: {
        Mode = TimeSetMode;                        
        LCDScreenIndex = Screen_5;
        TimeBeforeHint=0;
      }
      break;
      case TimeSetMode: {
        Mode = SelectMode;                        
        LCDScreenIndex = Screen_4;
        TimeBeforeHint=0;
      }
      break;
      case WaitMode: 
      case DryMode: 
      case DoneMode: {
        Mode = Dry_Hint;
        LCDScreenIndex = Screen_10;
        InfoScreen_TimeOut = InfoScreen_TimeOut_recharge;                                 
      }
      break;   
      case Intro_Hint:{
        //nothing to do
      }
      break; 
      case Select_Hint:{
        Mode = SelectMode;
        LCDScreenIndex = Screen_4;
      }
      break;
      case TimeSet_Hint:{
        Mode = TimeSetMode;
        LCDScreenIndex = Screen_5;            
      }
      break; 
      case Wait_Hint:{
        //nothing to do            
      }
      break;                                                                                                                       
      case Dry_Hint:{
        //nothing to do            
      }
      break;                                                                                                                       
      case Done_Hint:{
        //nothing to do            
      }
      break;                                                                                                  
    }
  } 
  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  if (true == TimerUpdated){    
    TimerUpdated = false;
    if ( Mode == SelectMode )  {
      lcd.setCursor(0, 0);  
      lcd.print(Menus[Screen_4*2]);
      enc1.tick();     // отработка теперь находится здесь 
      lcd.setCursor(0, 1);  
      lcd.print(PlasticSelect_Menu[PlasticType_index]);
      enc1.tick();     // отработка теперь находится здесь               
    }
    else if ( Mode == TimeSetMode ) {
      lcd.setCursor(0, 0);  
      lcd.print(Menus[LCDScreenIndex*2]);
      lcd.setCursor(0, 1);
      TotalTime =  WaitTime_5min_step * 5 + DryTime[PlasticType_index];       
      Hours   = TotalTime/60;
      Minutes = TotalTime - Hours * 60;
      if (Hours < 10 )
        lcd.print(" ");
      lcd.print(Hours);
      lcd.print(" hours ");
      if (Minutes < 10 )
        lcd.print(" ");
      lcd.print(Minutes);
      lcd.print(" min "); 
      WaitTime_01sec_step =  WaitTime_5min_step * 5 * 600;                               
    }
    else if (Mode == WaitMode){
      if (WaitTime_01sec_step == 0) {
        Mode = DryMode;
        DryTime_01sec_step = DryTime[PlasticType_index] * 600;
      }
      else {
        WaitTime_01sec_step--;
        lcd.setCursor(0, 0);  
        lcd.print("  Waiting  ");
        if (Current_tempr_value < 10 )
          lcd.print(" ");
        lcd.print(Current_tempr_value);        
        lcd.print("oC ");    
        lcd.setCursor(0, 1);
        lcd.print("  Left  ");
        Hours   =  WaitTime_01sec_step / 36000;
        Minutes = (WaitTime_01sec_step - Hours * 3600) /600; 
        Seconds = (WaitTime_01sec_step - Hours * 3600 - Minutes * 600 ) / 10;
        if (Hours < 10 )
          lcd.print(" ");
        lcd.print(Hours);
        lcd.print(":");
        if (Minutes < 10 )
          lcd.print("0");
        lcd.print(Minutes);  
        lcd.print(":");
        if (Seconds < 10 )
          lcd.print("0");
        lcd.print(Seconds);       
      }                
    }
    else if (Mode == DryMode){
      if (DryTime_01sec_step == 0) {
        Mode = DoneMode;
        LCDScreenIndex = Screen_8;
      }
      else {
        DryTime_01sec_step--;
        lcd.setCursor(0, 0);  
        lcd.print("  Drying  ");
        if (Current_tempr_value < 10 )
          lcd.print(" ");
        lcd.print(Current_tempr_value);        
        lcd.print("oC ");    
        lcd.setCursor(0, 1);
        lcd.print("  Left  ");
        Hours   =  DryTime_01sec_step / 36000;
        Minutes = (DryTime_01sec_step - Hours * 3600) /600; 
        Seconds = (DryTime_01sec_step - Hours * 3600 - Minutes * 600 ) / 10;
        if (Hours < 10 )
          lcd.print(" ");
        lcd.print(Hours);
        lcd.print(":");
        if (Minutes < 10 )
          lcd.print("0");
        lcd.print(Minutes);  
        lcd.print(":");
        if (Seconds < 10 )
          lcd.print("0");
        lcd.print(Seconds);    
      }                   
    }    
    else {
      lcd.setCursor(0, 0);  
      lcd.print(Menus[LCDScreenIndex*2]);
      enc1.tick(); 
      lcd.setCursor(0, 1);  
      lcd.print(Menus[LCDScreenIndex*2+1]);
      enc1.tick();
    }
            
    switch (InSecondCounter) {
      case Time4CountTime: {
        if ( 0 == InfoScreen_TimeOut ) { 
          if      (IntroMode == Mode ) { LCDScreenIndex = Screen_1; }
          else if (Wait_Hint == Mode ) { LCDScreenIndex = Screen_6; Mode = WaitMode; }
          else if (Dry_Hint  == Mode ) { LCDScreenIndex = Screen_7; Mode = DryMode;  }
          else if (Done_Hint == Mode ) { LCDScreenIndex = Screen_8; Mode = DoneMode; }             
        }
        else {
           InfoScreen_TimeOut--;
        }
        if (Mode == SelectMode) {
          TimeBeforeHint++;
          if (TimeBeforeHint >= TimeBeforeHint_max)   {
            TimeBeforeHint =0;
            Mode = Select_Hint;
            LCDScreenIndex = Screen_9;       
          }
        }
        if (Mode == TimeSetMode) {
          TimeBeforeHint++;
          if (TimeBeforeHint >= TimeBeforeHint_max)   {
            TimeBeforeHint = 0;
            Mode = TimeSet_Hint;
            LCDScreenIndex = Screen_9;       
          }
        }          
      }
      break;
      case Time4TempMeasure: {
        Current_tempr_value = dallas_getTemp(DS_PIN);
        dallas_requestTemp(DS_PIN); // запрос на начало изменения температуры
        if (  ( Current_tempr_value > TemperatureMaxWorkRange ) || (Current_tempr_value  < TemperatureMinWorkRange)  ) {
          digitalWrite(PowerRele_pin, HIGH);   // turn Rele Off
          PowerRele_State = OFF;
          // todo Go to sensor Error Mode
        }       
        else if ( Mode == DryMode )  { 
          if ( (Current_tempr_value >= DryTemp[PlasticType_index] + TemperatureWindow) &&  ( ON == PowerRele_State)   ) {
            PowerRele_State = OFF;
            digitalWrite(PowerRele_pin, HIGH);   // turn Rele Off
          }
          else if ( (Current_tempr_value < DryTemp[PlasticType_index] - TemperatureWindow) &&  ( OFF == PowerRele_State) ) {
            PowerRele_State = ON;
            digitalWrite(PowerRele_pin, LOW);   // turn Rele ON 
          }
        }
        else {        
          PowerRele_State = OFF;
          digitalWrite(PowerRele_pin, HIGH);   // turn Rele Off       
        }
      }      
      break;                           
      case Time4ResetTimerCounter: 
        InSecondCounter = 0;          
      break;
    }
  }
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


uint8_t tochki[8] = {B0, B00000, B0, B0, B0, B0, B10101};
uint8_t bukva_P[8] = {0x1F, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11};
uint8_t bukva_Ya[8] = {B01111, B10001, B10001, B01111, B00101, B01001, B10001};
uint8_t bukva_L[8] = {0x3, 0x7, 0x5, 0x5, 0xD, 0x9, 0x19};
uint8_t bukva_Lm[8] = {0, 0, B01111, B00101, B00101, B10101, B01001};
uint8_t bukva_Mz[8] = {0x10, 0x10, 0x10, 0x1E, 0x11, 0x11, 0x1E};
uint8_t bukva_I[8] = {0x11, 0x13, 0x13, 0x15, 0x19, 0x19, 0x11};
uint8_t bukva_D[8] = {B01111, B00101, B00101, B01001, B10001, B11111, 0x11};
uint8_t bukva_G[8] = {B11111, B10001, B10000, B10000, B10000, B10000, B10000};
uint8_t bukva_IY[8] = {B01110, B00000, B10001, B10011, B10101, B11001, B10001};
uint8_t bukva_Z[8] = {B01110, B10001, B00001, B00010, B00001, B10001, B01110};
uint8_t bukva_ZH[8] = {B10101, B10101, B10101, B11111, B10101, B10101, B10101};
uint8_t bukva_Y[8] = {B10001, B10001, B10001, B01010, B00100, B01000, B10000};
uint8_t bukva_B[8] = {B11110, B10000, B10000, B11110, B10001, B10001, B11110};
uint8_t bukva_CH[8] = {B10001, B10001, B10001, B01111, B00001, B00001, B00001};
uint8_t bukva_IYI[8] = {B10001, B10001, B10001, B11001, B10101, B10101, B11001};
uint8_t bukva_TS[8] = {B10010, B10010, B10010, B10010, B10010, B10010, B11111, B00001};


  // create a new character
  lcd.createChar(0, heart);
  // create a new character
  lcd.createChar(1, smiley);
  // create a new character
  lcd.createChar(2, frownie);
  // create a new character
  lcd.createChar(3, armsDown);
  // create a new character
  lcd.createChar(4, armsUp);

  


*/
//
//// функция для настройки PCINT для ATmega328 (UNO, Nano, Pro Mini)
uint8_t attachPCINT(uint8_t pin) {
  if (pin < 8) { // D0-D7 // PCINT2
    PCICR |= (1 << PCIE2);
    PCMSK2 |= (1 << pin);
    return 2;
  }
  else if (pin > 13) { //A0-A5  // PCINT1
    PCICR |= (1 << PCIE1);
    PCMSK1 |= (1 << pin - 14);
    return 1;
  }
  else { // D8-D13  // PCINT0
    PCICR |= (1 << PCIE0);
    PCMSK0 |= (1 << pin - 8);
    return 0;
  }
}
//
//// Векторы PCINT, нужно кинуть сюда тики
//// не обязательно в каждый вектор, достаточно в тот, который задействован
//// пины 0-7: PCINT2
//// пины 8-13: PCINT0
//// пины A0-A5: PCINT1
ISR(PCINT0_vect) {
  enc1.tick();  
}
ISR(PCINT1_vect) {
//  enc1.tick();
}
ISR(PCINT2_vect) {
//  enc1.tick();
}





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
