/* Mathew Duhon
 * cpe 435
 * Nico lab 1
 */
#include<stdio.h>
#include<sys/types.h>
#include<unistd.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<stdlib.h>
#include<sys/wait.h>


#define READ_END 0 
#define WRITE_END 1



int main () {

int pipe1 [2];
int outFile;
pid_t child [2];

/* Make the pipe */
    if(pipe(pipe1)) 
        fprintf(stderr, "Pipe didn't work");


/* Make the output file */
    if((outFile = open("outfile",O_RDWR |  O_CREAT | O_TRUNC)) < 0) {
        perror("can't open outfile");
        exit(-1);
    }

/* Fork off child one */
    if(!(child[0] = fork())){
/* Copy the pipe's write end to the child's std out */
        if(-1 == dup2(pipe1[WRITE_END], STDOUT_FILENO)) {
            perror("dup2:0");
            exit(-1);
        }
/* Close pipe file descriptors */
       close(pipe1[WRITE_END]);
       close(pipe1[READ_END]);
        if(-1 == execlp("ls","ls",NULL)) {
            perror("execlp");
        }
        exit(1);
    }
/* Fork off child two */
    if(!(child[1] = fork())){
/* Copy the output file to the child's std out */
        if(-1 == dup2(outFile,STDOUT_FILENO)) {
            perror("dup2:1");
            exit(-1);
        }
/* Copy the pipe's read end to the child's std in */
        if(-1 == dup2(pipe1[READ_END], STDIN_FILENO)) {
            perror("dup2:2");
            exit(-1);
        }
/* Close pipe file descriptors */
       close(pipe1[WRITE_END]);
       close(pipe1[READ_END]);
        if(-1 == execlp("sort","sort","-r",NULL)) {
            perror("execlp");
        }
        exit(1);
    }
/* Close pipe file descriptors */
    close(pipe1[WRITE_END]);
    close(pipe1[READ_END]);
    if ( -1 == wait(NULL) ) { /* wait for child[0] */
        perror("wait");
    }
    if ( -1 == wait(NULL) ) { /* wait for child[1] */
        perror("wait");
    }

/* Close outFile file descriptors */
   close(outFile);
return 0;
}
