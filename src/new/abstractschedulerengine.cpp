#include "abstractschedulerengine.h"
#include <csignal>
#include <cstdlib>
#include <cstring>


AbstractSchedulerEngine::AbstractSchedulerEngine ( const string& filename) : AbstractEngine(filename){
  
    engineType="abstractscheduler";
    
}

void AbstractSchedulerEngine::init() {
    
  log->record(GreasyLog::devel, "AbstractSchedulerEngine::init", "Entering...");
  
  AbstractEngine::init();
  
  // Fill the freeWorkers queue
  for (int i=1;i<=nworkers; i++) {
    freeWorkers.push(i);
  }
  
  log->record(GreasyLog::devel, "AbstractSchedulerEngine::init", "Exiting...");
  
}

void AbstractSchedulerEngine::finalize() {

  AbstractEngine::finalize();
  
}

void AbstractSchedulerEngine::writeRestartFile() {

  AbstractEngine::writeRestartFile();
  
}

void AbstractSchedulerEngine::dumpTasks() {

  AbstractEngine::dumpTasks();
  
}

void AbstractSchedulerEngine::runScheduler() {
  
  //Master code
  set<int>::iterator it;
  GreasyTask* task = NULL;

  log->record(GreasyLog::devel, "AbstractSchedulerEngine::runScheduler", "Entering...");
  
  // Dummy check: let's see if there is any worker...
  if (nworkers==0) {
    log->record(GreasyLog::error, "No workers found. Rerun greasy with more resources");    
    return;
  }
  
  globalTimer.start();

  // Initialize the task queue with all the tasks ready to be executed
  for (it=validTasks.begin();it!=validTasks.end(); it++) {
    task = taskMap[*it];
    if (task->isWaiting()) taskQueue.push(task);
    else if (task->isBlocked()) blockedTasks.insert(task);
  }
   
  // Main Scheduling loop
  while (!(taskQueue.empty())||!(blockedTasks.empty())) {
    while (!taskQueue.empty()) {
      if (!freeWorkers.empty()) {
	// There is room to allocate a task...
	task =  taskQueue.front();
	taskQueue.pop();
	allocate(task);
      } else {
	// All workers are busy. We need to wait anyone to finish.
	waitForAnyWorker();
      }
    }
    
    if (!(blockedTasks.empty())) {
      // There are no tasks to be scheduled on the queue, but there are
      // dependencies not fulfilled and tasks already running, so we have
      // to wait for them to finish to release blocks on them.
      waitForAnyWorker();
    }
  }

  // At this point, all tasks are allocated / finished
  // Wait for the last tasks to complete
  while (freeWorkers.size()!=nworkers) {
    waitForAnyWorker();
  }
  
  globalTimer.stop();
  
  log->record(GreasyLog::devel, "AbstractSchedulerEngine::runScheduler", "Exiting...");
  
}


void AbstractSchedulerEngine::updateDependencies(GreasyTask* parent) {
  
  int taskId, state;
  GreasyTask* child;
  list<int>::iterator it;

  log->record(GreasyLog::devel, "AbstractSchedulerEngine::updateDependencies", "Entering...");
  
  taskId = parent->getTaskId();
  state = parent->getTaskState();
  log->record(GreasyLog::devel, "AbstractSchedulerEngine::updateDependencies", "Inspecting reverse deps for task " + toString(taskId));
  
  if ( revDepMap.find(taskId) == revDepMap.end() ){
      log->record(GreasyLog::devel, "AbstractSchedulerEngine::updateDependencies", "The task "+ toString(taskId) + " does not have any other dependendant task. No update done.");
      log->record(GreasyLog::devel, "AbstractSchedulerEngine::updateDependencies", "Exiting...");
      return;
  }

  for(it=revDepMap[taskId].begin() ; it!=revDepMap[taskId].end();it++ ) {
    child = taskMap[*it];
    if (state == GreasyTask::completed) {
      log->record(GreasyLog::devel, "AbstractSchedulerEngine::updateDependencies", "Remove dependency " + toString(taskId) + " from task " + toString(child->getTaskId()));
      child->removeDependency(taskId);
      if (!child->hasDependencies()) { 
	log->record(GreasyLog::devel, "AbstractSchedulerEngine::updateDependencies", "Moving task from blocked set to the queue");
	blockedTasks.erase(child);
	taskQueue.push(child);
      } else {
	log->record(GreasyLog::devel, "AbstractSchedulerEngine::updateDependencies", "The task still has dependencies, so leave it blocked");
      }
    }
    else if ((state == GreasyTask::failed)||(state == GreasyTask::cancelled)) {
      log->record(GreasyLog::warning,  "Cancelling task " + toString(child->getTaskId()) + " because of task " + toString(taskId) + " failure");
      log->record(GreasyLog::devel, "AbstractSchedulerEngine::updateDependencies", "Parent failed: cancelling task and removing it from blocked");
      child->setTaskState(GreasyTask::cancelled);
      blockedTasks.erase(child);
      updateDependencies(child);
    }
  }
  
  log->record(GreasyLog::devel, "AbstractSchedulerEngine::updateDependencies", "Exiting...");
  
}

void AbstractSchedulerEngine::taskEpilogue(GreasyTask *task) {
  
  int maxRetries=0;

  log->record(GreasyLog::devel, "AbstractSchedulerEngine::taskEpilogue", "Entering...");
  
  if (config->keyExists("MaxRetries")) fromString(maxRetries, config->getValue("MaxRetries"));
      
  if (task->getReturnCode() != 0) {
    log->record(GreasyLog::error,  "Task " + toString(task->getTaskId()) + 
		    " failed with exit code " + toString(task->getReturnCode()) + " on node " + 
		    task->getHostname() +". Elapsed: " + GreasyTimer::secsToTime(task->getElapsedTime()));
    // Task failed, let's retry if we need to
    if ((maxRetries > 0) && (task->getRetries() < maxRetries)) {
      log->record(GreasyLog::warning,  "Retry "+ toString(task->getRetries()) + 
		    "/" + toString(maxRetries) + " of task " + toString(task->getTaskId()));
      task->addRetryAttempt();
      allocate(task);
    } else {
      task->setTaskState(GreasyTask::failed);
      updateDependencies(task);
    }
  } else {
    log->record(GreasyLog::info,  "Task " + toString(task->getTaskNum()) + " located in line " + toString(task->getTaskId()) + 
		    " completed successfully on node " + task->getHostname() + ". Elapsed: " + 
		    GreasyTimer::secsToTime(task->getElapsedTime()));
    task->setTaskState(GreasyTask::completed);
    updateDependencies(task);
  }
  
  log->record(GreasyLog::devel, "AbstractSchedulerEngine::taskEpilogue", "Exiting...");
  
}

void AbstractSchedulerEngine::getDefaultNWorkers() {
 
  nworkers = sysconf(_SC_NPROCESSORS_ONLN);
  if ( nworkers>4 ) nworkers=4;
  
  log->record(GreasyLog::devel, "AbstractSchedulerEngine::getDefaultNWorkers", "Default nworkers: " + toString(nworkers));
  
}
