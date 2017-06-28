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

#ifndef ABSTRACTSCHEDULERENGINE_H
#define ABSTRACTSCHEDULERENGINE_H

#include <string>
#include <queue>

#include "abstractengine.h"

/**
  * This engine inherits AbstractEngine, and implements a basic scheduler for Greasy.
  * Child classes can use the scheduler contained here and implement the specific details
  * like task allocation.
  *
  */
class AbstractSchedulerEngine : public AbstractEngine
{
  
public:

  /**
   * Constructor that adds the filename to process.
   * @param filename path to the task file.
   */
  AbstractSchedulerEngine (const string& filename );
  
  /**
   * reimplementation of the init() method adding the workers init code.
   */
  virtual void init();
  /**
   * Methdod that  should provide all the finalization
   * and clean-up code of the engine. An implementation is provided to do the basic cleanup
   * of abstract engine, and MUST be called from the subclass if reimplemented.
   */
  virtual void finalize();

  /**
   * Base method to write the restart file. It creates a file named taskFile.rst, and adds to it
   * all the tasks that didn't complete successfully (they were waiting, running, failed or cancelled).
   * It will also add invalid tasks, but commented out. All dependencies will be recorded along each task
   * with the indexes updated to the new line numbering.
   */  
  virtual void writeRestartFile();

  /**
   * Method that should print the contents of the taskMap,
   * calling dumpTaskMap(). This method is only for debugging purposes
   */
  virtual void dumpTasks();

protected:
  
  /**
   * Perform the scheduling of tasks to workers
   */
  virtual void runScheduler();
  
  /**
   * Allocate a task in a free worker, sending the command to it.
   * @param task A pointer to a GreasyTask object to allocate.
   */
  virtual void allocate(GreasyTask* task) = 0;
  
  /**
   * Wait for any worker to complete their tasks and retrieve
   * results. It MUST be implemented in subclasses.
   */
  virtual void waitForAnyWorker() = 0;
  
  /**
   * Update all the tasks depending from the parent task which has finished.
   * @param parent A pointer to a GreasyTask object that finished.
   */
  virtual void updateDependencies(GreasyTask* parent);
  
  /**
   * Get default number of workers according to the cpus available in the computer.
   */
  virtual void getDefaultNWorkers();
  
  /**
   * All the checks after a task finishes are done here, updating task and engine metadata.
   */  
  virtual void taskEpilogue(GreasyTask *task);
  
  
  map <int,int> taskAssignation; ///<  Map that holds the task assignation to workers.
				 ///< worker -> taskId
  queue <int> freeWorkers; ///< The queue of free worker ids, from where the candidates
			   ///< to run a task will be taken.
  queue <GreasyTask*> taskQueue; ///< The queue of tasks to be executed.
  set <GreasyTask*> blockedTasks; ///< The set of blocked tasks.

};

#endif // ABSTRACTSCHEDULERENGINE_H
