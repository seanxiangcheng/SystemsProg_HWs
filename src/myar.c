/*THIS CODE IS MY OWN WORK. IT WAS WRITTEN WITHOUT CONSULTING A TUTOR OR CODE WRITTEN BY OTHER STUDENTS - XIANG CHENG*/

/* This code is a mimic of UNIX command "ar". 
 * Written by XIANG CHENG
 * EMAIL: CHENGXIANG.CN@GMAIL.COM; XIANG.CHENG@EMORY.EDU
 */
#include <stdio.h>
#include <stdlib.h> 
#include <unistd.h> 
#include <stdio.h> 
#include <errno.h>


void usage()
{
    printf("Usage: ar key archive-file file...\n");
    printf("keys/commands\n");
    printf("  q : quickly append named files to archive\n");
    printf("  x : extract named files\n");
    printf("  xo: extract named files restoring mtime\n");
    printf("  t : print a concise table of contents of the archive\n");
    printf("  tv: print a verbose table of contents of the archive\n");
    printf("  d : delete named files from archive\n");
    printf("  A : quickly append all \"regular\" files in the current directory\n");
}

int main(int argc, char **argv)
{
    if ( argc < 3)
    {
        printf("Error: Only %d argument entered! 3 or more are needed!\n", argc);
        usage();
    }
    
    
    usage();
    
    return(1);
}
