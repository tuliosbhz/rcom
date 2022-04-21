#ifndef LINKLAYER
#define LINKLAYER

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "linklayer.h"

//ROLE
#define NOT_DEFINED -1
#define TRANSMITTER 0
#define RECEIVER 1


//SIZE of maximum acceptable payload; maximum number of bytes that application layer should send to link layer
#define MAX_PAYLOAD_SIZE 1000

//CONNECTION deafault values
#define BAUDRATE_DEFAULT B38400
#define MAX_RETRANSMISSIONS_DEFAULT 3
#define TIMEOUT_DEFAULT 4
#define _POSIX_SOURCE 1 /* POSIX compliant source */

//MISC
#define FALSE 0
#define TRUE 1

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

int ua_st_machine(int *rd, unsigned char ua_byte, int state) 
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

int set_st_machine(int *rd,unsigned char byte_set, int state)
{
  *rd=1;  
  switch(state)
  {
    case 0:
      if(byte_set == FLAG_SET)
      {
        state++;
      }else
      {
        //printf("Header flag of SET message is not correct\n");
        //printf("Header flag received: 0x%02X | Value Expected: 0x%02X \n", byte_set, FLAG_SET);
        state=0;
      } 
      break;
    case 1:
      if(byte_set == A_SET_SERV_CLIENT) 
      {
        state++;
      } else 
            if(byte_set==FLAG_SET){*rd=0; state--;}
            else
            {
                printf("Address flag of SET message is not correct\n");
                printf("Address flag received: 0x%02X | Value Expected: 0x%02X \n", byte_set, A_SET_SERV_CLIENT);
                state = 0;
            }
      
        break;
    case 2:
      if(byte_set == C_SET) 
      {
        state++;
      } else if(byte_set==A_SET_SERV_CLIENT){*rd=0;state--;}
        else{
        printf("Control flag of SET message is not correct");
        printf("Control flag received: 0x%02X | Value Expected: 0x%02X \n", byte_set, C_SET);
        state=0;
        }
      break;
    case 3:
      if(byte_set == (unsigned char)(A_SET_SERV_CLIENT ^ C_SET)) 
      {
        //printf("BCC flag received: 0x%02X | Value Expected: 0x%02X \n", byte_set, (A_SET_SERV_CLIENT ^ C_SET)); 
        state++;
      } else 
        if(byte_set==C_SET){*rd=0;state--;}
        else{
        printf("BCC flag of SET message is not correct");
        printf("BCC flag received: 0x%02X | Value Expected: 0x%02X \n", byte_set, (A_SET_SERV_CLIENT ^ C_SET));
        state=0;
        }
      break;
    case 4:
      if(byte_set == FLAG_UA) 
      {
        state++;
      } 
        else
        if(byte_set==(A_SET_SERV_CLIENT^C_SET)){*rd=0;state--;}
         else 
      {
        printf("Tail flag of SET message is not correct");
        printf("Tail flag received: 0x%02X | Value Expected: 0x%02X \n", byte_set, FLAG_UA);
        state=0;
      }
     
      break;
      default:
      break;
  }
  return state;
}

// void signal_handler(int sig)
// {
//   printf("Timeout\n");
//   timeout = 1;
//   //Creating SET message
//   buf[0] = FLAG_SET;
//   buf[1] = A_SET_SERV_CLIENT;
//   buf[2] = C_SET;
//   buf[3] = A_SET_SERV_CLIENT ^ C_SET; //XOR, definir paridade
//   buf[SET_SIZE-1] = FLAG_SET;
//   int res = write(fd,buf,SET_SIZE);
//   if(res < 0)
//   {
//     perror("Read on serial file at /dev/ttyS0 failed");
//     STOP=TRUE;
//   }
//   count_retransmissions++;
//   if(count_retransmissions == 3)
//   {
//     perror("Too many retransmissions");
//     alarm(0);
//     STOP=TRUE;
//   } else alarm(3);
// }
// Opens a conection using the "port" parameters defined in struct linkLayer, returns "-1" on error and "1" on sucess

