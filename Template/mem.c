#include "global.h"

// CACHE STATISTICS

extern int TRACE;
extern struct cacheblock CACHE[][CACHESIZE];



extern int cache_write_hits[], cache_write_misses[];
extern int cache_read_hits[], cache_read_misses[];
extern int cache_upgrades[], silent_upgrades[], cache_writebacks[];

extern  int MakeBusRequest(int, struct genericQueueEntry *);  
extern  struct genericQueueEntry  * getFromQueue(int);
extern int getSizeOfQueue(int);
extern void displayCacheBlock(int, int);
extern void releaseBus(int);

extern char* f(int);
extern char* h(int);

// Synchronization Variables
extern SEMAPHORE *sem_memreq[], *sem_memdone[];

int HandleCacheMiss(int procId, struct genericQueueEntry *req) {
  unsigned blkNum, myTag, op;

  // Parse the request
  blkNum = (req->address >> BLKSIZE) % CACHESIZE;
  myTag = (req->address >> BLKSIZE) / CACHESIZE;
  op = req->type;   // LOAD or STORE

  if ((CACHE[procId][blkNum].TAG != myTag)|| (CACHE[procId][blkNum].STATE == I)) { // Cache MISS
    if (TRACE)
      printf("Processor %d -- Cache Miss: Address %x Time %5.2f\n", procId,req->address, GetSimTime());
	   
    switch (op) {
    case  LOAD:
      cache_read_misses[procId]++;  // Read Miss  Counter
      MakeBusRequest(procId, req);  // Reads block into cache
      break;

    case STORE:
      cache_write_misses[procId]++;  // Write Miss Counter
      MakeBusRequest(procId, req);  // Reads block into cache
      break;
    }
    releaseBus(procId);  // Release bus. Note this processor will call LookupCache() again.
  }

  else {  // Cache hit: indicates an error
    if (TRACE)
      printf("Processor: %d -- HandleCacheMiss found address %x in cache. Time: %5.2f\n", procId, req->address, GetSimTime());
    exit(1);
  }

  if (TRACE)
    printf("Processor %d -- Completed  cache miss for address %x. Time: %5.2f\n", procId, req->address, GetSimTime());

}

/* ******************************************************************************************
Checks cache for requested word. Returns FALSE if the word is not  valid in the cache (cache miss).
On cache hit: a silent transaction is applied directly the cache; returns after delay of 1  CLOCK_CYCLE.
A hit requiring a bus transaction is handled before returning.
/* ***************************************************************************************/

