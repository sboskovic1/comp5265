#include "global.h"

extern int NUM_PROCESSORS;
extern int MODE;
extern int TRACE;
extern struct cacheblock CACHE[][CACHESIZE];  // Cache

// Cache Statistics
extern int cache_write_hits[], cache_write_misses[];
extern int cache_upgrades[],cache_read_hits[], cache_read_misses[];
extern int cache_writebacks[], converted_cache_upgrades[];
extern int numCacheToCacheTransfer[], numMemToCacheTransfer[];

extern struct busrec BROADCAST_CMD;   // Bus Commands
extern int BUS_REQUEST[], BUS_GRANT[], BUS_RELEASE; // Bus Handshake Signals

extern SEMAPHORE *sem_bussnoop[]; // Signal each snooper that a bus command is being broadcast
extern SEMAPHORE *sem_bussnoopdone[]; // Signal that command has been handled at this snooper 
extern int CACHE_READY;
extern int CACHE_TRANSFER;

extern int map(int);
extern char * f(int);
extern char * g(int);
extern void displayCacheBlock(int, int);
extern char * f(int);

extern int PFlag, PBit[];
extern int OFlag, OBit[];
 
void atomicUpdate(int procId, int blkNum, int state) {
  CACHE[procId][blkNum].STATE = state;
}

void writebackMemory(int address, int * blkDataPtr) {  // Copy cache block to memory
  int i;
  int  *memptr = (int *) (long int) map(address);
  
  for (i=0; i < INTS_PER_BLOCK; i++){
    *memptr++  =  blkDataPtr[i];  
  }
}

void readFromMemory(int address, int *blkDataPtr) {  // Copy memory block to cache 
  int i;
  int *memptr = (int *) (long int) map(address);

  for (i=0; i < INTS_PER_BLOCK; i++){
     blkDataPtr[i] = *memptr;
     memptr++;
  }
}

void transferCache(int *blkDataPtrSrc, int *blkDataPtrDest) {  // Copy memory block to cache 
  int i;
  for (i=0; i < INTS_PER_BLOCK; i++)
    blkDataPtrDest[i] = blkDataPtrSrc[i];
}


int wireOR(int bits[]) {
  int i;
  int Flag;
  Flag = FALSE;
  for (i=0; i < NUM_PROCESSORS; i++)
    Flag  |=  bits[i];
  return Flag;
}

/* *********************************************************************************************
 Called by  HandleCacheMiss() and LookupCache() to create a bus transaction.
 It waits till granted bus access by the  Bus Arbiter, then broadcasts the command, and waits till 
 all snoopers acknowledge completion. 
 ********************************************************************************************* */

int  MakeBusRequest(int procId, struct genericQueueEntry * req){    
   
  int blkNum,  op;
  int i;

  BUS_REQUEST[procId] = TRUE;   // Assert BUS_REQUEST signal
  if (TRACE)
    printf("Processor %d --  Made BUS REQUEST Time %5.2f\n", procId, GetSimTime());

  while (BUS_GRANT[procId] == FALSE)  // Wait for BUS_GRANT
      ProcessDelay(CLOCK_CYCLE);
  
  BUS_REQUEST[procId] = FALSE;   // De-assert request

  // Create Bus Command based on type of request and current state
  blkNum = (req->address >> BLKSIZE) % CACHESIZE;
  op = req->type;

  BROADCAST_CMD.address  =  req->address;
  if  ((CACHE[procId][blkNum].STATE == SM) || (CACHE[procId][blkNum].STATE == OM)) 
    BROADCAST_CMD.reqtype  = INV;
  else  if  (op == LOAD)
    BROADCAST_CMD.reqtype  = BUS_RD;
  else 
    BROADCAST_CMD.reqtype  = BUS_RDX;

  if (TRACE)
    printf("Processor %d -- Received BUS GRANT.  Broadcast COMMAND: %s Address: %x Time %5.2f\n", procId, g(BROADCAST_CMD.reqtype), BROADCAST_CMD.address, GetSimTime());
  
  if (TRACE)
    printf("Processor %d -- Received BUS GRANT.  Address: %x Time %5.2f\n", procId,  BROADCAST_CMD.address, GetSimTime());
  
  for (i=0; i < NUM_PROCESSORS; i++) 
     SemaphoreSignal(sem_bussnoop[i]);  // Wake up each BusSnooper 
   
     for (i=0; i < NUM_PROCESSORS; i++)
       SemaphoreWait(sem_bussnoopdone[i]);  // Wait for all Bus Snoopers to complete
}


