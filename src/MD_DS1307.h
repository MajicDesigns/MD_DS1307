/**
\mainpage Arduino DS1307 Library
The Maxim DS1307 Real Time Clock IC
-----------------------------------
The DS1307 serial real-time clock (RTC) is a low-power,
full binary-coded decimal (BCD) clock/calendar plus 56 
bytes of non-volatile SRAM.

Microprocessor interface is exclusively through an I2C 
bidirectional bus.

The device 
- incorporates a battery input, and maintains timekeeping when main power 
to the device is interrupted. 
- maintains seconds, minutes, hours, day, date, month, and year information. 
- automatically adjusts dates for months with fewer than 31 days, including 
corrections for leap year. 
- operates in either the 24-hour or 12-hour format with an AM/PM indicator.
- has and frequency programmable square-wave output.

This library features access to all on-chip features
- Read and write clock time registers
- Access to the 64 byte battery backed up RAM
- Read and write alarm registers and controls
- Read/write clock and alarm registers as RAM
- Control of square wave generator (on/off & frequency)
- Control of clock features (on/off, 12/24H, day of week)

If you like and use this library please consider making a small donation using [PayPal](https://paypal.me/MajicDesigns/4USD)
___

\subpage pageSoftware

___

Revision History 
----------------
Dec 2018 version 1.3.5
- Fixed RTC (re)declaration clash with SAMD architecture.

Jun 2018 version 1.3.4
- Cleaned up dependency on ESP8266 for constructor change.

Jun 2018 version 1.3.3
- Added overloaded constructor specifying sca and scl.

Apr 2018 version 1.3.2
- Minor changes to code

Feb 2018 version 1.3.1
- Fixed text in header file
- Updated documentation to MD standards

May 2017 v1.3
- Changed examples to remove 'deprecated' compiler warning.

Apr 2016 v1.2
- Fixed bug in setting the time when in 12 hour mode.

Aug 2015 v1.1
- Changed class to MD_DS1307 and standardised on lower letter case 1st letter for method names.
- Added calcDoW method

Copyright
---------
Copyright (C) 2015 Marco Colli. All rights reserved.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA

\page pageSoftware Software Overview
Using the Library
-----------------

The library has a simple interface to the RTC hardware implemented through:
- a set of time (h, m, s and dow) and date (yyyy, mm, dd) variables. All data to and from
the RTC passes through these. Clock or alarm time data is read or written to these interface
registers before the appropriate methods are invoke to act on that data.
- control() and status() methods to set and query the functionality of the RTC.

___

The Control and Status Methods
----------------------------------
The control() and status() methods are the main interface to action setting parameters or 
status values. The parameters used for each are similar, but not symmetrical, due
to the way the hardware is implemented. The table below highlights the combinations of valid 
parameters for the control() method and whether a parameter can be changed by user code.

The status() method will return the current value from the parameter nominated and is guaranteed to
be in the valid set.

 Function            |R|W| Write value
---------------------|-|-|-------------------------------------------------------------------|
DS1307_CLOCK_HALT    |Y|Y| DS1307_ON, DS1307_OFF 
DS1307_SQW_RUN       |Y|Y| DS1307_ON, DS1307_OFF
DS3231_SQW_TYPE_ON   |Y|Y| DS1307_SQW_1HZ, DS1307_SQW_4KHZ, DS1307_SQW_8KHZ, DS1307_SQW_32KHZ
DS1307_SQW_TYPE_OFF  |Y|Y| DS1307_SQW_HIGH, DS1307_SQW_LOW
DS1307_12H           |Y|Y| DS1307_ON, DS1307_OFF 

___

Working with the Current Time
-----------------------------
__Reading__ the current time from the clock is a call to the readTime() method. The current date 
and time is then available in the interface registers.

__Writing__ the current time is a sequence of writing to the interface registers followed by a call 
to the writeTime() method.

The DS1307_LCD_Time example has examples of the different ways of interacting with the RTC.
*/

#ifndef MD_DS1307_h
#define MD_DS1307_h

#include <Arduino.h>
/**
 * \file
 * \brief Main header file for the MD_DS1307 library
 */

