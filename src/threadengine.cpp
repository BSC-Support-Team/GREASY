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

#include "threadengine.h"



ThreadEngine::ThreadEngine ( const string& filename) : AbstractEngine(filename){
  
  engineType="thread";

}

void ThreadEngine::run(){
	log->record(GreasyLog::devel, "ThreadEngine::run", "Entering...");
	if (isReady()) runScheduler();
	log->record(GreasyLog::devel, "ThreadEngine::run", "Exiting...");
}

void ThreadEngine::init() {

  log->record(GreasyLog::devel, "ThreadEngine::init", "Entering...");

  AbstractEngine::init();

  log->record(GreasyLog::devel, "ThreadEngine::init", "Exiting...");

}

void ThreadEngine::writeRestartFile() {

  AbstractEngine::writeRestartFile();

}

void ThreadEngine::finalize() {

  AbstractEngine::finalize();

}

void ThreadEngine::dumpTasks(){
    AbstractEngine::dumpTasks();
}

void ThreadEngine::getDefaultNWorkers() {

    nworkers = tbb::task_scheduler_init::default_num_threads();
    log->record(GreasyLog::debug, "ThreadEngine::getDefaultNWorkers", "Default nworkers: " + toString(nworkers));

}

void ThreadEngine::runScheduler(){

  log->record(GreasyLog::devel, "ThreadEngine::runScheduler", "Entering...");

  // Dummy check: let's see if there is any worker...
  if (nworkers==0) {
    log->record(GreasyLog::error, "No workers found. Rerun greasy with more resources");
    return;
  }

  // initialize the task scheduler
  tbb::task_scheduler_init init(nworkers) ;

  log->record(GreasyLog::debug, "ThreadEngine::runScheduler", "Starting to launch tasks...");

  set<int>::iterator it;
  GreasyTask* gtask  = NULL;
  vector<GreasyTask*> runnableTasks;

  for ( it=validTasks.begin(); it!=validTasks.end(); it++ ) {
      gtask = taskMap[*it];
      GreasyLog::getInstance()->record(GreasyLog::debug, "ThreadEngine::runScheduler", "Task "+ toString(gtask->getTaskId())+" state is '"+ gtask->printTaskState() +"'");
      if ( gtask->isWaiting() ){
          GreasyLog::getInstance()->record(GreasyLog::debug, "ThreadEngine::runScheduler", "Scheduling task "+ toString(gtask->getTaskId()) );
          runnableTasks.push_back(gtask);
      }
  }

  // start counter
  globalTimer.start();

  tbb::parallel_do (runnableTasks.begin(), runnableTasks.end(),GreasyTBBTaskEngine(&taskMap,&validTasks,&revDepMap) );

  // end counter
  globalTimer.stop();

  log->record(GreasyLog::debug, "ThreadEngine::runScheduler", "All tasks lauched");

  log->record(GreasyLog::devel, "ThreadEngine::runScheduler", "Exiting...");
}

/*************************************************************************************/
/***  TBB-related classes
/*************************************************************************************/

/***********************************/
/***   GreasyTBBTaskEngine
/***********************************/

GreasyTBBTaskEngine::GreasyTBBTaskEngine(map<int,GreasyTask*> * taskMap_, set<int>* validTasks_ ,map<int,list<int> >*revDepMap_ )
    : taskMap(taskMap_), validTasks(validTasks_),revDepMap(revDepMap_)
{;}


void GreasyTBBTaskEngine::operator()( argument_type item, tbb::parallel_do_feeder<argument_type>& feed_it) const
{
    GreasyLog::getInstance()->record(GreasyLog::devel, "GreasyTBBTaskEngine::()", "Entering...");

    GreasyLog::getInstance()->record(GreasyLog::debug, "GreasyTBBTaskEngine::()", "Executing command: " + item->getCommand());

    item->setTaskState(GreasyTask::running);

    GreasyTimer timer;
    timer.reset();
    timer.start();
    int retcode = system(item->getCommand().c_str());
    timer.stop();

    item->setReturnCode(retcode);
    item->setElapsedTime( timer.secsElapsed() );

    // task is finished here ...
    if (!taskEpilogue(item, feed_it) )
    {
        // task ended Ok ...
        GreasyLog::getInstance()->record(GreasyLog::devel, "GreasyTBBTaskEngine::()", "Task "+  toString(item->getTaskId()) +" ended Ok"  );
    }else{
        // task failed ...
        GreasyLog::getInstance()->record(GreasyLog::devel, "GreasyTBBTaskEngine::()", "Task "+  toString(item->getTaskId()) +" failed"  );
    }

    GreasyLog::getInstance()->record(GreasyLog::devel, "GreasyTBBTaskEngine::()", "Exiting...");
}


