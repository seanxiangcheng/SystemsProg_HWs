/*THIS CODE IS MY OWN WORK. IT WAS WRITTEN WITHOUT CONSULTING A TUTOR OR CODE WRITTEN BY OTHER STUDENTS - XIANG CHENG*/

/* Written by XIANG CHENG (xcheng7@emory.edu; 1938871)
 * EMAIL: CHENGXIANG.CN@GMAIL.COM; XIANG.CHENG@EMORY.EDU
 */

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

#define MAX_STR_LEN 41
#define MIN_STR_LEN 3
#define DEFAULT_SORT_NUM 4

void suppressor(int num, int ins[][2], int outs[][2]);
void usage(char **argv); // print useage information
int word2print(int num, char words[][MAX_STR_LEN]);
void hdl(int signal); // process signal handler 
int *procs; // global variable to store the process ids 


int main(int argc, char **argv){
    int num_sort;   // num of sorting processes
    int status; // status for wait
    int i=0; // iterator
    int child_pid=0, flag=0; // for fork process 
    int sorter_ind = 0, read_num, len_word=0; // for word parsering 
    struct sigaction act; // process sigaction act
    char word[MAX_STR_LEN];
    if ( argc == 1){
        num_sort = DEFAULT_SORT_NUM;
        printf("  Message: number of sorting proc not specified. Default value %d is used.\n", DEFAULT_SORT_NUM);
        printf("  If you have no stdin file, USE \"Ctrl-C\" to END the program!\n");
        usage(argv);        
    }
    else if(argc == 2){
        num_sort = atoi(argv[1]);
        if( num_sort < 1){
            printf("   Error: main(): Invalid sorting number: %s!\n", argv[1]);
            usage(argv); exit(EXIT_FAILURE);
        }
    }
    else if(argc > 2){
        printf("   Error: main(): only 2 args are needed!\n");
        usage(argv); exit(EXIT_FAILURE);
    }
    else{
        printf("   Error: wrong input!\n");
        usage(argv); exit(EXIT_FAILURE);
    }
        
    procs = malloc(sizeof(int)*(num_sort + 1)); // sorting+suppressor 
    int ins[num_sort][2];  // for parser
    int outs[num_sort][2]; // for supressor
    FILE *p_writes[num_sort]; // fds for write-parsers
    //FILE *p_reads[num];
    
    // Initialize signal handler 
    act.sa_handler = hdl; act.sa_flags = 0;
    sigemptyset(&act.sa_mask);
    sigaction(SIGINT,  &act, NULL);
    sigaction(SIGQUIT, &act, NULL);
    sigaction(SIGHUP,  &act, NULL);
            
    // Setup all pipes in the parent
    for ( i=0; i<num_sort; i++) {
        if ((pipe(ins[i]) != 0) || (pipe(outs[i]) != 0))
            perror("   Error: main(): pipe() error.");
    }
        
    /***************** Parse all the words ********************/
    for ( i = 0; i < num_sort; i++) {
        if ((p_writes[i] = fdopen(ins[i][1], "w")) == NULL)
            perror("   Error: main(): word parsing: fdopen()");
    }
    do{
        memset(word,'\0', MAX_STR_LEN);
        scanf("%*[^a-zA-Z]");
        read_num = scanf("%[a-zA-Z]", word);
        if (read_num == 1) {
            len_word = strlen(word);
            if(len_word >= MIN_STR_LEN){
                if (len_word>=MAX_STR_LEN){
                    word[MAX_STR_LEN-1] = '\0';
                    len_word = MAX_STR_LEN-1;
                }
                for(i=0; i<len_word; i++) word[i]=tolower(word[i]);
                    fputs(word, p_writes[sorter_ind]); // write to one of the sorters
                    //printf("sortid:%-3d:%s\n", sorter_ind, word);
                    fputs("\n", p_writes[sorter_ind]); //
                    sorter_ind = (sorter_ind + 1) % num_sort;
                    memset(word,'\0', MAX_STR_LEN);
            }
        }
        read_num = scanf("%*[^a-zA-Z]");
    } while (read_num != EOF);
    // close to flush buffer to sorting processes
    for ( i=0; i < num_sort; i++) fclose(p_writes[i]);
    /***************** Parse all the words Done ********************/
    
    //printf("   test: parser finished!\n");
    
    /***************** Sorting  *********************/
    // Fork the sorting processes with all buffered pipes
    for (i=0; i < num_sort; ++i) {
        child_pid = fork();
        //printf( "child_pid: %d\n", child_pid);
        if(child_pid == 0){ // child process 
            // child process get input from parser and output to suppressor
            close(STDIN_FILENO) ; // close std in 
            close(STDOUT_FILENO); // close std out
            if(dup2(ins[i][0], STDIN_FILENO)==-1)   perror("   main(): dup2(ins[i])");
            if(dup2(outs[i][1], STDOUT_FILENO)==-1) perror("   main(): dup2(outs[i])");
            close(outs[i][0]);
            flag = execlp("sort", "sort", (char *) NULL);
            if (flag==-1){
                    perror("   \\bin\\sort: execlp()");
                    _exit(EXIT_FAILURE);
            }
        }
            else if(child_pid == -1) perror("   Error: fork_sorters()."); // fork error
            else{ // parent process 
                procs[i] = child_pid;
                close(ins[i][0]); close(outs[i][1]);
            }
        }
        /***************** Sorting Done *********************/
        
        
        
        /*************** suppressor ***************/
        child_pid = fork();
        if(child_pid==0){
            //printf("Starting suppressor...\n");
            suppressor(num_sort, ins, outs); // run_suppressor
            printf("|-------------------------------------------------|\n\n");
        } 
        else if(child_pid==-1) perror("   Error: main(): fork suppressor failed.");
        else procs[num_sort]=child_pid; // suppressor id
        /************* suppressor Done *************/
        
        
        // wait for all to finish 
        for(i= 0; i<num_sort+1; i++) wait(&status);
        //printf("process %d ended!\n", i); // del
        
        /****** Close all the pipes: maybe it has been alread closed.*******/
        for(i=0; i<num_sort; i++) {
            close(ins[i][0]); close(ins[i][1]);
            close(outs[i][0]); close(outs[i][1]);
            //printf("closed: %d\n", i); // del
        }
        free(procs);
        return(0);
}


