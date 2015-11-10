/*THIS CODE IS MY OWN WORK. IT WAS WRITTEN WITHOUT CONSULTING A TUTOR OR CODE WRITTEN BY OTHER STUDENTS - XIANG CHENG*/

/* Written by XIANG CHENG (xcheng7@emory.edu; 1938871)
 * EMAIL: CHENGXIANG.CN@GMAIL.COM; XIANG.CHENG@EMORY.EDU
 */
#include <sys/shm.h>
#include <sys/msg.h>
#include <unistd.h>
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "hw4.h"

#define MQ_KEY 55100
#define SM_KEY 55111

#define GET_PROCESS_INDEX 1
#define GET_MANAGE_PID    2
#define FOUND_PERFECT_NUM 3
#define PROCESS_INDEX_YES 4
#define MANAGE_PID_YES    5

#define NUMBER_OF_INTS    32000
#define PERFECT_CAPACITY  20
#define PROCESS_CAPACITY  20

typedef struct {
	int pid;		// process id
	int countFound;		// count of perfect numbers found
	int countTested;	// count of numbers tested
	int countSkipped;	// count of numbers skipped
} stats;

typedef struct {
	int bits[32000];	// bitmap array
	int perfect[20];	// stores perfect numbers when found
	stats process[20];	// array of process statistics
} sharedMemory;

typedef struct {
	long type;		// message type
	int data;		// data (as an int)
} msg;

extern int smid, mqid;
extern sharedMemory *memory;
