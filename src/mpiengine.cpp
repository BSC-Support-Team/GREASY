#include "mpiengine.h"
#include <csignal>
#include <cstdlib>
#include <cstring>



typedef struct {
    int workerStatus;
    int retcode;
    unsigned long elapsed;
    char hostname[MPI_MAX_PROCESSOR_NAME] ;
} reportStruct;


MPIEngine::MPIEngine ( const string& filename) : AbstractSchedulerEngine(filename){
  
  engineType="mpi";
  workerId = -1;
  
}

bool MPIEngine::isMaster() { 
  
  return (workerId==0); 
  
}

bool MPIEngine::isWorker() { 
  
  return (workerId>0); 
  
}

void MPIEngine::init() {
  
  //Dummy arguments for MPI::Init
  int argc=0;
  char **argv;
  int size = MPI_MAX_PROCESSOR_NAME;
  char workerHost[MPI_MAX_PROCESSOR_NAME];
  
  log->record(GreasyLog::devel, "MPIEngine::init", "Entering...");
  
  MPI::Init(argc, argv);
  workerId = MPI::COMM_WORLD.Get_rank();
  nworkers = MPI::COMM_WORLD.Get_size();

 // MPI::Datatype type = MPI::CHAR.Create_contiguous(size);
 // type.Commit();
 // MPI_Get_processor_name(workerHost,&size);

  
  // We don't count the master
  nworkers--;
  
  // Only the master has to perform the initialization of tasks.
  if (isMaster()) { 
//  	log->record(GreasyLog::info, "Running with " +toString(nworkers) + " workers");
    AbstractSchedulerEngine::init();
		MPIEngine::executionSummary();
  } else {
    
    // Disable signal handling for workers.
    // We only want to have the master in charge of the restarts and messages.
    signal(SIGTERM, SIG_DFL);
    signal(SIGINT, SIG_DFL);
    
    //Get the name of the node of this worker
    int size = MPI_MAX_PROCESSOR_NAME;
    MPI_Get_processor_name(hostname,&size);
    
    //Worker at this point is ready.
    ready = true;
  }
  
  log->record(GreasyLog::devel, "MPIEngine::init", "Exiting...");
  
}

void MPIEngine::finalize() {

  if (isMaster()) { 
    // At this point all tasks have finished and all nodes are free
    // Let's fire the workers!
    fireWorkers();
    // The master has to do some cleanup.
    AbstractSchedulerEngine::finalize();
  }
  
  MPI::Finalize();
  
}

void MPIEngine::run() {
  
  log->record(GreasyLog::devel, "MPIEngine::run", "Entering...");
  
  if (isReady()) {
    if (isMaster()) runMaster();
    else if (isWorker()) runWorker();
    else log->record(GreasyLog::error,  "Could not run MPI engine");
  }

  log->record(GreasyLog::devel, "MPIEngine::run", "Exiting...");

}

/*
 * Master Methods
 * 
 */

void MPIEngine::runMaster() {
  
  log->record(GreasyLog::devel, "MPIEngine::runMaster", "Entering...");
  
  runScheduler();
  
  log->record(GreasyLog::devel, "MPIEngine::runMaster", "Exiting...");
  
}

void MPIEngine::writeRestartFile() {
 
  if (isMaster()) AbstractSchedulerEngine::writeRestartFile();
  
}

void MPIEngine::allocate(GreasyTask* task) {
  
  int worker, cmdSize;
  const char *cmd;
  
  log->record(GreasyLog::devel, "MPIEngine::allocate", "Entering...");
   
  worker = freeWorkers.front();
  freeWorkers.pop();
  
  log->record(GreasyLog::info,  "Allocating task " + toString(task->getTaskNum()) + " located in line "+ toString(task->getTaskId()) + " to Worker " + toString(worker));
  
  log->record(GreasyLog::debug, "MPIEngine::allocate", "Sending task " + toString(task->getTaskNum()) + " located in line "+ toString(task->getTaskId()) + " to worker " + toString(worker));
  taskAssignation[worker] = task->getTaskId();
  task->setTaskState(GreasyTask::running);
  
  // Send the command size and the actual command
  cmdSize = task->getCommand().size()+1;
  cmd = task->getCommand().c_str();
  MPI::COMM_WORLD.Send( &cmdSize, 1, MPI::INT, worker, 0);
  MPI::COMM_WORLD.Send((void*)cmd, cmdSize, MPI::CHAR, worker , 0);
  
  log->record(GreasyLog::devel, "MPIEngine::allocate", "Exiting...");
  
}

void MPIEngine::waitForAnyWorker() {
  
  int retcode;
  int worker;
  GreasyTask* task = NULL;
  MPI::Status status;
  reportStruct report; 
  
  log->record(GreasyLog::devel, "MPIEngine::waitForAnyWorker", "Entering...");
  
  log->record(GreasyLog::debug,  "Waiting for any task to complete...");
  MPI::COMM_WORLD.Recv ( &report, sizeof(reportStruct), MPI::CHAR, MPI::ANY_SOURCE, MPI::ANY_TAG, status );
  
  retcode = report.retcode;
  worker = status.Get_source ();
  task = taskMap[taskAssignation[worker]];
    
  // Push worker to the free workers queue again
  freeWorkers.push(worker);
  
  // Update task info with the report
  task->setElapsedTime(report.elapsed);
  task->setReturnCode(report.retcode);
  task->setHostname(string(report.hostname));

  taskEpilogue(task);
  
  log->record(GreasyLog::devel, "MPIEngine::waitForAnyWorker", "Exiting...");
  
}

