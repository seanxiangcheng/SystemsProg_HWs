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

void handler(int signum) {
	int i, tmp;
	for (i = 0; i < PROCESS_LEN; i++) {
		tmp = (sharedMem->proc[i].pid);
		if ( tmp != 0) {
			if (kill(tmp, SIGINT) != 0) {
				perror(" manage.c: handler(): failed to kill computing process!");
				exit(EXIT_FAILURE);
			}
		}
	}
	sleep(5);
	
	if (shmdt(sharedMem)) {
		perror(" manage.c: shmdt(): shared memory failed to detach!");
		exit(EXIT_FAILURE);
	}
	if (shmctl(sid, IPC_RMID, 0)) {
		perror(" manage.c: shmctl(): shared memory fail to control IPC_RMID!");
		exit(EXIT_FAILURE);
	}
	
	if (msgctl(qid, IPC_RMID, NULL)) {
		perror(" manage.c: msgctl(): message queue failed to control IPC_RMID!");
	}
	exit(EXIT_SUCCESS);
}


int main(int argc, char *argv[]) {
	int i=0;
	int proc_ind = -1;
	int PN_ind= 0;
	int flag=0, tmp=0;
	struct My_Msg message;

	sid = shmget(SHMM_KEY, sizeof(struct Shared_Info_Block), IPC_CREAT | IPC_EXCL | 0666);
	if (sid == -1) {
		perror("  manage.c: error shmget!\n");
		exit(1);
	}
	sharedMem = shmat(sid, NULL, 0);
	memset(sharedMem, 0, sizeof(struct Shared_Info_Block));
	qid = msgget(MESQ_KEY, IPC_CREAT | IPC_EXCL | 0666);
	if (qid == -1) {
		perror("  manage.c: error msgget!\n");
		exit(1);
	}
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

	while (1) {
		msgrcv(qid, &message, sizeof(message.msg), -3, 0);
		if (message.msgtype == TP_PROC_INDEX) {
			proc_ind = -1;
			for (i = 0; i < BITMAP_INT_LEN; i++){
				if ((sharedMem->proc[i].pid) == 0) {proc_ind=i; break;}
			}
			if (proc_ind > -1) {
				sharedMem->proc[proc_ind].pid = message.msg;
				message.msgtype = IS_PROC_INDEX;
				message.msg = proc_ind;
				if (msgsnd(qid, &message, sizeof(message.msg), 0) != 0) {
					perror(" manage.c: main(): failed to send message to compute!");
					handler(0);
					exit(EXIT_FAILURE);
				}
			} else {kill(message.msg, SIGKILL);} // unkown type
		} else if (message.msgtype == TP_NUM_FOUND_PERFECT) {
			flag = 0;
			for (i = 0; i < PN_ind; i++) {
				if (sharedMem->pnums[i] == message.msg) {
					flag = 1; break;
				}
			}
			if (flag == 0) {
				sharedMem->pnums[PN_ind] = message.msg;
				PN_ind++;
			}
			else
				printf(" Message: perfect number %d has been found by other process.\n", message.msg);
		} else if (message.msgtype == TP_MANAGE_PID) {
			message.msgtype = IS_MANAGE_PID;
			message.msg = getpid();
			if (msgsnd(qid, &message, sizeof(message.msg), 0) != 0) {
				perror(" manage.c: failed to send manage pid!");
				handler(0);
				exit(EXIT_FAILURE);
			}
		}
		else{
			printf(" manage.c: wrong message type!\n");
			for (i = 0; i < PROCESS_LEN; i++) {
				tmp = (sharedMem->proc[i].pid);
				if ( tmp != 0) {
					if (kill(tmp, SIGINT) != 0) {
						perror(" manage.c: handler(): failed to kill computing process!");
						exit(EXIT_FAILURE);
					}
				}
			}
			sleep(5);
			if (shmdt(sharedMem)) {
				perror(" manage.c: shmdt(): shared memory failed to detach!");
				exit(EXIT_FAILURE);
			}
			if (shmctl(sid, IPC_RMID, 0)) {
				perror(" manage.c: shmctl(): shared memory fail to control IPC_RMID!");
				exit(EXIT_FAILURE);
			}
			
			if (msgctl(qid, IPC_RMID, NULL)) {
				perror(" manage.c: msgctl(): message queue failed to control IPC_RMID!");
			}
			exit(EXIT_FAILURE);
		}
	}
}


