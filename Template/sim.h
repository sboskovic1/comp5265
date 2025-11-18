    /*****************************************************************************/
    /*****************************************************************************/
    /**                    Copyright 1993 by Rice University                    **/
    /**                              Houston, Texas                             **/
    /**                                                                         **/
    /**                           All Rights Reserved                           **/
    /**                                                                         **/
    /**    Permission to use, copy, modify, and distribute this software and    **/
    /**    its documentation  for  any research  purpose and  without fee is    **/
    /**    hereby granted, provided that the  above copyright  notice appear    **/
    /**    in all  copies and  that both  that  copyright  notice  and  this    **/
    /**    permission  notice appear in supporting  documentation,  and that    **/
    /**    the name of Rice University not be  used  in  advertising  or  in    **/
    /**    publicity pertaining to  distribution  of  the  software  without    **/
    /**    specific, written prior permission.  The inclusion of this  soft-    **/
    /**    ware  or its  documentation in  any  commercial  product  without    **/
    /**    specific, written prior permission is prohibited.                    **/
    /**                                                                         **/
    /**    RICE  UNIVERSITY  DISCLAIMS  ALL  WARRANTIES WITH  REGARD TO THIS    **/
    /**    SOFTWARE,  INCLUDING  ALL IMPLIED WARRANTIES  OF  MERCHANTABILITY    **/
    /**    AND FITNESS.  IN NO EVENT SHALL RICE UNIVERSITY BE LIABLE FOR ANY    **/
    /**    SPECIAL, INDIRECT  OR CONSEQUENTIAL DAMAGES  OR ANY DAMAGES WHAT-    **/
    /**    SOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN    **/
    /**    ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS  ACTION, ARISING    **/
    /**    OUT OF  OR IN  CONNECTION  WITH  THE  USE  OR PERFORMANCE OF THIS    **/
    /**    SOFTWARE.                                                            **/
    /*****************************************************************************/
    /*****************************************************************************/

#ifndef SIMH
#define SIMH

#include <stdio.h>

/**************************************************************************************\
****************************************************************************************
**                                                                                    **
**                                     CONSTANTS                                      **
**                                                                                    **
****************************************************************************************
\**************************************************************************************/


/********************************** Blocking Actions **********************************/

#define INDEPENDENT              0      /* Unrelated child scheduled                  */
#define NOBLOCK                  0      /* Nonblocking MessageReceive                 */
#define BLOCK                    1      /* Blocking Schedule or Message Receive       */
#define FORK                     2      /* Forking Schedule                           */

/*******************************  Event Characteristics  ******************************/

#define DELETE                   1      /* Event deleted at termination               */
#define NODELETE                 0      /* Event not deleted at termination           */

/******************************  Message Parameters  **********************************/

#define ANYTYPE                 -1      /* Receiving process ignores message type     */
#define ANYSENDER                0      /* Recieving process ignores message sender   */

#define ANYSENDERPROCESSOR      -1      /* Receiving processor ignores mesage sender  */
#define PROCESSDEST              1      /* Message sent to a process                  */
#define PROCESSORDEST            2      /* Message sent to a processor                */
#define BLOCK_UNTIL_RECEIVED     1
#define BLOCK_UNTIL_SENT         2      
#define BLOCK_UNTIL_PACKETIZED   3
#define BLOCK_UNTIL_BUFFERED     4
#define DEFAULT_PKT_SZ          64

/***********************  Statistics Record Characteristics  **************************/

#define NOMEANS                  0      /* Means not computed                         */
#define MEANS                    1      /* Means are computed                         */
#define HIST                     2      /* Histogram is collected                     */
#define NOHIST                   3      /* Histogram not collected                    */
#define POINT                    4      /* Point statistics record type               */
#define INTERVAL                 5      /* Interval statistics record type            */

/*****************************  Argument and Buffer Size  *****************************/

#define UNKNOWN                 -1      /* Size not specified                         */
#define DEFAULTSTK               0      /* Default stack size                         */

/****************************  Types of Queue Disciplines  ****************************/

#define FCFS                     1      /* First Come First Served                    */
#define FCFSPRWP                 2      /* FCFS Preemptive Resume with Priorities     */
#define LCFSPR                   3      /* Last Come First Served Preemptive Resume   */
#define PROCSHAR                 4      /* Processor Sharing                          */
#define RR                       5      /* Round Robin                                */
#define RAND                     6      /* Random                                     */
#define LCFSPRWP                 7      /* LCFS Preemptive Resume with Priorities     */
#define SJN                      8      /* Shortest Job Next                          */
#define RRPRWP                   9      /* RR Preemptive Resume with Priorities       */
#define LCFS                    10      /* Last Come First Served                     */

