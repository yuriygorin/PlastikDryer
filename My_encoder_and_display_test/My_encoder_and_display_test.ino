#define CLK 8
#define DT 7
#define SW 6

#include "GyverEncoder.h"
//Encoder enc1(CLK, DT);      // для работы без кнопки
Encoder enc1(CLK, DT, SW);  // для работы c кнопкой
//Encoder enc1(CLK, DT, SW, TYPE2);  // для работы c кнопкой и сразу выбираем тип
//Encoder enc1(CLK, DT, ENC_NO_BUTTON, TYPE2);  // для работы без кнопки и сразу выбираем тип

// Варианты инициализации:
// Encoder enc;									// не привязан к пину
// Encoder enc(пин CLK, пин DT);				// энкодер без кнопки (ускоренный опрос)
// Encoder enc(пин CLK, пин DT, пин SW);		// энкодер с кнопкой
// Encoder enc(пин CLK, пин DT, пин SW, тип);	// энкодер с кнопкой и указанием типа
// Encoder enc(пин CLK, пин DT, ENC_NO_BUTTON, тип);	// энкодер без кнопкой и с указанием типа


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


void setup() {
  Serial.begin(9600);
  enc1.setType(TYPE2);
  lcd.begin(16, 2);
  lcd.noCursor();
  dallas_begin(DS_PIN); // инициализация  
  PowerRele_State = OFF;
  digitalWrite(PowerRele_pin, LOW);   // turn Rele Off
  lcd.setCursor(0, 0);  
  lcd.print("Rele Off        "); 
}

void loop() {
  
  dallas_requestTemp(DS_PIN); // запрос
  delay(1000);
  float Current_tempr_value = dallas_getTemp(DS_PIN);
  Serial.println(Current_tempr_value); // получаем температуру
    
  if (  ( Current_tempr_value > TemperatureMaxWorkRange ) || (Current_tempr_value  < TemperatureMinWorkRange)  ) 
  {
    Serial.println("TempSensor error - out of range"); 
    lcd.setCursor(0, 0);  
    lcd.print("TempSensor error");    
    digitalWrite(PowerRele_pin, LOW);   // turn Rele Off
    PowerRele_State = OFF;
  }
  
  else if ( (Current_tempr_value >= TargetTemperature + TemperatureWindow) &&  ( ON == PowerRele_State)   )
  {
    PowerRele_State = OFF;
    digitalWrite(PowerRele_pin, LOW);   // turn Rele Off
    lcd.setCursor(0, 0);  
    lcd.print("Rele Off        ");   
  }
  else if ( (Current_tempr_value < TargetTemperature - TemperatureWindow) &&  ( OFF == PowerRele_State)   )
  {
    PowerRele_State = ON;
    digitalWrite(PowerRele_pin, HIGH);   // turn Rele Off
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
