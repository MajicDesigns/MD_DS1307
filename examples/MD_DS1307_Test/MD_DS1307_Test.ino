// Example program for the MD_DS1307 library
//
// Allows testing of all library functions and RTC chip from the Serial Monitor.

#include <MD_DS1307.h>
#include <Wire.h>

#define PRINTS(s) Serial.print(F(s));
#define PRINT(s, v) { Serial.print(F(s)); Serial.print(v); }

#ifdef ARDUINO_ARCH_SAMD
MD_DS1307 myRTC;  ///< Locally created instance of the RTC class
#else
#define myRTC RTC ///< Library created instance of the RTC class
#endif

void setup()
{
  Serial.begin(57600);
  PRINTS("[MD_DDS1307_Test]");
  
  usage();
}

void usage(void)
{
  PRINTS("\n?\thelp - this message");
  PRINTS("\n\ntr\tread the current time");
  PRINTS("\ntw yyyymmdd hhmmss dw\twrite the current date, time and day of week (1-7)");
  PRINTS("\n\nrr\tread the contents of RAM buffer");
  PRINTS("\nrw aa nn vv [vv...]\twrite RAM address hex aa with nn hex values vv");
  PRINTS("\n\ns\tstatus of the RTC");
  PRINTS("\nd\tcalculate day of week from current date");
  PRINTS("\n\nc n v\twrite the value v to status n, where n is");
  PRINTS("\n\t0 - Clock Halt (n 0=run, 1=halt)");
  PRINTS("\n\t1 - SQW Enable(n 0=halt, 1=run)");
  PRINTS("\n\t2 - SQW Type (on) (n 1=1Hz, 2=4kHz, 3=8kHz, 4=32kHz)");
  PRINTS("\n\t3 - SQW Type (off) (n 0=low, 1=high)");
  PRINTS("\n\t4 - 12 hour mode (n 0=24h, 1=12h)\n");
}

