/* 
 * This file is part of GREASY software package
 * Copyright (C) by the BSC-Support Team, see www.bsc.es
 * 
 * GREASY is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 * 
 * GREASY is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with GREASY. If not, see <http://www.gnu.org/licenses/>.
 *
*/

#ifndef GREASYLOG_H
#define GREASYLOG_H

#include <string>
#include <iostream>
#include <fstream>

#include "greasyutils.h"
#include "config.h"

#define NUM_LEVELS 6

using namespace std;


/**
 * This class implements a multi-level log system. Each entry will be recorded
 * in the log file. Thanks to the different levels, an entry will only be
 * recorded if the entry's level is less or equal than the log level 
 * configured at the beginning.
 * 
*/
class GreasyLog{

public:
  
  /** 
  * Available levels of the log.
  */
  enum LogLevels {
	  silent, /**< No log information will be generated. */
	  error, /**< Only fatal errors will be recorded. */
	  warning, /**< All errors & warnings will be recorded.  */
	  info, /**< Standard information will be printed. */
	  debug, /**< Show also debug information. */
	  devel /**< Show also development debug information. */
  };
  
  /** 
  * Description of the log levels.
  */
  static const string logLevelsDesc[6];

  /**
    * Get the unique GreasyLog instance. Implementation of the Singleton Pattern.
    * @return A pointer to the GreasyLog instance.
    */
  static GreasyLog* getInstance();
  
  /**
    * Initialize the log system with the log file.
    * @param fileName 
    * @return True if all is ok, false otherwise.
    */
  bool logToFile(string fileName);
  
  /**
    * Close the log.
    */
  void logClose();
  
  /**
    * Get the default level for the log
    * @return the level.
    */
  int getDefaultLogLevel();
  
  /**
    * Get the default level for the log
    * @return the level.
    */
  int getCurrentLogLevel();
  
  /**
    * Initialize the log system with the level.
    * @param level 
    */
  void setLogLevel(LogLevels level);

  /**
    * Record an entry to the log with a prefix.
    * @param level The log level of this message.
    * @param prefix The prefix to be set in the message.
    * @param message The message to record.
    */
  void record(LogLevels level, string prefix, string message);
  
  /**
    * Record an entry to the log.
    * @param level The log level of this message.
    * @param message The message to record.
    */
  void record(LogLevels level, string message);
 

private:
  
  /**
    * Default constructor, hidden from everyone. If anyone wants to use the 
    * class, they should use the getInstance function.
    */
  GreasyLog();

  static GreasyLog instance; /**< Unique instance of the log. */
  ofstream *logFile; /**< Descriptor of the log file. */
  int logLevel; /**< LogLevel configured. */
  bool isFile; /**< Boolean to know if the log is going to a file */

};

#endif
