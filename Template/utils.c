#include "global.h"
#include <string.h>

extern unsigned makeTrace(char *);

extern struct cacheblock CACHE[][CACHESIZE];
extern int VABaseAddress;
extern int MEM[];
extern int NUM_PROCESSORS;
extern int TRACE;
extern int MAXDELAY;
extern int BUS_RELEASE;
extern FILE *fp[];

void displayCacheBlock(int , int );

// Get Command Line arguments and create Trace Files
void getParams(int argc, char *argv[]) {
  int i;
  char *fname; // Workload Name
  char *tracefile = malloc(50);
  
  for (i=1; i < argc ; i = i+2) {
    if (strcmp(argv[i], "--numProcs") == 0) 
      NUM_PROCESSORS  = atoi(argv[i+1]);  
    else  if (strcmp(argv[i], "--cpuDelay") == 0)
      MAXDELAY  = atoi(argv[i+1]);
    else  if (strcmp(argv[i], "--trace") == 0)
      TRACE  = atoi(argv[i+1]);
    else if (strcmp(argv[i], "--workload") == 0) {
      if (strcmp(argv[i+1], "arrayUpdate") == 0)
	fname = argv[i+1];
      else 	if (strcmp(argv[i+1], "sharedArray") == 0)
	fname = argv[i+1];
      else {
	 printf("Unsupported Workload\n");
	 exit(1);
      }
    }
    else {
       printf("Unmatched Argument %s BYE!!!\n", argv[i]);
       exit(1);
    }
  }

   VABaseAddress =  (int) makeTrace(fname);
  printf("Workload Name:  %s\n", fname);
  printf("Base Virtual Address:  %p\n", VABaseAddress);

  for (i=0; i < NUM_PROCESSORS; i++) {
    sprintf(tracefile, "%s%d\0", fname,i);
    fp[i] = fopen(tracefile,"r");
  }
  BUS_RELEASE = TRUE;  // No bus owner at start

}

// Returns next trace record from file specified
int  getNextRef(FILE *fp, struct tracerecord *ref) { 
  if (fread(ref, sizeof(struct tracerecord), 1,fp)  == 1) {
   return(TRUE);
   }
  else {  
    return(FALSE); // End of Tracefile fp
  }
}

// Convert Virtual to Physcial Address
int map(int address) {
  int phyAddress;

  phyAddress = (address - VABaseAddress) + (int) (long int) MEM;
  return phyAddress;
}


char *h(int type){
  if (type == 0) return ("WRITE");
  else return("READ");
}

// Pretty print helper functions
char *f(int state){
    switch (state) {
        case 1:  return("S");
        case 2:  return("M");
        case 3:return("I");
        case 4:return("E");
        case 5: return("O");
        case 6:return("SM");
        case 7:return("OM");
    }
}

char * g(int reqtype) {
    switch (reqtype) {
        case 1: return("BUS_RD"); break;
        case 2: return("BUS_RDX"); break;
        case 3: return("INV"); break;          
    }
}

void displayCacheBlock(int processor, int blkNum) {
  int i;

  for (i=0; i < INTS_PER_BLOCK; i++) 
    printf("CACHE[%d][%d]: Word[%d]: %d\n", processor, blkNum, i, CACHE[processor][blkNum].DATA[i]);
  printf("********************************\n");
}

// Queeu Functions

int Qallocated[MAX_NUM_QUEUES] = {0};
int NumInQueue[MAX_NUM_QUEUES] = {0};
struct queueNode * Qhead[MAX_NUM_QUEUES], * Qtail[MAX_NUM_QUEUES];

void makeQueue(int id) {
  if (id > MAX_NUM_QUEUES) {
    printf("Too many open queues. Upgrade to professional version. Charges may appy!\n");
    exit(1);
  }

  if (Qallocated[id] == TRUE) {
    printf("Trying to allocate a duplicate queue with same id: %d\n", id);
    exit(1);
  }
  Qallocated[id] = TRUE;
  Qhead[id] = Qtail[id] = (struct queueNode *) NULL;
  NumInQueue[id] = 0;
}


void insertQueue(int id, struct genericQueueEntry *data) {
  struct  queueNode * ptr;

  if (Qallocated[id] == FALSE) 
    exit(1);
  
    ptr = (struct queueNode *) malloc(sizeof(struct queueNode));
    ptr->next = NULL;
    ptr->data = (void *) data;
    NumInQueue[id]++;

  if (Qhead[id] == NULL) 
    Qhead[id] = ptr;
  else 
    Qtail[id]->next = ptr;
  
  Qtail[id]  = ptr;    
}

struct genericQueueEntry *  getFromQueue(int id) { 
  struct genericQueueEntry *  ptr;
  struct  queueNode * temp;
  
  if (Qallocated[id] == FALSE) {
    printf("Accessing  non-existent queue %d\n", id);
    exit(1);
  }
  if (Qhead[id] == NULL) {
    printf("Accessing empty queue %d\n", id);
    exit(1);
  }

  temp = Qhead[id];
  Qhead[id] = Qhead[id]-> next;
  ptr = temp->data;
  free(temp);
  NumInQueue[id]--;
  return ptr;
}


struct genericQueueEntry *  pokeQueue(int id) {
  struct genericQueueEntry *  ptr;
 
  if (Qallocated[id] == FALSE) {
    printf("Accessing  non-existent queue %d\n", id);
    exit(1);
  }
  if (Qhead[id] == NULL) {
    printf("Accessing empty queue %d\n", id);
    exit(1);
  }

  ptr  = Qhead[id]-> data;
  if (DEBUG)
    printf("Returning  request for address %x from queue %d\n", ptr->address,  id);
  return ptr;
}


int getSizeOfQueue(int id){
  if (Qallocated[id] == FALSE) {
    printf("Getting size of non-existent queue %d\n", id);
    exit(1);
  }
  if (NumInQueue[id] < 0) {
    printf("Why did my queue get negative?\n");
    exit(1);
  }
   return(NumInQueue[id]);
}

void displayQueue(int id) {
  struct queueNode * ptr;
  ptr = Qhead[id];
  printf("*************************************\n");
  while (ptr != NULL) {
    printf("ADDRESS: %x\n", (ptr->data) -> address);
    ptr = ptr->next;
  }
  printf("*************************************\n");
}

struct  queueNode *  getLastMatchingEntry(struct queueNode * ptr, int address){
  struct queueNode *temp;

 if (ptr == NULL)
   return NULL;
 else {
   temp = getLastMatchingEntry(ptr->next, address);
if (temp != NULL)
     return temp;
   else
     if ( (ptr->data)->address  == address)
       return ptr ;
     else 
       return NULL;
 }
}

struct queueNode * getLastEntry(int id, int address) {
  struct queueNode * ptr;
   ptr = Qhead[id];
  return( getLastMatchingEntry(ptr, address));
}



double getServiceTime(struct genericQueueEntry *req, double MINSERVICETIME, double MAXSERVICETIME) {
  double interval;
  double time;
  
  interval =  MAXSERVICETIME - MINSERVICETIME;
  time = (drand48() * interval)  + (double) MINSERVICETIME;
  return time;
}