/* Release the bus for use by another processor */
void releaseBus(int procId) {  
	BUS_GRANT[procId] = FALSE;
	BUS_RELEASE = TRUE;
	ProcessDelay(epsilon);
}

void receiveCacheTransfer() {
       CACHE_READY = TRUE; // ASSERT CACHE_READY
       while (CACHE_TRANSFER == FALSE)
	 ProcessDelay(CLOCK_CYCLE);
       CACHE_READY = FALSE;  // DE-ASSERT CACHE_READY
}

void receiveMemTransfer(int procId, int blkNum, int address) {
    int BLKMASK = (-1 << BLKSIZE);

    ProcessDelay(MEM_CYCLE_TIME);
  readFromMemory(address & BLKMASK, CACHE[procId][blkNum].DATA);  // Read block from MEMORY  into cache
}

void doWriteback(int procId, int blkNum) {
  struct genericQueueEntry writeback;
	writeback.address = (CACHE[procId][blkNum].TAG * TAGSHIFT) | (blkNum << BLKSIZE);
	cache_writebacks[procId]++; 
	writebackMemory(writeback.address, CACHE[procId][blkNum].DATA);  // Copy cache block contents to MEMORY
	ProcessDelay(MEM_CYCLE_TIME);
}

void sendCacheTransfer(int procId, int requester, int blkNum) {
      if (DEBUG)
	printf("Snooper %d doing a cache-to-cache transfer for Block %d to Processor %d at time %5.2f\n", procId, blkNum, requester, GetSimTime()); 

      while (CACHE_READY == FALSE)
	ProcessDelay(CLOCK_CYCLE);
      transferCache(CACHE[procId][blkNum].DATA, CACHE[requester][blkNum].DATA);  // Allow to access remote processors  cache
      ProcessDelay(CACHE_TRANSFER_TIME); 
      CACHE_TRANSFER = TRUE;
      while  (CACHE_READY == TRUE) {
	ProcessDelay(CLOCK_CYCLE);
      }
      CACHE_TRANSFER = FALSE;
}

int getRequester() {
  int i;
  for (i = 0; i < NUM_PROCESSORS; i++) {
     if (BUS_GRANT[i] == TRUE) 
      break;
  }
  if (i == NUM_PROCESSORS) {
    printf("No requester when BusSnooper activated at time %5.2f\n", GetSimTime());
    exit(1);
  }
  return(i);
}
/* *********************************************************************************************
BusSnooper process for each processor/cache. Woken  with broadcast command (in struct BROADCAST_CMD).
********************************************************************************************* */

