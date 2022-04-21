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

int timeout=0, count_retransmissions = 0;
int fd;
unsigned char buf[SET_SIZE];

int st_machine(int *rd, unsigned char ua_byte, int state) 
{
  *rd = 1;
  switch(state)
  {
    case 0: //TODO: Incomplete estate machine, if received 2 flags stay at the same state
      if(ua_byte == FLAG_UA) 
      {
        //printf("Header flag received: 0x%02X | Value Expected: 0x%02X \n", ua_byte, FLAG_UA);
        state++;
      }
      else {
        printf("Header flag of UA message is not correct\n");
        printf("Header flag received: 0x%02X | Value Expected: 0x%02X \n", ua_byte, FLAG_UA);
        state=0;
      }
      break;
    case 1:
      if(ua_byte == A_UA_CLIENT_SERV) 
      {
        //printf("Address flag received: 0x%02X | Value Expected: 0x%02X \n", ua_byte, A_UA_CLIENT_SERV);
        state++;
      }
      else 
        if(ua_byte == FLAG_UA) {*rd=0; state--;}
        else
        {
          printf("Address flag of UA message is not correct\n");
          printf("Address flag received: 0x%02X | Value Expected: 0x%02X \n", ua_byte, A_UA_CLIENT_SERV);
          state = 0;
        }
      break;
    case 2:
      if(ua_byte == C_UA) 
      {
        //printf("Control flag received: 0x%02X | Value Expected: 0x%02X \n", ua_byte, C_UA);
        state++;
      }
      else 
        if(ua_byte == A_UA_CLIENT_SERV) {*rd=0; state--;}
        else
        {
          printf("Control flag of UA message is not correct\n");
          printf("Control flag received: 0x%02X | Value Expected: 0x%02X \n", ua_byte, C_UA);
          state=0;
        }
      break;
    case 3:
      if(ua_byte == (A_UA_CLIENT_SERV ^ C_UA)) 
      {
        //printf("BCC flag received: 0x%02X | Value Expected: 0x%02X \n", ua_byte, (A_UA_CLIENT_SERV ^ C_UA));
        state++;
      }
      else
        if(ua_byte == C_UA) {*rd=0; state--;}
        else
        {
          printf("BCC flag of UA message is not correct\n");
          printf("BCC flag received: 0x%02X | Value Expected: 0x%02X \n", ua_byte, (A_UA_CLIENT_SERV ^ C_UA));
          state=0;
        } 
      break;
    case 4:
      if(ua_byte == FLAG_UA) 
      {
        //printf("Tail flag received: 0x%02X | Value Expected: 0x%02X \n", ua_byte, FLAG_UA);
        state++;
      }
      else if(ua_byte == (A_UA_CLIENT_SERV ^ C_UA)) {*rd=0; state--;}
        else
        {
          printf(" Tail flag of UA message is not correct\n");
          printf("Tail flag received: 0x%02X | Value Expected: 0x%02X \n", ua_byte, FLAG_UA);
          state=0;
        }
      break;
    default:
      //printf("Out of range State machine receiveing UA message\n");
      break;
  }
  return state;

}

void signal_handler(int sig)
{
  printf("Timeout\n");
  timeout = 1;
  //Creating SET message
  buf[0] = FLAG_SET;
  buf[1] = A_SET_SERV_CLIENT;
  buf[2] = C_SET;
  buf[3] = A_SET_SERV_CLIENT ^ C_SET; //XOR, definir paridade
  buf[SET_SIZE-1] = FLAG_SET;
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
}

int main(int argc, char** argv)
{
    int timeout = 0;
    signal(SIGALRM, signal_handler);

    //int fd,c, res;
    int c, res;
    struct termios oldtio,newtio;
    //unsigned char buf[BUFF_SIZE];
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
    newtio.c_cc[VMIN]     = 0;   /* blocking read until 5 chars received */

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

    //Handshake SET-UA
    int count_retransmisions = 0, error = 0;
    int state = 0;
    //Creating SET message
    buf[0] = FLAG_SET;
    buf[1] = A_SET_SERV_CLIENT;
    buf[2] = C_SET;
    buf[3] = A_SET_SERV_CLIENT ^ C_SET; //XOR, definir paridade
    buf[SET_SIZE-1] = FLAG_SET;
    printf("Content of buffer sent: 0x%02X | 0x%02X | 0x%02X | 0x%02X| 0x%02X\n", 
                                      buf[0], buf[1], buf[2],buf[3], buf[4]);
    //Write SET message
    res = write(fd,buf,SET_SIZE);
    if(res < 0)
    {
      perror("Read on serial file at /dev/ttyS0 failed");
      STOP=TRUE;
    } else printf("%d bytes written\n", res);
    //Start timeout clock
    alarm(3);
    int rd = 1;
    while (STOP==FALSE) /* loop for input */
    {
      if (rd == 1) read(fd,buf,1);  /* returns after 1 chars have been input */
      //State machine to check UA message 
      state = st_machine(&rd, buf[0], state);
      if(state == 5) 
      {
        printf("UA message received with sucess\n");
        alarm(0);
        STOP = TRUE;
      }
    }

    if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }
    close(fd);
    return 0;
}