void MPIEngine::fireWorkers() {
  
  int fired = -1;
  int worker;
  
  log->record(GreasyLog::devel, "MPIEngine::fireWorkers", "Entering...");
  
  log->record(GreasyLog::debug,  "Sending fire comand to all workers...");
  for(int worker=1;worker<=nworkers;worker++) {
    MPI::COMM_WORLD.Send( &fired, 1, MPI::INT, worker, 0);
  } 
  
  log->record(GreasyLog::devel, "MPIEngine::fireWorkers", "Exiting...");
  
}

void MPIEngine::dumpTasks() {

  if (isMaster()) {
    log->record(GreasyLog::devel, dumpTaskMap());
  }

}

void MPIEngine::executionSummary() {

  char *pwd=NULL;
	char *job_id;
	char *n_nodes=NULL;
	pwd=get_current_dir_name();
	log->record(GreasyLog::info, "Current Working Dir " + toString(pwd));

#ifdef LSF
	char command_nodes[250];
	char buf[10];
	sprintf(command_nodes,"cat $LSB_DJOB_HOSTFILE | uniq | wc -l | tr \"\n\" \" \"");
	FILE *fp = popen(command_nodes,"r");
  n_nodes=(char*)malloc(10*sizeof(char));
	while (fgets(buf, 10, fp)) {
		strcpy(n_nodes,buf);
	}
	if(n_nodes) log->record(GreasyLog::info, "Run on " + toString(n_nodes)+ "nodes");
  job_id=getenv(JOBID);	

#elif defined(SLURM)
	n_nodes=getenv("SLURM_NNODES");
	strcat(n_nodes," ");
	if(n_nodes) log->record(GreasyLog::info, "Run on " + toString(n_nodes)+ "nodes");
  job_id=getenv(JOBID);	
#endif

  //log->record(GreasyLog::info, toString(command_nodes));
	if(job_id) log->record(GreasyLog::info, "Job ID " + toString(job_id));


}

/*
 * Worker Methods
 * 
 */

void MPIEngine::runWorker() {
  
  int cmdSize = 0;
  char *cmd = NULL;
  MPI::Status status;
  int retcode = -1;
  int err;
  reportStruct report;
  GreasyTimer timer;
  
  // Worker code  
  if (!isWorker()) return;
  
  log->record(GreasyLog::devel, "MPIEngine::runWorker("+toString(workerId)+")", "Entering...");
  
  // Main worker loop 
  while(1) {
    report.workerStatus = 1;
    report.retcode = -1;
    report.elapsed = 0;
    strcpy(report.hostname,hostname);
    // Receive command size, including '\0'
    try {
      MPI::COMM_WORLD.Recv ( &cmdSize, 1, MPI::INT, 0, 0, status );
    } catch (MPI::Exception e) {
      log->record(GreasyLog::error, "WORKER("+toString(workerId)+")", "Error receiving command size: "+toString(e.Get_error_string()));
      report.workerStatus = 0;
      MPI::COMM_WORLD.Send( &report, sizeof(report), MPI::CHAR, 0, 0);
      continue;
    }
    
    // A negative size means the worker is fired
    if (cmdSize < 0) {
      log->record(GreasyLog::debug, toString(workerId), "Received fired signal!");  
      break;
    }
    
    // Allocate the memory for the command
    cmd = (char*) malloc(cmdSize*sizeof(char));
    if(!cmd) {
      log->record(GreasyLog::error, "WORKER("+toString(workerId)+")", "Could not allocate memory");
      report.workerStatus = 0;
      MPI::COMM_WORLD.Send( &report, sizeof(report), MPI::CHAR, 0, 0);
      break;
    }
    
    // Receive the command
    try {
      MPI::COMM_WORLD.Recv ( cmd, cmdSize, MPI::CHAR, 0, 0, status );
    } catch (MPI::Exception e) {
      log->record(GreasyLog::error, "WORKER("+toString(workerId)+")", "Error receiving command: "+toString(e.Get_error_string()));
      report.workerStatus = 0;
      MPI::COMM_WORLD.Send( &report, sizeof(report), MPI::CHAR, 0, 0);
      continue;
    }

    log->record(GreasyLog::debug, toString(workerId), "Running task " + toString(cmd));

	  int size = MPI_MAX_PROCESSOR_NAME;
	  char workerHost[MPI_MAX_PROCESSOR_NAME];
	  MPI_Get_processor_name(workerHost,&size);

		log->record(GreasyLog::info, "Worker " +toString(workerId) + " on node " +  toString(workerHost));

    // Execute the command
    timer.reset();
    timer.start();
    retcode = system(cmd);    
    timer.stop();
    
    report.retcode = retcode;
    report.elapsed = timer.secsElapsed();
    
    // Report to the master the end of the task
    log->record(GreasyLog::debug, toString(workerId), "Task finished with retcode (" + toString(retcode) + "). Elapsed: " + GreasyTimer::secsToTime(report.elapsed));
    MPI::COMM_WORLD.Send( &report, sizeof(report), MPI::CHAR, 0, 0);
    
    if (cmd) free(cmd);
  }
  
    
  log->record(GreasyLog::devel, "MPIEngine::runWorker("+toString(workerId)+")", "Exiting...");
  
}
