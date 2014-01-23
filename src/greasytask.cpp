#include "greasytask.h"
#include "greasyutils.h"
#include "greasyregex.h"

#include <iostream>
#include <vector>

GreasyTask::GreasyTask ( ) {
  
  initAttributes();
  
}

GreasyTask::GreasyTask ( int id, string cmd ) {

  initAttributes();
  taskId = id;
  command = cmd;
  
}

void GreasyTask::initAttributes ( ) {
  
  taskId = -1;
	taskNum = -1;
  command = "";
  dependencies.clear();
  taskState = GreasyTask::waiting;
  returnCode = 0;
  retries = 0;
  elapsed = 0;
  elapsedAcc = 0;
  
}

int GreasyTask::getTaskId ( )   {
  
  return taskId;
  
}

int GreasyTask::getTaskNum ( ) {

	return taskNum;

}

void GreasyTask::setTaskId ( int new_var )   {
  
  taskId = new_var;

}

void GreasyTask::setTaskNum ( int new_var ) {
	taskNum = new_var;
}

string GreasyTask::getCommand ( )   {
  
  return command;
  
}

void GreasyTask::setCommand ( string new_var )   {
  
  command = new_var;
  
}

int GreasyTask::getTaskState() {
  
  return taskState;
  
}

string GreasyTask::printTaskState(){

    string x = "Unknown";

    switch ( taskState ){
        case 0:
                x = "invalid";
                return x;
        case 1:
                x="blocked";
                return x;
        case 2:
                x="waiting";
                return x;
        case 3:
                x="running";
                return x;
        case 4:
                x="completed";
                return x;
        case 5:
                x="failed";
                return x;
        case 6:
                x="canceled";
                return x;
    default:
            return x;
    }
}


bool GreasyTask::isBlocked() {
 
  return (taskState==GreasyTask::blocked);
  
}

bool GreasyTask::isWaiting() {
 
  return (taskState == GreasyTask::waiting);
  
}

bool GreasyTask::isInvalid() {

  return ( taskState == GreasyTask::invalid );

}


void GreasyTask::setTaskState(TaskStates state) {
  
  taskState = state;
  
}

int GreasyTask::getReturnCode() {
  
  return returnCode; 
  
}

void GreasyTask::setReturnCode(int code) {
  
  returnCode = code;
  
}

unsigned long GreasyTask::getElapsedTime() {
  
  return elapsed;
  
}

unsigned long GreasyTask::getElapsedTimeAcc() {
  
  return elapsedAcc;
  
}

void GreasyTask::setElapsedTime(unsigned long et) {
  
  elapsed = et;
  elapsedAcc += elapsed;
  
}

string GreasyTask::getHostname() {
  
  return hostname;
  
}

void GreasyTask::setHostname(string h) {
  
  hostname = h;  
  
}

int GreasyTask::getRetries() {
  
  return retries;
  
}

void GreasyTask::addRetryAttempt() {
  
  retries++;
  
}

bool GreasyTask::hasDependencies() {
 
  return (!dependencies.empty());
  
}

list<int> GreasyTask::getDependencies(){
  
  return dependencies;
  
}
  
void GreasyTask::addDependency(int parentTask) {
  
  if ((parentTask != taskId)&&(parentTask>0))
    dependencies.push_back(parentTask);
  
  if (isWaiting()) setTaskState(GreasyTask::blocked);
  
}

void GreasyTask::removeDependency(int parentTask){
  
  dependencies.remove(parentTask);
  if ((dependencies.size() == 0)
    &&(taskState == GreasyTask::blocked)) {
      taskState = GreasyTask::waiting;
  }
  
}

