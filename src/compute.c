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
#include "hw3_header.h"

int sid; 	// shared memeory id
int qid;	// messageQ id
struct Shared_Info_Block *sharedMem;
int proc_ind = -1;

void handler_kill(int signum) {
	if (proc_ind == -1) exit(EXIT_SUCCESS);
	sharedMem->proc[proc_ind].pid = 0;
	sharedMem->proc[proc_ind].numFound = 0;
	sharedMem->proc[proc_ind].numSkipped = 0;
	sharedMem->proc[proc_ind].numTested = 0;
	exit(EXIT_SUCCESS);
}


int compute(int num2start) {
	struct My_Msg message;
	int num2test;
	int pidNow, intNow, bitNow;
	int flag = 0;
	int i=0, sums=0, flag2 = 0;

	if ((sid = shmget(SHMM_KEY, sizeof(struct Shared_Info_Block), 0)) == -1 ) {
		perror(" compute.c: Shared memory segment has not been created!\n");
		handler_kill(1);
		exit(EXIT_FAILURE);
	}
	if ((sharedMem = shmat(sid, NULL, 0)) == (void *) -1) {
		perror(" compute.c: Shared memory segment has not been created!\n");
		handler_kill(1);
		exit(EXIT_FAILURE);
	}
	if ((qid = msgget(MESQ_KEY, 0)) == -1) {
		perror(" compute.c: Message queue has not been created!\n");
		handler_kill(1);
		exit(EXIT_FAILURE);
	}

	message.msgtype = TP_PROC_INDEX;
	pidNow = getpid();
	message.msg = pidNow;
	if (msgsnd(qid, &message, sizeof(message.msg), 0) != 0) {
		perror(" compute.c: compute(): message cannot be send to manage!\n");
		handler_kill(1);
		exit(EXIT_FAILURE);
	}
	msgrcv(qid, &message, sizeof(message.msg), IS_PROC_INDEX, 0); // get an process 'id' from manage 
	proc_ind = message.msg;
	if (sharedMem->proc[proc_ind].pid != getpid()) exit(EXIT_FAILURE);
	
	num2test = num2start-1;
	while(1){
		num2test++;
		if (num2test == num2start && flag == 1)	{handler_kill(SIGQUIT);}
		if (num2test > MAX_NUM2TEST) {
			num2test = 2;
			flag = 1;
		}
		intNow = (num2test - 2) / 32;
		bitNow = (num2test - 2) % 32;
		if ( ( sharedMem->bitmap[intNow] & ((int)1 << bitNow) ) == 0) {
			sums = 0;
			flag2 = 0;
			for (i=1; i<num2test; i++) {
			    if (num2test % i == 0) {sums += i;}
			}
			if (sums == num2test) 	flag2 = 1;
			if ( flag2 == 1 ) {
				sharedMem->proc[proc_ind].numFound += 1;
				message.msgtype = TP_NUM_FOUND_PERFECT;
				message.msg = num2test;
				if (msgsnd(qid, &message, sizeof(message.msg), 0) != 0) {
					perror(" compute.c: compute(): fail to send perfect number to manage!");
					handler_kill(1);
					exit(EXIT_FAILURE);
				}
			} 
			sharedMem->proc[proc_ind].numTested += 1;
			sharedMem->bitmap[intNow] |= (1 << bitNow);
		} else 
			sharedMem->proc[proc_ind].numSkipped += 1;
	}
}

int main(int argc, char *argv[]) {
	int num2start;
	struct sigaction signal;
	if (argc != 2) {
		printf(" compute.c: main(): wrong argument(s)!\
		\n usage: ./compute [start number]\n");
		exit(EXIT_FAILURE);
	}
	num2start = atoi(argv[1]);
	if (num2start > MAX_NUM2TEST || num2start < 2) {
		printf("Start number x is outside the acceptable range! 2 <= x <= 1,024,000\n");
		exit(EXIT_FAILURE);
	}
	memset(&signal, 0, sizeof(signal));
	signal.sa_handler = handler_kill;
	if (sigaction(SIGINT, &signal, NULL) != 0) {
		perror(" compute.c: main(): failed to sigation SIGINT");
		handler_kill(1);
		exit(EXIT_FAILURE);
	}
	if (sigaction(SIGQUIT, &signal, NULL) != 0) {
		perror(" compute.c: main(): failed to sigation SIGQUIT");
		handler_kill(1);
		exit(EXIT_FAILURE);
	}
	if (sigaction(SIGHUP, &signal, NULL) != 0) {
		perror(" compute.c: main(): failed to sigation SIGHUP");
		handler_kill(1);
		exit(EXIT_FAILURE);
	}
	compute(num2start);
	return(0);
}