const char *dow2String(uint8_t code)
{
  static const char *str[] = { "---", "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };

  if (code > 7) code = 0;
  return(str[code]);
}


const char *ctl2String(uint8_t code)
{
  static const char *str[] = 
  {
    "CLOCK_HALT",
    "SQW_RUN",
    "SQW_TYPE_ON",
    "SQW_TYPE_OFF",
    "12H MODE"
  };

  return(str[code]);
}

const char *sts2String(uint8_t code)
{
  static const char *str[] = 
  {
    "ERROR",
    "ON",
    "OFF",
    "1Hz",
    "4KHz",
    "8KHz",
    "32KHz",
    "HIGH",
    "LOW"
  };

  return(str[code]);
}

uint8_t htoi(char c)
{
  c = toupper(c);
  
  if (c >= '0' && c <= '9')
      return(c - '0');
  else if (c >= 'A' && c <= 'F')
      return(c - 'A' + 10);
  else
      return(0);
}
        
uint8_t i2dig(uint8_t mode)
// input 2 digits in the specified base
{
  uint8_t  v = 0;
  char    c[3] = { "00" };

  c[0] = ReadNext();
  c[1] = ReadNext();

  switch (mode)
  {
    case DEC: v = atoi(c);  break;
    case HEX: v = (htoi(c[0]) << 4) + htoi(c[1]); ;  break;
  }
  
  return(v);
}

char htoa(uint8_t i)
{
  if (i < 10)
  {
    return(i + '0');
  }
  else if (i < 16)
  {
    return(i - 10 + 'a');
  }
  return('?');
}

const char *p2dig(uint8_t v, uint8_t mode)
// print 2 digits leading zero
{
  static char c[3] = { "00" };

  switch(mode)
  {
    case HEX:
    {
      c[0] = htoa((v >> 4) & 0xf);
      c[1] = htoa(v & 0xf);
    }
    break;
  
    case DEC:
    {
      c[0] = ((v / 10) % 10) + '0';
      c[1] = (v % 10) + '0';
    }
    break;
  }

  return(c);
}

void showStatus()
{
  PRINT("\nClock Halt:\t", sts2String(myRTC.status(DS1307_CLOCK_HALT)));
  PRINT("\nIs running:\t", myRTC.isRunning());
  PRINT("\nSQW Output:\t", sts2String(myRTC.status(DS1307_SQW_RUN)));
  PRINT("\nSQW Type (on):\t", sts2String(myRTC.status(DS1307_SQW_TYPE_ON)));
  PRINT("\nSQW Type (off):\t", sts2String(myRTC.status(DS1307_SQW_TYPE_OFF)));
  PRINT("\n12h mode:\t", sts2String(myRTC.status(DS1307_12H)));
}

void printTime()
{
  PRINT("", myRTC.yyyy);
  PRINT("-", p2dig(myRTC.mm, DEC));
  PRINT("-", p2dig(myRTC.dd, DEC));
  PRINT(" ", p2dig(myRTC.h, DEC));
  PRINT(":", p2dig(myRTC.m, DEC));
  PRINT(":", p2dig(myRTC.s, DEC));
  if (myRTC.status(DS1307_12H) == DS1307_ON)
    PRINT(" ", myRTC.pm ? "pm" : "am");
  PRINT(" ", dow2String(myRTC.dow));
}

void showTime()
{
  myRTC.readTime();
  PRINTS("\n");
  printTime();
}

void showRAM()
{
  #define  MAX_READ_BUF  (DS1307_RAM_MAX / 8)  // do 8 lines
 
  uint8_t  buf[MAX_READ_BUF];

  for (int i=0; i<DS1307_RAM_MAX; i+=MAX_READ_BUF)
  {
    myRTC.readRAM(i, buf, MAX_READ_BUF);
    
    PRINT("\n", p2dig(i, HEX));
    PRINTS(":");
    for (int j = 0; j < MAX_READ_BUF; j++)
      PRINT(" ", p2dig(buf[j], HEX));
    PRINTS("  ");
    for (int j=0; j<MAX_READ_BUF; j++)
    {
      if (isalnum(buf[j]) || ispunct(buf[j]))
      {
        PRINT(" ", (char)buf[j]);
      }
      else
        PRINTS(" .");
    } 
  }
}

void writeRAM()
{
  uint8_t  addr = i2dig(HEX);
  uint8_t  len = i2dig(HEX);
  uint8_t  val[DS1307_RAM_MAX];

  if ((len == 0) || (len > DS1307_RAM_MAX))
  {
    PRINTS("\nInvalid data length");
    return;
  }
  
  for (int i=0; i<len; i++)
    val[i] = i2dig(HEX);
  
  PRINT("\nAddress 0x", p2dig(addr, HEX));
  PRINTS(" write value");
  for (int i=0; i<len; i++)
    PRINT(" ", p2dig(val[i], HEX));
  
  PRINT("\n", myRTC.writeRAM(addr, val, len));
  PRINTS(" bytes written");
}

void showDoW(void)
{
  myRTC.readTime();
  PRINT("\nCalculated DoW is ", dow2String(myRTC.calcDoW(myRTC.yyyy, myRTC.mm, myRTC.dd)));
}

void writeTime()
{
  myRTC.yyyy = i2dig(DEC)*100 + i2dig(DEC);
  myRTC.mm = i2dig(DEC);
  myRTC.dd = i2dig(DEC);
  
  myRTC.h = i2dig(DEC);
  myRTC.m = i2dig(DEC);
  myRTC.s = i2dig(DEC);
  
  myRTC.dow = i2dig(DEC);
  
  PRINTS("\nWriting ");
  printTime();
  
  myRTC.writeTime(); 
}

void writeControl()
{
  char  c = ReadNext();
  uint8_t  item, value;
  
  switch (c)
  {
    case '0':  // halt
      item = DS1307_CLOCK_HALT;
      c = ReadNext();
      switch (c)
      {
        case '0': value = DS1307_OFF;  break;
        case '1': value = DS1307_ON;  break;
        default: goto error;
      }
      break;
      
    case '1':  // enable
      item = DS1307_SQW_RUN;
      c = ReadNext();
      switch (c)
      {
        case '0': value = DS1307_OFF;  break;
        case '1': value = DS1307_ON;   break;
        default: goto error;
      }
      break;
      
    case '2':  // type on
      item = DS1307_SQW_TYPE_ON;
      c = ReadNext();
      switch (c)
      {
        case '1': value = DS1307_SQW_1HZ;    break;
        case '2': value = DS1307_SQW_4KHZ;   break;
        case '3': value = DS1307_SQW_8KHZ;   break;
        case '4': value = DS1307_SQW_32KHZ;  break;
        default: goto error;
      }
      break;
      
    case '3':  // type off
      item = DS1307_SQW_TYPE_OFF;
      c = ReadNext();
      switch (c)
      {
        case '0': value = DS1307_SQW_LOW;   break;
        case '1': value = DS1307_SQW_HIGH;  break;
        default: goto error;
      }
      break;
      
    case '4':  // 12 h mode
      item = DS1307_12H;
      c = ReadNext();
      switch (c)
      {
        case '0': value = DS1307_OFF;  break;
        case '1': value = DS1307_ON;   break;
        default: goto error;
      }
      break;
      
    default:
 error:
      PRINTS("\nBad control element or parameter");
      return;
  }
  
  // do it
  PRINT("\nControlling ", ctl2String(item));
  PRINT(" value ", sts2String(value));
  
  myRTC.control(item, value);
  
  return;
}

char ReadNext()
// Read the next character from the serial input stream, skip whitespace.
// Busy loop with a delay.
{
  char  c;
  
  do
  {
    while (!Serial.available())
      delay (50);  // wait for the next character
    c = Serial.read();
  } while (isspace(c));

  return(c);
}

void loop()
{
   char  c;
   
   // we need to get the next character to know what command we want to process 
   c = ReadNext();
   switch (toupper(c))
   {
     case '?': usage();       break;
     
     // Status and control
     case 'S': showStatus();  break;
     case 'C': writeControl();  break;
     case 'D': showDoW();     break;
     
     // Time functions
     case 'T':  // Display updates      
       c = ReadNext();
       switch (toupper(c))
       {
         case 'R': showTime();  break;
         case 'W': writeTime();  break;
         default: goto no_good;
       }
       break;
    
     // RAM functions
     case 'R':  // Display updates      
       c = ReadNext();
       switch (toupper(c))
       {
         case 'R': showRAM();   break;
         case 'W': writeRAM();   break;
         default: goto no_good;
       }
       break;

       default:  // don't know what to do with this! 
no_good:           // label for default escape when we can't process a character 
        {
          PRINT("\nBad parameter '", Serial.print(c));
          PRINTS("'");
          while (Serial.available())    // flush the buffer
            c = ReadNext();
        }
        break;
   }     
   PRINTS("\n");
 }

