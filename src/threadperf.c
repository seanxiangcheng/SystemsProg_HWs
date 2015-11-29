/*THIS CODE IS MY OWN WORK. IT WAS WRITTEN WITHOUT CONSULTING A TUTOR OR CODE WRITTEN BY OTHER STUDENTS - XIANG CHENG*/

/*  Written by XIANG CHENG (xcheng7@emory.edu; 1938871)
    EMAIL: CHENGXIANG.CN@GMAIL.COM; XIANG.CHENG@EMORY.EDU
 */

#include <string.h>
#include <pthread.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <sys/times.h>
#include <sys/resource.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>


#define THREAD_MAX_NUM 64
#define PERF_LEN 8
#define BM_INT_MAX_LEN 320000
#define MAX2TEST 10240000


#define CMD_LEN 64
#define LINE_MAX 128


#define T_RUNNING 1
#define T_IDLE 2
#define T_WAITING 3
#define T_FINISHED 4

#define C_START 1
#define C_IDLE 2
#define C_RESTART 3
#define C_WAIT 4
#define C_REPORT 5
#define C_QUIT 6
#define C_EMPTY 7
#define C_UNKNOWN 8




long long Max; // max number to test 
int Block; // block size for each bit
int Bitmap_Len; // length of Bitmap
long long Perfnums[PERF_LEN]; // store perfect numbers
int NumFound = 0;
long long Ki; // start Ki; idle Ni; restart Ni; 
int Si, Ni;  
unsigned int Bitmap[BM_INT_MAX_LEN];
pthread_t Tids[THREAD_MAX_NUM];
time_t Start_Time, End_Time;


char Cmd[2][CMD_LEN];
char *Line;
int Tid_num = 0; // number of threads running, idle, and ended


pthread_mutex_t mtx;
pthread_cond_t cond;
pthread_attr_t attr;

struct Thread_Info {
	pthread_t tid;		// tid
	int numFound;		// num of perf found
	long long perfnums[PERF_LEN];	// perfect numbers
	long long numSkipped;		// number Skipped, unit 1
	long long numTested;		// number tested, unit 1
	int curBlock;		// current block index
	int curStatus;		// current status
} Threads[THREAD_MAX_NUM];

void * compute(void *ki);
int getcmd(ssize_t size);
void print_cmd_code(int cmd_code);
int check_perf(long long num2test);
void init_thread_info (struct Thread_Info *thread);
void report_threads();
void quit_threads();

