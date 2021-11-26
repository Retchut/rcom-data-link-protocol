#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>

#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "ll.h"

struct termios oldtio;

unsigned char buildBCC2(unsigned char *data, size_t size){
    unsigned char bcc2 = data[0];

    for(size_t p = 0; p<size; p++){
        bcc2 ^= data[p];
    }

    return bcc2;
}

int llopen(int port, bool transmitter){
    int fd;
    struct termios newtio;

    char serialPort[11] = "/dev/ttyS10";
    sprintf(serialPort, "%s %d", "/dev/ttyS", port);
    
    //open serial port
    fd = open(serialPort, O_RDWR | O_NOCTTY);

    if (fd < 0)
    {
        perror("open");
        return(-1);
    }

    //save original serial port settings
    if (tcgetattr(fd, &oldtio) == -1)
    {
        perror("tcgetattr");
        return(-1);
    }

    //prepares new serial port settings
    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;
    newtio.c_lflag = 0; //input mode (non-canonical, no echo,...)
    newtio.c_cc[VTIME]= 30; /* inter-character timer unused */
    newtio.c_cc[VMIN] = 0;  /* blocking read until 5 chars received */

    tcflush(fd, TCIOFLUSH); //discards data written to fd

    //apply new serial port settings
    if (tcsetattr(fd, TCSANOW, &newtio) == -1)
    {
        perror("tcsetattr");
        return(-1);
    }
    printf("New termios structure set\n");

    if(transmitter){
        unsigned char *setFrame = (unsigned char *) malloc(SU_FRAME_SIZE);
        if(setFrame == NULL){
            printf("Error in malloc - SET frame.");
            return -1;
        }
        if(buildFrame(setFrame, A_WRT_CMD, C_SET, NULL, 0) != 0){
            printf("Error building frame - SET frame.");
            return -1;
        }

        //Send SET
        //Wait to receive UA
    }
    else{
        //Wait to receive SET
        //Send UA
    }

    return fd;
}

int llclose(int fd){
    //restore original serial port settings
    if(tcsetattr(fd, TCSANOW, &oldtio) == -1){
        perror("tcsetattr");
        return(-1);
    }
    
    //close serial port
    if(close(fd) == -1){
        perror("close");
        return(-1);
    }

    return 0;
}

int buildFrame(unsigned char *frame, unsigned char addr, unsigned char cmd, unsigned char *infoPtr, size_t infoSize){
    frame[0] = FLAG;
    frame[1] = addr;
    frame[2] = cmd;
    frame[3] = BCC1(addr, cmd);

    //check if we're not building an information frame
    if(infoPtr == NULL){
        frame[4] = FLAG;
        return 0;
    }
    else{
        frame[4] = infoPtr;
        frame[5] = buildBCC2(infoPtr, infoSize);
        frame[6] = FLAG;
        return 0;
    }

    return -1;
}