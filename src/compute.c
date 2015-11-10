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
int processIndex = -1;

// handler code that zeroes out proc statistics and exits
void terminate(int signum) {

	if (processIndex == -1) {
		exit(0);
	}
	sharedMem->proc[processIndex].pid = 0;
	sharedMem->proc[processIndex].numFound = 0;
	//printf("Total numbers processed: %d\n", sharedMem->proc[processIndex].countTested + sharedMem->proc[processIndex].countSkipped);
	sharedMem->proc[processIndex].numTested = 0;
	sharedMem->proc[processIndex].numSkipped = 0;

	exit(0);
}

int whichInt(int j) {
	return ((j - 2) / 32);
}

int whichBit(int j) {
	return ((j - 2) % 32);
}

// test if a number is perfect
int test(int testNumber) {
	int sumSoFar = 0, sumOfDivisors = 0, i;

	for (i = 1; i < testNumber; i++) {
		if (testNumber % i == 0) {
			sumSoFar += i;
		}
	}
	sumOfDivisors = sumSoFar;

	if (sumOfDivisors == testNumber) {
		return 1;	// perfect
	} else {
		return 0;	// not perfect
	}
}

// loops through bitmap starting at startNumber
int compute(int startNumber) {
	int thisInt, thisBit, testNumber = startNumber;
	int thisPID, wrappedAround = 0;

	// get and attach shared sharedMem segment and message queue
	sid = shmget(SHMM_KEY, sizeof(struct Shared_Info_Block), 0);
	if (sid == -1 ) {
		perror(" compute.c: Shared memory segment has not been created!");
		exit(1);
	}
	sharedMem = shmat(sid, NULL, 0);
	if (sharedMem == (void *) -1) {
		perror(" compute.c: Shared memory segment has not been created!");
		exit(1);
	}
	qid = msgget(MESQ_KEY, 0);
	if (qid == -1) {
		perror(" compute.c: Message queue has not been created!");
		exit(1);
	}

	// create message that registers compute with manage
	struct My_Msg message;
	thisPID = getpid();
	message.msgtype = TP_PROC_INDEX;
	message.msg = thisPID;
	if (msgsnd(qid, &message, sizeof(message.msg), 0) != 0) {
		perror("proc[] index failed");
		exit(1);
	}

	// waits for response message with proc statistics array index
	// and verifies that the slot belongs to it
	msgrcv(qid, &message, sizeof(message.msg), IS_PROC_INDEX, 0);
	processIndex = message.msg;
	if (sharedMem->proc[processIndex].pid != thisPID) {
		perror("pids don't match!");
		exit(1);
	}

	// main loop
	while (1) {
		if (testNumber > MAX_NUM2TEST) {
			testNumber = 2;
			wrappedAround = 1;
		}
		if (testNumber == startNumber && wrappedAround == 1) {
			// we have looped around the entire bitmap
			terminate(SIGQUIT);
		}

		// get integer index and bit index of bit that represents testNumber
		thisInt = whichInt(testNumber);
		thisBit = whichBit(testNumber);

		// test the number if it has not been tested yet
		// then set the bit and update proc statistics
		if ((sharedMem->bitmap[thisInt] & (1 << thisBit)) == 0) {
			if (test(testNumber)) {
				// tell manager that testNumber is perfect
				message.msgtype = TP_NUM_FOUND_PERFECT;
				message.msg = testNumber;
				if (msgsnd(qid, &message, sizeof(message.msg), 0) != 0) {
					perror("Perfect number send failed");
					exit(1);
				}
				sharedMem->proc[processIndex].numFound++;
			} 
			sharedMem->bitmap[thisInt] |= (1 << thisBit);
			sharedMem->proc[processIndex].numTested++;
		} else {
			sharedMem->proc[processIndex].numSkipped++;
		}
		testNumber++;
	}
}

// entry point
int main(int argc, char *argv[]) {

	// validate number of arguments
	if (argc != 2) {
		printf("usage: ./compute [start number]\n");
		exit(0);
	}

	// validate start number
	int startNumber = atoi(argv[1]);
	if (startNumber > 1024001 || startNumber < 2) {
		printf("Start number x is outside the acceptable range! 2 <= x <= 1,024,001\n");
		exit(0);
	}

	// set up signal handler behavior
	struct sigaction signal;
	memset(&signal, 0, sizeof(signal));
	signal.sa_handler = terminate;
	if (sigaction(SIGINT, &signal, NULL) != 0) {
		perror("SIGINT set failed");
		exit(1);
	}
	if (sigaction(SIGQUIT, &signal, NULL) != 0) {
		perror("SIGQUIT set failed");
		exit(1);
	}
	if (sigaction(SIGHUP, &signal, NULL) != 0) {
		perror("SIGHUP set failed");
		exit(1);
	}

	// begin computation at startNumber
	compute(startNumber);
	return(0);
}