int main(int argc, char *argv[]){
    time(&Start_Time);
    int i;
    int cmd_code=0; 
    ssize_t read_len;
    size_t read_ok=0;
    
    if (argc != 3) {
	printf(" Error: main(): wrong argument(s)!\
	\n usage: %s [MAX] [BLOCK]\n", argv[0]);
	exit(EXIT_FAILURE);
    }
    
    Max = atoll(argv[1]);
    Block = atoi(argv[2]);
    if(Max>MAX2TEST){
	printf(" Message: %lld is too big; reset to %d", Max, MAX2TEST);
	Max = MAX2TEST;
    }
    else if(Max<2){
	printf(" Message: %lld is too big; reset to %d", Max, 10);
	Max = 10;
    }
    
    if( (Max-2)%Block == 0 )
	Bitmap_Len = (Max-2)/Block;
    else
	Bitmap_Len = (Max-2)/Block + 1;
	
    if(Max==2)
	Bitmap_Len = 1;
    
    for (i = 0; i < THREAD_MAX_NUM; i++) {
	init_thread_info(&Threads[i]);
	//printf("%d : %d\n", i, Threads[i].curStatus); // del
    }
    for (i = 0; i < PERF_LEN; i++){
	Perfnums[i]=-1;
    }
    memset( Bitmap, (unsigned int)0, sizeof(unsigned int)*BM_INT_MAX_LEN); // set bitmap to 0
    
    /************ set up pthread  **************/
    if(pthread_attr_init(&attr)!= 0){
	perror("main(): pthread_attr_init()");
	exit(EXIT_FAILURE);
    }
    if(pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM)!=0){
	perror("main(): pthread_attr_setscope");
	exit(EXIT_FAILURE);
    }
    //init mutex
    if(pthread_mutex_init(&mtx, NULL)!=0){
	perror("main(): pthread_mutex_init");
	exit(EXIT_FAILURE);
    }
    //init condition
    if(pthread_cond_init(&cond, NULL)!=0){
	perror("main(): pthread_cond_init");
	exit(EXIT_FAILURE);
    }

    while(1){
	memset(Cmd, '\0', sizeof(char)*2*CMD_LEN);
	printf("xcheng7>> ");
	read_len = getline(&Line, &read_ok, stdin);
	cmd_code = getcmd(read_len);
	//print_cmd_code(cmd_code); // del
	switch(cmd_code){
	    case C_START:
		    Ki = atoi(Cmd[1]);
		    if(Ki<2) {Ki=2;}
		    else if(Ki>Max){
			Ki = Max;
			printf(" MAX set in the beginning is %lld!\n Starting number is reset to be %lld!\n", Max, Max);
		    }
		    if(Tid_num < THREAD_MAX_NUM){
			Threads[Tid_num].numFound = 0;
			Threads[Tid_num].numSkipped = 0;
			Threads[Tid_num].numTested = 0;
			Threads[Tid_num].curStatus = T_RUNNING;
			Threads[Tid_num].curBlock = (Ki-2)/Block;
			pthread_create(&(Tids[Tid_num]), &attr, compute, (void *)&Ki);
			printf(" Thread %d started.\n", Tid_num+1);
		    }
		    else{
			printf(" Max number (%d) of threads reached!\n", THREAD_MAX_NUM);
		    }
		    sleep(0);
		    break;
	    
	    case C_IDLE:
		    Ni = atoi(Cmd[1]);
		    if(Ni > THREAD_MAX_NUM || Ni>Tid_num){
			printf(" Thread No.%d doesn't exist! Only %d threads started!\n", Ni, Tid_num);
		    } else{
			Threads[Ni-1].curStatus = T_IDLE;
		    }
		    sleep(0);
		    break;
		
	    case C_RESTART:
		    Ni = atoi(Cmd[1]);
		    if(Ni > THREAD_MAX_NUM || Ni>Tid_num){
			printf(" Thread No.%d doesn't exist! Only %d threads started!\n", Ni, Tid_num);
		    }
		    else if(Ni==0){
			printf(" Invalid Threads No. Threads No. starts at 1!\n");
		    } 
		    else if(Threads[Ni-1].curStatus != T_IDLE){
			Threads[Ni-1].curStatus = T_RUNNING;
			pthread_cond_broadcast(&cond);
			printf(" Thread No. %d is not idle; restart anyway!\n", Ni);
		    }
		    else if(Ni==0){
			printf(" Thread No.0 is invalid; No. starts at 1!\n" );
		    }
		    else{
			Threads[Ni-1].curStatus = T_RUNNING;
			pthread_cond_broadcast(&cond);
		    }
		    sleep(0);
		    break;
		    
	    case C_WAIT:
		    Si = atoi(Cmd[1]);
		    sleep(Si);
		    break;
		    
	    case C_REPORT:
		    report_threads();
		    break;
		    
	    case C_QUIT:
		    quit_threads();
		    sleep(0);
		    break;
	    
	    case C_EMPTY:
		    sleep(0);
		    break;
	    case C_UNKNOWN:
		    printf(" %s %s: Unknown command!\n", Cmd[0], Cmd[1]);
		    break;
	    default:
		    break;
	}
	
	
    }
    return(0);
}

void report_threads(){
    int i=0, j=0;
    int width = 69;
    printf("\n");
    for (i = 0; i < width; i++)
	printf("=");
    printf("\n|                                Report                             |\n");
    for (i = 0; i < width; i++)
	printf("=");
    printf("\n| %-3s| %-8s | %-8s | %-6s | %-6s | %-20s |\n",
	    "No.", "tested", "skipped", "block", "status", "perfect numbers");

    for(i=0; i<Tid_num; i++){
	printf("|");
	for (j = 0; j < width-2; j++)
	    printf("-");
	printf("|");
	printf("\n| %-2d | %-8lld | %-8lld | %-6d ", i+1, Threads[i].numTested, Threads[i].numSkipped, Threads[i].curBlock);
	switch (Threads[i].curStatus){
		case T_RUNNING:
		    printf("| %-6s ", "normal");
		    break;
		case T_IDLE:
		    printf("| %-6s ", "idle");
		    break;
		case T_WAITING:
		    printf("| %-6s ", "wait");
		    break;
		case T_FINISHED:
		    printf("| %-6s ", "done");
		    break;
		default:
		    break;
	}
	printf("| ");
	for(j=0; j<Threads[i].numFound; j++){
		if(Threads[i].perfnums[j]>0)
		    printf("%-4lld ", Threads[i].perfnums[j]);
	}
	for(j=0; j<(20-Threads[i].numFound*5); j++){
	    printf(" ");
	}
	printf(" |\n");
    }
    for(j=0; j<width; j++){
	printf("=");
    }
    printf("\n\n");
}


