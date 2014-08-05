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
--Ex: if you want to interrupt your code to do something exactly every 1 second, this code should can do that, to a precision of approx. +8/-7ms.

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

#if ARDUINO >= 100
 #include <Arduino.h>
#else
 #include <WProgram.h>
#endif

//Libraries to include
#include "eRCaGuy_WDTimer.h"
#include <avr/wdt.h>

//pre-instantiate an object of this library class, for use within the ISR & by the user
eRCaGuy_WDTimer wdt;

//-------------------------------------------------------------------------------------------------------------------
//Watchdog Timer ISR (Interrupt Service Routine)
//-gets called automatically every time the Watchdog Timer expires
//-------------------------------------------------------------------------------------------------------------------
ISR(WDT_vect)
{
  //prevent Watchdog Timer from resetting the microcontroller
  wdt_disable(); //disable the Watchdog timer (ie: set the WDE bit to 0) in order to prevent the mcu from entering System Reset Mode 
                 //right after this ISR exits; see here for more info: http://www.nongnu.org/avr-libc/user-manual/group__avr__watchdog.html
								 //Also see datasheet Table 11-1 pg. 55
  wdt_reset(); //reset elapsed Watchdog timer to zero
  
  //determine if the delay is over yet, or if we need to delay some more
  long t_elapsed = millis() - wdt._t_WDT_start; //ms
  wdt._t_WDT_delay_remaining = wdt._t_WDT_delay_desired - t_elapsed; //ms; update this value by subtracting off the elapsed time
  
	wdt.update_WDT_period(); //use the _t_WDT_delay_remaining value to determine the new _WDT_period value
  if (wdt._WDT_period!=_DESIRED_DELAY_TOO_SHORT)
    wdt.WDT_begin(); //start the new WDT delay time
  else //delay is over, time to call the userFunc() function!
  {
		WDTCSR &= ~_BV(WDIE); //clear the WatchDog Interrupt Enable (WDIE) bit to 0, in order to disable the WDT_vect interrupt!
		wdt._t_WDT_end = millis(); //ms; grab the end time
    wdt._userInterruptCalled = true;
		wdt._t_WDT_delay_actual = wdt._t_WDT_end - wdt._t_WDT_start; //ms; actual time elapsed
    
		//prepare to delay again *only* if we are on REPEAT mode
		if (wdt._WDT_mode==_WDT_REPEAT)
		{
			//prepare to do it again!
			long t_excess = wdt._t_WDT_delay_actual - wdt._t_WDT_delay_desired; //ms; determine excess time we delayed, but shouldn't have
			wdt._t_WDT_start = wdt._t_WDT_end - t_excess; //update; subtract the excess time from the start time so that we only wait the *exact* period we desired to wait;
																										//this basically acts like a real-time error correction to ensure consistent & very precise
																										//timing over long periods (many delay iterations), despite jitter over short periods (each delay iteration).
			
			//find the new time remaining to wait
			t_elapsed = millis() - wdt._t_WDT_start; //ms
			wdt._t_WDT_delay_remaining = wdt._t_WDT_delay_desired - t_elapsed; //ms; update this value by subtracting off the elapsed time
			
			wdt.update_WDT_period(); //use the new _t_WDT_delay_remaining value to determine the new _WDT_period value
			if (wdt._WDT_period!=_DESIRED_DELAY_TOO_SHORT)
				wdt.WDT_begin(); //start the new WDT delay time
		}
		wdt.userFunc(); //do some user-defined, attached function
  }
}

//-------------------------------------------------------------------------------------------------------------------
//class constructor method
//-------------------------------------------------------------------------------------------------------------------
eRCaGuy_WDTimer::eRCaGuy_WDTimer()
{
}

//-------------------------------------------------------------------------------------------------------------------
//attachInterrupt
//-use the Watchdog Timer to do some action every ___ ms
//-returns whether or not the delay time is valid; ie: any delay time <8ms is too short
//-the mode can be _WDT_DO_ONCE, or _WDT_REPEAT
//--_WDT_DO_ONCE will interrupt your code & execute the interrupt function one time after the specified time
//--_WDT_REPEAT will repeatedly call the interrupt function at the specified time interval
//-------------------------------------------------------------------------------------------------------------------
boolean eRCaGuy_WDTimer::attachInterrupt(void (*isr)(),long t_desired_delay_ms,boolean mode)
{  
  //local variables
  boolean valid_desired_delay_time;
	
	userFunc = isr; //attach the user's function to the interrupt
	_WDT_mode = mode;

	valid_desired_delay_time = timedInterrupt(t_desired_delay_ms,mode);
	return valid_desired_delay_time;
}

