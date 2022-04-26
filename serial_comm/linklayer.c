#include "linklayer.h"

#define UNNUM_FRAME_SIZE 5
#define FLAG 0x7E //0b0111 1110
#define A_SERV_CLIENT 0x03 //0b0000 0011 Commands from transmitter and responses from receiver
#define A_CLIENT_SERV 0x01 //0b0000 0001 

#define C_SET 0x03 //0b0000 0011 //Control field command UA message (Handshake) 
#define C_UA 0x07 //0b00000111 //Control field response UA message (Handshake)
#define C_DISC 0x0B //0b00001011 //Control field DISC message (Handshake)
#define C_REJ0 0x05 //Reject message supervised using 0
#define C_REJ1 0x25 //Reject message supervised using 1
#define C_RR0 0x01 // Receiver ready message 0
#define C_RR1 0x21 // Receiver ready message 1

//volatile int STOP=FALSE;
int C_I = 0x00;
//Connecting
char trama_set[5]={FLAG, A_SERV_CLIENT, C_SET, A_SERV_CLIENT^C_SET,FLAG};
char trama_ua[5]={FLAG, A_CLIENT_SERV, C_UA, A_CLIENT_SERV^C_UA,FLAG};
//Changing information
char trama_rej0[5] = {FLAG, A_SERV_CLIENT, C_REJ0, A_SERV_CLIENT^C_REJ0, FLAG};
char trama_rej1[5] = {FLAG, A_SERV_CLIENT, C_REJ1, A_SERV_CLIENT^C_REJ1, FLAG};
char trama_rr0[5] = {FLAG, A_SERV_CLIENT, C_RR0, A_SERV_CLIENT^C_RR0, FLAG};
char trama_rr1[5] = {FLAG, A_SERV_CLIENT, C_RR1, A_SERV_CLIENT^C_RR1, FLAG};
//Closing
char trama_disc_transmitter[5] = {FLAG, A_SERV_CLIENT, C_DISC, A_SERV_CLIENT^C_DISC, FLAG};
char trama_disc_receiver[5] = {FLAG, A_CLIENT_SERV, C_DISC, A_CLIENT_SERV^C_DISC, FLAG};

