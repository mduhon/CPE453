#define _XOPEN_SOURCE 600

#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<termios.h>
#include<unistd.h>
#include<sys/types.h>
#include<pwd.h>
#include<sys/ioctl.h>
#include<sys/wait.h>
#include<time.h>

#define BUF_SIZE 100
extern char **environ;


static void get_winsize(int fd, struct winsize *ws); 
static void set_winsize(int fd, struct winsize *ws);
void restore_mode(int fd, struct termios *old);
void raw_mode(int fd, struct termios *old);
void printId(int where);

pid_t inputId, slaveId;


int main (int argc,char * argv[]) {
    pid_t input, slave;
    time_t time;
    int outfile, pty, slavePty, toPrint, status;
    char inData [BUF_SIZE];
    char outData [BUF_SIZE];
    char *shell;
    struct winsize win;
    struct termios original;
    struct sigaction new, old;
    struct passwd *pass;
    
    /*Opens the correct output file depending on the request */
    if(argc != 1 && argc != 2) {
        perror("Please enter correct # of arguments\n");
        exit(-1);
    }
    if(argc == 2) {
        outfile = fileno(fopen(argv[1], "w+")); 
        printf("Script started. File is %s.\n", argv[1]);
    } else {
        outfile = fileno(fopen("typescript", "w+")); 
        printf("Script started. File is typescript.\n");
    }
    fflush(stdout);

    /*Print pid incase I need to terminate it manually */
    printId(1);

    /*make sure you are in a tty */
    if(isatty(STDIN_FILENO) != 1){
        perror("Not a tty\n");
        exit(-1);
    }
    /*get window size for later and then palce into raw mode*/
    get_winsize(STDIN_FILENO,&win);
    raw_mode(STDIN_FILENO, &original);

    /*opens the pty and sets flags */
    pty = posix_openpt(O_RDWR | O_NOCTTY);
    if(pty == -1) {
        perror("No pty");
        exit(-1);
    }
    
    /*do slave stuff */
    if(!(slave = fork())){
        /*Print pid incase I need to terminate it manually */
        printId(2);
        slaveId = setsid();
        if( grantpt(pty) == -1 || unlockpt(pty) == -1){
            perror("can't get slave end");
            exit(-1);
        }
        slavePty = open(ptsname(pty), O_RDWR);
        if(isatty(slavePty) != 1) { 
            perror("slave open");
            exit(-2);
        }
        restore_mode(slavePty, &original);
        set_winsize(slavePty,&win);

        if(-1 == dup2(slavePty, STDOUT_FILENO)) {
            perror("dup2:0");
            exit(-1);
        }
        if(-1 == dup2(slavePty, STDIN_FILENO)) {
            perror("dup2:0");
            exit(-1);
        }
        if(-1 == dup2(slavePty, STDERR_FILENO)) {
            perror("dup2:0");
            exit(-1);
        }
        
        close(outfile);
        close(pty);
        close(slavePty);
        shell = getenv("SHELL");
        if(shell == NULL || *shell == '\0') {
            pass = malloc(sizeof(*pass));
            if(pass == NULL) {
                perror("Malloc");
                exit(-1);
            }
            pass = getpwuid(getuid());
            shell = pass->pw_shell;
        }
        execlp(shell,shell, NULL);
        perror("Execl didn't work");

        exit(1);
    }
    /*do input stuff */
    if(!(input = fork())){
        /*set up sig handler*/
        if(sigaction(SIGTERM,&new,&old )) {
            close(pty);
            close(outfile);
            exit(1);
        }
        printId(3);
        inputId = getpid();
        
        while((toPrint = read(STDIN_FILENO, inData, BUF_SIZE)) > 0) {
            write(pty, inData, toPrint);
          }
        close(pty);
        close(outfile);
    }
    /*read from pty and write to both log and stdout*/
    while((toPrint = read(pty, outData, BUF_SIZE)) > 0) {
        write(outfile, outData, toPrint);
        write(STDOUT_FILENO, outData, toPrint);
    }
    /*send signal to input*/

    kill(inputId,SIGTERM);
    waitpid(inputId, &status, 0);
    waitpid(slaveId, &status, 0);
    /* wait for slave *//*
    if ( -1 == wait(NULL) ) { 

        perror("wait");
    }*/
    /* wait for input *//*
    if ( -1 == wait(NULL) ) { 

        perror("wait");
    }*/

    /* clean up stuff */
    restore_mode(STDIN_FILENO, &original);
    set_winsize(STDIN_FILENO,&win);
    close(pty);
    close(outfile);
    printf("Script finished on %s\n", asctime(localtime(&time)));
    fflush(stdout);

    return 0;
}
void raw_mode(int fd, struct termios *old) {
/* save the old terminal modes and set it into raw mode
 *    */
    struct termios tio;
    if ( !isatty(fd) ) {        /* sanity check */
        fprintf(stderr,"raw_mode: fd not a tty\n");
        exit(-1);
    }
    if ( tcgetattr(fd, old) ) {
        perror("tcgetattr");
        exit(-1); 
    }
    tio = *old; /* make a copy of these modes */
/* turn off echoing and signal generation and move to non–canonical mode */
    tio.c_lflag &= ~ (ECHO | ECHOE | ECHOK | ECHONL | ICANON | ISIG ) ;
    tio.c_cc[VMIN] = 1;            /* block for one character, but no more */
    tio.c_cc[VTIME] = 0;
    if ( tcsetattr(fd, TCSAFLUSH, &tio) ) {
        perror("tcsetattr");
        exit(-1);
    }
}
void restore_mode(int fd, struct termios *old) {
      /* apply the given modes to the given file descriptor. */
      if (!isatty(fd)) {        /* sanity check */
        fprintf(stderr,"restore_mode: fd not a tty\n");
        exit(-1);
      }
      if ( tcsetattr(fd, TCSAFLUSH, old) ) {
         perror("tcsetattr");
         exit(-1);
      }
}


static void get_winsize(int fd, struct winsize *ws) {
      /* get the window size of the given tty */
      if ( -1 == ioctl(fd,TIOCGWINSZ,ws) ) {
        perror("ioctl get winsize");
      }
}
static void set_winsize(int fd, struct winsize *ws) {
      /* set the window size of the given tty */
      if ( -1 == ioctl(fd,TIOCSWINSZ,ws) ) {
         perror("ioctl set winsize");
      }
}

void printId(int where) {
    /*Print pid incase I need to terminate it manually */
    switch (where) {
        case 1:
            fprintf(stderr, "Parent process id is %d\n",getpid()); 
            break;
        case 2: 
            fprintf(stderr, "Slave process id is %d\n",getpid()); 
            break;
        case 3: 
            fprintf(stderr, "input process id is %d\n",getpid()); 
            break;
    }
}
