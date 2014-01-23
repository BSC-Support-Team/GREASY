#include "greasylog.h"
#include "greasyconfig.h"
#include "abstractengine.h"
#include "config.h"

#include <string>
#include <csignal>
#include <cstdlib>

#ifndef SYSTEM_CFG
#define SYSTEM_CFG "../etc/greasy.conf"
#endif
#ifndef USER_CFG
#define USER_CFG "./greasy.conf"
#endif

using namespace std;

AbstractEngine* engine = NULL;
int my_pid;
bool readConfig();
void termHandler( int sig );

int main(int argc, char *argv[]) {
  my_pid=getpid();
  GreasyLog* log = GreasyLog::getInstance();
  GreasyConfig* config = GreasyConfig::getInstance();
    
  if (argc != 2) {
      cout << "Usage: greasy filename" << endl;
      return (0);
  }
  
  string filename(argv[1]);
  
  // Read config
  if (!readConfig()) {
    cerr << "Failed to load config file" << endl;
    return -1;
  }

  // Log Init
  if(config->keyExists("LogFile"))
    log->logToFile(config->getValue("LogFile"));
  
  if(config->keyExists("LogLevel")) {
    int logLevel = fromString(logLevel,config->getValue("LogLevel"));
    log->setLogLevel((GreasyLog::LogLevels)logLevel);
  }
  
  // Handle interrupting signals appropiately.
  signal(SIGTERM, termHandler);
  signal(SIGINT,  termHandler);
  signal(SIGUSR1, termHandler);
  signal(SIGUSR2, termHandler);
 
  // Create the proper engine selected and run it!
  engine = AbstractEngineFactory::getAbstractEngineInstance(filename,config->getValue("Engine"));
  if (!engine) {
      log->record(GreasyLog::error,"Greasy could not load the engine");
      return -1;
  }
  // Initialize the engine
  engine->init();
  
  // Start it. All tasks will be scheduled and run from now on
//   engine->dumpTasks();
  engine->run();
  
  // Finalize the engine once all tasks have finished.
  engine->finalize();
  
  // Ok we're done
  log->logClose();
  
}


bool readConfig () {
 
  GreasyConfig* config = GreasyConfig::getInstance();
  
  // First, try to read USER config. If it is not present, then use the System defaults.
  if (!config->readConfig(USER_CFG)) return config->readConfig(SYSTEM_CFG);
  
  return true;
  
}

void termHandler( int sig ) {
  char killTree[100];
  
  GreasyLog* log = GreasyLog::getInstance();
  log->record(GreasyLog::error, "Caught TERM signal");
  if (engine) engine->writeRestartFile();
  log->record(GreasyLog::error, "Greasy was interrupted. Check restart & log files");
  log->logClose();
  sprintf(killTree, "kill  -- -%d", my_pid);
  system(killTree);
  exit(1);
}
  