//-------------------------------------------------------------------------------------------------------------------
//timedInterrupt()
//-use the Watchdog Timer to do some action every ___ ms
//-returns whether or not the delay time is valid; ie: any delay time <8ms is too short
//-the mode can be _WDT_DO_ONCE, or _WDT_REPEAT
//--_WDT_DO_ONCE will interrupt your code & execute the interrupt function one time after the specified time
//--_WDT_REPEAT will repeatedly call the interrupt function at the specified time interval
//-------------------------------------------------------------------------------------------------------------------
boolean eRCaGuy_WDTimer::timedInterrupt(long t_desired_delay_ms,boolean mode)
{
	_WDT_mode = mode;
	
	//local variables
  boolean valid_desired_delay_time;
	
	//first, disable the WDT just in case its in the middle of an operation right now
	detachInterrupt();
	
	uint8_t SREG_old = SREG; //back up the AVR Status Register; see example in datasheet on pg. 14, as well as Nick Gammon's "Interrupts" article - http://www.gammon.com.au/forum/?id=11488
  noInterrupts(); //prepare for critical section of code (ex: writing to a volatile variable)
  _t_WDT_start = millis(); //initialize
  _t_WDT_delay_desired = t_desired_delay_ms; //initialize
  _t_WDT_delay_remaining = t_desired_delay_ms; //initialize
  SREG = SREG_old; //restore previous interrupt status
  
	//update the global volatile variable byte "_WDT_period"
	update_WDT_period();
	if (_WDT_period!=_DESIRED_DELAY_TOO_SHORT)
  {
    WDT_begin(); //start the initial WDT delay time
    valid_desired_delay_time = true;
  }
  else
  {
		Serial.println(F("desired delay too short"));
    valid_desired_delay_time = false;
  }
  return valid_desired_delay_time;
} //end of timedInterrupt() function
	
//-------------------------------------------------------------------------------------------------------------------
//detachInterrupt()
//-------------------------------------------------------------------------------------------------------------------
void eRCaGuy_WDTimer::detachInterrupt()
{
	wdt_disable();
	wdt_reset();
	WDTCSR &= ~_BV(WDIE); //clear the WatchDog Interrupt Enable (WDIE) bit to 0
}

//-------------------------------------------------------------------------------------------------------------------
//stop()
//-same as detachInterrupt()
//-------------------------------------------------------------------------------------------------------------------
void eRCaGuy_WDTimer::stop()
{
	detachInterrupt();
}

//-------------------------------------------------------------------------------------------------------------------
//WDT_begin()
//-INPUT: requires the public member (acts like a global variable within the class) _WDT_period to already by updated
//-OUTPUT: none; but prepares the WDT interrupt to go off
//-------------------------------------------------------------------------------------------------------------------
void eRCaGuy_WDTimer::WDT_begin()
{
  //set up the Watchdog Timer (WDT)
  wdt_enable(_WDT_period); //enable the WD Timer (set WDE bit to 1 [Table 11-1 pg. 55 --> System Reset Mode]) & set the specified timeout period
  wdt_reset(); //reset the timer to zero and let it start counting
  WDTCSR |= _BV(WDIE); //set the WatchDog Interrupt Enable (WDIE) bit to 1, in order to ENABLE the WDT_vect interrupt!
}

//-------------------------------------------------------------------------------------------------------------------
//update_WDT_period()
//-use the _t_WDT_delay_remaining value to determine the new _WDT_period value
//-INPUT: requires the public member (acts like a global variable within the class) _t_WDT_delay_remaining to already by updated
//-OUTPUT: updates the public member _WDT_period
//-------------------------------------------------------------------------------------------------------------------
void eRCaGuy_WDTimer::update_WDT_period()
{
  //copy out a volatile variable
  uint8_t SREG_old = SREG; //back up the AVR Status Register; see example in datasheet on pg. 14, as well as Nick Gammon's "Interrupts" article - http://www.gammon.com.au/forum/?id=11488
  noInterrupts(); //prepare for critical section of code (ex: writing to a volatile variable)
  long _t_WDT_delay_remaining_cpy = _t_WDT_delay_remaining; //copy value
  SREG = SREG_old; //restore previous interrupt status
  
  if (_t_WDT_delay_remaining_cpy >= 8192)
    _WDT_period = WDTO_8192MS;
  else if (_t_WDT_delay_remaining_cpy >= 4096)
    _WDT_period = WDTO_4096MS;
  else if (_t_WDT_delay_remaining_cpy >= 2048)
    _WDT_period = WDTO_2048MS;
  else if (_t_WDT_delay_remaining_cpy >= 1024)
    _WDT_period = WDTO_1024MS;
  else if (_t_WDT_delay_remaining_cpy >= 512)
    _WDT_period = WDTO_512MS;
  else if (_t_WDT_delay_remaining_cpy >= 256)
    _WDT_period = WDTO_256MS;
  else if (_t_WDT_delay_remaining_cpy >= 128)
    _WDT_period = WDTO_128MS;
  else if (_t_WDT_delay_remaining_cpy >= 64)
    _WDT_period = WDTO_64MS;
  else if (_t_WDT_delay_remaining_cpy >= 32)
    _WDT_period = WDTO_32MS;
  else if (_t_WDT_delay_remaining_cpy >= 16/2) //If the desired delay is 8ms, then I will delay 16, and I will have delayed *8 too many*.  
																							 //If the desired delay is 9ms, then I will delay 16, which is *7 too many*. If the desired delay is 7ms, 
																							 //then I will not delay at all, which is *7ms too few.*
                                               //So, with this statement as-is, the precision is only +8ms/-7ms. 
                                               //Ie: For any given desired_delay in ms, you will delay somewhere between 8ms too many, and 7ms too few.
    _WDT_period = WDTO_16MS;
  else //don't delay at all
    _WDT_period = _DESIRED_DELAY_TOO_SHORT; //set to indicate the delay is too short
} //end of update_WDT_period()



  