void * compute(void *ki) {
	pthread_mutex_lock(&mtx);
	long long num2start = *((long long *)ki);
	int tid_ind = Tid_num;
	Tid_num++;
	pthread_mutex_unlock(&mtx);
	
	
	struct Thread_Info *pt = &(Threads[tid_ind]);
	Threads[tid_ind].tid = Tids[tid_ind];
	Threads[tid_ind].numFound = 0;
	Threads[tid_ind].numSkipped = 0;
	Threads[tid_ind].numTested = 0;
	Threads[tid_ind].curStatus = T_RUNNING;
	Threads[tid_ind].curBlock = (num2start-2)/Block;
	
	long long num2test = (num2start-2)/Block * Block + 2;
	num2start = num2test;
	//printf(" tid: %lu; No. %d (ind: %d); num2st: %lld (%d)\n", Threads[tid_ind].tid, Tid_num, tid_ind, num2start, Threads[tid_ind].curBlock);// del

	int all_done_flag = 0;
	
	int intnow;
	int bitnow;
	int i, j, testflag = 0, diff_pf_flag=0;
	while(1){
	    if (num2test > Max){
		num2test = 2;
		Threads[tid_ind].curBlock=0;
		all_done_flag=1;
	    }
	    if(num2test>=num2start && all_done_flag==1) {
		Threads[tid_ind].curStatus = T_FINISHED;
		pthread_exit(NULL);
	    }
	    intnow = Threads[tid_ind].curBlock / 32;
	    bitnow = Threads[tid_ind].curBlock % 32;
	    testflag = 0;
	    diff_pf_flag = 0;
	    
	    pthread_mutex_lock(&mtx);
	    if( (Bitmap[intnow] & ((unsigned int)1 << bitnow) ) == 0){
		Bitmap[intnow] |= ((unsigned int)1 << bitnow);
		testflag = 1;
	    }
	    pthread_mutex_unlock(&mtx);
	    
	    if(testflag==0){
		Threads[tid_ind].numSkipped = Threads[tid_ind].numSkipped + Block;
		if(Bitmap_Len == (Threads[tid_ind].curBlock+1)){
		    Threads[tid_ind].numSkipped = Threads[tid_ind].numSkipped - Block + Max%Block-1;
		}
		Threads[tid_ind].curBlock++;
		num2test = num2test + Block;
	    }
	    else{
		for (i=0; i<Block; i++){
		    if(check_perf(num2test)==1){
			    pthread_mutex_lock(&mtx);
			    for(j=0; j<NumFound; j++){
				if(Perfnums[j]==num2test){
				    diff_pf_flag=1;
				    break;
				}
			    }
			    if(diff_pf_flag==0){
				Perfnums[NumFound] = num2test;
				NumFound++;
				Threads[tid_ind].perfnums[Threads[tid_ind].numFound] = num2test;
				Threads[tid_ind].numFound++;
			    }
			    pthread_mutex_unlock(&mtx);
			    diff_pf_flag=0;
		    }
		    Threads[tid_ind].numTested = Threads[tid_ind].numTested + 1;
		    if(Threads[tid_ind].curStatus==T_IDLE){
				pthread_mutex_lock(&mtx);
				do{
					//printf(" Thread %d is suspended!\n", tid_ind);
					pthread_cond_wait(&cond, &mtx);
					//printf(" Thread %d is awaken\n", tid_ind);
				}while(Threads[tid_ind].curStatus==T_IDLE);
				//printf(" Thread %d is restarted\n", tid_ind);
				pthread_mutex_unlock(&mtx);
		    }
		    num2test++;
		    if(num2test>Max) break;
		}
		pt->curBlock++;
	    }
	}
	
	pthread_exit(NULL);
}


