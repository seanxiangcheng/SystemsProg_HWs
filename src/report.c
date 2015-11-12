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
#include "hw4_header.h"


int sid; 	// shared memeory id
int qid;	// message quee id
struct Shared_Info_Block *sharedMem;

int BitMapReader_Tested(){
	int intnum=0;
	int bitnum=0;
	int numTested = 0;
	for (intnum = 0; intnum < BITMAP_INT_LEN; intnum++) {
		if (sharedMem->bitmap[intnum] == -1) 	numTested += 32;
		else {
			for (bitnum = 0; bitnum < 32; bitnum++) 
				if ((sharedMem->bitmap[intnum] & (1 << bitnum)) == 1) 	numTested++;
		}
	}
	return(numTested);
}

int main(int argc, char *argv[]) {
	int i, number;
	int totalFound = 0;
	struct My_Msg msge;  // message struct
	if ((sid = shmget(SHMM_KEY, sizeof(struct Shared_Info_Block), 0)) == -1){
		perror(" report.c: Shared memory segment has not been created!\n\
	Call \"./manage\" first!");
		exit(EXIT_FAILURE);
	}
	if ((sharedMem = shmat(sid, NULL, 0)) == (void *) -1) {
		perror(" report.c: Shared memory segment does not exist!\n");
		exit(EXIT_FAILURE);
	}
	if ((qid = msgget(MESQ_KEY, 0)) == -1) {
		perror(" report.c: Message queue has not been created!\n\
		          Call \"./manage\" first");
		exit(EXIT_FAILURE);
	}
	printf("\n * Perfect numbers found:  \n -=============-\n");

	for(i=0; i<PERFECT_LEN; i++) {
		number = sharedMem->pnums[i];
		if (number != 0) {
			totalFound++;
			printf(" | P%-2d |  %-4d | \n", totalFound, number);
		}
	}
	printf(" -=============-\n");
	printf(" * Total integer numbers tested: %-6d\n", BitMapReader_Tested());
	printf(" * Total perfect numbers found : %-6d\n", totalFound);

	printf("\n -=======================================-\n");
	printf(" |       Current Computing Processes     |\n");
	printf(" |   pid   | found |  skipped  |  tested |\n");
	printf(" -=======================================-\n");
	for (i = 0; i < PROCESS_LEN; i++) {
		if (sharedMem->proc[i].pid != 0) {
			printf(" | %6d  | %4d  | %7d  | %8d |\n", 
			sharedMem->proc[i].pid, sharedMem->proc[i].numFound, 
			sharedMem->proc[i].numSkipped, sharedMem->proc[i].numTested);
		}
	}
	printf(" -=======================================-\n\n");	

	if (argc == 2) {
		if (strcmp(argv[1], "-k") == 0) {
			printf(" Message: processes of mange and compute are being killed!\n\n");
			msge.msgtype = TP_MANAGE_PID;
			msge.msg = 0;
			if (msgsnd(qid, &msge, sizeof(msge.msg), 0) != 0) {
				perror(" report.c: cannot send the message to manage.");
				exit(EXIT_FAILURE);
			}
			msgrcv(qid, &msge, sizeof(msge.msg), IS_MANAGE_PID, 0);
			if (kill((int)msge.msg, SIGINT) != 0) {
				perror(" report.c: kill-signal SIGINT failed to send to manage.");
				exit(EXIT_FAILURE);
			}
		} else {
			printf(" Error: option \"%s\" not recognized!\n \
			       usage of report:./report [-k]\n", argv[1]);
			exit(EXIT_FAILURE);
		}
	}
	else if(argc != 1){
		printf(" Error: option \"%s\" not recognized!\n \
		       usage of report:./report [-k]\n", argv[1]);
		exit(EXIT_FAILURE);
	}
	return(0);
}
