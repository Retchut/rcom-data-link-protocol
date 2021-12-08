#ifndef RCOM_UTILS_H_
#define RCOM_UTILS_H_

#include <stddef.h>

/**
 * @brief builds the BCC2 block, based on the original input data
 *
 * @param data Data to be verified
 * @param Size of the input data
 *
 * @return The BCC of the data given
 */
unsigned char build_BCC2(unsigned char *data, size_t size);

/**
 * @brief Unstuffs the information received
 *
 * @param stuffed_msg Messaged to be unstuffed
 * @param size Size of the message to be unstuffed
 * @param unstuffed_msg Buffer to hold the actual message
 *
 * @return Size of the unstuffed information if successfull, -1 otherwise
 */
int unstuff_frame(unsigned char *stuffed_msg, size_t size,
                  unsigned char *unstuffed_msg);

/**
 * @brief Stuffs the information received
 *
 * @param data Messaged to be stuffed
 * @param data_size Size of the message to be unstuffed
 * @param stuffed_data Buffer to hold the stuffed message
 *
 * @return Size of the stuffed information if successfull, -1 otherwise
 */
int stuff_data(unsigned char *data, size_t data_size,
               unsigned char *stuffed_data);

#endif // RCOM_UTILS_H_
