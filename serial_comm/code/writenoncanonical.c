/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <signal.h>

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

#define SET_SIZE 5
#define FLAG_SET 0b01111110
#define A_SET_SERV_CLIENT 0b00000011 
#define A_SET_CLIENT_SERV 0b00000001 
#define C_SET 
#define BCC_SET


#define BUFF_SIZE 255

volatile int STOP=FALSE;

void signal_handler(int sig)
{
  printf("Timeout\n");
}

int main(int argc, char** argv)
{
    int fd,c, res;
    struct termios oldtio,newtio;
    char buf[255];
    int i, sum = 0, speed = 0;
    
    if (argc < 2) //|| ((strcmp("/dev/ttyS0", argv[1])!=0) && (strcmp("/dev/ttyS1", argv[1])!=0) )) 
    {
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
    newtio.c_cc[VMIN]     = 1;   /* blocking read until 5 chars received */

  /* 
    VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a 
    leitura do(s) pr�ximo(s) caracter(es)
  */
    tcflush(fd, TCIOFLUSH);

    if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    printf("New termios structure set\n");

    // u_int8_t set[SET_SIZE];
    // set[0] = FLAG_SET;
    // set[1] = A_SET
    
    // signal (SIGALRM, signal_handler);
    // alarm(2);

    gets(buf);

    /*testing*/
    buf[254] = '\0';
    printf("Content of buffer sended: %s\n", buf);
    res = write(fd,buf,sizeof(buf));   
    printf("%d bytes written\n", res);

    //tcflush(fd, TCIOFLUSH);

    sleep(1);

    //char rcv_buf[BUFF_SIZE];
    //Read response of noncanonical
    while (STOP==FALSE) /* loop for input */
    {       
      res = read(fd,buf,1);  /* returns after 1 chars have been input */
      if(res < 0)
      {
        perror("Read on serial file at /dev/ttyS0 failed");
        STOP=TRUE;
      } else 
      {
        buf[res]=0;               /* so we can printf... */
        printf(":%s:%d\t", buf, res);
      }
      if (buf[0]=='\0') STOP=TRUE;
    }
//    res = write(fd,buf,255);   
//    printf("%d bytes written\n", res);
    

 

  /* 
    O ciclo FOR e as instru��es seguintes devem ser alterados de modo a respeitar 
    o indicado no gui�o 
  */



   
    if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }




    close(fd);
    return 0;
}
