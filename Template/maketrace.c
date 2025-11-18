 #include "global.h"
#include <string.h>

//extern int MAXDELAY;
// extern int NUM_PROCESSORS;

//extern char *tracefile;
extern int TRACE;
extern int MODE;
extern int MAXDELAY;
extern int NUM_PROCESSORS;

FILE *tracefp;


struct tracerecord tracerec;
int a[TOTALSIZE];
int numRecords = 0;


void init(int *p, int size) {
  int i;
  for (i=0; i  < size; i++)
    *p++ = i;
  if (TRACE)
    printf("Data initialized\n");
}

 
int  * record(FILE *tracefp, int *q, int type, int data){
   tracerec.address = (long unsigned) q;
   tracerec.delay= (int) MAXDELAY;
   tracerec.type = type;
   tracerec.data = data;
   numRecords++;
   fwrite(&tracerec, sizeof(struct tracerecord), 1,tracefp);
   return q;
 }


void doLoad(FILE *tracefp, int id) {
  int *p;
  int temp;
  int mode;
  int i;
  int  datasize = TOTALSIZE;

  mode =  LOAD;
    for (i=0,  p = a; i < datasize; i++)    {
      temp = *p;
      record(tracefp, p,  mode, temp); 
      p++;
    }
}


void doStore(FILE *tracefp, int id) {
  int *p;
  int temp;
  int mode;
  int i;
  int  datasize = TOTALSIZE;
  mode = STORE;
    for (i=0,  p = a; i < datasize; i++)    {
      temp = *p + 1;
      temp = temp  * temp;
      record(tracefp, p,  mode, temp); 
      p++;
    }
}


// All processoqrs process the entire array
void doSA(FILE *tracefp, int id) {

  if (id == 0) 
    doLoad(tracefp,  id); // Change to "doStore" for WW 
    else
      doLoad(tracefp, id);  // Change to "doStore" for LW and WW 
}


// Processor i gets the ith chunk of adjacent array elements.
void doAU(FILE *tracefp, int id){
  int i;
  int *p;
  int chunksize = TOTALSIZE/NUM_PROCESSORS;
  int temp;

  for (i=0,  p = a + id*chunksize; i < chunksize; i++)    {
    temp =    *(record(tracefp, p,  LOAD, *p));
    temp = temp * temp;
    record(tracefp, p,  STORE, temp);
    p++;
  }
}


unsigned makeTrace(char *workloadName) {
  int i;
  char *tracefile = malloc(50);

  init(a, TOTALSIZE);

  for (i=0; i < NUM_PROCESSORS; i++) {
    sprintf(tracefile, "%s%d\0",workloadName,i);
    tracefp = fopen(tracefile,"w+");
    numRecords = 0;

    if (strcmp(workloadName, "arrayUpdate") == 0) 
      doAU(tracefp,i);
    else  if (strcmp(workloadName, "sharedArray") == 0)
      doSA(tracefp,i);
   else {
      printf("Unknown Workload encountered im memtrace.c\n");
      exit(1);
    }
    fclose(tracefp);
    }
    printf("Created %d Trace Files for Workload \"%s\"\n", i, workloadName);
    return((unsigned) (unsigned long) a);
}
