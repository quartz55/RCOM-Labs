/*Non-Canonical Input Processing*/
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <strings.h>
#include <stdlib.h>

#define BAUDRATE B38400
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

#define FLAG 0x7e
#define A 0x03
#define C_UA 0x03

#define FLAG 0x7e
#define A 0x03
#define C_SET 0x07

volatile int STOP=FALSE;
int main(int argc, char** argv)
{
  int fd,c, res;
  struct termios oldtio,newtio;
  char buf[255];
  unsigned char UA[5];
  UA[0]=FLAG;
  UA[1]=A;
  UA[2]=C_UA;
  UA[3]=A^C_UA;
  UA[4]=FLAG;

<<<<<<< HEAD
	if(argc < 2) {
	printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
	exit(1);
	}
=======
  if (argc < 2) {
      printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
      exit(1);
  }
>>>>>>> Better makefile

/*
  Open serial port device for reading and writing and not as controlling tty
  because we don't want to get killed if linenoise sends CTRL-C.
*/
  fd = open(argv[1], O_RDWR | O_NOCTTY );
  if (fd <0) { perror(argv[1]); exit(-1); }
  tcgetattr(fd,&oldtio); /* save current port settings */
  bzero(&newtio, sizeof(newtio));
  newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
  newtio.c_iflag = IGNPAR;
  newtio.c_oflag = 0;

/* set input mode (non-canonical, no echo,...) */
  newtio.c_lflag = 0;
  newtio.c_cc[VTIME] = 0; /* inter-character timer unused */
  newtio.c_cc[VMIN] = 5; /* blocking read until 5 chars received */
  tcflush(fd, TCIFLUSH);
  tcsetattr(fd,TCSANOW,&newtio);
/* printf("New termios structure set\n"); */

  res=0;
  while (res== 0) { /* loop for input */
      res=read(fd,buf,255);
  }
  printf("Message Read\n");
  res = write(fd, UA, res);
  printf("Message resent\n");
  tcsetattr(fd,TCSANOW,&oldtio);
  close(fd);
  return 0;
}
