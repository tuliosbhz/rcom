/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

//To timeout alarm SIGALRM
#include <signal.h>


#define BAUDRATE B38400
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

void signal_handler(int sig)
{
  printf("Timeout\n");
}

int main(int argc, char** argv)
{
    signal (SIGALRM, signal_handler);

    int fd,c, res;
    struct termios oldtio,newtio;
    BYTE buf[BUFF_SIZE];
    ///dev/ttyS0
    if (argc < 2) //|| ((strcmp("/tmb/vboxS0", argv[1])!=0) && (strcmp("/tmb/vboxS0", argv[1])!=0) )) 
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
    newtio.c_cc[VMIN]     = 1;   /* blocking read until 1 chars received */

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
    char echobuf[BUFF_SIZE];

    int i=0, count_retransmisions=0, error = 0;
    //Receiving the SET Message
    while (STOP==FALSE) /* loop for input */
    {
      //Start timeout clock
      //alarm(5); 
      //Reads each byte of SET message
      res = read(fd,buf, SET_SIZE);  /* returns after 1 chars have been input */
      if(res < 0)
      {
        perror("Read on serial file at /dev/ttyS0 failed");
        STOP=TRUE;
        break;
      }
      //Check SET content message 
      //State machine
      while(i<5)
      {
        switch(i)
        {
          case 0:
            if(buf[0] != (BYTE)FLAG_SET)
            {
              perror("Header flag of SET message is not correct");
              i=0;
              error++;
            } else 
            {
              error =0; //To avoid the block of the read() retransmission
              i++; //Only change state if received correctly the message
            }
            printf("Header flag received: 0x%02X | Value Expected: 0x%02X \n", buf[0], FLAG_SET);
            break;
          case 1:
            if(buf[1] != (BYTE)A_SET_SERV_CLIENT) 
            {
              perror("Address flag of SET message is not correct");
              i=1;
              error++;
              break;
            } else 
            {
              error =0;//To avoid the block of the read() retransmission
              i++; //Only change state if received correctly the message
            }
            printf("Header flag received: 0x%02X | Value Expected: 0x%02X \n", buf[0], A_SET_SERV_CLIENT);
            break;
          case 2:
            if(buf[2] != (BYTE)C_SET) 
            {
              perror("Control flag of SET message is not correct");
              i=2;
              error++;
              break;
            } else 
            {
              error =0;//To avoid the block of the read() retransmission
              i++; //Only change state if received correctly the message
            }
            printf("Header flag received: 0x%02X | Value Expected: 0x%02X \n", buf[0], C_SET);
            break;
          case 3:
            if(buf[3] != (BYTE)(A_SET_SERV_CLIENT ^ C_SET)) 
            {
              perror("BCC flag of SET message is not correct");
              i=3;
              error++;
              break;
            } else 
            {
              error =0;//To avoid the block of the read() retransmission
              i++; //Only change state if received correctly the message
            }
            printf("Header flag received: 0x%02X | Value Expected: 0x%02X \n", buf[0], (A_SET_SERV_CLIENT ^ C_SET));
            break;
          case 4:
            if(buf[4] != (BYTE)FLAG_UA) 
            {
              perror("Tail flag of SET message is not correct");
              i=4;
              error++;
              break;
            } else 
            {
              alarm(0);
              error=0;//To avoid the block of the read() retransmission
              i++;
            }
            printf("Header flag received: 0x%02X | Value Expected: 0x%02X \n", buf[0], FLAG_UA);
            break;
          default: 
            perror("Out of range State machine receiveing UA message");
            error++;
            break;
        }
        if(error > 0) break; //Read retransmission
      } //End of State Machine
      if(i == 5)
      {
        printf("SET message received with success\n");
        i=0;
        STOP=TRUE;
      } else 
      {
        perror("Receiving SET message");
      }
    } //End of Receive SET message LOOP
    //Sending the UA response
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
        printf("%d bytes written\n", res);
      }
      STOP=TRUE;
      // count_retransmisions++;
      // if (count_retransmisions == 3) 
      // {
      //   perror("To many tries(3 retransmissions)");
      //   STOP=TRUE;
      // }
      // error = 0; //To not break the state machine in case of retransmission
    }

  /* 
    O ciclo WHILE deve ser alterado de modo a respeitar o indicado no gui�o 
  */



    tcsetattr(fd,TCSANOW,&oldtio);
    close(fd);
    return 0;
}
