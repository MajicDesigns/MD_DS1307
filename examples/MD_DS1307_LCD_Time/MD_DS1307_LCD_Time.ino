// Example program for the MD_DS1307 library
//
// Implements a clock display on a LCD shield.

#include <LiquidCrystal.h>
#include <MD_DS1307.h>
#include <Wire.h>

LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

void setup()
{
  // initialise the LCD
  lcd.begin(16,2);
  lcd.clear();
  lcd.noCursor();

  // Ensure the clock is running
  if (!RTC.isRunning())
    RTC.control(DS1307_CLOCK_HALT, DS1307_OFF);
}

void p2dig(uint8_t v)
// print 2 digits leading zero
{
  if (v < 10) lcd.print("0");
  lcd.print(v);
}

const char *dow2String(uint8_t code)
{
  static const char *str[] = {"--- ", "Sun ", "Mon ", "Tue ", "Wed ", "Thu ", "Fri ", "Sat "};

  if (code > 7) code = 0;
  return(str[code]);
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

