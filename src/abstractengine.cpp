
#ifdef MPI_ENGINE
    #include "mpiengine.h"
#endif
#include "abstractengine.h"
#include "greasyregex.h"
#include "basicengine.h"


#ifdef SLURM_ENGINE
    #include "slurmengine.h"
#endif

#ifdef THREAD_ENGINE
    #include "threadengine.h"
#endif

#include <fstream>
#include <cstdlib>
#include <time.h>

AbstractEngine* AbstractEngineFactory::getAbstractEngineInstance(const string& filename, const string& type ) {
  
 GreasyLog::getInstance()->record(GreasyLog::devel, "AbstractEngineInstance::getAbstractEngineInstance", "Entering with type: '" + type+"'");

  if (type == "basic" || type.empty() ){
 	GreasyLog::getInstance()->record(GreasyLog::devel, "AbstractEngineInstance::getAbstractEngineInstance", "Creating engine type: 'basic'");
	return new BasicEngine(filename);
  }

  #ifdef MPI_ENGINE
  if (type == "mpi"){
 	GreasyLog::getInstance()->record(GreasyLog::devel, "AbstractEngineInstance::getAbstractEngineInstance", "Creating engine type: 'mpi'");
        return new MPIEngine(filename);
	}
  #endif

  #ifdef SLURM_ENGINE
  if (type == "slurm"){
 	GreasyLog::getInstance()->record(GreasyLog::devel, "AbstractEngineInstance::getAbstractEngineInstance", "Creating engine type: 'slurm'");
        return new SlurmEngine(filename);
	}
  #endif

  #ifdef THREAD_ENGINE
  if (type == "thread"){ 
 	GreasyLog::getInstance()->record(GreasyLog::devel, "AbstractEngineInstance::getAbstractEngineInstance", "Creating engine type: 'thread'");
        return new ThreadEngine(filename);
	}
  #endif

  GreasyLog::getInstance()->record(GreasyLog::error, "Wrong engine type requested!!! This version does not supports '" + type + "' scheduler");

  return NULL;
}

AbstractEngine::AbstractEngine ( string filename ) {
  
  taskFile = filename;
  //Check for a relative path
  if ((GreasyRegex::match(filename,"^[:blank:]*/(.*)$")==""))
    taskFile = getWorkingDir() + filename;
  
  vector<string>path = split(filename,'/');
  restartFile = getWorkingDir() + path.back() + ".rst";
  
  log = GreasyLog::getInstance();
  config = GreasyConfig::getInstance();
  engineType="abstract";
  nworkers = 0;
  fileErrors= false;
  ready = false;
  if (config->keyExists("strictCheck")&&(config->getValue("strictCheck")=="yes")) {
   strictChecking = true;  
  } else {
   strictChecking = false; 
  }
  
}

bool AbstractEngine::isReady() {
 
  return ready;
  
}

void AbstractEngine::init() {
  
  log->record(GreasyLog::devel, "AbstractEngine::init", "Entering...");
  
  log->record(GreasyLog::silent,"Start greasing " + taskFile);
  parseTaskFile();
  checkDependencies();
 	log->record(GreasyLog::info, "File with " + toString(validTasks.size()) + " correct Tasks"); 
  if ((validTasks.size() != taskMap.size())&&(!strictChecking)) {
    log->record(GreasyLog::warning,  "Invalid tasks found. Greasy will ignore them");  
  }
  
  // Only set the number of workers if any subclass has not changed the value before.
  if (nworkers == 0){
    // Set the number of workers
    if (config->keyExists("NWorkers")) {
      log->record(GreasyLog::devel, "Using defined NWorkers: " + toString(nworkers));
      nworkers = fromString(nworkers, config->getValue("NWorkers"));
    } else { 
      getDefaultNWorkers();
      log->record(GreasyLog::warning, "Falling back to the default number of workers " + toString(nworkers));
      log->record(GreasyLog::warning, "Consider setting environment variable GREASY_NWORKERS to the desired cpus to use");
    }
  }
  
  if (!fileErrors) {
    if(nworkers>0) {  
      ready = true;
      log->record(GreasyLog::info,  toUpper(engineType) + " engine is ready to run with "
				  + toString(nworkers) + " workers"); 
    } else {
      log->record(GreasyLog::error,  toUpper(engineType) + " engine has no workers. Please check your greasy setup");
    }
  }
  log->record(GreasyLog::devel,  "Configuration contents:\n\n" + config->printContents()); 
  log->record(GreasyLog::devel,  "End of configuration contents"); 
  
  log->record(GreasyLog::devel, "AbstractEngine::init", "Exiting...");
  
}