/**
 * Control and Status Request.
 *
 * These definitions are used with the control() and status() methods to identify 
 * the control action request.
 */
#define DS1307_CLOCK_HALT   0 ///< Controls the clock halt (CH) bit. When set OFF the oscillator/clock is disabled. CH is disabled on power up.
#define DS1307_SQW_RUN      1 ///< Controls the Square Wave Enable (SQWE) bit. SQWE is OFF when power is first applied.
#define DS1307_SQW_TYPE_ON  2 ///< Controls the two RS bits that set the frequency of the square wave when the square wave output is enabled.
#define DS1307_SQW_TYPE_OFF 3 ///< Controls the OUT bit that sets the output level of the SQW/OUT pin when the square wave output is disabled. On power up this hardware bit set to LOW.
#define DS1307_12H          4 ///< Controls whether the library is using 24 hour time or AM/PM designation. Setting ON enables 12 hour time and AM/PM.

/**
 * Control and Status Request return values.
 *
 * These values are used as the return status from the
 * control() and status() methods.
 */
#define DS1307_ERROR      0 ///< An error occurred executing the requested action
#define DS1307_ON         1 ///< Represents an ON status to set or returned from a get
#define DS1307_OFF        2 ///< Represents an OFF status to set or returned from a get
#define DS1307_SQW_1HZ    3 ///< Set or get 1Hz square wave specifier for SQW_TYPE parameter
#define DS1307_SQW_4KHZ   4 ///< Set or get 4kHz square wave specifier for SQW_TYPE parameter
#define DS1307_SQW_8KHZ   5 ///< Set or get 8kHz square wave specifier for SQW_TYPE parameter
#define DS1307_SQW_32KHZ  6 ///< Set or get 32kHz square wave specifier for SQW_TYPE parameter
#define DS1307_SQW_HIGH   7 ///< Represents a HIGH status to set or returned from a get
#define DS1307_SQW_LOW    8 ///< Represents an LOW status to set or returned from a get

// Device parameters
#define DS1307_RAM_MAX  64  ///< Total number of RAM registers that can be read from the device

/**
 * Core object for the MD_DS1307 library
 */
class MD_DS1307
{
  public:
 /** 
  * Class Constructor
  *
  * Instantiate a new instance of the class. One instance of the class is 
  * created in the libraries as the RTC object.
  * 
  */
  MD_DS1307();

  /**
  * Overloaded Class Constructor (ESP8266 only)
  *
  * Provides a way to assign custom SCL and SDA pins if the architecture
  * supports them.
  *
  * \param sda  Pin number for the SDA signal
  * \param scl  Pin number for the SCL signal
  */
  MD_DS1307(int sda, int scl);

  //--------------------------------------------------------------
 /** \name Methods for object and hardware control.
  * @{
  */
 /** 
  * Set the control status of the specified parameter to the specified value.
  * 
  * The device has a number of control parameters that can be set through this method. 
  * The type of control action required is passed through the mode parameter and 
  * should be one of the defined control actions. The value that needs to be supplied 
  * on the control action required is one of the defined status values.
  * Not all combinations of item and value are valid.
  *
  * \sa Software Overview section in the introduction for a table of valid combinations.
  *
  * \param item    one of the defined code request values.
  * \param value   value as one of the defined code status values.
  */
  void control(uint8_t item, uint8_t value);

 /**
  * Obtain the current setting for the specified parameter.
  * 
  * Any of the parameters that can be set from the control() method can be queried using
  * this method. The code status value returned will be one of the defined values for the 
  * control() method for each specific code request parameter.
  *
  * \sa Software Overview section in the introduction for a table of valid combinations.
  *
  * \param item  one of the defined control request values.
  * \return one of the defined code status values or DS1307_ERROR if an error occurred.
  */
  uint8_t status(uint8_t item);

  /** @} */

 //--------------------------------------------------------------
 /** \name Methods for RTC operations
  * @{
  */

 /**
  * Read the current time into the interface registers
  *
  * Query the RTC for the current time and load that into the library interface registers 
  * (yyyy, mm, dd, h, m, s, dow, pm) from which the data can be accessed.
  *
  * \return false if errors, true otherwise.
  */
  void readTime(void);

