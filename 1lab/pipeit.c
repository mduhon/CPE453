#include<stdio.h>
#include<sys/types.h>
#include<unistd.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<stdlib.h>

#define READ_END 0 
#define WRITE_END 1



int main () {

int pipe1 [2];
int pipe2 [2];
int outFile;
pid_t child [2];


    if(pipe(pipe1)) 
        fprintf(stderr, "Pipe didn't work");

    if(pipe(pipe2)) 
        fprintf(stderr, "Pipe2 didn't work");

    if((outFile = open("outfile", O_CREAT | O_TRUNC)) < 0) {
        perror("can't open outfile");
        exit(-1);
    }

    child[0] = fork();
    child[1] = fork();

    if(child[0] == 0){
        if(-1 == dup2(pipe1[WRITE_END], STDOUT_FILENO)) {
            perror("dup2:0");
            exit(-1);
        }
        if(close(pipe2[WRITE_END]) || close(pipe2[READ_END]) || close(pipe1[WRITE_END]) || close(pipe1[READ_END])){
            perror("close");
            exit(-1);
        }
        if(-1 == execlp("ls","ls",NULL)) {
            perror("execlp");
        }
        exit(1);
    }
    if(child[1] == 0){
        if(-1 == dup2(outFile,STDOUT_FILENO)) {
            perror("dup2:1");
            exit(-1);
        }
        if(-1 == dup2(pipe2[READ_END], STDIN_FILENO)) {
            perror("dup2:2");
            exit(-1);
        }
        if(close(pipe2[WRITE_END]) || close(pipe2[READ_END]) || close(pipe1[WRITE_END]) || close(pipe1[READ_END])){
            perror("close");
            exit(-1);
        }
        if(-1 == execlp("sort","sort","-r",NULL)) {
            perror("execlp");
        }
        exit(1);
    }
    else{
       if(close(pipe2[WRITE_END]) || close(pipe2[READ_END]) || close(pipe1[WRITE_END]) || close(pipe1[READ_END] || close(outFile))){
          perror("close");
          exit(-1);
       }
    }
return 0;
}