/**********************************  Queue Statistics  ********************************/

#define TIME                     1
#define UTIL                     2
#define LENGTH                   3
#define BINS                     4
#define BINWIDTH                 5
#define EMPTYBINS                6

/***********************************  Event List Types  *******************************/

#define CALQUE                   0
#define LINQUE                   1

/*********************************  Netsim Routing modes  *****************************/

#define NOWAIT                   0
#define WAIT                     1

/**************************** Netsim Statistics Constants  ****************************/

#define NETTIME                  1
#define BLKTIME                  2
#define OPORTTIME                3
#define MOVETIME                 4
#define LIFETIME                 5

/***************************************  Misc.  **************************************/

#define ME                       0
#define GLOBAL                   0
#define LOCAL                    1
#define ALL                      0
#define BOTH                     2


/**************************************************************************************\
****************************************************************************************
**                                                                                    **
**                                  TYPEDEFS                                          **
**                                                                                    **
****************************************************************************************
\**************************************************************************************/

typedef struct YS__PktData    PKTDATA;

typedef int  ACTIVITY;
typedef int  PROCESS;
typedef int  EVENT;
typedef int  LIST;
typedef int  LISTELEM;
typedef int  SEMAPHORE;
typedef int  FLAG;
typedef int  BARRIER;
typedef int  RESOURCE;
typedef int  CONDITION;
typedef int  IVAR;
typedef int  FVAR;
typedef int  STATREC;
typedef int  (*cond)();              /* Pointer to a function that returns int        */
typedef void (*func)();              /* Pointer to a function with no return value    */

typedef int  PROCESSOR;
typedef int  MODULE;
typedef int  BUFFER;
typedef int  MUX;
typedef int  DEMUX;
typedef int  UDMOD;
typedef int  MODTERM;
typedef int  NETPORT;
typedef int  OPORT;
typedef int  IPORT;
typedef int  OPORTARR;
typedef int  IPORTARR;
typedef int  PACKET;
typedef int  NETWORK;
typedef int  SOURCE;
typedef int  SINK;
typedef int  (*rtfunc)();  /* Pointer to a routing function                           */


struct YS__PktData
{
   int     seqno;          /* User supplied id used for sequencing packts of a msg    */
   char    *mesgptr;       /* Pointer the message for this packet                     */
   int     pktsize;        /* Number of flits in the packet                           */
   int     srccpu;         /* ID of CPU sending message                               */
   int     destcpu;        /* ID of CPU receiving message                             */
   double  recvtime;       /* Time to move the packet out of an output port           */
   double  createtime;     /* Time the packet was created                             */
   double  nettime;        /* Time the packet spent in the network                    */
   double  blktime;        /* Time the packet was blocked in the network              */
   double  oporttime;      /* Time the packet spent waiting in an output port         */
};


/**************************************************************************************\
****************************************************************************************
**                                                                                    **
**                                  GLOBAL VARIABLES                                  **
**                                                                                    **
****************************************************************************************
\**************************************************************************************/

extern        FVAR     *SIMTIME;      /* An FVAR equal to the current simulation time */
extern        int      TraceLevel;    /* Controls the amount of trace information     */
extern        int      TraceIDs;      /* If != 0, show object ids in trace output     */

/**************************************************************************************\
****************************************************************************************
**                                                                                    **
**                                   USER OPERATIONS                                  **
**                                                                                    **
****************************************************************************************
\**************************************************************************************/

/******************************** QUEUE Operations ************************************/

void   QueueCollectStats();     /* Initiates statistics collection for a queue        */
void   QueueResetStats();       /* Resets statistics collectin for a queue            */
STATREC* QueueStatPtr();        /* Returns a pointer to a queue's statistics          */

/********************************* LIST Operations ************************************/

LIST*     NewList();         /* Create and return a pointer to a new list             */
LISTELEM* ListGetHead();     /* Removes & returns a pointer to the head of a list     */
void      ListPutHead();     /* Puts a queue element at the head of a list            */
void      ListPutTail();     /* Puts a queue element at the tail of a list            */

/***************************** LISTELEM Operations ************************************/

