/*
  MD_DS1307 - Library for using a DS1307 Real Time Clock.
  
  Created by Marco Colli May 2012
  
	Features access to all on-chip features
	* Read and write clock time registers
	* Access to the 64 byte battery backed up RAM
	* Control of square wave generator (on/off & frequency)
	* Control of clock features (on/off, 12/24H, day of week)
  
  v1.2 April 2016
  - Fixed bug in setting the time when in 12 hour mode.
  
  v1.1 August 2015
  - Changed class to MD_DS1307 and standardised on lower letter case 1st letter for method names.
  - Added calcDoW method
  
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
 */
#ifndef MD_DS1307_h
#define MD_DS1307_h

#include <Arduino.h>

// Control and status requests
#define	DS1307_CLOCK_HALT		0
#define	DS1307_SQW_RUN			1
#define	DS1307_SQW_TYPE_ON	2
#define	DS1307_SQW_TYPE_OFF	3
#define	DS1307_12H					4

// Control value and Status values returned
#define	DS1307_ERROR		0
#define	DS1307_ON				1
#define	DS1307_OFF			2
#define	DS1307_SQW_1HZ		3
#define	DS1307_SQW_4KHZ		4
#define	DS1307_SQW_8KHZ		5
#define	DS1307_SQW_32KHZ	6
#define	DS1307_SQW_HIGH		7
#define	DS1307_SQW_LOW		8

// Device parameters
#define	DS1307_RAM_MAX		64	// 64 bytes RAM in the device

class MD_DS1307
{
  public:
    MD_DS1307();
    
	// Read the RTC into and write RTC from the public variables
  void readTime(void);		// read the time into the public variables				
	void writeTime(void);		// write the time from the public variables
	
	// Read and write a block of date to the RAM on the clock
	// Read address starts at 0, write at 8 (first 7 bytes are for clock registers)
	uint8_t readRAM(uint8_t addr, uint8_t* buf, uint8_t len);
	uint8_t writeRAM(uint8_t addr, uint8_t* buf, uint8_t len);
	
	// Set a retrieve the status of control data for the clock
	// The item is one of the control and status request defines above
	// The value is one status values defined above
	void control(uint8_t item, uint8_t value);
	uint8_t status(uint8_t item);

	// calculate day of week for a given date
	// This algorithm good for dates  yyyy > 1752 and  1 <= mm <= 12
	// Returns dow  01 - 07, 01 = Sunday
  uint8_t calcDoW(uint16_t yyyy, uint8_t mm, uint8_t dd);
	
	// Nice functions that allow compatibility with other RTC libraries 
	void now(void) { readTime(); }
	boolean isRunning(void) { return(status(DS1307_CLOCK_HALT) != DS1307_ON); }
	
	// Variables for reading and writing data
	uint16_t yyyy; 	// year including the century
	uint8_t mm;		// month (1-12)
	uint8_t dd;		// date of the month (1-31)
	uint8_t h;		// hour of the day (1-12) or (0-23) depending on the mode
	uint8_t m;		// minutes past the hour (0-59)
	uint8_t s;		// Seconds past the minute (0-59)
	uint8_t dow;	// day of the week (1-7). Sequential number; coding depends on the application
	uint8_t	pm;		// set to non-zero if 12 hour clock and PM indicator
	
private:
	// BCD to binary number packing/unpacking functions
	inline uint8_t BCD2bin(uint8_t v) { return v - 6 * (v >> 4); };
	inline uint8_t bin2BCD (uint8_t v) { return v + 6 * (v / 10); };

	// Interface functions for the RTC device
	uint8_t readDevice(uint8_t addr, uint8_t* buf, uint8_t len);
	uint8_t writeDevice(uint8_t addr, uint8_t* buf, uint8_t len);
};

extern MD_DS1307	RTC;

#endif

