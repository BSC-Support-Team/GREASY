#include "greasylog.h"
#include "greasytimer.h"

// Description of each one of the levels.
const string GreasyLog::logLevelsDesc[NUM_LEVELS] = {"", "ERROR: ", "WARNING: ","INFO: ","DEBUG: ","DEVEL: "};

GreasyLog::GreasyLog()
{
  
  // By default, set level log to info until it is inited.
  logLevel =  getDefaultLogLevel();
  isFile = false;
  logFile = (ofstream*) &cerr;

}

GreasyLog* GreasyLog::getInstance() {

  static GreasyLog instance;
  return &instance;
	
}

int GreasyLog::getDefaultLogLevel() {
  
  return GreasyLog::info;
    
}

int GreasyLog::getCurrentLogLevel() {
  
  return logLevel;
    
}

void GreasyLog::setLogLevel(LogLevels level) {

  string leveldesc;
  if ((level!=logLevel)&&(level>=0)&&(level<NUM_LEVELS)) {
    if (level) leveldesc =logLevelsDesc[level];
    else leveldesc = "SILENT";
    logLevel=level;
    record(GreasyLog::devel, "", "Switching to " + leveldesc + " log level");
  }

}

bool GreasyLog::logToFile(string fileName) {

  if(fileName=="") {
    logFile = (ofstream*) &cerr;
  } else {
    if((isFile)&&(logFile->is_open())) logFile->close();
    logFile = new ofstream();
    logFile->open(fileName.c_str(), ios::out| ios::app );
    if(!logFile->is_open()) {
      delete logFile;
      logFile = (ofstream*) &cerr;
    } else {
      isFile = true;
    }
  }
  return isFile;

}

void GreasyLog::logClose() {

  logLevel = 0;
  if (isFile) {
    if (logFile->is_open()) logFile->close();
    if (logFile) delete logFile;
    isFile=0;
  }
  logFile = (ofstream*) &cerr;

}

void GreasyLog::record(LogLevels level, string message) {
  
  record(level,"",message);
  
}

void GreasyLog::record(LogLevels level, string prefix, string message) {

  string mes="";
  if (level<=logLevel) {
    if (prefix.empty()) 
      mes = "[" + GreasyTimer::now() + "] " + logLevelsDesc[level] + message + "\n";
    else
      mes = "[" + GreasyTimer::now() + "] " + logLevelsDesc[level] + "[" + prefix + "] " + message + "\n";
    
    (*logFile) << mes;
    logFile->flush();
  }

}