int llopen(linkLayer cnnectionParameters)
{
    //Check parameters
    if(connectionParameters.baudRate != BAUDRATE_DEFAULT) return -1;
    if(connectionParameters.numTries > MAX_RETRANSMISSIONS_DEFAULT ) return -1;
    if(connectionParameters.timeOut != TIMEOUT_DEFAULT) return -1;
    
    //Open the file of serial port
    int fd, res, rd;
    //Handshake SET-UA
    char buf[SET_SIZE];
    int count_retransmisions = 0, error = 0;
    int state = 0;
    struct termios oldtio,newtio;

    //signal(SIGALRM, signal_handler);

    //The code to open a file is equal to a TRANSMITER AND RECEIVER

    /*
        Open serial port device for reading and writing and not as controlling tty
        because we don't want to get killed if linenoise sends CTRL-C.
    */
    fd = open(connectionParameters.serialPort, O_RDWR | O_NOCTTY );
    if (fd <0) {perror(connectionParameters.serialPort); exit(-1); }

    if ( tcgetattr(fd,&oldtio) == -1) { /* save current port settings */
    perror("tcgetattr");
    exit(-1);
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = connectionParameters.baudRate | CS8 | CLOCAL | CREAD;
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

    if ( tcsetattr(fd,TCSANOW,&newtio) == -1) 
    {
        perror("tcsetattr");
        exit(-1);
    }
    if(connectionParameters.role == 0) //If you are a transmitter (TX)
    {
      //Creating SET message
      buf[0] = FLAG_SET;
      buf[1] = A_SET_SERV_CLIENT;
      buf[2] = C_SET;
      buf[3] = A_SET_SERV_CLIENT ^ C_SET; //XOR, definir paridade
      buf[SET_SIZE-1] = FLAG_SET;
      //printf("Content of buffer sent: 0x%02X | 0x%02X | 0x%02X | 0x%02X| 0x%02X\n", 
      //                                  buf[0], buf[1], buf[2],buf[3], buf[4]);
      //Write SET message
      res = write(fd,buf,SET_SIZE);
      if(res < 0)
      {
        perror("Read on serial file at /dev/ttyS0 failed");
        STOP=TRUE;
      } //else printf("%d bytes written\n", res);
      //Start timeout clock
      alarm(3);
      rd = 1;
      while (STOP==FALSE) /* loop for input */
      {
        if (rd == 1) read(fd,buf,1);  /* returns after 1 chars have been input */
        //State machine to check UA message 
        state = ua_st_machine(&rd, buf[0], state);
        if(state == 5) 
        {
          //printf("UA message received with sucess\n");
          alarm(0);
          STOP = TRUE;
        }
      }
    } else if(connectionParameters.role == 1) //If you are a receiver (RX)
    {
      // Receiving the SET Message
    while (STOP==FALSE) /* loop for input */
    {
      //Reads each byte of SET message
       /* returns after 1 chars have been input */
      if(rd==1) read(fd,buf,1);

      state=set_st_machine(&rd,buf[0],state);
      
      if(state == 5)
      {
        printf("SET message received with success\n");
        alarm(0);
        STOP=TRUE;
      } 

    } //End of Receive SET message LOOP
    
    //UA response
    buf[0] = FLAG_UA;
    buf[1] = A_UA_CLIENT_SERV;
    buf[2] = C_UA;
    buf[3] = A_UA_CLIENT_SERV ^ C_UA; //XOR, definir paridade
    buf[SET_SIZE-1] = FLAG_UA;
    printf("Content of buffer sended: 0x%02X | 0x%02X | 0x%02X | 0x%02X| 0x%02X\n", 
                                      buf[0], buf[1], buf[2],buf[3], buf[4]);
    //Write SET message

    res = write(fd,buf,SET_SIZE);
     if(res < 0)
    {
      perror("Read on serial file at /dev/ttyS0 failed");
      STOP=TRUE;
    }else 
    {
      printf("%d bytes written\n", res);
    }
    }

    return fd;
}
// Sends data in buf with size bufSize
int llwrite(char* buf, int bufSize)
{

}
// Receive data in packet
int llread(char* packet)
{
    int state=0;
    int rd = 1; 
    
}
// Closes previously opened connection; if showStatistics==TRUE, link layer should print statistics in the console on close
int llclose(int showStatistics)
{
  int fd; //How i find the file descriptor
  struct termios oldtio;
  if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
    perror("tcsetattr");
    exit(-1);
  }
  close(fd);
}   


#endif