LISTELEM* NewListElem();      /* Creates & returns ptr to new user List element       */
void      ListElemSetData();  /* Set a pointer to the list element's data             */
char*     ListElemGetData();  /* Get a pointer to the list element's data             */
void      ListElemReturn();   /* Returns a list element to its pool                   */

/***************************** SEMAPHORE Operations ***********************************/

SEMAPHORE *NewSemaphore();      /* Creates & returns a pointer to a new semaphore     */
int        SemaphoreInit();     /* If queue is empty its value is set to i            */
void       SemaphoreSignal();   /* Signals the semaphore                              */
void       SemaphoreSet();      /* Set sem. to 1 if empty, or release an waiting act. */
int        SemaphoreDecr();     /* Decrement the sem. value and return the new value  */
void       SemaphoreWait();     /* Wait on a semaphore                                */
int        SemaphoreValue();    /* Returns the value of the semaphore                 */
int        SemaphoreWaiting();  /* Returns the # of activities in the queue           */

/******************************** FLAG Operations *************************************/

FLAG *NewFlag();                /* Creates and returns a pointer to a new flag        */
int   FlagSet();                /* Sets a flag                                        */
int   FlagRelease();            /* Releases activities waiting at a flag              */
void  FlagWait();               /* Waits for a flag to be set or released             */
int   FlagWaiting();            /* Returns the # of activities in the queue           */

/******************************** BARRIER Operations **********************************/

BARRIER *NewBarrier();          /* Creates and returns a pointer to a new barrier     */
int      BarrierInit();         /* If a barrier's queue is empty, sets its value to i */
void     BarrierSync();         /* Waits at a barrier synchronization point           */
int      BarrierNeeded();       /* Returns # of additional syncs to free barrier      */
int      BarrierWaiting();      /* Returns the number of processes waiting at barrier */

/******************************** STVAR Operations ************************************/

IVAR  *NewIvar();               /* Creates and returns a pointer to a new Ivar        */
FVAR  *NewFvar();               /* Creates and returns a pointer to a new Fvar        */
void   Iset();                  /* Sets the value of an Ivar                          */
void   Fset();                  /* Sets the value of an Fvar                          */
int    Ival();                  /* Returns the value of an Ivar                       */
double Fval();                  /* Returns th value of an Fvar                        */

/******************************** STVAR Operations ************************************/

IVAR  *NewIvar();               /* Creates and returns a pointer to a new Ivar        */
FVAR  *NewFvar();               /* Creates and returns a pointer to a new Fvar        */
void   Iset();                  /* Sets the value of an Ivar                          */
void   Fset();                  /* Sets the value of an Fvar                          */
int    Ival();                  /* Returns the value of an Ivar                       */
double Fval();                  /* Returns th value of an Fvar                        */

/******************************** CONDITION Operations ********************************/

CONDITION *NewCondition();      /* Creates and returns a pointer to a new condition   */
void       ConditionWait();     /* Waits until a condition holds                      */
int        ConditionState();    /* Returns the state of a condition                   */
int        ConditionWaiting();  /* Returns the # of activities in the queue           */
void       ConditionSetArg();   /* Sets the argument pointer of a condition           */

/****************************** RESOURCE Operations ***********************************/

RESOURCE *NewResource();        /* Creates and returns a pointer to a new resource    */
void     ResourceUse();         /* Requests service from a resource                   */
int      ResourceWaiting();     /* Returns the # of processes in the queue            */
int      ResourceServicing();   /* Returns the # of activities getting service        */
void     ResourceResetStats();  /* Resets all statistics records of the resource      */

/******************************** ACTIVITY Operations *********************************/

void     ActivitySetArg();      /* Sets the argument pointer of an activity           */
char     *ActivityGetArg();     /* Returns the argument pointer of an activity        */
int      ActivityArgSize();     /* Returns the size of an argument                    */
void     ActivitySchedTime();   /* Schedules an activity to start in the future       */
void     ActivitySchedSema();   /* Schedules an activity to wait for a semaphore      */
void     ActivitySchedFlag();   /* Schedules an activity to wait for a flag           */
void     ActivitySchedCond();   /* Schedules an activity to wait for a condition      */
void     ActivitySchedRes();    /* Schedules an activity to wait use a resource       */
void     ActivityCollectStats();/* Activates statistics collection for an activity    */
void     ActivityStatRept();    /* Prints a report of an activity's statistics        */
STATREC  *ActivityStatPtr();    /* Returns a pointer to an activity's stat record     */
int      ActivityStatus();      /* Returns the status of an activity                  */
ACTIVITY *ActivityGetMyPtr();   /* Returns a pointer to the active activity           */
ACTIVITY *ActivityGetParPtr();  /* Returns a pointer to the active activity's parent  */