void suppressor(int num, int ins[][2], int outs[][2]){
    char word[MAX_STR_LEN];
    char words[num][MAX_STR_LEN];
    int word_count=0, i, num_emt_ps=0, wd_ind;
    int line_out = 0;
    FILE *p_reads[num];
    memset(word,'\0', MAX_STR_LEN);
    memset(words,'\0', MAX_STR_LEN*num);
    for (i=0; i<num; i++) {
	p_reads[i] = fdopen(outs[i][0], "r");
	if (fgets(words[i], MAX_STR_LEN, p_reads[i]) == NULL) {
	    memset(words[i], '\0', MAX_STR_LEN);
	    num_emt_ps++;
	}
	if(strlen(words[i]) < 3) words[i][0] = '\0';
    }
    word_count = 0;
    wd_ind = word2print(num, words);
    strcpy(word, words[wd_ind]);
    memset(words[wd_ind], '\0', MAX_STR_LEN);
    //if(word[strlen(word)-1] == '\n') word[strlen(word)-1] = '\0';
    ++word_count;
    printf("+------------------------------------------------+\n");
    printf("|  num | word                                    |\n");
    printf("|------|-----------------------------------------|\n");
    while (num_emt_ps <= num) {
	if(fgets(words[wd_ind], MAX_STR_LEN, p_reads[wd_ind])==NULL) {
	    words[wd_ind][0] = '\0'; 
	    num_emt_ps++;
	}
	wd_ind = word2print(num, words);
	if( (wd_ind != -1) && (strcmp(word, words[wd_ind])==0) ) word_count++;
	else{
	    if(word[strlen(word)-1]=='\n') word[strlen(word)-1] = '\0';
	    if(strlen(word)>2) {
		printf("|%5d | %-40s|\n", word_count, word);
		word_count = 0;
		line_out++;
	    }
	    if(num_emt_ps == num){
		printf("+------------------------------------------------+\n");
		printf("\n"); 
	    }
	    if (wd_ind >= 0) {
		memset(word,'\0', MAX_STR_LEN);
		strcpy(word, words[wd_ind]);
		word_count++;
	    }
	}

    }
    for(i=0; i<num; i++) fclose(p_reads[i]);
    _exit(EXIT_SUCCESS); // exit the suppressor process
}



int word2print(int num, char words[][MAX_STR_LEN])
{
    int ind=-1, i=0;
    for(i=0; i<num; i++){
        if (words[i][0] != '\0') {
            if(ind==-1) ind=i;
            else if((strcmp(words[i], words[ind]) < 0))ind = i;
        }
    }
    return (ind);
}


void hdl(int signal)
{
    int sig = SIGQUIT;
    int procs_num = sizeof(procs)/sizeof(int);
    int i = 0, status;
    if(signal==SIGINT) sig=SIGINT;
    for (i=0; i<procs_num; i++)  kill(procs[i], sig);
    free(procs);
    for(i = 0; i<procs_num; i++) wait(&status);
    exit(EXIT_SUCCESS);
}


void usage(char **argv)
{
    printf("  Usage:   %s num_sort <filename\n", argv[0]);
    printf("  example: %s 4 <file.txt\n", argv[0]);
    printf("           uniquify the words in file.txt using 4 sorting processes.\n");
}
