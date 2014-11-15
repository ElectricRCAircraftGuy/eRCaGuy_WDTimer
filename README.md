**Readme for eRCaGuy_WDTimer library**

Readme last updated: 5 Aug 2014
By Gabriel Staples
http://electricrcaircraftguy.blogspot.com/

**Instructions**
Download the code by clicking "Download ZIP" to the right.  Extract the files, and install the library by renaming the main folder to "eRCaGuy_WDTimer," and then copying the whole thing to the Arduino "libraries" directory in your main "Sketches" folder.  Restart your Arduino IDE (Integrated Development Environment).  Now, you may view the examples in the IDE under File --> Examples --> eRCaGuy_WDTimer.
For more help on how to install Arduino libraries, see here: https://learn.adafruit.com/adafruit-all-about-arduino-libraries-install-use?view=all

**Basic Summary**
-provides a watchdog-timer-based interrupt function.

**Description**
-use this code to use the watchdog timer to attach an interrupt that automatically perform some action every ___ms (user-defined).  The benefit here is that your attached function is guaranteed to execute at the interval you specify (it can't get blocked by delay() or other functions in your main loop), and that is uses the *watchdog timer*, thereby keeping your other timers (ex: Timer1, Timer2, Timer0) free to perform other functions and/or be used by other libraries!

**Version History**
See the .cpp file

Be sure to check out the links above.  I hope you find this useful.
Sincerely,
Gabriel Staples
http://electricrcaircraftguy.blogspot.com/