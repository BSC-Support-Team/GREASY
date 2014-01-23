#ifndef GREASYTIMER_H
#define GREASYTIMER_H

#include <string>
#include <cstdio>
#include <sys/time.h>

using namespace std;

/**
 * A simple timer class with usecs resolution. Timer can be started and stopped as many 
 * times as desired, and elapsed will be added each time until reset is called.
 * It also provide some useful static methods for datetime querying.
 */
class GreasyTimer {

public:
  
  /**
    * Default constructor. It initializes the timer, but it does not start it.
    */
  GreasyTimer();
  
  /**
    * Start the timer. Only if it was not already started.
    */
  void start();
  
  /**
    * Stop the timer. Only if it was started before.
    */
  void stop();
  
  /**
    * Reset the timer to 0.
    */
  void reset();
  
  /**
    * Get the start time.
    * @return The start time in usecs.
    */
  unsigned long getStartTime();
  
  /**
    * Get the end time.
    * @return The end time in usecs.
    */
  unsigned long getEndTime();
  
  /**
    * Get the seconds elapsed.
    * @return The seconds elapsed.
    */
  unsigned long secsElapsed();
  
  /**
    * Get the miliseconds elapsed.
    * @return The miliseconds elapsed.
    */
  unsigned long msecsElapsed();
  
  /**
    * Get the microseconds elapsed.
    * @return The microseconds elapsed.
    */
  unsigned long usecsElapsed();
  
  /**
   * Get a well formatted HH:MM:SS of the elapsed time of the timer.
   * @return string with the time in hours, minutes and seconds.
   */
  string getElapsed();
  
  /**
   * Check if the timer is on.
   * @return true if it is on, false otherwise.
   * */
  bool isOn();
  
  /**
   * Function to get current date time as a string in the format 
   * "YYYY-MM-DD HH:MM:SS".
   * @return The string containing the pretty time.
   */
  static string now();
  
  /**
   * Static method to convert seconds to a string in the format
   * HH:MM:SS
   * @param secs the seconds to convert
   * @return the seconds in HH:MM:SS
   */
  static string secsToTime(unsigned long secs);
  
 
  /**
   * Function to transform a time struct as given by time syscall to a
   * string in the format "YYYY-MM-DD HH:MM:SS".
   * @param ts The time structure to transform.
   * @return The string containing the pretty time.
   */
  static string timeToString(struct tm * ts);
	
protected:
  
  unsigned long startTime; /**< Start time in usecs.*/
  unsigned long endTime; /**< End time in usecs. */
  unsigned long elapsed; /**< Elapsed time in usecs. */
  bool on; /** Flag to know if the timer is on */

};

#endif