void AbstractEngine::finalize() {

  log->record(GreasyLog::devel, "AbstractEngine::finalize", "Entering...");
  
  log->record(GreasyLog::info, toUpper(engineType) + " engine finished");  
  
  globalTimer.stop();
  
  buildFinalSummary();
  
  map<int,GreasyTask*>::iterator it;
  for (it=taskMap.begin();it!=taskMap.end(); it++) {
    if (it->second) delete(it->second);
  }
  
  log->record(GreasyLog::silent,"Finished greasing " + taskFile);
  
  log->record(GreasyLog::devel, "AbstractEngine::finalize", "Exiting...");
}

void AbstractEngine::parseTaskFile() {
  
  log->record(GreasyLog::devel, "AbstractEngine::parseTaskFile", "Entering...");
  ifstream myfile(taskFile.c_str());
  
  string blankLineP= "^([[:blank:]]*)$";
  string commentLineP = "^[[:blank:]]*([#]).*$";
  string TaskLineWithDepsP = "^[[:blank:]]*([[][#]).*$";
  string basicTaskLineP = "^[[:blank:]]*(.*)$";
  string depTaskLineP = "^[[:blank:]]*[[]([#].*[#])[]][[:blank:]]*(.*)$";
  string depP="^([0-9, -]*)$";
  

  string line;
  int taskId=0;
	int taskNum=0;
  vector<string> matches;
  string dependencies="";
  string command="";

  if (myfile.is_open()) {
    log->record(GreasyLog::debug, "Reading tasks");
    // Read the task file
    while (!myfile.eof()){
      taskId++;
      getline (myfile,line);
      
      // Skip blank lines and comments
      if(line=="") continue;
      if(GreasyRegex::match(line,blankLineP)!="") continue;
      if(GreasyRegex::match(line,commentLineP)!="") continue;

      taskNum++;
      taskMap[taskId]=new GreasyTask(taskId,"");
			taskMap[taskId]->setTaskNum(taskNum);

      // Check line syntax
      if(GreasyRegex::match(line,TaskLineWithDepsP)!="") {
	log->record(GreasyLog::devel, "line "+toString(taskId), "Working as line with deps");
	//line should have deps
	GreasyRegex entryReg = GreasyRegex(depTaskLineP);
	if(entryReg.multipleMatch(line,matches) == 3) {
	  // Remove leading and trailing brakets from string #x# -> x
	  log->record(GreasyLog::devel, "line "+toString(taskId), "Correct closing of dep brackets");
	  dependencies = matches[1].substr(1,matches[1].size()-2);
	  command = matches[2];
	  if (command=="") { 
	    recordInvalidTask(taskId);
	  } else {
	    //Let's see if syntax is correct inside dependency brackets
	    if (dependencies == "" || GreasyRegex::match(dependencies,depP)!="") {
	      log->record(GreasyLog::devel, "line "+toString(taskId), "Correct character content inside dep brackets");
	      taskMap[taskId]->setCommand(command);
	      if (taskMap[taskId]->addDependencies(dependencies)) {
		validTasks.insert(taskId);
	      }      
	    } else {
	      log->record(GreasyLog::devel, "line "+toString(taskId), "deps: "+dependencies);
	      recordInvalidTask(taskId);  
	    }
	  }  
	} else {
	  recordInvalidTask(taskId);  
	}
      } else {
	//line has no deps
	log->record(GreasyLog::devel, "line "+toString(taskId), "Working as line with no deps");
	command = GreasyRegex::match(line,basicTaskLineP);
	if (command!="") {
	  taskMap[taskId]->setCommand(command);
	  validTasks.insert(taskId);
	} else {
	  recordInvalidTask(taskId);  
	}
      }
      
      matches.clear();
      dependencies.clear();
      command.clear();
    }
    myfile.close();
    log->record(GreasyLog::debug, "Tasks loaded");
  } else {
    log->record(GreasyLog::error,  "Could not read task file " + taskFile);
    fileErrors=true;
  }
  
  log->record(GreasyLog::devel, "AbstractEngine::parseTaskFile", "Exiting...");
  
}

