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


void setup() {
  Serial.begin(9600);
  enc1.setType(TYPE2);
  lcd.begin(16, 2);
  dallas_begin(DS_PIN); // инициализация  

}

void loop() {

  dallas_requestTemp(DS_PIN); // запрос
//  delay(1000);
  
	// обязательная функция отработки. Должна постоянно опрашиваться
  enc1.tick();
  
  if (enc1.isTurn()) {     // если был совершён поворот (индикатор поворота в любую сторону)
    // ваш код
  }
  
  if (enc1.isRight()) { Serial.println("Right");     lcd.print("Right"); }     // если был поворот
  if (enc1.isLeft())  { Serial.println("Left");      lcd.print("Left"); }
  
  if (enc1.isRightH()) Serial.println("Right holded"); // если было удержание + поворот
  if (enc1.isLeftH())  Serial.println("Left holded");
  
  //if (enc1.isPress()) Serial.println("Press");         // нажатие на кнопку (+ дебаунс)
  //if (enc1.isRelease()) Serial.println("Release");     // то же самое, что isClick
  
  if (enc1.isClick()) Serial.println("Click");         // одиночный клик
  if (enc1.isSingle()) Serial.println("Single");       // одиночный клик (с таймаутом для двойного)
  if (enc1.isDouble()) Serial.println("Double");       // двойной клик
  
  
  if (enc1.isHolded()) Serial.println("Holded");       // если была удержана и энк не поворачивался
  //if (enc1.isHold()) Serial.println("Hold");         // возвращает состояние кнопки
  
  float value = dallas_getTemp(DS_PIN);
  Serial.println(value); // получаем температуру

}

//Далее следует кусок чужого кода, который заявлен как самый легкий и быстрый
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
