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
#include <dirent.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void d_printc(char *s, int l)
{
    int i=0;
    for (i = 0; i < l; i++)
    {
        printf("%c", s[i]);
    }
    printf("\n");
    
}

int main (int argc, char **argv)
{
    struct ar_hdr myheader;
    printf("ar_hdr:   %ld\n", sizeof(struct ar_hdr));
    printf("myheader: %ld\n", sizeof(myheader));
    struct stat sb;

    
    printf("S_IRUSR:%o\n", S_IRUSR);
    printf("%o : %d\n", 0400 
    & S_IRUSR , 0400 & S_IRUSR);
    int fd = open("four", O_RDONLY);
    char s[3];
    lseek(fd, 0, SEEK_SET);
    printf("read: %d \n", (int)read(fd, s, sizeof(s)));
    printf("%s \n", s);
    d_printc(s, 3);
    int b=0;
    int a[b];
    printf(" %d %d %d \n", a[0], a[1], a[2]);
    printf("%d\n\n\n", stat("sss", &sb));

/* Program illustrating reading of directories and mode bits */
/* This version uses chdir and relative path names */
printf("success: %d\n", lstat("Lc", &sb));
printf("fail: %d\n", lstat("four", &sb));

    return(0);
}
