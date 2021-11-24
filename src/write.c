#include "ll.h"

int main(int argc, char **argv){

    //check arguments
    if ((argc < 3) || (argc > 3)  || ((strcmp("/dev/tty10", argv[1]) != 0) && (strcmp("/dev/tty11", argv[1]) != 0)))
    {
        printf("Usage:\t./recv SerialPort File\n\tex: nserial /dev/tty11 pinguim.jpg\n");
        exit(1);
    }

    //compose port
    int port = 0;
    sscanf(argv[1], "/dev/tts%d", &port);

    //open
    int fd = -1;
    if((fd = llopen(port, RECEIVER)) < 0){
        printf("Error in llopen.\n");
        exit(1);
    }


    //treat data


    //close
    if(llclose(fd) < 0){
        printf("Error in llclose.\n");
        exit(1);
    }
}