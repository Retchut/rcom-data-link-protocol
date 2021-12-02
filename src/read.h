#ifndef _RCOM_READ_H_
#define _RCOM_READ_H_

/**
 * @brief Reads a supervision Frame, altering the state based on the information
 * received. Times out after 3 seconds of not receiving anything..
 *
 * @param fd File descriptor of the serial port
 *
 * @return The size of a Control frame is succeeded, -1 otherwise
 */
int readSupervisionFrame(int fd);

/**
 * @brief Reads a response to an Information Frame,
 *
 * @param fd File Descriptor of the serial port
 *
 * @return If the I Frame was accepted, returns the RR bit. If it was rejected,
 * it returns REJECTED. otherwise, it returns -1.
 */
int readInformationFrameResponse(int fd);

/**
 * @brief Reads an Information Frame, altering the stated accordingly and saving
 * the information part of the packet
 *
 * @param fd File Descriptor of the serial port
 * @param stuffed_msg Buffer to store the data part of the frame received
 *
 * @return size of the stuffed message (-1 as to not count for the FLAG, which
 * is also included), or -1 otherwise
 */
int readInformationMessage(int fd, unsigned char *stuffed_msg);

#endif /* _RCOM_READ_H_ */
