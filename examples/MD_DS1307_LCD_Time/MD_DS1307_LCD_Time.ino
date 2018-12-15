// Example program for the MD_DS1307 library
//
// Implements a clock display on a LCD shield.

#include <LiquidCrystal.h>
#include <MD_DS1307.h>
#include <Wire.h>

#ifdef ARDUINO_ARCH_SAMD
MD_DS1307 myRTC;  ///< Locally created instance of the RTC class
#else
#define myRTC RTC ///< Library created instance of the RTC class
#endif

LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

void setup()
{
  // initialise the LCD
  lcd.begin(16,2);
  lcd.clear();
  lcd.noCursor();

  // Ensure the clock is running
  if (!myRTC.isRunning())
    myRTC.control(DS1307_CLOCK_HALT, DS1307_OFF);
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
  lcd.print(dow2String(myRTC.dow));
  lcd.print(myRTC.yyyy);
  lcd.print("-");
  p2dig(myRTC.mm);
  lcd.print("-");
  p2dig(myRTC.dd);

  lcd.setCursor(0,1);
  p2dig(myRTC.h);
  lcd.print(":");
  p2dig(myRTC.m);
  lcd.print(":");
  p2dig(myRTC.s);
  if (myRTC.status(DS1307_12H) == DS1307_ON)
    lcd.print(myRTC.pm ? " pm" : " am");
}

void loop()
{
  myRTC.readTime();
  printTime(); 
  delay(100);
}