bool GreasyTask::addDependencies(string deps) {
  
  bool valid = true;
  string dep;
  int parent1,parent2, i;
  vector<string> matches;
  vector<string> tokens;
  vector<string>::iterator it;
  
  //Skip empty dependency string...
  if (deps.empty()) return valid;
  dep = GreasyRegex::match(deps,"^([[:blank:]])+$");
  if(!dep.empty()) return valid;   
  
  // Split the dependency string into tokens using "," as the separator.
  tokens = split(deps,',');

  // Treat each one of the tokens individually
  for ( it=tokens.begin() ; it < tokens.end(); it++ ){

    // Basic dependency [1] or [12]
    dep = GreasyRegex::match(*it,"^[[:blank:]]*([1-9][0-9]*)[[:blank:]]*$");	
    if(!dep.empty()) {
      addDependency(fromString(parent1,dep));
      continue;
    }
    
    // Relative previous dependency [-1] or [-12]
    dep = GreasyRegex::match(*it,"^[[:blank:]]*-([1-9][0-9]*)[[:blank:]]*$");	
    if(!dep.empty()) {	  
      addDependency(taskId-fromString(parent1,dep));
      continue;
    }

/*  // Not supported at this time due to the dependency complexity
    // Relative forward dependency [+1] or [+12]
    dep = GreasyRegex::match(*it,"^[[:blank:]]*\\+([1-9][0-9]*)[[:blank:]]*$");	
    if(!dep.empty()) {	  
      addDependency(taskId+fromString(parent1,dep));
      continue;
    }
*/

    // Range dependency [1-2] [2-1] or [1-1]
    string rangePattern = "^[[:blank:]]*([1-9][0-9]*)[[:blank:]]*-[[:blank:]]*([1-9][0-9]*)[[:blank:]]*$";
    GreasyRegex rangeReg = GreasyRegex(rangePattern);
      
    if (rangeReg.multipleMatch(*it,matches) > 0) {
      parent1 = fromString(parent1,matches.front());
      parent2 = fromString(parent2,matches.back());
      
      // [1-2]
      if (parent1<parent2) {
	for ( i=parent1;i<=parent2;i++) {
	  addDependency(i);
	}
      // [2-1]	   
      } else if (parent1>parent2) {
	for ( i=parent2;i<=parent1;i++) {
	  addDependency(i);
	}
      // [1-1]
      } else {
	addDependency(parent1);
      }
      
      matches.clear();
      continue;
    }
    
    // If we reach this point, the token is not valid.
    valid=false;
    taskState=GreasyTask::invalid;
    break;
	
  }
  dependencies.sort();
  dependencies.unique();
  return valid;
  
}

string GreasyTask::dump() {
  
  string out = "";
  list<int>::iterator it;
  
  string stateDesc[]={"invalid","blocked", "waiting","running","completed", "failed","cancelled"};
  
  out+="Taskid: " + toString(taskId) +"\n";
  out+="State: " + stateDesc[taskState] +"\n";
  out+="Command: " + command +"\n";
  out+="Dependencies: [# " + dumpDependencies() + " #]\n";
/*
  out+="Dependencies: [# ";
  for (it=dependencies.begin();it!=dependencies.end();it++) {
    if (it!=dependencies.begin()) out+=",";
    out+=toString(*it);
  }
  out+=" #]\n";
*/
  return out;
  
}

string GreasyTask::dumpDependencies(){

  string out = "";
  list<int>::iterator it;
  int last = 0;
  int left = 1;
  
  //Make sure list is sorted
  dependencies.sort();
  dependencies.unique(); 
  
  if (!hasDependencies()) return out;

  // Walk through the dependency list always comparing with the previous value
  // to properly write intervals when possible.
  for (it = dependencies.begin(); it != dependencies.end(); it++) {
    if (*it == last + 1){
      last = *it;
    } else {
      if (last > 0) {
	if (left==last) {
	  out += toString(last);
	} else if (left + 1 == last) {
	  out += toString(left) + "," + toString(last);
	} else {
	  out += toString(left) + "-" + toString(last);
	}
	out += ",";
      }
      left = *it;
      last = *it;
    }
  }
  
  // Make last addition to the list
  if (last > 0) {
    if (left==last) {
      out += toString(last);
    } else if (left + 1 == last) {
      out += toString(left) + "," + toString(last);
    } else {
      out += toString(left) + "-" + toString(last);
    } 
  }
    
  return out;
  
}