/****************************** PROCESS Operations ************************************/

PROCESS *NewProcess();          /* Creates and return a pointer to a new process      */
PROCESS *NewUserProcess();      /* Creates and return a pointer to a new user process */
PROCESS *NewOSProcess();        /* Creates and return a pointer to a new OS process   */
void     ProcessSetStkSz();     /* Set the default stack size                         */
void     ProcessSleep();        /* Suspends the current process                       */
void     ProcessJoin();         /* Suspend until all forked children terminate        */
void     ProcessDelay();        /* Suspends the current process for a time period     */
void     ProcessSetPriority();  /* Sets the priority of a process                     */
void     ProcessSendMsg();      /* Sends a message to a process                       */
int      ProcessReceiveMsg();   /* Copies received data into buf and returns its size */
int      ProcessCheckMsg();     /* Checks for a message of a given type and sender    */
int      ProcessProbeMsg();     /* Checks for a message of a given type and sender    */

/********************************* EVENT Operations ***********************************/

EVENT *NewEvent();              /* Creates and returns a pointer to a new event       */
EVENT *NewOSEvent();            /* Creates and return a pointer to a new OS event     */
int    EventGetType();          /* Returns the events type                            */
void   EventSetType();          /* Sets the event's type                              */
int    EventGetDelFlag();       /* Returns DELETE (1) or NODELETE (0)                 */
void   EventSetDelFlag();       /* Makes an event deleting                            */
int    EventGetState();         /* Returns the state of an event                      */
void   EventSetState();         /* Sets state used to designate a return point        */
void   EventReschedTime();      /* Reschedules an event to occur in the future        */
void   EventReschedSema();      /* Reschedules an event to wait for a semaphore       */
void   EventReschedFlag();      /* Reschedules an event to wait for a flag            */
void   EventReschedCond();      /* Reschedules an event to wait for a condition       */
void   EventReschedRes();       /* Reschedules an event to use a resource             */

/******************************** STATREC Operations **********************************/

STATREC  *NewStatrec();         /* Creates and returns a pointer to a new statrec     */
void     StatrecSetHistSz();    /* Sets the default histogram size                    */
void     StatrecReset();        /* Resets the statrec                                 */
void     StatrecUpdate();       /* Updates the statrec                                */
void     StatrecReport();       /* Generates and displays a statrec report            */
int      StatrecSamples();      /* Returns the number of samples                      */
double   StatrecMaxVal();       /* Returns the maximum sample value                   */
double   StatrecMinVal();       /* Returns the minimum sample value                   */
int      StatrecBins();         /* Returns the number of bins                         */
double   StatrecLowBin();       /* Returns the low bin upper bound                    */
double   StatrecHighBin();      /* Returns the high bin lower bound                   */
double   StatrecBinSize();      /* Returns the bin size                               */
double   StatrecHist();         /* Returns the value of the ith histogram element     */
double   StatrecMean();         /* Returns the mean                                   */
double   StatrecSdv();          /* Returns the standard deviation                     */
double   StatrecVar();          /* Returns the variance                               */
double   StatrecInterval();     /* Returns the sampling interval                      */
double   StatrecRate();         /* Returns the sampling rate                          */
void     StatrecEndInterval();  /* Terminates a sampling interval                     */

/********************************* EVENT LIST Operations ******************************/

int      EventListSize();         /* Returns the size of the event list               */
void     EventListSelect();       /* Selects the type of event list to use            */
void     EventListCollectStats(); /* Activates auto stats collection for event list   */
void     EventListResetStats();   /* Resest a statistics record of a queue            */
STATREC  *EventListStatPtr();     /* Returns a pointer to a event list's statrec      */

/*********************************** DRIVER Operations ********************************/

void DriverReset();             /* Resets the driver (Sets YS__Simtime to 0)          */
void DriverInterrupt();         /* Interrupts the driver and returns to the user      */
int  DriverRun();               /* Activates the simulation driver; returns a value   */
void BreakPoint();              /* Causes the simulation manager to halt immediately  */

/************************************ UTILITY Operations ******************************/

