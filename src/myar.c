/*THIS CODE IS MY OWN WORK. IT WAS WRITTEN WITHOUT CONSULTING A TUTOR OR CODE WRITTEN BY OTHER STUDENTS - XIANG CHENG*/

/* This code is a mimic of UNIX command "ar". 
 * Written by XIANG CHENG
 * EMAIL: CHENGXIANG.CN@GMAIL.COM; XIANG.CHENG@EMORY.EDU
 */
#include <ar.h>
#include <stdio.h>
#include <stdlib.h> 
#include <unistd.h> 
#include <stdio.h> 
#include <errno.h>


// 'q': quickly append named files to archive
int append_files (int argc, char **argv);

// 'x' : extract named files; 'xo': extract named files restoring mtime
int extract_files (int argc, char **argv, int xo);

// 't' : print a concise table of contents of the archive
// 'tv': print a verbose table of contents of the archive
int print_table (int argc, char **argv, int tv);

// 'd' : delete named files from archive
int delete_files (int argc, char **argv);

// 'A' : quickly append all 'regular' files in the current directory
int append_dir (int argc, char **argv);


void usage();

void wrong_key(char *arg);

int main(int argc, char **argv)
{
    int key_len = 0;
    if ( argc < 3){
        printf("Error: Only %d argument entered! 3 or more are needed!\n", argc);
        usage(); exit(EXIT_FAILURE);
    }
    
    while(argv[1][++key_len] != '\0'){}
    printf("    key_len = %d\n", key_len);
    if (key_len > 2)
    {
        printf("    Error: key \"%s\" not available in command myar!\n", argv[1]);
        usage();
        exit(EXIT_FAILURE);
    }
    
    switch (argv[1][0]){
        case 'q':
            if(key_len==1){
                append_files(argc, argv);
                printf("    key 'q' passed\n");
            }
            else
                wrong_key(argv[1]);
            break;
        case 'x':
            if ( key_len == 1){
                extract_files(argc, argv, 0);
                printf("    key 'x' passed\n");
            }
            else if (argv[1][1] == 'o'){
                extract_files(argc, argv, 1);
                printf("    key \"xo\" passed\n");
            }
            else
                wrong_key(argv[1]);
            break;
        case 't':
            if ( key_len == 1){
                print_table(argc, argv, 0);
                printf("    key 't' passed\n");
            }
            else if (argv[1][1] == 'v'){
                extract_files(argc, argv, 1);
                printf("    key \"tv\" passed\n");
            }
            else
                wrong_key(argv[1]);
            break;
        case 'd':
            if(key_len == 1){
                delete_files(argc, argv);
                printf("    key 'd' passed\n");
            }
            else
                wrong_key(argv[1]);
            break;
        case 'A':
            if(key_len == 1) {
                append_dir(argc, argv);
                printf("    key 'A' passed\n");
            }
            else
                wrong_key(argv[1]);
            break;
        default:
            wrong_key(argv[1]);
    }
    
    return(1);
}

// 'q': quickly append named files to archive
int append_files (int argc, char **argv)
{
    return(0);
}

// 'x' : extract named files; 'xo': extract named files restoring mtime
int extract_files (int argc, char **argv, int xo)
{
    return(0);
}

// 't' : print a concise table of contents of the archive
// 'tv': print a verbose table of contents of the archive
int print_table (int argc, char **argv, int tv)
{
    struct ar_hdr header;
    
    return(0);
}

// 'd' : delete named files from archive
int delete_files (int argc, char **argv)
{
    return(0);
}

// 'A' : quickly append all 'regular' files in the current directory
int append_dir (int argc, char **argv)
{
    return(0);
}

void usage()
{
    printf("    Usage: ./myar key archive-file file...\n");
    printf("    keys/commands\n");
    printf("      q : quickly append named files to archive\n");
    printf("      x : extract named files\n");
    printf("      xo: extract named files restoring mtime\n");
    printf("      t : print a concise table of contents of the archive\n");
    printf("      tv: print a verbose table of contents of the archive\n");
    printf("      d : delete named files from archive\n");
    printf("      A : quickly append all \"regular\" files in the current directory\n");
}

void wrong_key(char *arg)
{
    printf("    Error: key \"%s\" not available in command myar!\n", arg);
    usage();
    exit(EXIT_FAILURE);  
}
