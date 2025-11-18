#include "global.h"

extern void getParams();
extern void displayCacheBlock(int, int);
extern char * h(int);
extern int  getNextRef(FILE *, struct tracerecord *);
extern void insertQueue(int,  struct genericQueueEntry *);
extern void * getFromQueue(int);
extern void makeQueue(int);

// Default System Parameter Values
int NUM_PROCESSORS = 1;
int TRACE=  FALSE;
int  MAXDELAY = 0;

int MEM[TOTALSIZE]; // Physical Memory
struct cacheblock CACHE[MAX_NUM_PROCESSORS][CACHESIZE];  // Cache

// Synchronization Variables
SEMAPHORE *sem_memreq[MAX_NUM_PROCESSORS];
SEMAPHORE *sem_memdone[MAX_NUM_PROCESSORS];
SEMAPHORE *sem_bussnoop[MAX_NUM_PROCESSORS], *sem_bussnoopdone[MAX_NUM_PROCESSORS];

//   Bus Arbitration Signals
int BUS_REQUEST[MAX_NUM_PROCESSORS];  // Bus Request
int BUS_GRANT[MAX_NUM_PROCESSORS];  // Bus Grant
int BUS_RELEASE; // Bus Released
int PFlag, PBit[MAX_NUM_PROCESSORS];  // Presence Flag
int OFlag, OBit[MAX_NUM_PROCESSORS]; // Owner flag
int CACHE_READY;
int CACHE_TRANSFER;
struct busrec BROADCAST_CMD;   // Broadcast request

// Yacsim Processes
PROCESS *proccntrl, *memcntrl, *buscntrl, *bussnooper, *cachecntrl, *memwritecntrl;


// Statistics
int cache_write_hits[MAX_NUM_PROCESSORS], cache_write_misses[MAX_NUM_PROCESSORS];
int cache_read_hits[MAX_NUM_PROCESSORS], cache_read_misses[MAX_NUM_PROCESSORS];
int cache_upgrades[MAX_NUM_PROCESSORS], cache_writebacks[MAX_NUM_PROCESSORS];
int  silent_upgrades[MAX_NUM_PROCESSORS], converted_cache_upgrades[MAX_NUM_PROCESSORS];
int numCacheToCacheTransfer[MAX_NUM_PROCESSORS], numMemToCacheTransfer[MAX_NUM_PROCESSORS];
int numThreadsCompleted = 0;


FILE *fp[MAX_NUM_PROCESSORS];  // Trace file pointer for each processor
int VABaseAddress;  // Virtual Base Address of Arrayy (Addresses in Trace File are virtual)

void cleanUp()  {
 int i;
 int totC2C = 0, totM2C = 0;

 printf("Simulation ended  at %5.2f\n",GetSimTime());

 printf(" \n ----------  Processor Level Statistics -----------------\n");
for (i=0; i < NUM_PROCESSORS; i++) {
  printf("\nProcessor: %d\tREAD HITS: %d\tREAD MISSES: %d\tWRITE HITS: %d\tUPGRADES: %d\t SILENT UPGRADES: %d\tCONVERTED UPGRADES: %d\tWRITE MISSES: %d\tWRITEBACKS: %d\tC2C: %d\tM2C: %d\tTime: %5.2f\n", i, cache_read_hits[i], cache_read_misses[i], cache_write_hits[i],   cache_upgrades[i], silent_upgrades[i], converted_cache_upgrades[i], cache_write_misses[i],cache_writebacks[i],numCacheToCacheTransfer[i], numMemToCacheTransfer[i], GetSimTime());
  totC2C += numCacheToCacheTransfer[i];
  totM2C += numMemToCacheTransfer[i];
 }
 printf("Total C2C: %d  Total M2C: %d\n",  totC2C, totM2C); 
}

void   createProcesses() {
  void Processor(), FrontEndCacheController(), BusSnooper(), BusArbiter();
    int i;

  for (i=0; i < NUM_PROCESSORS; i++) 
    makeQueue(MEM_CONTROLLER_QUEUE+i);
  
  // Create a Front End Cache  Controller  for each processor */
  for (i=0; i < NUM_PROCESSORS; i++) {
    memcntrl = NewProcess("memcntrl",FrontEndCacheController,0);
    ActivitySetArg(memcntrl,NULL,i);
    ActivitySchedTime(memcntrl,0.00000,INDEPENDENT);
  }
  printf("Done Creating FrontEndCacheControllers \n");


// Create a Bus Snooper for each processor
  for (i=0; i < NUM_PROCESSORS; i++) {
    bussnooper = NewProcess("bussnooper",BusSnooper,0);
    ActivitySetArg(bussnooper,NULL,i);
    ActivitySchedTime(bussnooper,0.000005,INDEPENDENT);
  }
  printf("Done Creating Bus Snoopers\n");


  // Create a Bus Arbiter
  buscntrl = NewProcess("buscntrl",BusArbiter,0);
  ActivitySetArg(buscntrl,NULL,1);
  ActivitySchedTime(buscntrl,0.00000,INDEPENDENT);
  printf("Done Creating Bus Arbiter  Process\n");
  
  
// Create a process to model activities of  each processor 
  for (i=0; i < NUM_PROCESSORS; i++){
    proccntrl = NewProcess("proccntrl",Processor,0);
    ActivitySetArg(proccntrl,NULL,i);
    ActivitySchedTime(proccntrl,STAGGER_DELAY*i,INDEPENDENT);
  }
  printf("Done Creating Processors\n");
  }


