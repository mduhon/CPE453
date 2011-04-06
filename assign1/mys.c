#include<stdio.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<stdlib.h>
#include<termios.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/ioctl.h>
#include<sys/wait.h>

#define BUF_SIZE 100


static void get_winsize(int fd, struct winsize *ws); 
static void set_winsize(int fd, struct winsize *ws);
void restore_mode(int fd, struct termios *old);
void raw_mode(int fd, struct termios *old);


int main (int argc,char * argv[]) {
    pid_t id, input, slave;
    int outfile, pty;
    char inData [BUF_SIZE];
    char outData [BUF_SIZE];
    struct winsize win;
    struct termios original;
    
    /*Opens the correct output file depending on the request */
    if(argc != 1 && argc != 2) {
        perror("Please enter correct # of arguments\n");
        exit(-1);
    }
    if(argc == 2) {
        outfile = open(argv[1],O_RDWR, O_CREAT | O_TRUNC); 
    } else {
        outfile = open("typescript",O_RDWR, O_CREAT | O_TRUNC); 
    }

    /*Print pid incase I need to terminate it manually */
    id = getpid();
    fprintf(stderr, "Parent process id is %d\n",id);

    /*make sure you are in a tty */
    if(isatty(STDIN_FILENO) != 1){
        perror("Not a tty\n");
        exit(-1);
    }
    /*get window size for later and then palce into raw mode*/
    get_winsize(STDIN_FILENO,&win);
    raw_mode(STDIN_FILENO, &original);

    /*opens the pty and sets flags */
    if((pty = posix_openpt(O_RDWR | O_NOCTTY)) == -1) {
        perror("No pty");
        exit(-1);
    }
    
    /*do slave stuff */
    if(!(slave = fork())){

        exit(1);
    }
    /*do input stuff */
    if(!(input = fork())){
        /*set up sig handler*/
        
        while(read(STDIN_FILENO, inData, BUF_SIZE) > 0) {
            write(pty, inData, BUF_SIZE);
        }
        close(pty);
        close(outfile);
        exit(1);
    }
    /*read from pty and write to both log and stdout*/

    /*send signal to input*/


    if ( -1 == wait(NULL) ) { /* wait for slave */

        perror("wait");
    }
    if ( -1 == wait(NULL) ) { /* wait for input */
        /*set up sig handler*/

        perror("wait");
    }

    /* clean up stuff */
    restore_mode(STDIN_FILENO, &original);
    set_winsize(STDIN_FILENO,&win);
    close(outfile);

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

