/*THIS CODE IS MY OWN WORK. IT WAS WRITTEN WITHOUT CONSULTING A TUTOR OR CODE WRITTEN BY OTHER STUDENTS - XIANG CHENG*/

/* Written by XIANG CHENG (xcheng7@emory.edu; 1938871)
 * EMAIL: CHENGXIANG.CN@GMAIL.COM; XIANG.CHENG@EMORY.EDU
 */
#include <sys/shm.h>
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


#define MESQ_KEY 50973
#define SHMM_KEY 37905

#define MAX_NUM2TEST 1024000
#define BITMAP_INT_LEN  256000 // 1024000/sizeof(int) = 256000
#define PERFECT_LEN  20
#define PROCESS_LEN  20

#define TP_PROC_INDEX 1
#define TP_MANAGE_PID 2
#define TP_NUM_FOUND_PERFECT 3
#define IS_PROC_INDEX 4
#define IS_MANAGE_PID 5

extern int sid; // shared memory id
extern int qid; // message Q id

struct My_Msg{
	long msgtype;	// message type
	int msg;	// message	
};

struct Proc_Info{
	int pid;
	int numFound;
	int numSkipped;
	int numTested;
};

struct Shared_Info_Block {
	unsigned int bitmap[BITMAP_INT_LEN];	// bitmap array
	int pnums[PERFECT_LEN];	// stores perfect numbers when found
	struct Proc_Info proc[PROCESS_LEN];	// array of process statistics
};

extern struct Shared_Info_Block *memory;