int ua_st_machine(int *rd, unsigned char ua_byte, int state) 
{
  *rd = 1;
  switch(state)
  {
    case 0: //TODO: Incomplete estate machine, if received 2 flags stay at the same state
      if(ua_byte == FLAG) 
      {
        //printf("Header flag received: 0x%02X | Value Expected: 0x%02X \n", ua_byte, FLAG);
        state++;
      }
      else {
        //printf("Header flag of UA message is not correct\n");
        //printf("Header flag received: 0x%02X | Value Expected: 0x%02X \n", ua_byte, FLAG);
        state=0;
      }
      break;
    case 1:
      if(ua_byte == A_CLIENT_SERV) 
      {
        //printf("Address flag received: 0x%02X | Value Expected: 0x%02X \n", ua_byte, A_CLIENT_SERV);
        state++;
      }
      else 
        if(ua_byte == FLAG) {*rd=0; state--;}
        else
        {
          printf("Address flag of UA message is not correct\n");
          printf("Address flag received: 0x%02X | Value Expected: 0x%02X \n", ua_byte, A_CLIENT_SERV);
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
        if(ua_byte == A_CLIENT_SERV) {*rd=0; state--;}
        else
        {
          printf("Control flag of UA message is not correct\n");
          printf("Control flag received: 0x%02X | Value Expected: 0x%02X \n", ua_byte, C_UA);
          state=0;
        }
      break;
    case 3:
      if(ua_byte == (A_CLIENT_SERV ^ C_UA)) 
      {
        //printf("BCC flag received: 0x%02X | Value Expected: 0x%02X \n", ua_byte, (A_CLIENT_SERV ^ C_UA));
        state++;
      }
      else
        if(ua_byte == C_UA) {*rd=0; state--;}
        else
        {
          printf("BCC flag of UA message is not correct\n");
          printf("BCC flag received: 0x%02X | Value Expected: 0x%02X \n", ua_byte, (A_CLIENT_SERV ^ C_UA));
          state=0;
        } 
      break;
    case 4:
      if(ua_byte == FLAG) 
      {
        //printf("Tail flag received: 0x%02X | Value Expected: 0x%02X \n", ua_byte, FLAG);
        state++;
      }
      else if(ua_byte == (A_CLIENT_SERV ^ C_UA)) {*rd=0; state--;}
        else
        {
          printf(" Tail flag of UA message is not correct\n");
          printf("Tail flag received: 0x%02X | Value Expected: 0x%02X \n", ua_byte, FLAG);
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
      if(byte_set == FLAG)
      {
        state++;
      }else
      {
        state=0;
      } 
      break;
    case 1:
      if(byte_set == A_SERV_CLIENT) 
      {
        state++;
      } else 
            if(byte_set==FLAG){*rd=0; state--;}
            else
            {
                printf("Address flag of SET message is not correct\n");
                printf("Address flag received: 0x%02X | Value Expected: 0x%02X \n", byte_set, A_SERV_CLIENT);
                state = 0;
            }
      
        break;
    case 2:
      if(byte_set == C_SET) 
      {
        state++;
      } else if(byte_set==A_SERV_CLIENT){*rd=0;state--;}
        else{
        printf("Control flag of SET message is not correct");
        printf("Control flag received: 0x%02X | Value Expected: 0x%02X \n", byte_set, C_SET);
        state=0;
        }
      break;
    case 3:
      if(byte_set == (unsigned char)(A_SERV_CLIENT ^ C_SET)) 
      {
        state++;
      } else 
        if(byte_set==C_SET){*rd=0;state--;}
        else{
        printf("BCC flag of SET message is not correct");
        printf("BCC flag received: 0x%02X | Value Expected: 0x%02X \n", byte_set, (A_SERV_CLIENT ^ C_SET));
        state=0;
        }
      break;
    case 4:
      if(byte_set == FLAG) 
      {
        state++;
      } 
        else
        if(byte_set==(A_SERV_CLIENT^C_SET)){*rd=0;state--;}
         else 
      {
        printf("Tail flag of SET message is not correct");
        printf("Tail flag received: 0x%02X | Value Expected: 0x%02X \n", byte_set, FLAG);
        state=0;
      }
     
      break;
      default:
      break;
  }
  return state;
}

int disc_st_machine(int *rd, unsigned char ua_byte, int state) 
{
  *rd = 1;
  switch(state)
  {
    case 0: //TODO: Incomplete estate machine, if received 2 flags stay at the same state
      if(ua_byte == FLAG) 
      {
        //printf("Header flag received: 0x%02X | Value Expected: 0x%02X \n", ua_byte, FLAG);
        state++;
      }
      else {
        printf("Header flag of DISC message is not correct\n");
        printf("Header flag received: 0x%02X | Value Expected: 0x%02X \n", ua_byte, FLAG);
        state=0;
      }
      break;
    case 1:
      if(ua_byte == A_CLIENT_SERV || ua_byte == A_SERV_CLIENT) 
      {
        state++;
      }
      else 
        if(ua_byte == FLAG) {*rd=0; state--;}
        else
        {
          printf("Address flag of DISC message is not correct\n");
          printf("Address flag received: 0x%02X | Value Expected: 0x%02X \n", ua_byte, A_CLIENT_SERV);
          state = 0;
        }
      break;
    case 2:
      if(ua_byte == C_DISC) 
      {
        state++;
      }
      else 
        if(ua_byte == A_CLIENT_SERV) {*rd=0; state--;}
        else
        {
          printf("Control flag of DISC message is not correct\n");
          printf("Control flag received: 0x%02X | Value Expected: 0x%02X \n", ua_byte, C_UA);
          state=0;
        }
      break;
    case 3:
      if(ua_byte == (A_CLIENT_SERV ^ C_UA)) 
      {
        state++;
      }
      else
        if(ua_byte == C_UA) {*rd=0; state--;}
        else
        {
          printf("BCC flag of DISC message is not correct\n");
          printf("BCC flag received: 0x%02X | Value Expected: 0x%02X \n", ua_byte, (A_CLIENT_SERV ^ C_UA));
          state=0;
        } 
      break;
    case 4:
      if(ua_byte == FLAG) 
      {
        state++;
      }
      else if(ua_byte == (A_CLIENT_SERV ^ C_UA)) {*rd=0; state--;}
        else
        {
          printf(" Tail flag of DISC message is not correct\n");
          printf("Tail flag received: 0x%02X | Value Expected: 0x%02X \n", ua_byte, FLAG);
          state=0;
        }
      break;
    default:
      break;
  }
  return state;

}

int rr_rej_st_machine(int *rd, unsigned char ua_byte, int state) 
{
  *rd = 1;
  switch(state)
  {
    case 0: //TODO: Incomplete estate machine, if received 2 flags stay at the same state
      if(ua_byte == FLAG) 
      {
        //printf("Header flag received: 0x%02X | Value Expected: 0x%02X \n", ua_byte, FLAG);
        state++;
      }
      else {
        //printf("Header flag of RR message is not correct\n");
        //printf("Header flag received: 0x%02X | Value Expected: 0x%02X \n", ua_byte, FLAG);
        state=0;
      }
      break;
    case 1:
      if(ua_byte == A_SERV_CLIENT) 
      {
        //printf("Address flag received: 0x%02X | Value Expected: 0x%02X \n", ua_byte, A_CLIENT_SERV);
        state++;
      }
      else 
        if(ua_byte == FLAG) {*rd=0; state--;}
        else
        {
          printf("Address flag of RR message is not correct\n");
          printf("Address flag received: 0x%02X | Value Expected: 0x%02X \n", ua_byte, A_CLIENT_SERV);
          state = 0;
        }
      break;
    case 2:
      if(ua_byte == C_REJ0 || ua_byte == C_REJ1 || ua_byte == C_RR0 || ua_byte == C_RR1) 
      {
        state++;
      }
      else 
        if(ua_byte == A_CLIENT_SERV) {*rd=0; state--;}
        else
        {
          printf("Control flag of RR message is not correct\n");
          printf("Control flag received: 0x%02X | Value Expected: 0x%02X \n", ua_byte, C_RR0);
          state=0;
        }
      break;
    case 3:
      if((ua_byte == (A_CLIENT_SERV^C_REJ0)) || (ua_byte == (A_CLIENT_SERV^C_REJ1)) || (ua_byte == (A_CLIENT_SERV^C_RR0)) || (ua_byte == (A_CLIENT_SERV^C_RR1))) 
      {
        state++;
      }
      else
        if(ua_byte == C_REJ0 || ua_byte == C_REJ1 || ua_byte == C_RR0 || ua_byte == C_RR1) {*rd=0; state--;}
        else
        {
          printf("BCC flag of RR message is not correct\n");
          printf("BCC flag received: 0x%02X | Value Expected: 0x%02X \n", ua_byte, (A_CLIENT_SERV ^ C_RR0));
          state=0;
        } 
      break;
    case 4:
      if(ua_byte == FLAG) 
      {
        state++;
      }
      else if(ua_byte == (A_CLIENT_SERV ^ C_UA)) {*rd=0; state--;}
        else
        {
          printf(" Tail flag of RR message is not correct\n");
          printf("Tail flag received: 0x%02X | Value Expected: 0x%02X \n", ua_byte, FLAG);
          state=0;
        }
      break;
    default:
      break;
  }
  return state;

}

int i_st_machine(char* packet, int *rd, unsigned char ua_byte, int state, size_t trama_size, int count_reads_payload) 
{ 
  *rd = 1;
  switch(state)
  {
    case 0:
      if(ua_byte == FLAG) 
      {
        //printf("Header flag received: 0x%02X | Value Expected: 0x%02X \n", ua_byte, FLAG);
        state++;
      }
      else {
        printf("Header flag of I message is not correct\n");
        printf("Header flag received: 0x%02X | Value Expected: 0x%02X \n", ua_byte, FLAG);
        state=0;
      }
      break;
    case 1:
      if(ua_byte == A_SERV_CLIENT) 
      {
        //printf("Address flag received: 0x%02X | Value Expected: 0x%02X \n", ua_byte, A_CLIENT_SERV);
        state++;
      }
      else 
        if(ua_byte == FLAG) {*rd=0; state--;}
        else
        {
          printf("Address flag of I message is not correct\n");
          printf("Address flag received: 0x%02X | Value Expected: 0x%02X \n", ua_byte, A_SERV_CLIENT);
          state = 0;
        }
      break;
    case 2:
      if(ua_byte == C_I) //C_I value will change at each I message exchange with success
      {
        //printf("Control flag received: 0x%02X | Value Expected: 0x%02X \n", ua_byte, C_UA);
        state++;
      }
      else 
        if(ua_byte == A_SERV_CLIENT) {*rd=0; state--;}
        else
        {
          printf("Control flag of I message is not correct\n");
          printf("Control flag received: 0x%02X | Value Expected: 0x%02X \n", ua_byte, C_I);
          state=0;
        }
      break;
    case 3:
      if(ua_byte == (A_SERV_CLIENT ^ C_I)) 
      {
        //printf("BCC flag received: 0x%02X | Value Expected: 0x%02X \n", ua_byte, (A_CLIENT_SERV ^ C_UA));
        state++;
        count_reads_payload = 0; 
      }
      else
        if(ua_byte == C_I) {*rd=0; state--;}
        else
        {
          printf("BCC flag of I message is not correct\n");
          printf("BCC flag received: 0x%02X | Value Expected: 0x%02X \n", ua_byte, (A_SERV_CLIENT ^ C_I));
          state=0;
        } 
      break;
    case 4:
      if(ua_byte == FLAG) 
      {
        //printf("Tail flag received: 0x%02X | Value Expected: 0x%02X \n", ua_byte, FLAG);
        state++;
      }
      else //Reads payload
        {
          packet[count_reads_payload] = ua_byte;
          count_reads_payload++;
        }
      break;
    default:
      printf("Out of range State machine receiveing I message\n");
      break;
  }
  return state;

}

// void signal_handler(int sig)
// {
//   printf("Timeout\n");
//   timeout = 1;
//   //Creating SET message
//   buf[0] = FLAG;
//   buf[1] = A_SERV_CLIENT;
//   buf[2] = C_SET;
//   buf[3] = A_SERV_CLIENT ^ C_SET; //XOR, definir paridade
//   buf[UNNUM_FRAME_SIZE-1] = FLAG;
//   int res = write(FD_SERIAL,buf,UNNUM_FRAME_SIZE);
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

int llopen(linkLayer connectionParameters)
{
    //Check parameters
    ROLE = connectionParameters.role;
    //Open the file of serial port
    int res, rd, stop = FALSE;
    //Handshake SET-UA
    char buf[UNNUM_FRAME_SIZE];
    int count_retransmisions = 0, error = 0;
    int state = 0;
    struct termios oldtio,newtio;

    //signal(SIGALRM, signal_handler);

    //The code to open a file is equal to a TRANSMITER AND RECEIVER

    /*
        Open serial port device for reading and writing and not as controlling tty
        because we don't want to get killed if linenoise sends CTRL-C.
    */
    FD_SERIAL = open(connectionParameters.serialPort, O_RDWR | O_NOCTTY );
    if (FD_SERIAL <0) {perror(connectionParameters.serialPort); exit(-1); }

    if ( tcgetattr(FD_SERIAL,&oldtio) == -1) { /* save current port settings */
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
    tcflush(FD_SERIAL, TCIOFLUSH);

    if ( tcsetattr(FD_SERIAL,TCSANOW,&newtio) == -1) 
    {
        perror("tcsetattr");
        exit(-1);
    }

    if(connectionParameters.role == TRANSMITTER) //If you are a transmitter (TX)
    {
      //Write SET message
      res = write(FD_SERIAL,trama_set,UNNUM_FRAME_SIZE);
      if(res < 0)
      {
        perror("Read on serial file at /dev/ttyS0 failed");
        return -1;
      } else printf("%d bytes written\n", res);
      //Start timeout clock
      alarm(connectionParameters.timeOut);
      rd = 1;
      while (stop==FALSE) /* loop for input */
      {
        if (rd == 1) read(FD_SERIAL,buf,1);  /* returns after 1 chars have been input */
        //State machine to check UA message 
        state = ua_st_machine(&rd, buf[0], state);
        if(state == 5) 
        {
          //printf("UA message received with sucess\n");
          alarm(0);
          state = 0;
          stop = TRUE;
        }
      }
    } else if(connectionParameters.role == RECEIVER) //If you are a receiver (RX)
      {
          // Receiving the SET Message
          while (stop==FALSE) /* loop for input */
          {
            //Reads each byte of SET message
            /* returns after 1 chars have been input */
            if(rd==1) read(FD_SERIAL,buf,1);

            state=set_st_machine(&rd,buf[0],state);
            
            if(state == 5)
            {
              printf("SET message received with success\n");
              alarm(0);
              stop=TRUE;
            } 

          } //End of Receive SET message LOOP

          //Write UA message
          res = write(FD_SERIAL, trama_ua, UNNUM_FRAME_SIZE);
          if(res < 0)
          {
            perror("Read on serial file at /dev/ttyS0 failed");
          }else 
          {
            printf("%d bytes written\n", res);
          }
      }

    return FD_SERIAL;
}
// Sends data in buf with size bufSize
int llwrite(char* buf, int bufSize)
{
  int res, rd, i;
  //Handshake SET-UA
  //char buf[UNNUM_FRAME_SIZE];
  int count_retransmisions = 0, error = 0;
  int state = 0;
  int stop = FALSE;

  if(ROLE == TRANSMITTER) //If you are a transmitter (TX)
  {
    int payload_size = strlen(buf);
    if(payload_size > MAX_PAYLOAD_SIZE) printf("Payload to large");

    int I_SIZE = 3 + payload_size + 1;
    char I[I_SIZE];
    I[0] = FLAG;
    I[1] = A_SERV_CLIENT;
    I[2] = C_I; // 0x00 or 0x02 //TODO: insert toogle feature
    for(i=0; i<bufSize; i++)
    {
      I[i+3] = buf[i];
    }
    I[I_SIZE - 1] = FLAG;
    //Write I message
    res = write(FD_SERIAL, I, bufSize + 4);
    if(res < 0)
    {
      perror("Read on serial file at /dev/ttyS0 failed");
      return -1;
    } else printf("%d bytes written\n", res);
    //Start timeout clock
    //alarm(connectionParameters.timeOut);
  } else if(ROLE == RECEIVER) //If you are a receiver (RX)
    {
      //Write RR or REJ message
      if(bufSize == 0)
      res = write(FD_SERIAL, trama_ua, UNNUM_FRAME_SIZE);
      if(res < 0)
      {
        perror("Read on serial file at /dev/ttyS0 failed");
        stop=TRUE;
      }else 
      {
        printf("%d bytes written\n", res);
      }
    }
  return 0;
}
// Receive data in packet
int llread(char* packet)
{
  int res, rd;
  //Handshake SET-UA
  char buf[UNNUM_FRAME_SIZE];
  int count_retransmisions = 0, error = 0;
  int state = 0;
  int reads_packet = 0;
  int stop = FALSE;

  if(ROLE == TRANSMITTER) //If you are a transmitter (TX)
  {
    //Read ACKs
    rd = 1;
    while (stop==FALSE) /* loop for input */
    {
      if (rd == 1) read(FD_SERIAL,buf,1);  /* returns after 1 chars have been input */
      //State machine to check UA message 
      state = rr_rej_st_machine(&rd, buf[0], state);
      if(state == 5) 
      {
        //printf("UA message received with sucess\n");
        state = 0;
        stop = TRUE;
      }
    }
  } else if(ROLE == RECEIVER) //If you are a receiver (RX)
        {
          // Read Information(I) Messages
          while (stop==FALSE) /* loop for input */
          {
            //Reads each byte of SET message
            /* returns after 1 chars have been input */
            if(rd==1) read(FD_SERIAL,buf,1);

            state=i_st_machine(packet,&rd, buf[0],state, sizeof(packet), reads_packet);
            if(state == 5)
            {
              printf("SET message received with success\n");
              stop=TRUE;
            } 

          } //End of Receive SET message LOOP
        }
    return 0;
}
// Closes previously opened connection; if showStatistics==TRUE, link layer should print statistics in the console on close
int llclose(int showStatistics)
{
  int res, rd; //How i find the file descriptor
  char buf[UNNUM_FRAME_SIZE];
  int state = 0;
  int stop = FALSE;
  //Check if exist a open connection 

  //Implement the disc state machine

if(ROLE == TRANSMITTER) //If you are a transmitter (TX)
    {
      //------------------------Exchange messages to disconnect (DISC)---------------------------------//
      //sprintf(buf,"%02X%02X%02X%02X%02X", FLAG, A_SERV_CLIENT, C_DISC, A_SERV_CLIENT ^ C_DISC, FLAG);
      res = write(FD_SERIAL,trama_disc_transmitter,UNNUM_FRAME_SIZE);
      if(res < 0)
      {
        perror("Read on serial file at /dev/ttyS0 failed");
        exit(1);
      } //else printf("%d bytes written\n", res);
      
      alarm(3); //timeout clock

      rd = 1; //Allow to read at first cycle
      while (stop==FALSE) /* routine to read DISC message */
      {
        if (rd == 1) read(FD_SERIAL, buf, 1);  /* returns after 1 chars have been input */
        //State machine to check DISC message 
        state = disc_st_machine(&rd, buf[0], state);
        if(state == 5) 
        {
          //printf("UA message received with sucess\n");
          alarm(0);
          state = 0;
          stop = TRUE;
        }
      }
      
      //UA response after 2 DISC messages exchange
      res = write(FD_SERIAL, buf, UNNUM_FRAME_SIZE);

    } else if(ROLE == RECEIVER) //If you are a receiver (RX)
    {
      // Receive DISC Message
      while (stop==FALSE) /* loop for input */
      {
        //Reads each byte of DISC message
        if(rd==1) read(FD_SERIAL,buf,1);

        state=disc_st_machine(&rd,buf[0],state);
        if(state == 5)
        {
          printf("DISC message received with success\n");
          alarm(0);
          state = 0;
          stop=TRUE;
        } 
      } //End of Receive DISC message LOOP
    
      //DISC response 
      //sprintf(buf,"%02X%02X%02X%02X%02X", FLAG, A_CLIENT_SERV, C_DISC, A_CLIENT_SERV ^ C_DISC, FLAG);

      //printf("Content of buffer sended: 0x%02X | 0x%02X | 0x%02X | 0x%02X| 0x%02X\n", 
      //                                  buf[0], buf[1], buf[2],buf[3], buf[4]);

      //Write UA message
      res = write(FD_SERIAL, trama_ua, UNNUM_FRAME_SIZE);
      if(res < 0)
      {
        perror("Read on serial file at /dev/ttyS0 failed");
      }
      while (stop==FALSE) /* routine to read UA message */
      {
        //Reads each byte of DISC message
        /* returns after 1 chars have been input */
        if(rd==1) read(FD_SERIAL,buf,1);

        state=ua_st_machine(&rd,buf[0],state);
        if(state == 5)
        {
          printf("SET message received with success\n");
          alarm(0);
          state = 0;
          stop=TRUE;
        } 
      } //End of Receive DISC message LOOP
    }
  
  close(FD_SERIAL);
  return 0;
} 


