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

int llopen(int port, bool transmitter){
    int fd;
    struct termios newtio;

    //open serial port
    fd = open(port, O_RDWR | O_NOCTTY);

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

int buildFrame(char **frame, char addr, char cmd, char *info){
    frame[0] = FLAG;
    frame[1] = addr;
    frame[2] = cmd;
    frame[3] = buildBCC();

    //check if we're not building an information frame
    if(info == NULL){
        frame[4] = FLAG;
        return 0;
    }
    else{
        frame[4] = info;
        frame[5] = buildBCC();
        frame[6] = FLAG;
        return 0;
    }

    return -1;
}
