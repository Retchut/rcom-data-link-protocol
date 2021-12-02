
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "config.h"
#include "defines.h"
#include "ll.h"
#include "rcom-ftp.h"
#include "send.h"

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
  memcpy(ctrlPacket + 3, &(fData->fileSize), FILE_SIZE_BYTES);
  // TLV2
  ctrlPacket[3 + FILE_SIZE_BYTES] = CTRL_FILE_NAME;
  memcpy(ctrlPacket + 4 + FILE_SIZE_BYTES, &(fData->fileNameSize), 1);
  memcpy(ctrlPacket + 5 + FILE_SIZE_BYTES, fData->fileName, fData->fileNameSize);
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
  printf("Opened file.\n");

  struct fileData fData;
  if (retrieveFileData(&fData, filePtr, fileName) != 0) {
    printf("Error retrieving the file's data.\n");
    return 1;
  }
  printf("Successfully retrieved the file's data.\n");

  // Set up start Control Packet
  unsigned int ctrlPacketSize =
      CTRL_PACKET_SIZE(FILE_SIZE_BYTES, fData.fileNameSize);
  unsigned char ctrlPacket[ctrlPacketSize];
  if (generateControlPacket(ctrlPacket, &fData, PACKET_CTRL_START) != 0) {
    printf("Error creating the start control packet.\n");
    return 1;
  }
  printf("Successfully generated the Start Control Packet.\n");

  // Send start Control Packet
  if (llwrite(portfd, ctrlPacket, ctrlPacketSize) != ctrlPacketSize) {
    printf("Error sending start control Packet.\n");
    return -1;
  }

  // Iterate through file, create and send Data Packets
  unsigned int packetsSent = 0;
  unsigned int allPackets = (fData.leftover == 0) ? fData.fullPackets : fData.fullPackets + 1;
  printf("Sending packets...\n");
  while (!feof(filePtr)) {
    // read data
    unsigned int dataSize = (packetsSent < fData.fullPackets)
                                ? MAX_DATA_CHUNK_SIZE
                                : fData.leftover;
    unsigned char data[dataSize];
    for (size_t i = 0; i < dataSize; i++) {
      data[i] = fgetc(filePtr);
    }

    // Create Data Packet
    unsigned int dataPacketSize = DATA_PACKET_SIZE(dataSize);
    unsigned char dataPacket[dataPacketSize];
    generateDataPacket(dataPacket, &fData, data, dataSize, packetsSent % 1);

    // Send Data Packet
    if (llwrite(portfd, dataPacket, dataPacketSize) != ctrlPacketSize) {
      printf("Error sending data packet number %u.\n", packetsSent);
      return -1;
    }
    packetsSent++;
    printf("Sent %u out of %u packets.\n", packetsSent, allPackets);
  }
  printf("All %u packets sent.\n", allPackets);

  // Set up End Control Packet
  ctrlPacket[0] = PACKET_CTRL_END;
  printf("Successfully generated the End Control Packet.\n");

  // Send End Control Packet
  if (llwrite(portfd, ctrlPacket, ctrlPacketSize) != ctrlPacketSize) {
    printf("Error sending end control Packet.\n");
    return -1;
  }
  printf("Successfully wrote the End Control Packet.\n");

  if(fclose(filePtr) != 0){
    perror("fclose");
    return 1;
  }
  printf("Successfully closed the file.\n");

  return 0;
}

int readStartPacket(int portfd, struct fileData *fData) {
  unsigned char startPacket[MAX_CTRL_PACKET_SIZE];
  if (llread(portfd, startPacket) == -1) {
    return 1;
  }

  if (startPacket[0] != PACKET_CTRL_START) {
    printf("Did not receive the start packet.\n");
    return 1;
  }

  if(startPacket[1] != 0){
    printf("Did not receive a correct start packet.\n");
    return 1;
  }

  if(startPacket[2] != FILE_SIZE_BYTES){
    printf("Did not receive a correct start packet.\n");
    return 1;
  }

  fData->filePtr = NULL;
  memcpy(&(fData->fileSize), startPacket + 3, FILE_SIZE_BYTES);

  if(startPacket[3+FILE_SIZE_BYTES] != 1){
    printf("Did not receive a correct start packet.\n");
    return 1;
  }

  memcpy(&(fData->fileNameSize), startPacket + 3 + FILE_SIZE_BYTES + 1, 1);
  memcpy(&(fData->fileName), startPacket + 3 + FILE_SIZE_BYTES + 3, fData->fileNameSize);
  fData->fullPackets = fData->fileSize / MAX_DATA_CHUNK_SIZE;
  fData->leftover = fData->fileSize % MAX_DATA_CHUNK_SIZE;

  return 0;
}

