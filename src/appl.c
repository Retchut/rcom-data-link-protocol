#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "appl.h"
#include "ll.h"

int retrieveFileData(struct fileData *fData, FILE *filePtr, char *fileName) {
  struct stat fileStat;
  if (fstat(fileno(filePtr), &fileStat) != 0) {
    perror("fstat");
    return 1;
  }

  fData->filePtr = filePtr;
  fData->fileName = fileName;
  fData->fileNameSize = (unsigned int)strlen(fileName);
  fData->fileSize = (unsigned int)fileStat.st_size;
  fData->fullPackets = fData->fileSize / MAX_DATA_CHUNK_SIZE;
  fData->leftover = fData->fileSize % MAX_DATA_CHUNK_SIZE;

  return 0;
}

int generateControlPacket(unsigned char *ctrlPacket, struct fileData *fData,
                          int start) {
  if (start != PACKET_CTRL_START && start != PACKET_CTRL_END) {
    return -1;
  }

  // Control field
  ctrlPacket[0] = start;
  // TLV1
  ctrlPacket[1] = CTRL_FILE_SIZE;
  ctrlPacket[2] = FILE_SIZE_BYTES;
  memcpy(ctrlPacket + 3, &(fData->fileSize),
         FILE_SIZE_BYTES); // TODO; check if this is fine
  // TLV2
  ctrlPacket[3 + FILE_SIZE_BYTES] = CTRL_FILE_NAME;
  memcpy(ctrlPacket + 4 + FILE_SIZE_BYTES, &(fData->fileNameSize),
         1); // TODO; check if this is fine
  memcpy(ctrlPacket + 5 + FILE_SIZE_BYTES, fData->fileName, fData->fileSize);

  return 0;
}

void generateDataPacket(unsigned char *dataPacket, struct fileData *fData,
                        unsigned char *data, unsigned int dataSize, int seqN) {
  unsigned int l1 = MAX_DATA_CHUNK_SIZE % 256;
  unsigned int l2 = MAX_DATA_CHUNK_SIZE / 256;
  unsigned int dataPacketHeader[4] = {PACKET_DATA, seqN, l1, l2};
  memcpy(dataPacket, dataPacketHeader, 4);
  memcpy(dataPacket + 4, data, dataSize);
}

int sendFile(int portfd, char *fileName) {
  FILE *filePtr = fopen(fileName, "r");
  if (filePtr == NULL) {
    perror("fopen");
    return 1;
  }

  struct fileData fData;
  if (retrieveFileData(&fData, filePtr, fileName) != 0) {
    printf("Error retrieving the file's data.\n");
    return 1;
  }

  // Set up start Control Packet
  unsigned int ctrlPacketSize =
      CTRL_PACKET_SIZE(FILE_SIZE_BYTES, fData.fileNameSize);
  unsigned char ctrlPacket[ctrlPacketSize];
  if (generateControlPacket(ctrlPacket, &fData, PACKET_CTRL_START) != 0) {
    printf("Error creating the start control packet.\n");
    return 1;
  }

  // Send start Control Packet
  if (llwrite(portfd, ctrlPacket, ctrlPacketSize) != 0) {
    printf("Error sending start control Packet.\n");
    return -1;
  }

  // Iterate through file, create and send Data Packets
  unsigned int packetsSent = 0;
  while (!feof(filePtr)) {
    // read data
    unsigned int dataSize =
        (packetsSent < fData.fullPackets) ? MAX_DATA_CHUNK_SIZE : fData.leftover;
    unsigned char data[dataSize];
    for (size_t i = 0; i < dataSize; i++) {
      data[i] = fgetc(filePtr);
    }

    // Create Data Packet
    unsigned int dataPacketSize = DATA_PACKET_SIZE(dataSize);
    unsigned char dataPacket[dataPacketSize];
    generateDataPacket(dataPacket, &fData, data, dataSize, packetsSent % 1);

    // Send Data Packet
    if (llwrite(portfd, dataPacket, dataPacketSize) != 0) {
      printf("Error sending data packet number %u.\n", packetsSent);
      return -1;
    }
    packetsSent++;
  }

  // Set up End Control Packet
  ctrlPacket[0] = PACKET_CTRL_END;

  // Send End Control Packet
  if (llwrite(portfd, ctrlPacket, ctrlPacketSize) != 0) {
    printf("Error sending end control Packet.\n");
    return -1;
  }

  return 0;
}

