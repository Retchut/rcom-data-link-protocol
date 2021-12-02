#ifndef _RCOM_SEND_H_
#define _RCOM_SEND_H_

#include <stddef.h>

/**
 * @brief Writes an Information Frame to the Serial Port
 *
 * @param fd File Descriptor of the serial port
 * @param addr Address of the sender of the frame
 * @param info_ptr Information to be written to the serial port
 * @param info_size Size of the information to be sent
 * @param msg_nr Number of the packet that will be sent
 *
 * @return 0 if the entire frame was written, -1 otherwise
 */
int writeInformationFrame(int fd, unsigned char addr, unsigned char *info_ptr,
                          size_t info_size, int msg_nr);

/**
 * @brief Writes an Information Frame to the Serial Port, retrying 3 times if
 * failed
 *
 * @param fd File Descriptor of the serial port
 * @param addr Address of the sender of the frame
 * @param info_ptr Information to be written to the serial port
 * @param info_size Size of the information to be sent
 * @param msg_nr Number of the packet that will be sent
 *
 * @return The size of the packet if the entire frame was written and accepted,
 * -1 otherwise
 */
int writeInformationAndRetry(int fd, unsigned char addr,
                             unsigned char *info_ptr, size_t info_size,
                             int msg_nr);
/**
 * @brief Writes a Supervision Frame to the Serial Port
 *
 * @param fd File Descriptor of the serial port
 * @param msg_addr Address of the sender of the frame
 * @param msg_ctrl Control message to be sent
 *
 * @return The size of the frame if the entire frame was written,
 * -1 otherwise
 */
int writeSupervisionFrame(int fd, unsigned char msg_addr,
                          unsigned char msg_ctrl);

/**
 * @brief Writes a Supervision Frame to the Serial Port, and retries if failing
 * to write everything
 *
 * @param fd File Descriptor of the serial port
 * @param msg_addr Address of the sender of the frame
 * @param msg_ctrl Control message to be sent
 *
 * @return 0 if successfull, -1 otherwise
 */
int writeSupervisionAndRetry(int fd, unsigned char msg_addr,
                             unsigned char msg_ctrl);

#endif /* _RCOM_SEND_H_ */