void quit_threads(){
	int i=0;
	long long total_tested=0, total_skipped=0;
	struct rusage comp_time;
	double user_time;
	printf("\n Quitting processing ......\n * %-23s ", "Perfect numbers found: ");
	for(i=0; i< NumFound; i++){
		printf("%lld  ", Perfnums[i]);
	}
	printf("\n");
	for (i = 0; i < Tid_num; i++){
	    Threads[i].curStatus = T_IDLE;
	}
	
	for (i = 0; i < Tid_num; i++){
	    total_tested = total_tested + Threads[i].numTested;
	    total_skipped = total_skipped + Threads[i].numSkipped;
	}
	printf(" * %-23s %lld \n", "Total numbers tested:", total_tested);
	printf(" * %-23s %lld \n", "Total numbers skipped:", total_skipped);

	

	if( getrusage(RUSAGE_SELF, &comp_time) != 0 ){
		perror(" quit_proc(): getrusage");
		exit(EXIT_FAILURE);
	}
	user_time = comp_time.ru_stime.tv_sec 
		    + comp_time.ru_utime.tv_sec 
		    + comp_time.ru_utime.tv_usec/1.0e6
		    + comp_time.ru_stime.tv_usec/1.0e6;
	printf(" * %-23s %-.6f seconds \n", "Total CPU time:", user_time);
	time(&End_Time);
	printf(" * %-23s %ld seconds\n\n", "Elapsed time:", End_Time-Start_Time);

	if(pthread_attr_destroy(&attr)!=0){
		perror(" quit_proc(): pthread_mutexattr_destroy");
		exit(EXIT_FAILURE);
	}
	if(pthread_mutex_destroy(&mtx)!=0){
		perror(" quit_proc(): pthread_mutex_destroy");
		exit(EXIT_FAILURE);
	}
	if(pthread_cond_destroy(&cond)!=0){
		perror(" quit_proc(): pthread_cond_destroy");
		exit(EXIT_FAILURE);
	}

	exit(EXIT_SUCCESS);	
}

int getcmd(ssize_t size){
	int cnt=0;
	ssize_t i=0;	
	if(size > LINE_MAX){
	    printf(" Error: max command per line is %d!\n", LINE_MAX);
	}
	
	while(isspace((int)Line[i])!=0 && i<size){
	    i++;
	}
	if(i==size){
	    //printf(" Message: only white spaced entered!\n");
	    return(C_EMPTY);
	}
	// first word
	while( isspace( (int)Line[i] )== 0 && i<size){
	    Cmd[0][cnt++] = toupper(Line[i]);
	    i++;
	}
	while(isspace((int)Line[i])!=0 && i<size){
	    i++;
	}
	// second word if any
	cnt = 0;
	while( isspace( (int)Line[i] )==0 && i<size){
	    Cmd[1][cnt++] = toupper(Line[i]);
	    i++;
	}

	//printf(" Command parsed: %s %s\n", Cmd[0], Cmd[1]);
	if(strlen(Cmd[0])==0){
		return(C_EMPTY);
	}
	
	if(strcmp(Cmd[0], "START")==0) return(C_START);
	else if(strcmp(Cmd[0], "IDLE")==0) return(C_IDLE);
	else if(strcmp(Cmd[0], "RESTART")==0) return(C_RESTART);	
	else if(strcmp(Cmd[0], "WAIT")==0) return(C_WAIT);
	else if(strcmp(Cmd[0], "REPORT")==0) return(C_REPORT);
	else if(strcmp(Cmd[0], "QUIT")==0) return(C_QUIT);
	else return(C_UNKNOWN);	 
}



void init_thread_info (struct Thread_Info *thread) {
	int i;
	thread->tid = 0;
	thread->numFound = -1;
	for (i = 0; i < PERF_LEN; i++){
	    thread->perfnums[i]=-1;
	}
	thread->numSkipped = -1;
	thread->numTested = -1;
	thread->curBlock = -1;
	thread->curStatus = -1;
}


int check_perf(long long num2test){
	int i = 1;
	unsigned int sums=0;
	for (i=1; i<num2test; i++) {
	    if (num2test % i == 0)
		sums += i;
	}
	if (sums == num2test) 
	    return(1);
	else
	    return(0);
}

void print_cmd_code(int cmd_code){
    switch(cmd_code){
	case 1: 	printf(" Start.\n"); break;
	case 2: 	printf(" Idle.\n"); break;
	case 3: 	printf(" Restart.\n"); break;
	case 4:  	printf(" Wait.\n"); break;
	case 5: 	printf(" Report.\n"); break;
	case 6: 	printf(" Quit.\n"); break;
	case 7:		break;
	case 8:		printf(" Unknown Command.\n"); break;
	default: 	printf(" Other cmd\n"); 
    }

}
