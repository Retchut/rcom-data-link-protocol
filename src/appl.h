#ifndef _RCOM_APPL_H_
#define _RCOM_APPL_H_

#include <stdbool.h>

#define DATA_CHUNK_SIZE     1024
#define DATA_PACKET_SIZE    DATA_CHUNK_SIZE + 4
#define CTRL_PACKET_SIZE(l1, l2) 5+l1+l2

#define PACKET_CTRL_START   2
#define PACKET_CTRL_END     3
#define CTRL_FILE_SIZE      0
#define CTRL_FILE_NAME      1
#define UNSIGNED_INT_SIZE   4

struct fileData{
    FILE *filePtr;
    char *fileName;
    unsigned int fileNameSize;
    unsigned int fileSize;
    unsigned int fullPackets;
};

/**
 * @brief Retrieves file data
 * 
 * @return 0 on success, -1 otherwise
 */
int retrieveFileData(struct fileData *fData, FILE *filePtr, char *fileName);

/**
 * @brief Generates a control packet packets from a file and sends them to the receiver
 * 
 * @param ctrlPacket    Pointer to the packet where we're storing data
 * @param fData         Struct holding the file's data
 * @param start         Which Control Packet to generate
 * @return 0 on success, -1 otherwise
 */
int generateControlPacket(unsigned char *ctrlPacket, struct fileData *fData, int start);

/**
 * @brief Creates packets from file and sends them to the receiver
 * 
 * @param portfd    File Descriptor of the serial port
 * @param fileName  Name of the file to send
 */
int write(int portfd, char *fileName);

/**
 * @brief Receives packets from the transmitter and decodes them
 * 
 * @param portfd    File Descriptor of the serial port
 */
int receive(int portfd);

int generatePacket(char *packet, size_t packetSize);

//struct stat {
//    dev_t     st_dev;         /* ID of device containing file */
//    ino_t     st_ino;         /* Inode number */
//    mode_t    st_mode;        /* File type and mode */
//    nlink_t   st_nlink;       /* Number of hard links */
//    uid_t     st_uid;         /* User ID of owner */
//    gid_t     st_gid;         /* Group ID of owner */
//    dev_t     st_rdev;        /* Device ID (if special file) */
//    off_t     st_size;        /* Total size, in bytes */
//    blksize_t st_blksize;     /* Block size for filesystem I/O */
//    blkcnt_t  st_blocks;      /* Number of 512B blocks allocated */
//
//    /* Since Linux 2.6, the kernel supports nanosecond
//        precision for the following timestamp fields.
//        For the details before Linux 2.6, see NOTES. */
//
//    struct timespec st_atim;  /* Time of last access */
//    struct timespec st_mtim;  /* Time of last modification */
//    struct timespec st_ctim;  /* Time of last status change */
//
//#define st_atime st_atim.tv_sec      /* Backward compatibility */
//#define st_mtime st_mtim.tv_sec
//#define st_ctime st_ctim.tv_sec
//};

#endif /* _RCOM_APPL_H_ */