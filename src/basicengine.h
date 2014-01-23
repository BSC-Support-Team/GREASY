#ifndef BASICENGINE_H
#define BASICENGINE_H

#include <string>
#include <queue>
#include <map>
#include <limits.h>

#include "abstractschedulerengine.h"

/**
  * This engine inherits AbstractSchedulerEngine, and implements a basic scheduler and launcher
  * for Greasy in a single machine using system fork.
  */
class BasicEngine : public AbstractSchedulerEngine
{
  
public:

  /**
   * Constructor that adds the filename to process.
   * @param filename path to the task file.
   */
  BasicEngine (const string& filename ); 
  
  /**
   * Perform the initialization of the engine and the MPI environment.
   */
  virtual void init();
  
  /**
   * Execute the engine. It is divided into 2 different parts, for master and workers.
   */
  virtual void run();


protected:
  
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
   * Run the command corresponding to a task.
   * @param task The task to be executed.
   * @param worker The index of the worker in charge.
   * @return The exit status of the command.
   */  
  virtual int executeTask(GreasyTask *task, int worker);
  
  /**
   * Checks if a given node is the local node.
   * @param task The node to be checked.
   * @return True if local node, false otherwise.
   */  
  bool isLocalNode(string node);

  /**
   * Get the name of the node where the worker is asigned.
   * @param task The worker id.
   * @return the string containing the node name.
   */  
  string getWorkerNode(int worker);
  
  map<pid_t,int> pidToWorker; /**<  Map to translate a pid to the corresponding worker. */
  map<int, GreasyTimer> workerTimers; /**<  Map of worker timers to know elapsed time of tasks. */
  map<int, string> workerNodes; /**<  Map of worker nodes to know in which node to send each worker's tasks. */
  string masterHostname; ///< String to hold the master hostname.
  bool remote; ///< Flag to know whether the engine will need to do remote tasks.
};

#endif // BASICENGINE_H