double YacRand();              /* Random Number Generator                             */
void   YacSeed();              /* Set the seed for yacrand                            */
double YacRandom();            /* Better Random Number Generator                      */
void   YacRandomInit();        /* Initializes YacRandom                               */
void   YacRandomStream();      /* Selects one of 16 random streams                    */
double UnixRandom();           /* Unix random number generator                        */
double SysRand();              /* YACSIM internal random number generator             */
void   SetSysRand();           /* Specify the internal random number generator to use */

double RandUniform();          /* Derived random number distributions                 */
double RandExponential();
double RandNormal();
double RandErlang();
double RandPareto();
int    RandUniformInt();
int    RandBernoulli();
int    RandBinomial();
int    RandGeometric();
int    RandNegBinomial();
int    RandPoisson();

double GetSimTime();           /* Returns the current simulation time                 */
void   TracePrint();
void   TracePrintTag();
int    LoadArgs();
char   *GetArg();

/******************************* PROCESSOR Operations *********************************/

PROCESSOR *NewProcessor();        /* Creates a new PROCESSOR                          */
void      ProcessorAttach();      /* Attaches a process to a processor                */
void      SetCycleTime();         /* Set the PARCSIM cycle time                       */
void      SetOSDelays();          /* Set various OS delays                            */
void      ProcessorSendMsg();     /* Sends a message to a processor                   */
int       ProcessorReceiveMsg();  /* Copies receives data into buf & returns its size */
int       ProcessorCheckMsg();    /* Checks for a message of a given type             */
void      ProcessorGlobalUtil();  /* Activates computation of global processor util.  */
void      ProcessorLocalUtil();   /* Activates computation of local processor util.   */
double    ProcessorGetUtil();     /* Returns the processor utiliaztion to date        */
void      ProcessorUtilRept();    /* Prints out the processor util statistics         */

/*********************************** MUX Operations ***********************************/

MUX       *NewMux();              /* Creates and returns a pointer to a new mux       */

/********************************** DEMUX Operations **********************************/

DEMUX     *NewDemux();            /* Creates and returns a pointer to a new demux     */

/********************************** IPORT Operations **********************************/

IPORT     *NewIPort();            /* Creates and returns a pointer to a new iport     */
IPORTARR  *NewIPortArr();         /* Create an array of iports                        */
SEMAPHORE *IPortSemaphore();      /* Returns a pointer to the ready sema of an iport  */
int       IPortSpace();           /* Returns the # of free packet slots in an iport   */

/********************************** OPORT Operations **********************************/

OPORT     *NewOPort();            /* Creates and returns a pointer to a new oport     */
OPORTARR  *NewOPortArr();         /* Create an array of output ports                  */
SEMAPHORE *OPortSemaphore();      /* Returns a pointer to the req sema of an oport    */
int       OPortPackets();         /* Returns the number of pkts avail at an oport     */

/********************************* BUFFER Operations **********************************/

BUFFER    *NewBuffer();           /* Creates and returns a pointer to a new buffer    */

/********************************* UDMOD Operations ***********************************/

UDMOD     *NewUDMod();            /* Creates and returns a pointer to a new UD module */

/******************************* PACKET Operations ************************************/

PACKET    *NewPacket();           /* Creates and returns a pointer to a new packet    */
double    PacketSend();           /* Sends a packet to a network iport                */
PACKET    *PacketReceive();       /* Receives a packet from a network oport           */
PKTDATA   *PacketGetData();       /* Returns a pointer to the data in a packet        */
void      PacketFree();           /* Returns a packet to the pool of free packets     */

/********************************* NETWORK Operations *********************************/

NETWORK *NewNetwork();         /* Created and returns a pointer to a new netork       */
SOURCE *NewSource();
SINK  *NewSink();
void  NetworkConnect();        /* Connects two network modules                        */
void  NetworkSetCycleTime();   /* Sets the cycle time; all other times are multiples  */
void  NetworkSetFlitDelay();   /* Sets the flit delay                                 */
void  NetworkSetMuxDelay();    /* sets the time to move a flit through a mux          */
void  NetworkSetArbDelay();    /* Sets the time for a flit to arbitrate at a mux      */
void  NetworkSetDemuxDelay();  /* Sets the time to move a flit through a demux        */
void  NetworkSetPktDelay();    /* Sets the packet delay                               */
void  NetworkSetThresh();      /* Sets the buffer threshold                           */
void  NetworkSetWFT();         /* Sets the WFT mode                                   */
void  NetworkPutData();        /* Send net statistics for one data point to stdout    */

#endif
