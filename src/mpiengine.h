/* 
 * This file is part of GREASY software package
 * Copyright (C) by the BSC-Support Team, see www.bsc.es
 * 
 * GREASY is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
*/


#ifndef MPIENGINE_H
#define MPIENGINE_H

#ifdef SLURM
#define JOBID "SLURM_JOBID"
#elif defined(LSF)
#define JOBID "LSB_BATCH_JID"
#endif

#include "mpi.h"
#include <string>
#include <queue>

#include "abstractschedulerengine.h"


/**
  * This engine inherits AbstractSchedulerEngine, and implements an MPI scheduler for Greasy.
  * 
  */
class MPIEngine : public AbstractSchedulerEngine
{
  
public:

  /**
   * Constructor that adds the filename to process.
   * @param filename path to the task file.
   */
  MPIEngine (const string& filename );
  
  /**
   * Perform the initialization of the engine and the MPI environment.
   */
  virtual void init();
  
  /**
   * Execute the engine. It is divided into 2 different parts, for master and workers.
   */
  virtual void run();

  /**
   * Finalization of the engine and MPI environment.
   */
  virtual void finalize();

  /**
   * Reimplementation of writeRestartFile() to prevent workers from 
   * writing the restart (only the master does the job).
   */
  virtual void writeRestartFile();

  /**
   * Reimplementation of dumpTasks() to prevent workers from 
   * recording the dump (only the master does the job).
   */
  virtual void dumpTasks();

protected:
  
  /**
   * Check if the engine is in Master mode.
   * @return true if engine is the master, false otherwise.
   */
  bool isMaster();
  
  /**
   * Check if the engine is in Worker mode.
   * @return true if engine is a worker, false otherwise.
   */
  bool isWorker();

  // Master Methods
  /**
   * Main master method.
   * All the scheduling of tasks is done here.
   */
  void runMaster();
  
  /**
   * Allocate a task in a free worker, sending the command to it.
   * @param task A pointer to a GreasyTask object to allocate.
   */
  virtual void allocate(GreasyTask* task);
  
  /**
   * Wait for any worker to complete their tasks and retrieve
   * results.
   */
  virtual void waitForAnyWorker();
  
  /**
   * Send the end signal to the workers. This method should be called
   * when all tasks have been completed and we want to finalize workers.
   */
  void fireWorkers();
  void executionSummary();
  
  // Worker Methods
  /**
   * Main Worker method.
   * It waits for a message from the master with the command
   * to execute, until the end signal is received.
   */
  void runWorker();
  
  int workerId; /**<  Id of the worker. Master is 0. */
  char hostname[MPI_MAX_PROCESSOR_NAME]; ///< Cstring to hold the worker hostname.

};

#endif // MPIENGINE_H
