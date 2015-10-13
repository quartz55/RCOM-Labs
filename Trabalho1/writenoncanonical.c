/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>
#include <signal.h>

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

#define FLAG 0x7e
#define A 0x03
#define C_SET 0x07
#define C_UA 0x03

volatile int STOP=FALSE;

int flag=1, counter = 0;

void atende() {
    flag = 1;
    ++counter;
    if (counter > 3) return;
    printf("------------------------\n");
    printf("3s timeout, re-sending...(%dx)\n", counter);
}

int main(int argc, char** argv)
{
    int fd, res;
    struct termios oldtio,newtio;

    if (argc < 2) {
        printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
        exit(1);
    }

    /*
       Open serial port device for reading and writing and not as controlling tty
       because we don't want to get killed if linenoise sends CTRL-C.
       */


    fd = open(argv[1], O_RDWR | O_NOCTTY );
    if (fd <0) {perror(argv[1]); exit(-1); }

    if ( tcgetattr(fd,&oldtio) == -1) { /* save current port settings */
        perror("tcgetattr");
        exit(-1);
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
    newtio.c_cc[VMIN]     = 0;   /* blocking read until 5 chars received */

    /*
       VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a
       leitura do(s) próximo(s) caracter(es)
       */

    tcflush(fd, TCIOFLUSH);

    if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
        perror("tcsetattr");
        exit(-1);
    }

    printf("New termios structure set\n");


    unsigned char SET[5];
    unsigned char UA[5];

    SET[0] = FLAG;
    SET[1] = A;
    SET[2] = C_SET;
    SET[3] = A^C_SET;
    SET[4] = FLAG;

    UA[0] = FLAG;
    UA[1] = A;
    UA[2] = C_UA;
    UA[3] = A^C_UA;
    UA[4] = FLAG;

    int received = 0;
    (void) signal(SIGALRM, atende);

    unsigned char buff[5];

    while(!received && counter <= 3) {
        if(flag){
            flag = 0;
            alarm(3);

            res = write(fd, SET, 5);
            printf("%d bytes written\n", res);
            printf("Waiting for answer..\n");
            while(!flag) {
                res = read(fd, buff, 5);
                if(res != 0) {
                    if (
                            buff[0] == UA[0] &&
                            buff[1] == UA[1] &&
                            buff[2] == UA[2] &&
                            buff[3] == UA[3]
                       ) {
                        received = 1;
                        flag = 1;
                        printf("Answer received!\n");
                        int i = 0;
                        for (; i < 5; i++) {
                            printf("|%x", buff[i]);
                        }
                        printf("|\n");
                    }
                    else {
                        printf("Wrong message\n");
                    }
                }
            }
        }
    }

    if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
        perror("tcsetattr");
        exit(-1);
    }

    close(fd);
    return 0;
}