 /**
  * Write the current time from the interface registers
  *
  * Write the data in the interface registers (yyyy, mm, dd, h, m, s, dow, pm) 
  * as the current time in the RTC.
  *
  */
  void writeTime(void);

 /**
 * Compatibility function - Read the current time
 *
 * Wrapper to read the current time.
 *
 * \sa readTime() method
 *
 * \return no return value.
 */
  void now(void) { readTime(); }

 /**
  * Compatibility function - Check if RTC is running
  *
  * Wrapper for a HALT status check
  *
  * \sa status() method
  *
  * \return true if running, false otherwise.
  */
  boolean isRunning(void) { return(status(DS1307_CLOCK_HALT) != DS1307_ON); }

  /** @} */

 //--------------------------------------------------------------
 /** \name Miscellaneous methods
  * @{
  */
 /**
  * Read the raw RTC clock data
  *
  * Read _len_ bytes from the RTC clock starting at _addr_ as raw data into the 
  * buffer supplied. The size of the buffer should be at least MAX_BUF bytes long 
  * (defined in the library cpp file).
  * 
  * Read address starts at 0.
  *
  * \sa writeRAM() method
  *
  * \param addr    starting address for the read.
  * \param buf      address of the receiving byte buffer.
  * \param len       number of bytes to read.
  * \return number of bytes successfully read.
  */
  uint8_t readRAM(uint8_t addr, uint8_t* buf, uint8_t len);

 /**
  * Write the raw RTC clock data
  *
  * Write _len_ bytes of data in the buffer supplied to the RTC clock starting at _addr_.
  * The size of the buffer should be at least _len_ bytes long.
  *
  * Write address starts at 8 (first 7 bytes are for clock registers).
  *
  * \sa readRAM() method
  *
  * \param addr    starting address for the write.
  * \param buf      address of the data buffer.
  * \param len       number of bytes to write.
  * \return number of bytes successfully written.
  */
  uint8_t writeRAM(uint8_t addr, uint8_t* buf, uint8_t len);

 /**
  * Calculate day of week for a given date
  *
  * Given the specified date, calculate the day of week.
  * 
  * \sa Wikipedia https://en.wikipedia.org/wiki/Determination_of_the_day_of_the_week
  *
  * \param yyyy  year for specified date. yyyy must be > 1752.
  * \param mm    month for the specified date where mm is in the range [1..12], 1 = January.
  * \param dd    date for the specified date in the range [1..31], where 1 = first day of the month.
  * \return dow value calculated [1..7], where 1 = Sunday.
  */
  uint8_t calcDoW(uint16_t yyyy, uint8_t mm, uint8_t dd);

 /** @} */

 //--------------------------------------------------------------
 /** \name Public variables for reading and writing time data
  * @{
  */
  uint16_t yyyy;///< Year including the century.
  uint8_t mm;   ///< Month (1-12)
  uint8_t dd;   ///< Date of the month (1-31)
  uint8_t h;    ///< Hour of the day (1-12) or (0-23) depending on the am/pm or 24h mode setting
  uint8_t m;    ///< Minutes past the hour (0-59)
  uint8_t s;    ///< Seconds past the minute (0-59)
  uint8_t dow;  ///< Day of the week (1-7). Sequential number; day coding depends on the application and zero is an undefined value
  uint8_t pm;   ///< Non-zero if 12 hour clock mode and PM, always zero for 24 hour clock. Check the time and if < 12 then check this indicator.

  /** @} */

private:
  // BCD to binary number packing/unpacking functions
  inline uint8_t BCD2bin(uint8_t v) { return v - 6 * (v >> 4); };
  inline uint8_t bin2BCD (uint8_t v) { return v + 6 * (v / 10); };

  // Interface functions for the RTC device
  uint8_t readDevice(uint8_t addr, uint8_t* buf, uint8_t len);
  uint8_t writeDevice(uint8_t addr, uint8_t* buf, uint8_t len);
  
  // Functions to Initialize the class internal variables
  void init(void);
};

#ifndef ARDUINO_ARCH_SAMD
extern MD_DS1307 RTC;     ///< Library created instance of the RTC class
#endif

#endif