int readDataPacket(int portfd, unsigned char *data, unsigned int dataSize,
                   unsigned int expPacketNum) {
  unsigned char dataPacket[dataSize];
  if (llread(portfd, dataPacket) == -1) {
    printf("llread failed at readDataPacket\n");
    return 1;
  }

  if (dataPacket[0] != PACKET_DATA) {
    printf("Did not receive a data packet.\n");
    return 1;
  }

  unsigned int n = dataPacket[1];

  // if we receive the previous packet(already read)
  if (n == ((expPacketNum - 1 + 256) % 256)) {
    return 2;
  }
  // if we receive a packet ahead
  else if (n == ((expPacketNum + 1) % 256)) {
    printf("Received a packet in advance... aborting.\n");
    return 1;
  }

  unsigned int l2 = dataPacket[2];
  unsigned int l1 = dataPacket[3];
  unsigned int readSize = 256 * l2 + l1;
  for (size_t p = 0; p < readSize; p++) {
    data[p] = dataPacket[4 + p];
  }

  return 0;
}

int readEndPacket(int portfd) {
  unsigned char startPacket[MAX_CTRL_PACKET_SIZE];
  if (llread(portfd, startPacket) == -1) {
    return 1;
  }

  if (startPacket[0] != PACKET_CTRL_END) {
    printf("Did not receive the end packet.\n");
    return 1;
  }

  return 0;
}

int receiveFile(int portfd) {
  // read start packet
  struct fileData fData;
  if (readStartPacket(portfd, &fData) != 0) {
    printf("Error reading the start packet.\n");
    return 1;
  }
  printf("Successfully read the start packet and retrieved the file's data.\n");

  // DEBUG!!!!!
  FILE *fp = fopen("Pinguim-received.gif", "w");
  printf("Created file.\n");

  // create file with start packet values
  // FILE *fp = fopen(fData.fileName, "w");

  // fseek(fp, fData.fileSize-1, SEEK_SET);
  // fputc('\0', fp);
  // fseek(fp, 0, SEEK_SET);

  printf("Receiving packets...\n");
  unsigned int packetsRecvd = -1;
  unsigned int allPackets = (fData.leftover == 0) ? fData.fullPackets : fData.fullPackets + 1;
  while (true) {
    unsigned int dataSize = (packetsRecvd < fData.fullPackets) ? MAX_DATA_CHUNK_SIZE : fData.leftover;
    printf("Should be 1024: %u\n", dataSize);
    unsigned char data[dataSize];

    unsigned int expPacketNum = (packetsRecvd + 1) % 256;
    printf("Should be 0: %u\n", expPacketNum);
    if (readDataPacket(portfd, data, dataSize, expPacketNum) == 2) {
      printf("Received duplicate packet number %u... ignoring.\n",
             packetsRecvd);
      continue;
    } else if (readDataPacket(portfd, data, dataSize, expPacketNum) != 0) {
      printf("Error receiving data packet number %u.\n", packetsRecvd);
      return 1;
    }
    printf("Read data Packet,\n");

    // write data to file
    if (fwrite(data, 1, dataSize, fp) != dataSize) {
      printf("Error writing data packet number %u.\n", packetsRecvd);
      return 1;
    }
    packetsRecvd++;
    printf("Received %u out of %u packets.\n", packetsRecvd, allPackets);

    // break if we have read all packets
    if (packetsRecvd == allPackets)
      break;
  }
  printf("All %u packets received.\n", allPackets);

  if (readEndPacket(portfd) != 0) {
    printf("Error reading the end packet.\n");
    return 1;
  }
  printf("Successfully read the End Control Packet.\n");

  // close
  if (fclose(fp) != 0) {
    printf("Error closing the file.\n");
    return 1;
  }
  printf("Successfully closed the file.\n");

  return 0;
}

int main(int argc, char *argv[]) {

  if (!(argc == 3 && strcmp(argv[1], "receiver") == 0) &&
      !(argc == 4 && strcmp(argv[1], "emitter") == 0)) {
    fprintf(stderr, "Usage:\t./recv SerialPort File\n\tex: nserial /dev/tty11 "
                    "pinguim.jpg\n");
    exit(-1);
  }

  bool role = -1;
  int port = -1;
  if (strcmp(argv[1], "emitter") == 0) {
    role = TRANSMITTER;
  } else if (strcmp(argv[1], "receiver") == 0) {
    role = RECEIVER;
  }

  if (isdigit(argv[2][0]) && atoi(argv[2]) >= 0) {
    port = atoi(argv[2]);
  } else {
    fprintf(stderr, "Invalid port specified\n");
    exit(-1);
  }
  int fd = -1;
  if ((fd = llopen(port, role)) == -1) {
    fprintf(stderr, "Error opening port \n");
    exit(-1);
  }

  printf("main before.\n");
  if (role == TRANSMITTER) {
    if (sendFile(fd, argv[3]) != 0) {
      return 1;
    }
  } else if (role == RECEIVER) {
    if (receiveFile(fd) != 0) {
      return 1;
    }
  }
  printf("main after.\n");

  if (llclose(fd) == -1) {
    fprintf(stderr, "Error closing port \n");
    exit(-1);
  }

  return 0;
}