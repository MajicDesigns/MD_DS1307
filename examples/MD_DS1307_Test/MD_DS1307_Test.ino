#include <MD_DS1307.h>
#include <Wire.h>

void setup()
{
  Serial.begin(57600);
  Serial.println("[MD_DDS1307_Example]");
  
  usage();
  Serial.println();
}

void usage(void)
{
  Serial.println("?\thelp - this message");
  Serial.println("tw yyyymmdd hhmmss dw\twrite the current date, time and day of week (1-7)");
  Serial.println("tr\tread the current time");
  Serial.println("rw aa nn vv [vv...]\twrite RAM address hex aa with nn hex values vv");
  Serial.println("rr\tread the contents of RAM buffer");
  Serial.println("s\tstatus of the RTC");
  Serial.println("d\tcalculate day of week from current date");
  Serial.println("c n v\twrite the value v to status n, where n is");
  Serial.println("\t0 - Clock Halt (n 0=run, 1=halt)");
  Serial.println("\t1 - SQW Enable(n 0=halt, 1=run)");
  Serial.println("\t2 - SQW Type (on) (n 1=1Hz, 2=4Mhz, 3=8Mhz, 4=32MHz)");
  Serial.println("\t3 - SQW Type (off) (n 0=low, 1=high)");
  Serial.println("\t4 - 12 hour mode (n 0=24h, 1=12h)");
}

char *dow2String(uint8_t code)
{
  static char *str[] = { " ---", " Sun", " Mon", " Tue", " Wed", " Thu", " Fri", " Sat" };

  return(str[code]);
}


char *ctl2String(uint8_t code)
{
  static char *str[] = 
  {
    "CLOCK_HALT",
    "SQW_RUN",
    "SQW_TYPE_ON",
    "SQW_TYPE_OFF",
    "12H MODE"
  };

  return(str[code]);
}

char *sts2String(uint8_t code)
{
  static char *str[] = 
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
  char    c[3] = "00";

  c[0] = ReadNext();
  c[1] = ReadNext();

  switch (mode)
  {
    case DEC: v = atoi(c);  break;
    case HEX: v = (htoi(c[0]) << 4) + htoi(c[1]); ;  break;
  }
  
  return(v);
}

void p2dig(uint8_t v, uint8_t mode)
// print 2 digits leading zero
{
  uint8_t n = 0;

  switch(mode)
  {
    case HEX: n = 16;  break;
    case DEC: n = 10;  break;
  }

  if (v < n) Serial.print("0");
  Serial.print(v, mode);
  
  return;
}

void showStatus()
{
  Serial.print("Clock Halt:\t");
  Serial.println(sts2String(RTC.status(DS1307_CLOCK_HALT)));
  Serial.print("Is running:\t");
  Serial.println(RTC.isRunning());
  Serial.print("SQW Output:\t");
  Serial.println(sts2String(RTC.status(DS1307_SQW_RUN)));
  Serial.print("SQW Type (on):\t");
  Serial.println(sts2String(RTC.status(DS1307_SQW_TYPE_ON)));
  Serial.print("SQW Type (off):\t");
  Serial.println(sts2String(RTC.status(DS1307_SQW_TYPE_OFF)));
  Serial.print("12h mode:\t");
  Serial.println(sts2String(RTC.status(DS1307_12H)));
}

void printTime()
{
  Serial.print(RTC.yyyy, DEC);
  Serial.print("-");
  p2dig(RTC.mm, DEC);
  Serial.print("-");
  p2dig(RTC.dd, DEC);
  Serial.print(" ");
  p2dig(RTC.h, DEC);
  Serial.print(":");
  p2dig(RTC.m, DEC);
  Serial.print(":");
  p2dig(RTC.s, DEC);
  if (RTC.status(DS1307_12H) == DS1307_ON)
    Serial.print(RTC.pm ? " pm" : " am");
  Serial.println(dow2String(RTC.dow));
}

void showTime()
{
  RTC.readTime();
  printTime(); 
}

void showRAM()
{
  #define  MAX_READ_BUF  (DS1307_RAM_MAX / 8)  // do 8 lines
 
  uint8_t  buf[MAX_READ_BUF];
  
  for (int i=0; i<DS1307_RAM_MAX; i+=MAX_READ_BUF)
  {
    RTC.readRAM(i, buf, MAX_READ_BUF);
    
    p2dig(i, HEX);
    Serial.print(": ");
    for (int j=0; j<MAX_READ_BUF; j++)
    {
      p2dig(buf[j], HEX);
      Serial.print(" ");
    } 
    Serial.print("  ");
    for (int j=0; j<MAX_READ_BUF; j++)
    {
      if (isalnum(buf[j]) || ispunct(buf[j]))
        Serial.print((char) buf[j]);
      else
        Serial.print(".");
      Serial.print(" ");
    } 
    Serial.println();
  }
}

void writeRAM()
{
  uint8_t  addr = i2dig(HEX);
  uint8_t  len = i2dig(HEX);
  uint8_t  val[DS1307_RAM_MAX];

  if ((len == 0) || (len > DS1307_RAM_MAX))
  {
    Serial.println("Invalid data length");
    return;
  }
  
  for (int i=0; i<len; i++)
    val[i] = i2dig(HEX);
  
  Serial.print("Address 0x");
  p2dig(addr, HEX);
  Serial.print(" write value ");
  for (int i=0; i<len; i++)
  {
    p2dig(val[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
  
  Serial.print(RTC.writeRAM(addr, val, len));
  Serial.println(" bytes written");
}

void showDoW(void)
{
  RTC.readTime();
  Serial.print("Calculated DoW is");
  Serial.println(dow2String(RTC.calcDoW(RTC.yyyy, RTC.mm, RTC.dd)));
}

void writeTime()
{
  RTC.yyyy = i2dig(DEC)*100 + i2dig(DEC);
  RTC.mm = i2dig(DEC);
  RTC.dd = i2dig(DEC);
  
  RTC.h = i2dig(DEC);
  RTC.m = i2dig(DEC);
  RTC.s = i2dig(DEC);
  
  RTC.dow = i2dig(DEC);
  
  Serial.print("Writing ");
  printTime();
  
  RTC.writeTime(); 
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
      Serial.println("Bad control element or parameter");
      return;
  }
  
  // do it
  Serial.print("Controlling ");
  Serial.print(ctl2String(item));
  Serial.print(" value ");
  Serial.println(sts2String(value));
  
  RTC.control(item, value);
  
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
          Serial.print("Bad parameter '");
          Serial.print(c);
          Serial.println("'");
          while (Serial.available())    // flush the buffer
            c = ReadNext();
        }
        break;
   }     
   Serial.println();
 }

