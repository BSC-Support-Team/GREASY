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

#ifndef ABSTRACTENGINE_H
#define ABSTRACTENGINE_H

#ifndef JOBID
#define JOBID ""
#endif

#include <string>
#include <map>
#include <list>
#include <set>

#include "greasyconfig.h"
#include "greasylog.h"
#include "greasytimer.h"
#include "greasytask.h"

using namespace std;

/**
  * The AbstractEngine is the base abstract class of the main controller of the application.
  * It holds all the logic behind greasy. This class must be inherited by the specialized
  * class which will implement the low level stuff, like the scheduling of tasks.
  * Subclasses must implement init run and finalize methods.
  * To construct objects, use AbstractEngineFactory.
  */
class AbstractEngine
{
public:

  /**
   * Constructor that adds the filename to process.
   * @param filename path to the task file
   */
  AbstractEngine ( string filename );

  /**
   * It tells if the engine is ready to run or not. It will return false if the engine is not
   * propperly initalized or error have been found when parsing task file.
   * @return true if it is ready, false otherwise.
   */
  bool isReady();

  /**
   * It provides all the initialization code. After init, engine should have parsed the task file and
   * should be prepared to run the tasks and detect possible problems after it. If reimplemented, it
   * MUST be called from the subclass.
   */
  virtual void init() ;

  /**
   * Abstract method to be implemented in subclasses. It should provide all the scheduling
   * code for the tasks, managing their creation, execution and completion and updating metadata
   * objects accordingly.
   */
  virtual void run() = 0;

  /**
   * Abstract method to get default number of workers according to the current configuration.
   */
  virtual void getDefaultNWorkers() = 0;

  /**
   * Method that should provide all the finalization and clean-up code of the engine.
   * An implementation is provided to do the basic cleanup
   * of abstract engine, and MUST be called from the subclass if reimplemented.
   */
  virtual void finalize() ;

  /**
   * Method that should print the contents of the taskMap,
   * calling dumpTaskMap(). This method is only for debugging purposes
   */
   virtual void dumpTasks();

  /**
   * Base method to write the restart file. It creates a file named taskFile.rst, and adds to it
   * all the tasks that didn't complete successfully (they were waiting, running, failed or cancelled).
   * It will also add invalid tasks, but commented out. All dependencies will be recorded along each task
   * with the indexes updated to the new line numbering.
   */
   virtual void writeRestartFile();


protected:

  /**
   * It parses the task file and fills up the taskMap. It also checks if the task is syntactically
   * valid or not.
   */
  void parseTaskFile();

  /**
   * It checks all dependencies to see that there is no semantic error in any of them, and fills up
   * the revDepMap.
   */
  void checkDependencies();

  /**
   * Helper function to record an invalid task. It adds an entry to the log, and if the strict
   * checking is enabled, will raise the fileErrors flag, preventing the engine from running.
   */
  void recordInvalidTask(int taskId);

  /**
   * It produces a final summary of the execution of greasy, with some statistics on the tasks completed,
   * failed, etc., the total amount of time consumed and the resource utilitzation percentage.
   */
  void buildFinalSummary();

  /**
  * Debug method to dump in a pretty format the contents of the taskMap.
  */
  string dumpTaskMap();


  /**
  * Remove substrings inside a string
  */
  void removeSubStrs(string& str, const string& pattern) {
   string::size_type n = pattern.length();
   for (auto i = str.find(pattern);
       i != string::npos;
       i = str.find(pattern))
       str.erase(i, n);
   }

  string engineType; /**< Type of the engine. Each subclass will have a different type. */
  string taskFile; /**< Path to the file containing the tasks to execute. */
  string restartFile; /**< Path to the file where the restart will be written. */
  int nworkers; /**< Number of greasy workers (possibly the number of cpus available). */
  bool ready; /**< Flag to know if the engine is ready to run. */

  map<int,GreasyTask*> taskMap; ///< Main task map linking the taskId (the line in the file)
				///< with the actual GreasyTask object.
  set<int> validTasks; /**< Set containing the valid tasks read in the file. */
  map<int,list<int> > revDepMap; ///< Map that contains the reverse dependencies for each task.
				 ///< Useful, for example, to know which tasks had dependencies
				 ///< against a task that completed or failed.

  GreasyLog *log; /**< log instance. */
  GreasyConfig *config; /**< config instance. */
  GreasyTimer globalTimer; /**< Global timer to count the time that engine takes to run. */

private:
  bool fileErrors; /**< Flag to know if there were any errors in the task file once processed. */
  bool strictChecking; /**< Flag to know if strict checking of the file is enabled. */

};

/**
 * Factory to build AbstractEngine objects.
 */
class AbstractEngineFactory {

public:
  /**
    * Get the required AbstractEngine instance according to the type specified.
    * @param filename The path to the task file
    * @param type A string containing the type of the engine we want to build.
    * @return A pointer to the AbstractEngine instance.
    */
    static AbstractEngine* getAbstractEngineInstance(const string& filename, const string& type="" );

};


#endif // ABSTRACTENGINE_H
