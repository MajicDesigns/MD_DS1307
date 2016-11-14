#include <LiquidCrystal.h>
#include <MD_DS1307.h>
#include <Wire.h>

LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

void setup()
{
  lcd.begin(16,2);
  lcd.clear();
  lcd.noCursor();
}

void p2dig(uint8_t v)
// print 2 digits leading zero
{
  if (v < 10) lcd.print("0");
  lcd.print(v);
}

char *dow2String(uint8_t code)
{
  static char *str[] = {"Sun ", "Mon ", "Tue ", "Wed ", "Thu ", "Fri ", "Sat "};
  return(str[code-1]);
}

void printTime()
{
  lcd.setCursor(0,0);
  lcd.print(dow2String(RTC.dow));
  lcd.print(RTC.yyyy);
  lcd.print("-");
  p2dig(RTC.mm);
  lcd.print("-");
  p2dig(RTC.dd);
  
  lcd.setCursor(0,1);
  p2dig(RTC.h);
  lcd.print(":");
  p2dig(RTC.m);
  lcd.print(":");
  p2dig(RTC.s);
  if (RTC.status(DS1307_12H) == DS1307_ON)
    lcd.print(RTC.pm ? " pm" : " am");
}

void loop()
{
  RTC.readTime();
  printTime(); 
  delay(100);
}

