#ifndef _RCOM_LL_H_
#define _RCOM_LL_H_

#include <stdbool.h>

/**
 * @brief Opens the serial port, and sets the role for the program
 *
 * @param port Number of the serial port to be opened
 * @param role_p Role of the program (either RECEIVER or TRANSMITTER)
 *
 * @returns The fd of the connection if successfull, -1 otherwise
 */
int llopen(int port, bool role_p);

/**
 * @brief Read information from the serial port, storing it in buffer
 *
 * @param fd File descriptor of the serial port
 * @param buffer Buffer to store the information read
 *
 * @return Size of the information read if successful, -1 otherwise
 */
int llread(int fd, unsigned char *buffer);

/**
 * @brief Writes to the serial port,
 *
 * @param fd File descriptor of the serial port
 * @param buffer Buffer of information to be written
 * @param length Length of the information to be written
 *
 * @return Size of the information if successful, -1 otherwise
 */
int llwrite(int fd, unsigned char *buffer, unsigned int length);

/**
 * @brief Closes the serial port, restoring the initial state
 *
 * @param fd File descriptor of the serial port
 *
 * @return 0 if successful, -1 otherwise
 */
int llclose(int fd);

#endif /* _RCOM_LL_H_ */
