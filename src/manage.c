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
int qid;	// messageQ id
struct Shared_Info_Block *sharedMem;

// handler code that terminates running computes
void handler(int signum) {
	int pid, i;

	// sends a SIGINT signall to all running computes
	for (i = 0; i < PROCESS_LEN; i++) {
		pid = sharedMem->proc[i].pid;
		if (pid != 0) {
			if (kill(pid, SIGINT) != 0) {
				perror("Kill process failed");
				exit(1);
			}
		}
	}

	// sleeps 5 seconds, detaches shared memory and marks
	// shared memory and message queue for deletion, then exits
	sleep(5);
	if (shmdt(sharedMem)) {
		perror("Shared memory detach failed");
		exit(1);
	}
	if (shmctl(sid, IPC_RMID, 0)) {
		perror("shared memory IPC_RMID failed");
		exit(1);
	}
	if (msgctl(qid, IPC_RMID, NULL)) {
		perror("message queue IPC_RMID failed");
	}
	exit(0);
}

// finds free slot in statistics table, or -1 if there are none
int findIndex() {
	int i;
	for (i = 0; i < BITMAP_INT_LEN; i++) {
		if (sharedMem->proc[i].pid == 0) {
			return i;
		}
	}
	return -1;
}

// main manage loop
int main(int argc, char *argv[]) {
	int processIndex;
	int perfectIndex = 0;
	
	// create, attach, and zero-out shared memory segment
	// if the shared memory segment already exists, then that means there
	// is another instance of manage running, at which point we must exit
	sid = shmget(SHMM_KEY, sizeof(struct Shared_Info_Block), IPC_CREAT | IPC_EXCL | 0666);
	if (sid == -1) {
		perror("There can only be one instance of manage!");
		exit(1);
	}
	sharedMem = shmat(sid, NULL, 0);
	memset(sharedMem->bitmap, 0, sizeof(sharedMem->bitmap));
	memset(sharedMem->pnums, 0, sizeof(sharedMem->pnums));
	memset(sharedMem->proc, 0, sizeof(sharedMem->proc));

	// get message queue
	// if message queu already exists, then that means there is another
	// intance of manage running, at which point we must exit
	qid = msgget(MESQ_KEY, IPC_CREAT | IPC_EXCL | 0666);
	if (qid == -1) {
		perror("There can only be one instance of manage!");
		exit(1);
	}
	struct My_Msg message;

	// set up signal behavior
	struct sigaction signal;
	memset(&signal, 0, sizeof(signal));
	signal.sa_handler = handler;
	if (sigaction(SIGINT, &signal, NULL) != 0) {
		perror(" manage.c: sigaction(): SIGINT failed!");
		exit(EXIT_FAILURE);
	}
	if (sigaction(SIGQUIT, &signal, NULL) != 0) {
		perror(" manage.c: sigaction(): SIGQUIT failed!");
		exit(EXIT_FAILURE);
	}
	if (sigaction(SIGHUP, &signal, NULL) != 0) {
		perror(" manage.c: sigaction(): SIGHUP failed");
		exit(EXIT_FAILURE);
	}

	// loops and waits for messages
	while (1) {
		// we only want messages of type GET_PROCESS_ID, GET_MANAGE_ID, TP_NUM_FOUND_PERFECT
		// aka "incoming" messages
		msgrcv(qid, &message, sizeof(message.msg), -3, 0);
		//printf("Received message!\n");

		if (message.msgtype == TP_PROC_INDEX) {
			//printf("Process index %d requested from pid %d\n", processIndex, message.msg);
			// only respond with an index if there is one available,
			// otherwise kill the requesting process
			processIndex = findIndex();
			if (processIndex > -1) {
				sharedMem->proc[processIndex].pid = message.msg;
				message.msgtype = IS_PROC_INDEX;
				message.msg = processIndex;
				if (msgsnd(qid, &message, sizeof(message.msg), 0) != 0) {
					perror("Manage message send failed");
					exit(1);
				}
				//processIndex++;
			} else {
				kill(message.msg, SIGKILL);
			}
		} else if (message.msgtype == TP_NUM_FOUND_PERFECT) {
			//printf("Perfect number %d received\n", message.msg);

			// check if that perfect number has already been found
			int i, found = 0;
			for (i = 0; i < perfectIndex; i++) {
				if (sharedMem->pnums[i] == message.msg) {
					found = 1;
					break;
				}
			}
			if (found == 0) {
				sharedMem->pnums[perfectIndex] = message.msg;
				perfectIndex++;
			}
		} else if (message.msgtype == TP_MANAGE_PID) {
			message.msgtype = IS_MANAGE_PID;
			message.msg = getpid();
			if (msgsnd(qid, &message, sizeof(message.msg), 0) != 0) {
				perror("Manage PID message send failed");
				exit(1);
			}
		}
	}
}
