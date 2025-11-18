#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "sim.h"

#define STAGGER_DELAY  1.0

#define TOTALSIZE 12800 
#define MAX_NUM_PROCESSORS  32
#define MAX_NUM_QUEUES 40
#define MAX_SIMULATION_TIME  1000000.0



// Flags to turn on debugging levels
#define DEBUG FALSE
#define BUSTRACE FALSE

// Global constants
#define FALSE 0
#define TRUE 1
#define  CHUNK 1
#define INTERLEAVE 0
#define LOAD 1
#define STORE 0
#define STRING1 "UPGRADE"
#define STRING2 "Interrupted UPGRADE"
#define epsilon 0.000001

// Timings
#define CLOCK_CYCLE 1.0
#define MEM_CYCLE_TIME    100.0
#define BUS_CYCLE_TIME 1.0
#define CACHE_HIT_TIME 1.0
#define CACHE_TRANSFER_TIME 10.0

// Memory Controller Queue Id
#define MEM_CONTROLLER_QUEUE 0

// Bus Commands
#define BUS_RD  1
#define BUS_RDX 2
#define INV 3

// Cache States
#define   S 1
#define   M 2
#define   I 3
#define   E 4
#define   O 5
#define  SM 6
#define  OM 7

// Cache Parameters
#define CACHEBITS 0   // Cache Size: 1 Block
#define CACHESIZE (0x1 << CACHEBITS)  // Size in Blocks
#define BLKSIZE  5  // So 2^5 bytes per block
#define TAGSHIFT  (0x1 << (BLKSIZE + CACHEBITS))
#define INTS_PER_BLOCK  0x1 << (BLKSIZE-2)




/* The tracefile will store one record for each memory access that is recorded.
Each record consists of the memory address being accessed, the delay (in cycles)
between the  completion of this request and the next memory access, and whether the
access is a Load (type = 1) or Store (type = 0);    */

struct tracerecord {
  unsigned address;
  int delay;
  int type;
  int data;
};

/* A cache block is represented by the structure below, consisting of two status flags Vaild (V) and Dirty (D),
   a TAG field, and a data field implemented as a pointer to the actual data bytes in  the block */

struct cacheblock {
  int STATE;
  int TAG;
  int DIRTY;
  int DATA[0x1 << (BLKSIZE-2)];
};



/* Structures used to hold memory requests sent to the memory queue */
struct genericQueueEntry {
  unsigned address;
  unsigned delay;
  int type;
  int data;
};

/* Structures used to hold bus requests sent to the bus controller */
struct busrec{
  int   reqtype;  // BUSREAD, BUSWRITE,INVALIDATE, MEMABORT
  unsigned address;
  char *BLKDATAPTR;
  int type;
  int requester;
};


/* Generic queue entry with a pointer to the request and a next field for chaining the queue entries together */
struct queueNode {
  struct genericQueueEntry * data;
  struct queueNode  *next;
};