void AbstractEngine::checkDependencies() {
  
  map<int,GreasyTask*>::iterator it;
 
  log->record(GreasyLog::devel, "AbstractEngine::checkDependencies", "Entering...");
  
  // For each task, check if its dependencies are valid and fill the reverse
  // dependency map.
  for (it=taskMap.begin();it!=taskMap.end(); it++) {

    GreasyTask *task = it->second;

    if ( task->isInvalid() ) continue;

    list<int> deps = task->getDependencies();
    list<int>::iterator dep;
    for (dep=deps.begin();dep!=deps.end();dep++) {
      if ((taskMap.find(*dep)!=taskMap.end())
	&& (taskMap[*dep]->getTaskId() < task->getTaskId())
	&&(validTasks.find(*dep)!=validTasks.end())) {
	//The task is valid
	revDepMap[*dep].push_back(it->first);
      } else {
        //The task is not valid
	it->second->setTaskState(GreasyTask::invalid);
	validTasks.erase(it->first);

	if (strictChecking) {
	  fileErrors = true;
	  log->record(GreasyLog::error, "Dependency " + toString(*dep) + " of task " +
		toString(it->first) + " is not valid");
	} else {
	  // don't remove dependency from task but keep going
	  log->record(GreasyLog::warning, "Dependency " + toString(*dep) + " of task " +
		  toString(it->first) + " is not valid.");
	}
	
      }
    }
  }
  
  log->record(GreasyLog::devel, "AbstractEngine::checkDependencies", "Exiting...");
  
}

void AbstractEngine::recordInvalidTask(int taskId) {
  
  if (strictChecking) {
    log->record(GreasyLog::error,  "Task " + toString(taskId) +
			  " does not seem to be correct");
    fileErrors=true;
  } else {
    log->record(GreasyLog::warning,  "Task " + toString(taskId) +
			  " does not seem to be correct. Skipping...");

    if (taskMap[taskId]!= NULL )
        taskMap[taskId]->setTaskState( GreasyTask::invalid );
  }
  
}

void AbstractEngine::writeRestartFile() {
 
  GreasyTask *task;
  map<int,GreasyTask*>::iterator it;
  list<int> dependants;
  set<GreasyTask*> invalidTasks;
  list<int>::iterator lit;
  set<GreasyTask*>::iterator sit;
  int nindex;
  ofstream rstfile( restartFile.c_str(), ios_base::out);

  
  log->record(GreasyLog::devel, "AbstractEngine::writeRestartFile", "Entering...");

  if (!rstfile.is_open()) {
      log->record(GreasyLog::error,  "Could not create restart file " + restartFile);
      return;
  }
  
  log->record(GreasyLog::info, "Creating restart file " + restartFile + "...");
  
  string logFile = "Standard Error";
  if (config->keyExists("LogFile")&&((config->getValue("LogFile") != ""))) {
    logFile = config->getValue("LogFile");
    //Check for a relative path
    if ((GreasyRegex::match(logFile,"^[:blank:]*/(.*)$")==""))
      logFile = getWorkingDir() + logFile;
  }
  
  rstfile << "# " << endl;
  rstfile << "# Greasy restart file generated at "<< GreasyTimer::now() << endl;
  rstfile << "# Original task file: " << getWorkingDir() << "/" << taskFile << endl;
  rstfile << "# Log file: " << logFile  << endl;
  rstfile << "# " << endl;
  rstfile << endl;
  
  nindex = 7;
  
  for (it=taskMap.begin();it!=taskMap.end(); it++) {
    task = it->second;
    
    // Completed tasks will not be recorded in the restart file
    if (task->getTaskState() == GreasyTask::completed) continue;
    
    // Invalid tasks will be treated at the end
    if (task->getTaskState() == GreasyTask::invalid) {
      invalidTasks.insert(task);
      continue;
    }
    
    if (task->getTaskState() == GreasyTask::failed) {
      rstfile << "# Warning: Task " << task->getTaskId() << " failed" << endl; 
      nindex++;
    }
    
    if (task->getTaskState() == GreasyTask::cancelled) {
      rstfile << "# Warning: Task " << task->getTaskId() << " was cancelled due to a dependency failure" << endl; 
      nindex++;
    }
    
    // Write the task in the restart with its dependencies if any
    if (task->hasDependencies()) {
      rstfile << "[# " << task->dumpDependencies() << " #] ";
    }
    rstfile << ((*it).second)->getCommand() << endl;
    
    //Update indexes of dependencies to the new lines in the restart
    dependants = revDepMap[task->getTaskId()];
    if (!dependants.empty()) {
      for(lit=dependants.begin();lit!=dependants.end();lit++) {
	taskMap[*lit]->removeDependency(task->getTaskId());
	taskMap[*lit]->addDependency(nindex);
      }
    }
    
    nindex++;

  }
  
  if (!invalidTasks.empty()) {
    
    rstfile << endl << "# Invalid tasks were found. Check these lines on " << taskFile << ": " << endl << "# ";
    bool first = true;
    for (sit=invalidTasks.begin();sit!=invalidTasks.end(); sit++) {    
      task = *sit;
      if (first) {
	rstfile << toString(task->getTaskId());
	first = false;
      } else {
	 rstfile << ", " << toString(task->getTaskId());
      }
    }
    rstfile << endl;
  }
  
  
  rstfile << endl << "# End of restart file" << endl;

  // close restart file;
  rstfile.close();
  log->record(GreasyLog::info, "Restart file created");
  
  log->record(GreasyLog::devel, "AbstractEngine::writeRestartFile", "Exiting...");
  
}

