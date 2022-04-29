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
#define C_I0 0x00 // Control of Information Message with bit 0
#define C_I1 0x02 // Control of Information Message with bit 1

//volatile int STOP=FALSE;
char C_I = 0x00;
int TIMEOUT;
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

char *trama_response = trama_rr0;

char switchNs(char ns)
{
  if(ns == C_I0)
    return C_I1;
  else 
    return C_I0;
}

char * switchResponse(char C_IX)
{
  if(C_IX == C_I0) 
    return trama_rr0;
  else 
    return trama_rr1;
}

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
        //printf("Header flag of DISC message is not correct\n");
        //printf("Header flag received: 0x%02X | Value Expected: 0x%02X \n", ua_byte, FLAG);
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
          //printf("Address flag of DISC message is not correct\n");
          //printf("Address flag received: 0x%02X | Value Expected: 0x%02X \n", ua_byte, A_CLIENT_SERV);
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
          //printf("Control flag of DISC message is not correct\n");
          //printf("Control flag received: 0x%02X | Value Expected: 0x%02X \n", ua_byte, C_UA);
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
          //printf("BCC flag of DISC message is not correct\n");
          //printf("BCC flag received: 0x%02X | Value Expected: 0x%02X \n", ua_byte, (A_CLIENT_SERV ^ C_UA));
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
          //printf(" Tail flag of DISC message is not correct\n");
          //printf("Tail flag received: 0x%02X | Value Expected: 0x%02X \n", ua_byte, FLAG);
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
        if(ua_byte == A_SERV_CLIENT) {*rd=0; state--;}
        else
        {
          printf("Control flag of RR message is not correct\n");
          printf("Control flag received: 0x%02X | Value Expected: 0x%02X \n", ua_byte, C_RR0);
          state=0;
        }
      break;
    case 3:
      if((ua_byte == (A_SERV_CLIENT^C_REJ0)) || (ua_byte == (A_SERV_CLIENT^C_REJ1)) || (ua_byte == (A_SERV_CLIENT^C_RR0)) || (ua_byte == (A_SERV_CLIENT^C_RR1))) 
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

int i_st_machine(char* packet, int *rd, unsigned char ua_byte, int state, int count_reads_payload, char* trama_res) 
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
        //printf("Header flag of I message is not correct\n");
        //printf("Header flag received: 0x%02X | Value Expected: 0x%02X \n", ua_byte, FLAG);
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
        trama_res = switchResponse(C_I);
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
      if(ua_byte == (A_CLIENT_SERV ^ C_I)) //Errado
      {
        //printf("Tail flag received: 0x%02X | Value Expected: 0x%02X \n", ua_byte, FLAG);
        state++;
      }
      else //Reads payload
        {
          printf("%02X", ua_byte);
          packet[count_reads_payload] = ua_byte;
          count_reads_payload++;
        }
      break;
      case 5:
      if(ua_byte == FLAG) 
      {
        state++;
      }
      else //Reads payload
        {
          //printf("%c", ua_byte);
          packet[count_reads_payload] = ua_byte;
          count_reads_payload++;
          //printf(" Tail flag of I message is not correct\n");
          //printf("Tail flag received: 0x%02X | Value Expected: 0x%02X \n", ua_byte, FLAG);
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
int count_retransmissions = 0;
char *trama_i;
void signal_handler(int sig)
{
  printf("Timeout\n");
  TIMEOUT = 1;
  //Creating SET message

  int res = write(FD_SERIAL,trama_i,sizeof(trama_i));
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

int llopen(linkLayer connectionParameters)
{
    //Check parameters
    ROLE = connectionParameters.role;
    TIMEOUT = connectionParameters.timeOut;
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

    if ( tcgetattr(FD_SERIAL,&oldtio) == -1) 
    { /* save current port settings */
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
int STATE = 0;
int llwrite(char* buf, int bufSize)
{
  int res, rd, i;
  //Handshake SET-UA
  //char buf[UNNUM_FRAME_SIZE];
  int count_retransmisions = 0, error = 0;
  int state = 0;
  int stop = FALSE;
  int I_SIZE = 4 + bufSize + 2;
  char I[I_SIZE];
  char R_X;
  //TODO: Must have the stuffing here
  //int payload_size = strlen(buf);
  if(bufSize > MAX_PAYLOAD_SIZE) printf("Payload to large");
  if(buf[0] == 0) //End of application called llwrite
  {
    return 0;
  } else 
  {
    I[0] = FLAG;
    I[1] = A_SERV_CLIENT;
    I[2] = C_I; // 0x00 or 0x02 //TODO: insert toogle feature
    I[3] = A_SERV_CLIENT ^ C_I; //BCC1

    for(i=0; i<bufSize; i++)
    {
      I[i+4] = buf[i];
    }

    I[I_SIZE - 2] = A_CLIENT_SERV ^ C_I;
    I[I_SIZE - 1] = FLAG;

    //Timeout
    trama_i = &I;

    for(i=0; i<I_SIZE; i++)
    {
      printf("%02X\t", I[i]);
    }
    //Write I message
    res = write(FD_SERIAL, I, I_SIZE);
    if(res < 0)
    {
      perror("Read on serial file at /dev/ttyS0 failed");
      return -1;
    } else printf("I message sent: %d bytes written\n", res);

    //Read ACKs
    rd = 1;
    while (stop==FALSE) /* loop for input */
    {
      if (rd == 1) read(FD_SERIAL,&R_X,1);  /* returns after 1 chars have been input */
      //State machine to check UA message 
      STATE = rr_rej_st_machine(&rd, R_X, STATE);
      if(STATE == 5) 
      {
        //printf("UA message received with sucess\n");
        STATE = 0;
        stop = TRUE;
      }
    }
    //For the next I message sent
    C_I = switchNs(C_I);
  }
 
  return 0;
}

int BYTES_READ = 0;
// Receive data in packet
int llread(char* packet)
{
  int res, rd;
  //Handshake SET-UA
  char buf;
  int count_retransmisions = 0, error = 0;
  //int state = 0;
  int *reads_packet = &BYTES_READ;
  int stop = FALSE;

  // Read Information(I) Messages
  if(STATE > 6) 
  {
    res = write(FD_SERIAL, trama_response, UNNUM_FRAME_SIZE);
    if(res < 0)
    {
      perror("Read on serial file at /dev/ttyS0 failed");
    }else 
    {
      printf("LL: Confirmation of I message(RR or REJ): %d bytes written\n", res);
    }
    packet[0] = 0;
    return 1;
  }else{
      while(stop==FALSE) /* loop for input */
      {
      //Reads each byte of SET message
      if(STATE == 6)
      {
        printf("LL: I message received with success\n");
        sleep(1);
        stop = TRUE;
        STATE ++;
        return BYTES_READ;
      }else 
      { 
        /* returns after 1 chars have been input */
        if(rd==1) read(FD_SERIAL,&buf,1);
        //if(buf != 0x74) printf("%02X\t", buf);
        STATE = i_st_machine(packet,&rd, buf,STATE, *reads_packet, trama_response);
      }
    }
  } 
  packet[0] = 0;
  return 1;
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
      
      alarm(TIMEOUT); //timeout clock

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