int LookupCache(int procId, struct genericQueueEntry *req) {
  unsigned blkNum, myTag;
  unsigned op;
  int wordOffset;
  char *printString;

  //  Parse the memory request address  
  blkNum = (req->address >> BLKSIZE) % CACHESIZE;
  myTag = (req->address >> BLKSIZE) / CACHESIZE;
  wordOffset = (req->address % (0x1 << (BLKSIZE))) / sizeof(int);
  op = req->type;   // LOAD or STORE


  if ((CACHE[procId][blkNum].TAG == myTag) &&  (CACHE[procId][blkNum].STATE != I)) {   // Cache Hit
    if (TRACE)
      printf("Processor %d --  Cache Hit: Address %x Time %5.2f State: %s\n", procId,req->address,GetSimTime(), f(CACHE[procId][blkNum].STATE));
    
    switch (CACHE[procId][blkNum].STATE) {

 case M:  // Silent Transaction
    switch(op) {
    case LOAD: 
      cache_read_hits[procId]++;  // Read Hits Counter
      req->data =  CACHE[procId][blkNum].DATA[wordOffset];// Read requested word from cache
      break;

    case STORE: 
      cache_write_hits[procId]++;  // Write Hits Counter
      CACHE[procId][blkNum].DATA[wordOffset] =  req->data;  // Write word to cache 
      CACHE[procId][blkNum].DIRTY = TRUE;
      if (TRACE)
	displayCacheBlock(procId, blkNum);
      break;
    }
    ProcessDelay(CLOCK_CYCLE);  // Delay one cycle  
   break;
 
    
  case S:  
    switch(op) {
    case LOAD:                                    
      cache_read_hits[procId]++; // Read Hits Counter
      req->data =  CACHE[procId][blkNum].DATA[wordOffset];  // Read reequested word from cache
      ProcessDelay(CLOCK_CYCLE);  // Delay one cycle
      break;

    case STORE:
      if (TRACE)
	printf("Processor  %d: Need UPGRADE from State %s -- Address %x Time %5.2f\n", procId,f(CACHE[procId][blkNum].STATE), req->address,GetSimTime());
      cache_upgrades[procId]++;   // UPGRADE counter	
      CACHE[procId][blkNum].STATE = SM; // Transient state

      MakeBusRequest(procId, req); // Contend for bus
      
      // If my UPGRADE request was not interruped by another processors write (INV or BUS_RDX) my cache block state is still SM; else it is I

      /* NEW 2025 Removing section */
      /*
      if (CACHE[procId][blkNum].STATE == SM) 
	printString = STRING1;
      else
	printString = STRING2;
      
      CACHE[procId][blkNum].STATE = M;   // Block upgraded to M state
      CACHE[procId][blkNum].DIRTY = TRUE;
      */
      CACHE[procId][blkNum].DATA[wordOffset] = req->data; // Write the word to cache
      

      if (TRACE) {
	  printf("Processor  %d: Completed %s -- Address: %x ", procId, printString, req->address);
	  printf("STATE[%d][%d]: %s  Time: %5.2f\n", procId, blkNum, f(CACHE[procId][blkNum].STATE), GetSimTime());
      }
      if (TRACE)
	displayCacheBlock(procId, blkNum);
      
      releaseBus(procId);  // Safe to allow the next bus transaction
      break;
    }
    break;
  
   
  case E: 
    switch(op) {
    case LOAD:
      cache_read_hits[procId]++;  // Read Hits Counter                                                           
      req->data =  CACHE[procId][blkNum].DATA[wordOffset];// Read requested word from cache                      
      break;

    case STORE:
      silent_upgrades[procId]++;         // Silent Upgrade Counter                                           
      CACHE[procId][blkNum].STATE = M;
      CACHE[procId][blkNum].DATA[wordOffset] =  req->data;  // Write word to cache                                
      CACHE[procId][blkNum].DIRTY = TRUE;
      if (TRACE)
        displayCacheBlock(procId, blkNum);
      break;
    }
      ProcessDelay(CLOCK_CYCLE);  // Delay one cycle                                                               
    break;
  
 
  
case O:  
  switch(op) {
  case LOAD:
    cache_read_hits[procId]++;  // Cache Read Hit
    ProcessDelay(CLOCK_CYCLE);  // Delay one cycle
    break;
  
    case (STORE):
      if (TRACE) {
	printf("Processor  %d: Need UPGRADE from State %s\n", procId,f(CACHE[procId][blkNum].STATE));
	printf("Address %x Time %5.2f\n", req->address,GetSimTime());
      }
      cache_upgrades[procId]++;   // Number of INVALIDATES
   

      CACHE[procId][blkNum].STATE = OM; // Transient state
      
      MakeBusRequest(procId, req); // Contend for bus
      
      // If my UPGRADE request was not interruped by another processors write (INV or BUS_RDX) my cache block state is still SM; else it is I
      
      // NEW 2025 Removing Code section
      /*
      if (CACHE[procId][blkNum].STATE == OM) 
	printString = STRING1;
      else
	printString = STRING2;
      
      CACHE[procId][blkNum].STATE = M;   // Block upgraded to M state
      CACHE[procId][blkNum].DIRTY = TRUE;
      */
      
      CACHE[procId][blkNum].DATA[wordOffset] = req->data; // Write the word to cache
      
      if (TRACE) {
	  printf("Processor  %d: Completed %s -- Address: %x ", procId, printString, req->address);
	  printf("STATE[%d][%d]: %s  Time: %5.2f\n", procId, blkNum, f(CACHE[procId][blkNum].STATE), GetSimTime());
      }
      if (TRACE)
	displayCacheBlock(procId, blkNum);
      
      releaseBus(procId);  // Safe to allow the next bus transaction
  break;
  }
    }
  
   return(TRUE);  // Cache Hit serviced 
  }

  else  // Cache Miss
    return(FALSE);
}


/* *******************************************************************************************
Woken up by my processor on positive clock edge with a request in my  MEM_CONTROLLER_QUEUE.
Get request from the queue and call LookupCache() to process the request.
LookupCache() returns TRUE on a cache hit and FALSE on a miss.
A write hit requiring an Upgrade is handled completely within LookupCache().
On a cache miss HandleCacheMiss() is invoked to service the miss.
Notify the processor when  the memory request is complete.
*********************************************************************************************/
void FrontEndCacheController() {
  int procId;
  struct genericQueueEntry *req;

  procId = ActivityArgSize(ME) ;
  if (TRACE)
    printf("FrontEndCacheController[%d]: Activated at time %5.2f\n",   procId, GetSimTime());

  while(1) {
    SemaphoreWait(sem_memreq[procId]);  // Wait for memory request 
 
    if (getSizeOfQueue(MEM_CONTROLLER_QUEUE + procId) > 0) // Get request
      req = (struct genericQueueEntry *) getFromQueue(MEM_CONTROLLER_QUEUE + procId);
    else {
      printf("ERROR: No Memory Request To Service\n");
      exit(1);
    }       

    while (LookupCache(procId,req) == FALSE) {
      HandleCacheMiss(procId, req);  // Returns after reading  missed block into cache 
    }
    SemaphoreSignal(sem_memdone[procId]);  // Notify processor of request completion
  }
}




