#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "rcom-ftp.h"
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
  memcpy(ctrlPacket + 3, &(fData->fileSize), FILE_SIZE_BYTES);
  // TLV2
  ctrlPacket[3 + FILE_SIZE_BYTES] = CTRL_FILE_NAME;
  memcpy(ctrlPacket + 4 + FILE_SIZE_BYTES, &(fData->fileNameSize), 1);
  memcpy(ctrlPacket + 5 + FILE_SIZE_BYTES, fData->fileName,
         fData->fileNameSize);

  return 0;
}

void generateDataPacket(unsigned char *dataPacket, struct fileData *fData,
                        unsigned char *data, unsigned int dataSize, int seqN) {
  unsigned int l1 = dataSize % 256;
  unsigned int l2 = dataSize / 256;
  unsigned char dataPacketHeader[4] = {PACKET_DATA, seqN, l2, l1};
  memcpy(dataPacket, dataPacketHeader, 4);
  memcpy(dataPacket + DATA_PACKET_HEADER_SIZE, data, dataSize);
}

int sendFile(int portfd, char *fileName) {
  FILE *filePtr = fopen(fileName, "r");
  if (filePtr == NULL) {
    perror("fopen");
    return 1;
  }
  printf("\nOpened file.\n");

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
  printf("Successfully sent the Start Control Packet.\n");

  // Iterate through file, create and send Data Packets
  unsigned int packetsSent = 0;
  unsigned int allPackets =
      (fData.leftover == 0) ? fData.fullPackets : fData.fullPackets + 1;

  printf("\nSending packets...\n");
  while (packetsSent < allPackets) {
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
    generateDataPacket(dataPacket, &fData, data, dataSize, packetsSent % 256);

    if (llwrite(portfd, dataPacket, dataPacketSize) == -1) {
      fprintf(stderr, "Error writing packet %d\n", packetsSent);
      exit(-1);
    }

    packetsSent++;
    printf("Sent %u out of %u packets.\n", packetsSent, allPackets);
  }
  printf("\nAll %u packets sent.\n\n", allPackets);

  // Set up End Control Packet
  ctrlPacket[0] = PACKET_CTRL_END;
  printf("Successfully generated the End Control Packet.\n");

  // Send End Control Packet
  if (llwrite(portfd, ctrlPacket, ctrlPacketSize) != ctrlPacketSize) {
    printf("Error sending end control Packet.\n");
    return -1;
  }
  printf("Successfully wrote the End Control Packet.\n");

  if (fclose(filePtr) != 0) {
    perror("fclose");
    return 1;
  }
  printf("Successfully closed the file.\n\n");

  return 0;
}

int readStartPacket(int portfd, struct fileData *fData) {
  unsigned char startPacket[MAX_CTRL_PACKET_SIZE];
  int size = -1;
  if ((size = llread(portfd, startPacket)) == -1) {
    fprintf(stderr, "llread failed at reading start packet\n");
    return 1;
  }

  if (startPacket[0] != PACKET_CTRL_START) {
    printf("Did not receive the start packet.\n");
    return 1;
  }

  if (startPacket[1] != 0) {
    printf("Did not receive a correct start packet.\n");
    return 1;
  }

  if (startPacket[2] != FILE_SIZE_BYTES) {
    printf("Did not receive a correct start packet.\n");
    return 1;
  }

  fData->filePtr = NULL;
  memcpy(&(fData->fileSize), startPacket + 3, FILE_SIZE_BYTES);

  if (startPacket[3 + FILE_SIZE_BYTES] != 1) {
    printf("Did not receive a correct start packet.\n");
    return 1;
  }

  fData->fileNameSize = startPacket[4 + FILE_SIZE_BYTES];
  fData->fileName = (char *)malloc(fData->fileNameSize * sizeof(char));
  memcpy(fData->fileName, startPacket + 3 + FILE_SIZE_BYTES + 2,
         fData->fileNameSize);

  fData->fullPackets = fData->fileSize / MAX_DATA_CHUNK_SIZE;
  fData->leftover = fData->fileSize % MAX_DATA_CHUNK_SIZE;

  return 0;
}

