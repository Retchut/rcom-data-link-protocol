#include <stdio.h>
#include <sys/stat.h>
#include <string.h>

#include "appl.h"
#include "ll.h"

int retrieveFileData(struct fileData *fData, FILE *filePtr, char *fileName){
    struct stat fileStat;
    if(fstat(fileno(filePtr), &fileStat) != 0){
        perror("fstat");
        return 1;
    }

    fData->filePtr = filePtr;
    fData->fileName = fileName;
    fData->fileNameSize = (unsigned int) strlen(fileName);
    fData->fileSize = (unsigned int) fileStat.st_size;
    fData->fullPackets = fData->fileSize / DATA_CHUNK_SIZE;

    return 0;
}

int generateControlPacket(unsigned char *ctrlPacket, struct fileData *fData, int start){
    if(start != PACKET_CTRL_START && start != PACKET_CTRL_END){
        return -1;
    }
    
    //Control field
    ctrlPacket[0] = start;
    //TLV1
    ctrlPacket[1] = CTRL_FILE_SIZE;
    ctrlPacket[2] = UNSIGNED_INT_SIZE; //handles file size up to around 4gb
    memcpy(ctrlPacket+3, &(fData->fileSize), UNSIGNED_INT_SIZE);    //TODO; check if this is fine
    //TLV2
    ctrlPacket[3+UNSIGNED_INT_SIZE] = CTRL_FILE_NAME;
    memcpy(ctrlPacket+4+UNSIGNED_INT_SIZE, &(fData->fileNameSize), 1);    //TODO; check if this is fine
    ctrlPacket[5+UNSIGNED_INT_SIZE] = fData->fileName;

    return 0;
}

int write(int portfd, char *fileName){
    FILE *filePtr = fopen(fileName, "r");
    if(filePtr == NULL){
        perror("fopen");
        return 1;
    }

    struct fileData fData;
    if(retrieveFileData(&fData, filePtr, fileName) != 0){
        printf("Error retrieving the file's data.\n");
        return 1;
    }

    //Set up start Control Packet
    unsigned int ctrlPacketSize = CTRL_PACKET_SIZE(UNSIGNED_INT_SIZE, fData.fileNameSize);
    unsigned char ctrlPacket[ctrlPacketSize];
    if(generateControlPacket(ctrlPacket, &fData, PACKET_CTRL_START) != 0){
        printf("Error creating the start control packet.\n");
        return 1;
    }
    
    //Send start Control Packet
    if(llwrite(portfd, ctrlPacket, ctrlPacketSize) != 0){
        printf("Error sending start control Packet.\n");
        return -1;
    }

    //Iterate through file, create and send Data Packets

    
    //Set up End Control Packet
    ctrlPacket[0] = PACKET_CTRL_END;

    //Send End Control Packet
    if(llwrite(portfd, ctrlPacket, ctrlPacketSize) != 0){
        printf("Error sending end control Packet.\n");
        return -1;
    }

    return 0;