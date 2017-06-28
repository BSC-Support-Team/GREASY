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

#ifndef GREASYTASK_H
#define GREASYTASK_H

#include <string>
#include <list>

using namespace std;

/**
  * This class represents a Greasy task, corresponding to a line in the Greasy Task File.
  * 
  */

class GreasyTask
{
  
public:
  
  /** 
  * Possible task states.
  */
  enum TaskStates {
          invalid, /** Task is not valid. contains syntax error(s) or invalid dependencies */
	  blocked, /**< Task is blocked waiting for some dependencies to complete. */
	  waiting, /**< Task is waiting to be scheduled. */
	  running, /**< Task is running.  */
	  completed, /**< Task has completed fine. */
	  failed, /**< Task has failed for some reason. */
          cancelled /**< Task has been cancelled because of dependencies failed. */
  } ;

  /**
   * Empty Constructor.
   */
  GreasyTask ( );
  
  /**
   * Simple Constructor to fill taskId and command.
   */  
  GreasyTask ( int id, string cmd );
  
  /**
   * Get the value of taskId.
   * @return the value of taskId.
   */
  int getTaskId ( );

  /**
   * Get the value of taskNum.
   * @return the value of taskNum.
   */
  int getTaskNum ( );
  
    /**
   * Set the value of taskId.
   * @param new_var the new value of taskId.
   */
  void setTaskId ( int new_var );

    /**
   * Set the value of taskNum.
   * @param new_var the new value of taskId.
   */
	void setTaskNum ( int new_var );

  /**
   * Get the value of command.
   * @return the value of command.
   */
  string getCommand ( );

  /**
   * Set the value of command.
   * @param new_var the new value of command.
   */
  void setCommand ( string new_var );

  /**
   * Get the state of the task.
   * @return the state as int, but corresponding to the enum TaskStates.
   */
  int getTaskState();

  /**
   * Prints the the state of the task in a human readable form.
   * @return a string whith the task state
   */
  string printTaskState();

  /**
   * Check if the task is blocked (because a dependency is not fulfilled).
   * @return true if the task is blocked.
   */
  bool isBlocked();

  /**
   * Check if the task is waiting (ready to be run).
   * @return true if the task is waiting.
   */
  bool isWaiting();

  /**
   * Check if the task is valid (no syntax error(s) nor dependency failures).
   * @return true if the task is valid.
   */
  bool isInvalid();

  /**
   * Set the task state to state.
   * @param state The state of the task, a value from TaskStates.
   */
  void setTaskState(TaskStates state); 
  
  /**
   * Get the (last) return code of the task.
   * @return The exit code from the task.
   */
  int getReturnCode();
  
  /**
   * Set the return code of the task to code.
   * @param code the new exit code.
   */  
  void setReturnCode(int code);
  
  /**
   * Get the last value of elapsed time of the task.
   * @return the elapsed time in seconds of the task.
   */  
  unsigned long getElapsedTime();

  /**
   * Get the accumulated value of elapsed time of the task 
   * along the retries.
   * @return the total elapsed time in seconds of the task.
   */  
  unsigned long getElapsedTimeAcc();
  
  /**
   * Set the elapsed time of the task. The time will be also
   * added to the total counter elapsedAcc.
   * @param et the elapsed time in seconds.
   */    
  void setElapsedTime(unsigned long et);
  
  /**
   * Get the hostname where this task run.
   * @return the hostname.
   */    
  string getHostname();

  /**
   * Set the hostname where this task run.
   * @param h the hostname.
   */   
  void setHostname(string h);

  /**
   * Get the number of retries performed with this task.
   * @return the number of retries.
   */     
  int getRetries();

  /**
   * Adds a retry attempt to the counter.
   */       
  void addRetryAttempt();
  
  /**
   * Checks if the task has dependencies.
   * @return true if the task has dependencies, false otherwise.
   */     
  bool hasDependencies();
  
  /**
   * Obtain the list of dependencies as taskIds.
   * @return a list of taskIds
   */   
  list<int> getDependencies();
  
  /**
   * Adds the dependency parentTask to the list.
   * @param parentTask the parent task on which this task depends.
   */     
  void addDependency(int parentTask);

  /**
   * Removes the dependency parentTask from the list.
   * @param parentTask the parent task on which this task depends.
   */     
  void removeDependency(int parentTask);
  
  /**
   * Add task depenencies as written in the task file. It will parse the contents
   * of [# deps #]. Deps contains a list of comma separated tokens, which can be:
   *   - a number. Example: 6
   *   - a negative number meaning a relative dependency with preceding tasks. Example: -1
   *   - a range of numbers. Example 3-6
   * @param deps a string with the dependencies written as in the task file.
   * @return true if all went fine, false otherwise.
   */     
  bool addDependencies(string deps);
  
  /**
   * Debug function that generates a pretty string with the task contents.
   * @return string with pretty task contents.
   */     
  string dump();

  /**
   * Debug function that generates a pretty string with the task dependencies.
   * @return string with pretty task contents.
   */    
  string dumpDependencies();
  

protected:
  
  /**
   * Init Attributes of the task
   */
  void initAttributes ( ) ;

  int taskId;  /**< Task id corresponding to the line of the file (starting at 1). */
	int taskNum; /**< Real number of the task (ignoring comments). */
  string command; /**< Command to be executed. */
  int taskState; /**< Task state at a given time. */
  string hostname; /**< Return code of the executed command. */
  int returnCode;  /**< Return code of the executed command. */
  int retries; /**< Number of execution retries of the task. */
  unsigned long elapsed; /**< Seconds elapsed of the last execution of the task. */
  unsigned long elapsedAcc; /**< Seconds elapsed accumulated among retries. */
  list<int> dependencies; /**< List of the taskIds of the dependencies. */

};

#endif // GREASYTASK_H