bool GreasyTBBTaskEngine::taskEpilogue(argument_type gtask, tbb::parallel_do_feeder<argument_type>& feed_it ) const
{

    int maxRetries=0;
    bool retval = false;

    GreasyLog::getInstance()->record(GreasyLog::devel, "GreasyTBBTaskEngine::taskEpilogue", "Entering...");

    if ( GreasyConfig::getInstance()->keyExists("MaxRetries")) fromString(maxRetries, GreasyConfig::getInstance()->getValue("MaxRetries"));

    if (gtask->getReturnCode() != 0) {
      GreasyLog::getInstance()->record(GreasyLog::error,  "Task " + toString(gtask->getTaskId()) + " failed with exit code " + toString(gtask->getReturnCode()) +". Elapsed: " + GreasyTimer::secsToTime(gtask->getElapsedTime()));

      // Task failed, let's retry if we need to
      if ((maxRetries > 0) && (gtask->getRetries() < maxRetries)) {
        GreasyLog::getInstance()->record(GreasyLog::warning,  "Retry "+ toString(gtask->getRetries()) + "/" + toString(maxRetries) + " of task " + toString(gtask->getTaskId()));
        gtask->addRetryAttempt();

        // allocate task again...
        GreasyLog::getInstance()->record(GreasyLog::debug,  "Allocating again task " + toString(gtask->getTaskId()) + " retry attempt: " + toString(gtask->getRetries()) );
        feed_it.add(gtask);
        retval= true;

      } else {
        gtask->setTaskState(GreasyTask::failed);
        updateDependencies(gtask,feed_it);
      }
    } else {
      GreasyLog::getInstance()->record(GreasyLog::info,  "Task " + toString(gtask->getTaskId()) + " completed successfully. Elapsed: " +  GreasyTimer::secsToTime(gtask->getElapsedTime()));
      gtask->setTaskState(GreasyTask::completed);

      updateDependencies(gtask, feed_it);

    }

    GreasyLog::getInstance()->record(GreasyLog::devel, "GreasyTBBTaskEngine::taskEpilogue", "Exiting...");

    return retval;
}

void GreasyTBBTaskEngine::updateDependencies(argument_type child, tbb::parallel_do_feeder<argument_type>& feed_it) const
{

    int taskId, state;
    list<int>::iterator it;
    GreasyLog* log =  GreasyLog::getInstance();

    log->record(GreasyLog::devel, "GreasyTBBTaskEngine::updateDependencies", "Entering...");

    taskId = child->getTaskId();
    state  = child->getTaskState();

    log->record(GreasyLog::devel, "GreasyTBBTaskEngine::updateDependencies", "Inspecting reverse deps for task " + toString(taskId));

    if ( revDepMap->find(taskId) == revDepMap->end() ){
        log->record(GreasyLog::devel, "GreasyTBBTaskEngine::updateDependencies", "The task "+ toString(taskId) + " does not have any other dependendant task. No update done.");
        log->record(GreasyLog::devel, "GreasyTBBTaskEngine::updateDependencies", "Exiting...");
        return;
    }

    GreasyTask* dependant;
    for(it=(*revDepMap)[taskId].begin() ; it!=(*revDepMap)[taskId].end();it++ ) {

        // get the first dependant whose state is blocked
        dependant = (*taskMap)[*it];
        log->record(GreasyLog::devel, "GreasyTBBTaskEngine::updateDependencies", "State of dependant task " + toString(dependant->getTaskId()) + " is " +  dependant->printTaskState() );

      if (state == GreasyTask::completed) {
        log->record(GreasyLog::devel, "GreasyTBBTaskEngine::updateDependencies", "Remove dependency " + toString(taskId) + " from task " + toString(dependant->getTaskId()));
        dependant->removeDependency(taskId);
        if (!dependant->hasDependencies()) {
            log->record(GreasyLog::devel, "GreasyTBBTaskEngine::updateDependencies", "Dependant task "+ toString(dependant->getTaskId()) + " new state is: '"+ dependant->printTaskState()+"'");

            if (dependant->getTaskState() == GreasyTask::waiting ){
                GreasyLog::getInstance()->record(GreasyLog::debug,  "Allocating task " + toString(dependant->getTaskId())) ;
                feed_it.add(dependant);
            }

        } else {
            log->record(GreasyLog::devel, "GreasyTBBTaskEngine::updateDependencies", "The task still has dependencies, so leave its state '" + dependant->printTaskState() +"'" );
        }
      }
      else if ((state == GreasyTask::failed)||(state == GreasyTask::cancelled)) {
        log->record(GreasyLog::warning,  "Cancelling task " + toString(dependant->getTaskId()) + " because of task " + toString(taskId) + " failure");
        log->record(GreasyLog::devel, "GreasyTBBTaskEngine::updateDependencies", "Parent failed: cancelling task and removing it from blocked");
        dependant->setTaskState(GreasyTask::cancelled);

        updateDependencies(dependant,feed_it);
      }
    }

    log->record(GreasyLog::devel, "GreasyTBBTaskEngine::updateDependencies", "Exiting...");

}