int readStartPacket(int portfd, struct fileData *fData){
  unsigned char startPacket[MAX_CTRL_PACKET_SIZE];
  if(llread(portfd, startPacket) != 0){
    return 1;
  }

  if(startPacket[0] != PACKET_CTRL_START){
    printf("Did not receive the start packet.\n");
    return 1;
  }

  fData->filePtr = NULL;
  memcpy(&(fData->fileSize), startPacket+3, FILE_SIZE_BYTES);
  memcpy(&(fData->fileNameSize), startPacket+3+FILE_SIZE_BYTES+2);
  memcpy(&(fData->fileName), startPacket+3+FILE_SIZE_BYTES+3, fData->fileNameSize);
  fData->fullPackets = fData->fileSize / MAX_DATA_CHUNK_SIZE;
  fData->leftover = fData->fileSize % MAX_DATA_CHUNK_SIZE;

  return 0;
}

int readDataPacket(int portfd, unsigned char *data, unsigned int dataSize){
  unsigned char dataPacket[dataSize];
  if(llread(portfd, dataPacket) != 0){
    return 1;
  }

  if(dataPacket[0] != PACKET_DATA){
    printf("Did not receive a data packet.\n");
    return 1;
  }

  //TODO: descobrir porque é que preciso do N (número de sequência (módulo 255))

  unsigned int l1, l2, readSize;
  memcpy(l1, dataPacket + 3, 1);
  memcpy(l2, dataPacket + 2, 1);
  readSize = 256*l2 + l1;
  for(size_t p=0; p < readSize; p++){
    data[p] = dataPacket[4+p];
  }

  return 0;
}

int readEndPacket(int portfd){
  unsigned char startPacket[MAX_CTRL_PACKET_SIZE];
  if(llread(portfd, startPacket) != 0){
    return 1;
  }

  if(startPacket[0] != PACKET_CTRL_END){
    printf("Did not receive the end packet.\n");
    return 1;
  }

  return 0;
}

int receiveFile(int portfd) {
  // read start packet
  struct fileData fData;
  if(readStartPacket(portfd, &fData) != 0){
    printf("Error reading the start packet.\n");
    return 1;
  }

  // create file with start packet values
  FILE *fp = fopen(fData.fileName, "w");
  // fseek(fp, fData.fileSize-1, SEEK_SET);
  // fputc('\0', fp);
  // fseek(fp, 0, SEEK_SET);

  unsigned int packetsRecvd = 0;
  while(true){
    unsigned int dataSize = (packetsRecvd < fData.fullPackets) ? MAX_DATA_CHUNK_SIZE : fData.leftover;
    unsigned char data[dataSize];

    if(readDataPacket(portfd, data, dataSize) != 0){
      printf("Error receiving data packet number %u.\n", packetsRecvd);
      return 1;
    }

    //write data to file
    if(fwrite(data, 1, dataSize, fp) != dataSize){
      printf("Error writing data packet number %u.\n", packetsRecvd);
      return 1;
    }
    packetsRecvd++;

    //break if we have read all packets
    unsigned int allPackets = (fData.leftover == 0) ? fData.fullPackets : fData.fullPackets + 1;
    if(packetsRecvd == allPackets) break;
  }

  if(readEndPacket(portfd) != 0){
    printf("Error reading the end packet.\n");
    return 1;
  }

  // close
  if(fclose(fp) != 0){
    printf("Error closing the file.\n");
    return 1;
  }

  return 0;
}
