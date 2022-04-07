/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <inttypes.h>

#include <signal.h>

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

#define BUFF_SIZE 255

#define SET_SIZE 5
#define FLAG_SET 0x7E //0b0111 1110
#define A_SET_SERV_CLIENT 0x03 //0b0000 0011 
#define A_SET_CLIENT_SERV 0x01 //0b0000 0001 
#define C_SET 0x03 //0b0000 0011 //Control field command UA message (Handshake) 

#define UA_SIZE 5
#define FLAG_UA 0x7E //0b01111110
#define A_UA_SERV_CLIENT 0x03//0b00000011 //Address fied
#define A_UA_CLIENT_SERV 0x01 //0b00000001 //Address field UA
#define C_UA 0x07 //0b00000111 //Control field command UA message (Handshake)

volatile int STOP=FALSE;

typedef unsigned char BYTE;
int timeout=0, count_retransmissions = 0;
int fd, buf[SET_SIZE];

void signal_handler(int sig)
{
  timeout = 1;
  int res = write(fd,buf,SET_SIZE);
  if(res < 0)
  {
    perror("Read on serial file at /dev/ttyS0 failed");
    STOP=TRUE;
  }
  count_retransmissions++;
  if(count_retransmissions == 3)
  {
    perror("Too many retransmissions");
    alarm(0);
    STOP=TRUE;
  } else alarm(3);

  printf("Timeout\n");
}

int main(int argc, char** argv)
{
    int timeout = 0;
    signal(SIGALRM, signal_handler);

    //int fd,c, res;
    int c, res;
    struct termios oldtio,newtio;
    //BYTE buf[BUFF_SIZE];
    int sum = 0, speed = 0;
    
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

    //buf[254] = '\0';
    //char rcv_buf[BUFF_SIZE];
    //Read response of noncanonical
    //Handshake SET-UA
    int count_retransmisions = 0, error = 0;
    int state = 0;
    while (STOP==FALSE) /* loop for input */
    {
      //Creating SET message
      buf[0] = FLAG_SET;
      buf[1] = A_SET_SERV_CLIENT;
      buf[2] = C_SET;
      buf[3] = A_SET_SERV_CLIENT ^ C_SET; //XOR, definir paridade
      buf[SET_SIZE-1] = FLAG_SET;
      printf("Content of buffer sended: 0x%02X | 0x%02X | 0x%02X | 0x%02X| 0x%02X\n", 
                                        buf[0], buf[1], buf[2],buf[3], buf[4]);
      //Write SET message
      res = write(fd,buf,SET_SIZE);
      //Check for errors at writing the Message to the serial channel
      if(res < 0)
      {
        perror("Read on serial file at /dev/ttyS0 failed");
        STOP=TRUE;
      }else 
      {
        //Start timeout clock
        alarm(3);
        printf("%d bytes written\n", res);
        //Waits to read all of UA message
        res = read(fd,buf,UA_SIZE);  /* returns after 1 chars have been input */
        if(res != 5)
        {
          //perror("Read on serial file at /dev/ttyS0 failed");
          printf("Number of bytes read from UA messsage is wrong");
          STOP=TRUE;
          break;
        }
        //Check UA content message 
        //State machine
        while(state<5)
        {
          if(error > 0) break;

          switch(state)
          {
            case 0: //TODO: Incomplete estate machine, if received 2 flags stay at the same state
              if(buf[0] != (BYTE)FLAG_UA)
              {
                printf("Header flag of UA message is not correct\n");
                printf("Header flag received: 0x%02X | Value Expected: 0x%02X \n", buf[0], FLAG_UA);
                state=0;
                error++;
              } else state++;
              break;
            case 1:
              if(buf[0] != (BYTE)A_UA_CLIENT_SERV) 
              {
                printf("Address flag of UA message is not correct\n");
                printf("Address flag received: 0x%02X | Value Expected: 0x%02X \n", buf[0], A_UA_CLIENT_SERV);
                state=1;
                error++;
              } else state++;
              break;
            case 2:
              if(buf[0] != (BYTE)C_UA) 
              {
                printf("Control flag of UA message is not correct\n");
                printf("Control flag received: 0x%02X | Value Expected: 0x%02X \n", buf[0], C_UA);
                state=2;
                error++;
              } else state++;
              break;
            case 3:
              if(buf[0] != (BYTE)(A_UA_CLIENT_SERV ^ C_UA)) 
              {
                printf("BCC flag of UA message is not correct\n");
                printf("BCC flag received: 0x%02X | Value Expected: 0x%02X \n", buf[0], (A_UA_CLIENT_SERV ^ C_UA));
                state=3;
                error++;
              } else state++;
              break;
            case 4:
              if(buf[0] != (BYTE)FLAG_UA) 
              {
                printf(" Tail flag of UA message is not correct\n");
                printf("Tail flag received: 0x%02X | Value Expected: 0x%02X \n", buf[0], FLAG_UA);
                state=4;
                error++;
              } else 
              {
                alarm(0);
                state++;
                error++;
              }
              break;
            default: 
              printf("Out of range State machine receiveing UA message\n");
              error++;
              break;
          }
           
        } //End of State Machine
      }
      // count_retransmisions++;
      // if (count_retransmisions == 3) 
      // {
      //   perror("To many tries(3 retransmissions)");
      //   STOP=TRUE;
      // }
      error = 0; //Cannot break the state machine in case of retransmission
    }

    if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    close(fd);
    return 0;
}
