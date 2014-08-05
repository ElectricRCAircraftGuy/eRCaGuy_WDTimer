/*
eRCaGuy_WDTimer
-A library that uses the Watchdog Timer to call an event every ___ms.
By Gabriel Staples
Website: http://electricrcaircraftguy.blogspot.com
Contact Info: http://electricrcaircraftguy.blogspot.com/2013/01/contact-me.html
Written: 4 Aug 2014
Updated: 5 Aug 2014
*/

/*
History (newest on top)
20140805 - library first written
*/

/*
===================================================================================================
  LICENSE & DISCLAIMER
  Copyright (C) 2014 Gabriel Staples.  All right reserved.
  
  ------------------------------------------------------------------------------------------------
  License: GNU General Public License Version 3 (GPLv3) - https://www.gnu.org/licenses/gpl.html
  ------------------------------------------------------------------------------------------------

  This file is part of eRCaGuy_WDTimer.
  
  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see http://www.gnu.org/licenses/
===================================================================================================
*/

/*
-The purpose of this code is to create a general-purpose interrupt-based timer, which will run your specified function every ___ms that you specify.  
--Ex: if you want to interrupt your code to do something exactly every 1 second, this code should be able to do that, to a precision of approx. +8/-7ms.

References:
-Nick Gammon's Interrupt Article/web page: http://www.gammon.com.au/forum/?id=11488
-Narcoleptic library -- THIS WAS A *HUGE* HELP IN HELPING ME LEARN HOW TO DO THE CODE BELOW: https://code.google.com/p/narcoleptic/
--Thank you Peter Knight (Cathedrow)!
-AVRLibc sleep.h library: http://www.nongnu.org/avr-libc/user-manual/group__avr__sleep.html 
-AVRLibc wdt.h library: http://www.nongnu.org/avr-libc/user-manual/group__avr__watchdog.html
-TimerOne library: http://playground.arduino.cc/code/timer1

Watchdog Timer (WDT) macro delay options: http://www.nongnu.org/avr-libc/user-manual/group__avr__watchdog.html
Macro Name    Table 11-2 Time      Calculated Time    Actual, measured time             % error
              (datasheet pg. 55)   (ie: this is what  (determined experimentally        (act/calculated)
                                 it really should be)  using this code on a Pro Mini)  
WDTO_8S       8.0 sec              8192 ms             8.1300 sec                       0.76% 
WDTO_4S       4.0 sec              4096 ms             4.0645 sec                       0.76%
WDTO_2S       2.0 sec              2048 ms             2.0318 sec                       0.78%
WDTO_1S       1.0 sec              1024 ms             1016.060 ms
WDTO_500MS    500 ms               512 ms              508.240 ms
WDTO_250MS    250 ms               256 ms              254.160 ms
WDTO_120MS    125 ms               128 ms              127.100 ms
WDTO_60MS     64 ms                64 ms               63.570 ms
WDTO_30MS     32 ms                32 ms               31.798 ms                        0.63%
WDTO_15MS     16 ms                16 ms               15.914 ms                        0.54%
*/

#ifndef eRCaGuy_WDTimer_h
#define eRCaGuy_WDTimer_h

#if ARDUINO >= 100
 #include <Arduino.h>
#else
 #include <WProgram.h>
#endif

//macros/Defines
//use the exact ms delay values in the names, deduced from Table 11-2 in the datasheet, rather than the rounded values
//see here for the original defines: http://www.nongnu.org/avr-libc/user-manual/group__avr__watchdog.html
//Watchdog TimeOut values
#define WDTO_16MS 0
#define WDTO_32MS 1
#define WDTO_64MS 2
#define WDTO_128MS 3
#define WDTO_256MS 4
#define WDTO_512MS 5
#define WDTO_1024MS 6
#define WDTO_2048MS 7
#define WDTO_4096MS 8
#define WDTO_8192MS 9

#define _DESIRED_DELAY_TOO_SHORT 0xFF //arbitrary byte value >9 (in decimal) to help me know this

//Watchdog Timer modes
#define _WDT_DO_ONCE 0 //default mode
#define _WDT_REPEAT 1

class eRCaGuy_WDTimer
{
	public:
		//Declare public methods
		eRCaGuy_WDTimer(); //constructor
		
		//methods intended to be accessed by a user
		boolean attachInterrupt(void (*isr)(),long t_desired_delay_ms,boolean mode=_WDT_DO_ONCE); //attach an interrupt function & execute this
			                                                                                        //function after a specified time
		boolean timedInterrupt(long t_desired_delay_ms,boolean mode=_WDT_DO_ONCE); //do a timed interrupt on the attached function
		void detachInterrupt(); //stop doing the timed interrupt function
		void stop(); //same exact thing as detachInterrupt
		
		//methods intended to be accessed by an ISR (they are only public so that the ISR can have access to them too)
		//I'm fairly new to C++, so the only other alternative I know, other than making these methods & members public, is to make them global.  I chose to make them public instead.
		void WDT_begin();
		void update_WDT_period();
		
		//public members (variables) - these must all be public so that they can be accessed by an ISR
		void (*userFunc)(); //function pointer for the attachInterrupt method
		
		//volatile (used in ISRs)
		volatile byte _userInterruptCalled; //flag to specify if the WDT time is elapsed yet; must be manually reset by the user
		volatile unsigned long _t_WDT_start; //ms; time stamp of when the Watchdog Timer was turned on
		volatile unsigned long _t_WDT_end; //ms; time stamp of when the WDT interrupt occurs
		volatile long _t_WDT_delay_desired; //ms; desired delay time
		volatile long _t_WDT_delay_actual; //ms; actual delay time, determined after each delay period
		volatile long _t_WDT_delay_remaining; //ms; do NOT make unsigned, as it needs to be able to store neg. values the way my WDT ISR is written
		volatile byte _WDT_period; //a byte to indicate what we will set the period to be before the next WDT interrupt occurs
		volatile boolean _WDT_mode; //indicates if the delay and call to the user ISR should occur once or repeatedly, every specified delay (period)
	
  private:
		//Private methods (ie: functions)
		//NA
		
		//Private members (ie: variables)
		//NA
};

//Declare the external existence (defined in the .cpp file) of an object of this class, so that you can access it in your Arduino sketch simply by including this library, via its header file
//This is absolutely necessary or else the Arduino sketch that includes this library will not compile.
//For more info on "extern" see here: http://www.geeksforgeeks.org/understanding-extern-keyword-in-c/
extern eRCaGuy_WDTimer wdt;
#endif



