#ifndef _RCOM_APPL_H_
#define _RCOM_APPL_H_

#include <stdbool.h>
#include <stdio.h>

#define DATA_CHUNK_SIZE 1024
#define DATA_PACKET_SIZE(dSize) dSize + 4
#define CTRL_PACKET_SIZE(l1, l2) 5 + l1 + l2

#define PACKET_DATA 1
#define PACKET_CTRL_START 2
#define PACKET_CTRL_END 3
#define CTRL_FILE_SIZE 0
#define CTRL_FILE_NAME 1
#define UNSIGNED_INT_SIZE 4

struct fileData {
  FILE *filePtr;             // Pointer to the file
  char *fileName;            // Name of the file
  unsigned int fileNameSize; // Size of the file name in bytes
  unsigned int fileSize;     // Size of the file in bytes
  unsigned int fullPackets;  // Number of full packets we can send
  unsigned int leftover;     // Size of the last non-full packet in bytes
};

/**
 * @brief Retrieves file data
 *
 * @return 0 on success, -1 otherwise
 */
int retrieveFileData(struct fileData *fData, FILE *filePtr, char *fileName);

/**
 * @brief Generates a control packet from a file's data
 *
 * @param ctrlPacket    Pointer to the packet where we're storing data
 * @param fData         Struct holding the file's data
 * @param start         Which Control Packet to generate
 * @return 0 on success, -1 otherwise
 */
int generateControlPacket(unsigned char *ctrlPacket, struct fileData *fData,
                          int start);

/**
 * @brief Generates a data packet from a file's content
 *
 * @param dataPacket    Pointer to the packet where we're storing data
 * @param fData         Misc filedata
 * @param data          Data we're storing in the packet
 * @param dataSize      Size of the data in the packet
 * @param seqN          Sequence number
 */
void generateDataPacket(unsigned char *dataPacket, struct fileData *fData,
                        unsigned char *data, unsigned int dataSize, int seqN);

/**
 * @brief Creates packets from file and sends them to the receiver
 *
 * @param portfd    File Descriptor of the serial port
 * @param fileName  Name of the file to send
 */
int sendFile(int portfd, char *fileName);

/**
 * @brief Receives packets from the transmitter and decodes them
 *
 * @param portfd    File Descriptor of the serial port
 */
int receive(int portfd);

#endif /* _RCOM_APPL_H_ */
