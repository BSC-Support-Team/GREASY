
#include "greasytimer.h"
#include <cstring>

GreasyTimer::GreasyTimer() {

  startTime = 0;
  endTime = 0;
  elapsed=0;
  on = false;

}


void GreasyTimer::start() {

  struct timeval t;
  if (startTime == 0) {
    gettimeofday(&t,0);
    startTime = t.tv_sec*1000000+t.tv_usec;
    on = true;
  }

}

void GreasyTimer::stop() {

  struct timeval t;
  if ((on) && (startTime > 0)) {
    gettimeofday(&t,NULL);
    endTime = t.tv_sec*1000000+t.tv_usec;
    elapsed += endTime - startTime;
    startTime = endTime;
    on = false;
  }

}

void GreasyTimer::reset() {

  startTime = 0;
  endTime = 0;
  elapsed = 0;

}

bool GreasyTimer::isOn() {
 
  return on;
  
}

unsigned long GreasyTimer::getStartTime() {
 
  return startTime;
 
}


unsigned long GreasyTimer::getEndTime() {
  
  return endTime;
  
}

string GreasyTimer::getElapsed() {
  
 return secsToTime(secsElapsed());
 
}

unsigned long GreasyTimer::secsElapsed() {

  return elapsed/1000000;	

}

unsigned long GreasyTimer::msecsElapsed() {

  return elapsed/1000;	
}

unsigned long GreasyTimer::usecsElapsed() {

  return elapsed;

}


string GreasyTimer::now() {

  struct tm *tmp;
  time_t t;
  t = time(NULL);
  tmp = localtime(&t);
  return timeToString(tmp);
  
}

string GreasyTimer::secsToTime(unsigned long secs) {

  int hours, minutes, seconds;
  char buf[10];
  string time;
  
  hours=secs/3600;
  secs=secs%3600;
  minutes=secs/60;
  secs=secs%60;
  seconds=secs;
  
  sprintf(buf,"%02d:%02d:%02d",hours,minutes,seconds);
  return string(buf);
  
}

string GreasyTimer::timeToString(struct tm * ts) {
 
  char buffer[100];
  string result;
  
  strftime (buffer,sizeof(buffer),"%Y-%m-%d %X",ts);
  result = buffer;
  
  return result;
  
}
