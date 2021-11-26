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
        memcpy(frame+4, infoPtr, infoSize);
        frame[infoSize+4] = buildBCC2(infoPtr, infoSize);
        frame[infoSize+5] = FLAG;
        return 0;
    }

    return -1;
}

int writeFrame(int fd, unsigned char *frame, size_t size){
    if(write(fd, frame, size) == -1){
        perror("write frame");
        return -1;
    }
    return 0;
}

int readFrame(int fd, unsigned char *frame, size_t expectedSize){
    if(read(fd, frame, expectedSize) == -1){
        perror("read frame");
        return -1;
    }
    return 0;
}

bool testFrameEquality(unsigned char *frame1, unsigned char *frame2, size_t size){
    for(size_t p=0; p<0; p++){
        if(frame1[p]!=frame2[p]) return false;
    }
    return true;
}

int llopen(int port, bool transmitter){
    int fd;
    struct termios newtio;

    char serialPort[11];
    sprintf(serialPort, "%s%d", "/dev/ttyS", port);

    printf("%s\n", serialPort);
    
    //open serial port
    fd = open(serialPort, O_RDWR | O_NOCTTY);

    if (fd < 0)
    {
        perror(serialPort);
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

    //build frames
    //SET frame
    unsigned char *setFrame = (unsigned char *) malloc(SU_FRAME_SIZE);
    if(setFrame == NULL){
        printf("Error in malloc - SET frame.\n");
        return -1;
    }
    if(buildFrame(setFrame, A_WRT_CMD, C_SET, NULL, 0) != 0){
        printf("Error building frame - SET frame.\n");
        return -1;
    }
    //UA frame
    unsigned char *uaFrame = (unsigned char *) malloc (SU_FRAME_SIZE);
    if(setFrame == NULL){
        printf("Error in malloc - SET res frame.\n");
        return -1;
    }
    if(buildFrame(uaFrame, A_RCV_RESP, C_UA, NULL, 0) != 0){
        printf("Error building frame - UA response frame.\n");
        return -1;
    }

    bool completed = false;

    if(transmitter){
        //build test frame
        unsigned char *expectedUA = (unsigned char *) malloc (SU_FRAME_SIZE);
        if(setFrame == NULL){
            printf("Error in malloc - ua test frame.\n");
            return -1;
        }

        //TODO: implement timeout here
        while(!completed){
            //write command frame
            if(writeFrame(fd, setFrame, SU_FRAME_SIZE) != 0){
                printf("Error writing set frame.\n");
                continue;
            }
            printf("Sent SET frame.\n");

            //read response frame
            if(readFrame(fd, expectedUA, SU_FRAME_SIZE) != 0){
                printf("Error reading UA frame.\n");
                continue;
            }

            //test expected response
            if(!testFrameEquality(expectedUA, uaFrame, SU_FRAME_SIZE)){
                printf("SET response not UA frame.\n");
                continue;
            }
            printf("Received UA frame.\n");
            completed = true;
        }

        free(expectedUA);
    }
    else{
        //build test frame
        unsigned char *expectedSet = (unsigned char *) malloc(SU_FRAME_SIZE);
        if(setFrame == NULL){
            printf("Error in malloc - SET frame.\n");
            return -1;
        }

        //TODO: implement timeout here
        while(!completed){
            //read response frame
            if(readFrame(fd, expectedSet, SU_FRAME_SIZE) != 0){
                printf("Error reading UA frame.\n");
                continue;
            }

            //test expected response
            if(!testFrameEquality(expectedSet, uaFrame, SU_FRAME_SIZE)){
                printf("SET response not UA frame.\n");
                continue;
            }
            printf("Received SET frame.\n");

            //write command frame
            if(writeFrame(fd, uaFrame, SU_FRAME_SIZE) != 0){
                printf("Error writing set frame.\n");
                continue;
            }
            printf("Sent UA frame.\n");
            completed = true;
        }

        free(expectedSet);
    }

    free(setFrame);
    free(uaFrame);

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