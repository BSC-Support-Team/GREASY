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

#ifndef THREADENGINE_H
#define THREADENGINE_H

#include <queue>
#include "abstractengine.h"

#include "tbb/tbb.h"
#include "tbb/task.h"
#include "tbb/task_scheduler_init.h"
#include "tbb/parallel_do.h"

/**
  * This engine inherits AbstractEngine, and implements a Threaded scheduler for Greasy.
  * There is a similar engine called "ForkEngine" that works similarly to this except that
  * it calls "forks" instead of using threads.
  * This class inherits from "AbstractEngine" and not from "AbstractSchedulerEngine"  because
  * here we don't need the scheduler  functionality since TBB has its own scheduler.
  */
class ThreadEngine : public AbstractEngine
{
public:

  /**
   * Constructor that adds the filename to process.
   * @param filename path to the task file.
   */
  ThreadEngine (const string& filename );

 /**
  * Execute the engine. It is divided into 2 different parts, for master and workers.
  */
 virtual void run();


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
  void runScheduler();

  /**
   * Get default number of workers according to the cpus available in the computer.
   */
  virtual void getDefaultNWorkers();

};

/*************************************************************************************/
/***  TBB-related classes
/*************************************************************************************/

/**
  * This class wraps a task as seen from the TBB task scheduler.
  * This is the class that is called from the parallel_do and inspect
  * and launch the dependencies if needed.
  */
class GreasyTBBTaskEngine {

    public:

        /**
          * Constructor
          **/
        GreasyTBBTaskEngine(map<int,GreasyTask*> * taskMap_, set<int>* validTasks_ ,map<int,list<int> >*revDepMap_ );

        typedef GreasyTask* argument_type; // typedef for function object

        /**
          * Core: Overloading this operator makes parallel_do perform it for each value in the loop.
          * We later on dispatch dependant children as needed.
          */
        void operator()( argument_type item, tbb::parallel_do_feeder<argument_type>& feed_it) const ;

    protected:

        map<int,GreasyTask*> * taskMap;
        set<int>* validTasks;
        map<int,list<int> >*revDepMap;

        bool taskEpilogue(argument_type gtask, tbb::parallel_do_feeder<argument_type>& feed_it ) const;
        void updateDependencies(argument_type child, tbb::parallel_do_feeder<argument_type>& feed_it) const;
};

#endif // THREADENGINE_H
