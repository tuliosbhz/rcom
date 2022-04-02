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

#define BUFF_SIZE 255

#define SET_SIZE 5
#define FLAG_SET 0b01111110
#define A_SET_SERV_CLIENT 0b00000011 
#define A_SET_CLIENT_SERV 0b00000001 
#define C_SET 0b00000011 //Control field command UA message (Handshake) 

#define UA_SIZE 5
#define FLAG_UA 0b01111110
#define A_UA_SERV_CLIENT 0b00000011 //Address fied
#define A_UA_CLIENT_SERV 0b00000011 //Address field UA
#define C_UA 0b00000111 //Control field command UA message (Handshake)


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
    uint8_t buf[BUFF_SIZE];
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

    //gets(buf);
    // uint8_t flag = FLAG_SET;
    // uint8_t address_set = A_SET_SERV_CLIENT;
    // uint8_t control_set = C_SET;
    // uint8_t bcc_set = address_set ^ control_set;
    // sprintf(buf, "%u%u%u%u%u", flag, address_set, control_set, bcc_set, flag);
    
    //Creating SET message
    buf[0] = FLAG_SET;
    buf[1] = A_SET_SERV_CLIENT;
    buf[2] = C_SET;
    buf[3] = A_SET_SERV_CLIENT ^ C_SET; //XOR, definir paridade
    buf[SET_SIZE-1] = FLAG_SET;

    /*testing*/
    
    //buf[254] = '\0';
    //char rcv_buf[BUFF_SIZE];
    //Read response of noncanonical
    //Handshake SET-UA
    int count_retransmisions = 0;
    while (STOP==FALSE) /* loop for input */
    {
      printf("Content of buffer sended: %s\n", buf);
      //Write SET message
      res = write(fd,buf,sizeof(buf));
      if(res < 0)
      {
        perror("Read on serial file at /dev/ttyS0 failed");
        STOP=TRUE;
      }
      printf("%d bytes written\n", res);
      //Check UA content message 
      for(int i=0; i<5; i++)
      {
        //Start timeout clock
        alarm(3); 
        //Reads each byte of UA message
        res = read(fd,buf,1);  /* returns after 1 chars have been input */
        if(res < 0)
        {
          perror("Read on serial file at /dev/ttyS0 failed");
          STOP=TRUE;
        }
        switch(i)
        {
          case 0:
            if(buf[0] != FLAG_UA) perror("Header flag of UA message is not correct");
            break;
          case 1:
            if(buf[0] != A_UA_CLIENT_SERV) perror("Header flag of UA message is not correct");
            break;
          case 2:
            if(buf[0] != C_UA) perror("Header flag of UA message is not correct");
            break;
          case 3:
            if(buf[0] != (A_UA_CLIENT_SERV ^ C_UA)) perror("Header flag of UA message is not correct");
            break;
          case 4:
            if(buf[0] != FLAG_UA) perror("Header flag of UA message is not correct");
            alarm(0);
            break;
          default: 
            perror("Header flag of UA message is not correct");
            break;
        }
        //Cancel 
      }
      if(res < 0)
      {
        perror("Read on serial file at /dev/ttyS0 failed");
        STOP=TRUE;
      } //else 
      // {
      //   buf[res]=0;  /* so we can printf... */
      //   printf(":%s:%d\t", buf, res);
      // }
      count_retransmisions++;
      if (count_retransmisions == 3) 
      {
        perror("To many tries(3 retransmissions)");
        STOP=TRUE;
      }
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
