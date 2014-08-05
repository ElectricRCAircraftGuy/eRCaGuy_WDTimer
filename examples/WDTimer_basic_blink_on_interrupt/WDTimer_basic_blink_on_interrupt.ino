/*
Examples for library: eRCaGuy_WDTimer
-A library that uses the Watchdog Timer to interrupt your code and call an event every ___ms, either once per command, or repeatedly.
By Gabriel Staples
Website: http://electricrcaircraftguy.blogspot.com
Contact Info: http://electricrcaircraftguy.blogspot.com/2013/01/contact-me.html
Copyright (C) 2014 Gabriel Staples.  All right reserved.
*/

/*
Example Code:
WDTimer_basic_blink_on_interrupt
-tests my library by blinking LED 13 continually, toggling it once every 250ms
Written 5 Aug. 2014
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
//==================================================================================================
//LIBRARY DESCRIPTION - SEE THE .h and .cpp FILES FOR MORE INFO
//==================================================================================================

//--------------------------------------------------------------------------------------------------
//PUBLIC METHODS (ie: functions you can call)
//--------------------------------------------------------------------------------------------------
boolean attachInterrupt(userFunction,long t_desired_delay_ms, boolean mode [default is _WDT_DO_ONCE])
-attach an interrupt function & execute this function after a specified time, according to the mode
-returns a boolean to tell you whether or not the delay time is valid; ie: any delay time <8ms is always too short
-Modes: _WDT_DO_ONCE will interrupt your code & execute the interrupt function one time after the specified time, then it disables the timer & interrupt
        _WDT_REPEAT will repeatedly call the interrupt function at the specified time interval, until you call wdt.detachInterrupt() or wdt.stop().

boolean timedInterrupt(long t_desired_delay_ms, boolean mode [default is _WDT_DO_ONCE])
-same as attachInterrupt except that you don't have to pass in the name to the function to attach; so, once you've called
 attachInterrupt() once you can call this function for subsequent calls
 
void detachInterrupt()
-stop doing the timed interrupt function

void stop()
-same as detachInterrupt()

//--------------------------------------------------------------------------------------------------
//PUBLIC MEMBERS (ie: variables you might want to access)
//--------------------------------------------------------------------------------------------------
//volatile (used in ISR)
volatile byte _userInterruptCalled; //flag to specify if the WDT time is elapsed yet; must be manually reset by the user
volatile unsigned long _t_WDT_start; //ms; time stamp of when the Watchdog Timer was turned on
volatile unsigned long _t_WDT_end; //ms; time stamp of when the WDT interrupt occurs
volatile long _t_WDT_delay_desired; //ms; desired delay time
volatile long _t_WDT_delay_actual; //ms; actual delay time, determined after each delay period
volatile long _t_WDT_delay_remaining; //ms; remaining delay time until the next user interrupt is called
volatile boolean _WDT_mode; //indicates if the delay and call to the user ISR should occur once or repeatedly, every specified delay (period)
*/

#include <eRCaGuy_WDTimer.h>

const byte led = 13;
long dt_des = 250; //ms; desired time period between each call to our attached interrupt; read below for valid ranges for this value

void setup()
{
  pinMode(led,OUTPUT);
  
  //DESCRIPTION OF VALID DELAY TIMES, dt_des:
  //NOTE: THIS VALUE (dt_des) MUST BE BETWEEN 8 AND 2,147,483,647 ms, when using the _WDT_DO_ONCE mode. When using the _WDT_REPEAT mode, however, 
  //due to the fact that I have written /the code to adjust to individual iteration errors, the minimum value for 
  //dt_des must be a little higher...approximately 16 or greater, or else it will get stuck after a few iterations, since it wants to delay a smaller
  //amount of time than it is able.
  
  //PRECISION OF THE WDTimer1 library:
  //The precision of this Watchdog Timer interrupt is approximately +8/-7ms, and
  //great care has been taken to ensure that it is precise over long periods of time.  This means that the error between desired_time_elapsed and
  //actual_time_elapsed, for each iteration, is taken into consideration and used to adjust the next iteration.  The effect is that though a single iteration
  //has a precision of +8/-7ms, many iteration together have a total precision (ie: precision for the total period of time) of *less than* +/-1ms.
  
  //DESCRIPTION OF VALID MODES, _WDT_REPEAT, or _WDT_DO_ONCE:
  //_WDT_DO_ONCE will interrupt your code & execute the interrupt function one time after the specified time, then it disables the timer & interrupt
  //_WDT_REPEAT will repeatedly call the interrupt function at the specified time interval, until you call wdt.detachInterrupt() or wdt.stop().
  
  //attach an interrupt to use the Watchdog Timer to call the "blinkLED" function every "dt_des" # of milliseconds.  
  wdt.attachInterrupt(blinkLED,dt_des,_WDT_REPEAT);
}

void loop()
{
  //nothing here
}

void blinkLED()
{
  static boolean led_state = LOW;
  led_state = !led_state; //toggle
  digitalWrite(led,led_state);
}
