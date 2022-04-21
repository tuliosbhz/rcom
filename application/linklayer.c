#include "linklayer.h"

volatile int STOP=FALSE;
int flag=1, 




int llread(int fd,char* packet){
    int res, receiving=0,i=0,j;
	char str[2];

	STOP = FALSE;
	flag = 0;

	/* Enquanto a mensagem nao tiver sido lida ou esgotar o time-out ficar a espera*/
	while(STOP==FALSE && flag == 0){
		res=read(fd,str,1);
		if(res>0){                        			    // Check if read any character
			if (str[0]==0x7e && receiving == 0)  	        // vem ai uma trama -> receiving = 1
				receiving=1;
			else if (str[0]==0x7e && receiving == 1 && i==1)	// a trama terminou
				 continue;
		   else if (str[0]==0x7e && receiving == 1)
				STOP=TRUE;               		            // vou sair do ciclo de leitura
			if (receiving==1)			     	    		// receiving = 1: recebeu caracter da trama
				packet[i++] = str[0];
		}
	}
	return i;

}


int llwrite(int fd,char* buf, int bufSize){

    int res, i;
	res = write(fd,buf,bufSize);
	return res;
}


