#include<stdio.h>
#include<stdlib.h>
#include<termios.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/ioctl.h>


static void get_winsize(int fd, struct winsize *ws); 
static void set_winsize(int fd, struct winsize *ws);
void restore_mode(int fd, struct termios *old);
void raw_mode(int fd, struct termios *old);


int main (int argc,char * argv[]) {
    pid_t id;
    int outfile;
    struct winsize win;
    
    if(argc != 1 || argc != 2) {
        perror("Please enter correct # of arguments\n");
        exit(-1);
    }
    if(argc == 2) {
        outfile = open(argv[1],O_RDWR, O_CREAT | O_TRUNC); 
    } else {
        outfile = open("typescript",O_RDWR, O_CREAT | O_TRUNC); 
    }
    id = getpid();
    fprintf(stderr, "Parent process id is %d\n",id);


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