void BusSnooper() {
  int blkNum, broadcast_tag;
  unsigned  busreq_type;
  unsigned address;
  unsigned requester;
  int procId;
  int i;

  procId = ActivityArgSize(ME); // Id of this Snooper
  if (TRACE)
    printf("BusSnooper[%d]: Activated at time %5.2f\n", procId, GetSimTime());
  
  
  while (1) {
    SemaphoreWait(sem_bussnoop[procId]); // Wait for  a bus command
    if (BUSTRACE)
      printf("BusSnooper[%d] --  Woken with Bus COMMAND  at time %5.2f\n", procId, GetSimTime());


    // Parse  Bus Command 
  address = BROADCAST_CMD.address;
  busreq_type = BROADCAST_CMD.reqtype;
  blkNum =  (address >> BLKSIZE)% CACHESIZE;
  broadcast_tag = (address >> BLKSIZE)/ CACHESIZE;


    if (BUS_GRANT[procId] == TRUE)  { // My own Front End  initiated this request
  
    PBit[procId] = FALSE;
    OBit[procId] = FALSE;  

    ProcessDelay(epsilon);

    PFlag = wireOR(PBit);
    OFlag = wireOR(OBit);

    /*
     1. PFLAG (see notes): TRUE indicates that 1 or more processors holds the requested
     block in its cache. It is obtained by ORing the PBIT values set by each processor.

     2. OFLAG (used to simplify simulation): TRUE indicates that data will be provided by a cache
     -to-cache transfer and not from memory. Obtained by ORing the OBIT values of the processors.
   
     Based on  OFlag a receiving cache will use either  function "receiveCacheTransfer()" or 
     function "receiveMemTransfer()" to get the data response to its BUSRD or BUSRDX request.
     For a cache-to-cache transfer the sender should use the function "sendCacheTransfer()" to 
     send the block. The necessary handshaking and delays have already been implemented in the 
     functions. For a memory transfer, the "receiveMemTransfer" function will copy the data into 
     the cache and provide the necessary delay.

    3. For implementing MOESI the cache block maintains an additional DIRTY bit: it is set when 
       the block is written and is used to decide whether an evicted block must be written back. 
       Recall that due to cache-to-cache transfers a valis block in any state (except E) can 
       potentially be DIRTY (i.e. inconsistent with MEM).
    */
    

    if (DEBUG)
      printf("PFlag : %s   OFLag: %s\n", PFlag?"TRUE":"FALSE", OFlag ? "TRUE" : "FALSE");
   
    //  Handle the three possible requests initiated by my own front end.

    switch (busreq_type) {  
    case (INV): {
      // Handle INV. Update the  status bits of the cache block.
      }
      break;
    
    case (BUS_RD): {

      /* 
	 1. Perform a writeback if needed and update the DIRTY bit. 
	 2. Set the cache tag for the new block and read the value into cache. Update my 
	    "numCacheToCacheTransfer" or "numMemToCacheTransfer" counter as appropriate.
	 3. Set the status bits  of the new cache block.
      */
    }
      break;
      
    case BUS_RDX: { 
    
      /* 
	 1. If necessary write back the block using doWriteback() and set the DIRTY bit to FALSE.
	 2. Set the cache tag for the new block and read the value into cache. Update my 
	    "numCacheToCacheTransfer" or "numMemToCacheTransfer" counter as appropriate.
	 3. Set the status bots of the new cache block.
      */ 
    }
      break;
    }
    
    ProcessDelay(CLOCK_CYCLE);
    SemaphoreSignal(sem_bussnoopdone[procId]);  // Done handling Bus Command from my Front End
    if (BUSTRACE)
      printf("Snooper %d Done with my own %s request. Time: %5.2f\n", procId, g(busreq_type), GetSimTime());
    continue;
    }
  

  // If the Bus Command is from another processor
 

    /*  1. Set my PBit  and OBit appropriately */

    /*  Handle the case if the block is not  in my cache */

  switch (busreq_type) { // Handling command from some other processor for a block in my cache
  
  case (INV):
    /* Update state of cache block. If a race is detected update statistics counters:
       "cache_write_misses", "cache_upgrades", and "converted_cache_upgrades"
    */
    
    break;
    
    
  case (BUS_RD): 
    /* Handle received BUS_RD based on the state of my  cache block
     1. Update new cache state as needed using atomicUpdate()
     2. Use sendCacheTransfer() if you are the source of a cache-to-cache transfer
    */

    
    break;


   case (BUS_RDX): 

    /* Handle received BUS_RDX based on the state of my cache block
     1. Update new cache state as needed using atomicUpdate()
     2. Use sendCacheTransfer() if you are the source of a cache-to-cache transfer
     3. Update statistics: "cache_write_misses", "cache_upgrades", and "converted_cache_upgrades"
       as needed.
    */

     break;
  }

if (BUSTRACE)
	printf("Snooper %d finished %s at time %5.2f. CACHE[%d][%d].STATE: %s\n", procId, g(busreq_type), GetSimTime(),  procId, blkNum, f(CACHE[procId][blkNum].STATE));
     ProcessDelay(CLOCK_CYCLE);      
     SemaphoreSignal(sem_bussnoopdone[procId]);
  }
}

/* *********************************************************************************************
Bus Arbiter checks for an asserted  BUS_REQUEST signal every clock cycle.
*********************************************************************************************/
void BusArbiter(){
  int job_num, i;
static int procId = 0;

job_num = ActivityArgSize(ME) - 1;

 if (TRACE)
   printf("Bus Arbiter Activated at time %3.0f\n", GetSimTime());
 
 while(1){

       ProcessDelay(CLOCK_CYCLE);
       if (BUS_RELEASE == FALSE)   // Current bus-owner will release the bus when done
	 continue;


     for (i=0; i < NUM_PROCESSORS; i++)
       if (BUS_REQUEST[(i + procId)%NUM_PROCESSORS]) 
	 break;

     if (i == NUM_PROCESSORS) 
       continue;
     
     BUS_RELEASE = FALSE;     
     BUS_GRANT[(i+procId)%NUM_PROCESSORS] = TRUE;
     
       if (TRACE)
	 printf("BUS GRANT: Time %5.2f.  BUS_GRANT[%d]: %s\n", GetSimTime(), procId, BUS_GRANT[procId] ? "TRUE" : "FALSE");

     procId = (procId+1) % NUM_PROCESSORS;
 }
}