void AbstractEngine::buildFinalSummary() {
  
  int completed = 0;
  int failed = 0;
  int cancelled = 0;
  int invalid = 0;
  int total = taskMap.size();
  map<int,GreasyTask*>::iterator it;
  GreasyTask* task;
  unsigned long usedTime = 0;
  float rup = 0;

  log->record(GreasyLog::devel, "AbstractEngine::buildFinalSummary", "Entering...");
  // Compute final stats
  for (it=taskMap.begin();it!=taskMap.end(); it++) {
    task = it->second;
    usedTime += task->getElapsedTimeAcc();
    switch(task->getTaskState()) {
      case GreasyTask::invalid:
	invalid++;
	break;
      case GreasyTask::completed:
	completed++;
	break;
      case GreasyTask::failed:
	failed++;
	break;
      case GreasyTask::cancelled:
	cancelled++;
	break;
    }
  }
  
  // Compute the resource utilization %
  if (globalTimer.secsElapsed()>0&&nworkers>0) {
    int aux = usedTime*10000 / (globalTimer.secsElapsed()*nworkers);
    rup = (float)aux/(float)100;
  }
  
  log->record(GreasyLog::info,"Summary of " + toString(total) + " tasks: " + toString(completed) +
			      " OK, "+toString(failed) + " FAILED, " + toString(cancelled) + 
			      " CANCELLED, " + toString(invalid) + " INVALID.");
  log->record(GreasyLog::info,"Total time: " + globalTimer.getElapsed());
  log->record(GreasyLog::info,"Resource Utilization: " + toString(rup) +"%" );
  
  // Write a restart if we find not completed tasks
  if (failed + cancelled + invalid > 0) writeRestartFile();
  
  log->record(GreasyLog::devel, "AbstractEngine::buildFinalSummary", "Exiting...");
  
}

string AbstractEngine::dumpTaskMap() {
  
  log->record(GreasyLog::devel, "AbstractEngine::dumpTasks", "Entering...");
  map<int,GreasyTask*>::iterator it;
  string s = "\nList of tasks:\n===============\n";

  for (it=taskMap.begin();it!=taskMap.end(); it++) {
    s+=it->second->dump()+"\n";
  }
  s+="\n";
  
  log->record(GreasyLog::devel, "AbstractEngine::dumpTasks", "Exiting...");
  return s;
}

void AbstractEngine::dumpTasks() {
  
   log->record(GreasyLog::devel, dumpTaskMap());
   
}


