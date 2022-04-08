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

int st_machine(int *rd,unsigned char byte_set, int state)
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
        printf("Header flag of SET message is not correct");
        printf("Header flag received: 0x%02X | Value Expected: 0x%02X \n", byte_set, FLAG_SET);
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
      if(byte_set == (BYTE)(A_SET_SERV_CLIENT ^ C_SET)) 
      {
        printf("BCC flag received: 0x%02X | Value Expected: 0x%02X \n", byte_set, (A_SET_SERV_CLIENT ^ C_SET)); 
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














int main(int argc, char** argv)
{
    int fd,c, res;
    struct termios oldtio,newtio;
    unsigned char buf[SET_SIZE];
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
    newtio.c_cc[VMIN]     = 0;   /* blocking read until 1 chars received */

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

    int state=0;
    int rd = 1;



    

    
    // Receiving the SET Message
    while (STOP==FALSE) /* loop for input */
    {
      //Reads each byte of SET message
       /* returns after 1 chars have been input */
      if(rd==1) read(fd,buf,1);
     

      state=st_machine(&rd,buf[0],state);
      
      
      

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
    //Check for errors at writing the Message to the serial channel
    

    /* 
      O ciclo WHILE deve ser alterado de modo a respeitar o indicado no gui�o 
    */

    if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }
    close(fd);
    return 0;
}