void initCache() {
  int i, j, k;  
// Initialize all cache blocks  to the Invalid State
  for (i=0; i < NUM_PROCESSORS; i++) {
    for (j=0; j < CACHESIZE; j++) {
      CACHE[i][j].STATE = I;   
      for (k=0; k < INTS_PER_BLOCK; k++) {
	CACHE[i][j].DATA[k] = 0;
      }
      if (DEBUG) 
	displayCacheBlock(i, j);
    }
  }
   
  
  // Initialize Cache Statistics
  for (i=0; i < NUM_PROCESSORS; i++) {
    cache_write_hits[i] = 0;
    cache_write_misses[i]= 0;
    cache_upgrades[i] = 0;
    cache_read_hits[i] = 0;
    cache_read_misses[i] = 0;
    cache_writebacks[i] = 0;
  }
}

  
void initSem() {
  int i;
  for (i=0; i < NUM_PROCESSORS; i++) {
    sem_memreq[i] = NewSemaphore("memreq",0);          
    sem_memdone[i] = NewSemaphore("memdone",0);
    sem_bussnoop[i] = NewSemaphore("bussnoop", 0);    // BusSnooper i  waits on sem_bussnoop[i]
    sem_bussnoopdone[i] = NewSemaphore("bussnoopdone", 0);    // Must wait for all Snoopers to be done
  }
}


void  initMem() {// Initialize Memory to match the initial values in the trace 
    int i;
    for (i=0; i  < TOTALSIZE; i++)
      MEM[i] = i;
    printf("Base Physical Address: %p\n",  MEM);
  }

void   displayParams() {
    printf("PARAMETER VALUES: TRACE: %s\tNUM_PROCESSORS: %d\tCPUDELAY: %d\n", TRACE? "ON" : "OFF", NUM_PROCESSORS, MAXDELAY);
  }

void UserMain(int argc, char *argv[]){
  int i,j, k;

  getParams(argc, argv);


  initMem();
  initSem();
  initCache();
  createProcesses();
  displayParams(); 
  // Initialization is done, now start the simulation
    DriverRun(MAX_SIMULATION_TIME); // Maximum time of the simulation (in cycles). 
    // printf("Simulation ended without completing the trace  at %5.2f\n",GetSimTime());
     printf("Simulation ended   at %5.2f\n",GetSimTime());
}



// Processor model
void Processor()
{
  struct tracerecord  *traceref = malloc(sizeof(struct tracerecord));
  struct genericQueueEntry   *memreq = malloc(sizeof(struct genericQueueEntry));  
  int proc_num;

  proc_num = ActivityArgSize(ME) ;
  if (TRACE)
    printf("Processor[%d]: Activated at time %5.2f\n", proc_num, GetSimTime());
  
  while(1) {	  
    if (getNextRef(fp[proc_num], traceref) == 0) 
      break;  // Get next trace record; quit if done

    // Create a memory request and  insert in MEM_CONTROLLER_QUEUE for this processor
       memreq->address = traceref->address; 
       memreq->type = traceref->type;
       memreq->delay = traceref->delay;
       memreq->data = traceref->data;

       insertQueue(MEM_CONTROLLER_QUEUE + proc_num, memreq);
       
       if (TRACE) {
	 printf("\nProcessor %d makes %s request:", proc_num, h(memreq->type));
	 printf(" Address: %x Time:%5.2f\n", memreq->address, GetSimTime());
       }

       SemaphoreSignal(sem_memreq[proc_num]);  // Notify memory controller of request
       SemaphoreWait(sem_memdone[proc_num]);   // Wait for request completion 
       ProcessDelay((double) traceref->delay);  //  Delay between requests 
  }

  numThreadsCompleted++;
  if (numThreadsCompleted == NUM_PROCESSORS)
    cleanUp();

}


