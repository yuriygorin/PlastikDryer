// include the library code:
//#include <LiquidCrystal.h>
#include <LiquidCrystalRus.h>

#define _LCD_TYPE 2
#include <LCD_1602_RUS_ALL.h>

//LCD_1602_RUS <LiquidCrystal> lcd(12, 11, 5, 4, 3, 2);
LCD_1602_RUS <LiquidCrystal> lcd(12, 11, 5, 8, 7, 6 );//For LCD Keypad Shield


// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
//const int rs = 12, en = 11, d4 = 5, d5 = 8, d6 = 7, d7 = 6;
// lcd(rs, en, d4, d5, d6, d7);

uint8_t temp_cel[8] =
{
B00111,
B00101,
B00111,
B00000,
B00000,
B00000,
B00000
}; //закодировано в двоичной системе значек градуса

void setup() {
  // put your setup code here, to run once:
  lcd.begin(16, 2);       // Настройка дисплея
  lcd.noCursor();
  lcd.createChar(1, temp_cel);
  
}

void loop() {
  // put your main code here, to run repeatedly:

        lcd.print("Привет");
 while (1){}

 
  
//  for (int letter=0; letter <= 255; letter++) {    
//    lcd.setCursor(currentCol, currentRow);
//    lcd.print(char(letter));
//    currentCol++;    
//
//    if (currentCol > 15){
//      currentCol=0;
//      if (currentRow < 1) {        
//        currentRow++;        
//      }
//      else {                
//        delay(8000);
//        lcd.clear();
//        currentRow=0;
//      }
//    }
//  }
}