int readDataPacket(int portfd, unsigned char *data, unsigned int dataPacketSize,
                   unsigned int dataSize, unsigned int expPacketNum) {
  unsigned char *dataPacket = (unsigned char *)malloc(dataPacketSize);
  if (llread(portfd, dataPacket) == -1) {
    free(dataPacket);
    printf("llread failed at readDataPacket\n");
    return 1;
  }

  if (dataPacket[0] != PACKET_DATA) {
    free(dataPacket);
    printf("Did not receive a data packet.\n");
    return 1;
  }

  unsigned int n = dataPacket[1];

  //if we don't receive the correct packet
  if(n != expPacketNum){
    free(dataPacket);
    return 1;
  }

  unsigned int l2 = dataPacket[2];
  unsigned int l1 = dataPacket[3];
  unsigned int readSize = 256 * l2 + l1;
  for (size_t p = 0; p < readSize; p++) {
    data[p] = dataPacket[DATA_PACKET_HEADER_SIZE + p];
  }

  free(dataPacket);
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
  printf("\nSuccessfully read the start packet and retrieved the file's data.\n");

  // prepare modified fileName
  char newName[9 + fData.fileNameSize];
  sprintf(newName, "%s%s", "received-", fData.fileName);

  // create file
  FILE *fp;
  if (fData.fileNameSize + 9 <= MAX_FILENAME_SIZE) {
    fp = fopen(newName, "w");
  } else { // create file with start packet values if by modifying the name we
           // went above the max filename size
    fp = fopen(fData.fileName, "w");
  }
  printf("Created file.\n");

  printf("\nReceiving packets...\n");
  unsigned int packetsRecvd = 0;
  unsigned int allPackets =
      (fData.leftover == 0) ? fData.fullPackets : fData.fullPackets + 1;
  while (true) {
    unsigned int dataSize = (packetsRecvd < fData.fullPackets)
                                ? MAX_DATA_CHUNK_SIZE
                                : fData.leftover;
    unsigned int dataPacketSize = DATA_PACKET_SIZE(dataSize);

    unsigned char data[dataSize];

    unsigned int expPacketNum = packetsRecvd % 256;

    if (readDataPacket(portfd, data, dataPacketSize, dataSize, expPacketNum) != 0) {
      printf("Error receiving data packet number %u.\n", packetsRecvd);
      return 1;
    }

    // write data to file
    if (fwrite(data, 1, dataSize, fp) != dataSize) {
      printf("Error writing data packet number %u.\n", packetsRecvd);
      return 1;
    }

    // loop control
    packetsRecvd++;
    printf("Received %u out of %u packets.\n", packetsRecvd, allPackets);
    // break if we have read all packets
    if (packetsRecvd == allPackets)
      break;
  }
  printf("\nAll %u packets received.\n\n", allPackets);

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
  printf("Successfully closed the file.\n\n");

  return 0;
}

int main(int argc, char *argv[]) {

  if (!(argc == 3 && strcmp(argv[1], "receiver") == 0) &&
      !(argc == 4 && strcmp(argv[1], "emitter") == 0)) {
    fprintf(stderr,
            "Usage:\t./rcom-ftp Role SerialPortNum [File]\n\tex: \t./rcom-ftp "
            "emitter 11 pinguim.gif\n\t\t./rcom-ftp receiver 10\n");
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

  if (role == TRANSMITTER) {
    if (sendFile(fd, argv[3]) != 0) {
      return 1;
    }
  } else if (role == RECEIVER) {
    if (receiveFile(fd) != 0) {
      return 1;
    }
  }

  if (llclose(fd) == -1) {
    fprintf(stderr, "Error closing port \n");
    exit(-1);
  }

  return 0;
}
