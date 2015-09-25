// test for homework 1


#include <ar.h>
#include <stdio.h>
#include <stdlib.h> 
#include <unistd.h> 
#include <stdio.h> 
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>


int main (int argc, char **argv)
{
    struct ar_hdr myheader;
    printf("ar_hdr:   %ld\n", sizeof(struct ar_hdr));
    printf("myheader: %ld\n", sizeof(myheader));
    
    
    printf("S_IRUSR:%o\n", S_IRUSR);
    printf("%o : %d\n", 0400 & S_IRUSR , 0400 & S_IRUSR);
    return(0);
}
