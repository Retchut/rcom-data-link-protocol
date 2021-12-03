#include "utils.h"
#include "defines.h"
#include <stdio.h>

unsigned char build_BCC2(unsigned char *data, size_t size) {
  unsigned char bcc2 = data[0];

  for (size_t p = 1; p < size; p++) {
    bcc2 ^= data[p];
  }

  return bcc2;
}

int unstuff_frame(unsigned char *stuffed_msg, size_t size,
                  unsigned char *unstuffed_msg) {
  printf("before unstuffing:\n");
  printf("1018: %X\t1019: %X\t1020: %X\t 1021: %X\t1022: %X\t1023: %X\n", stuffed_msg[size-6-1], stuffed_msg[size-5-1], stuffed_msg[size-4-1], stuffed_msg[size-3-1], stuffed_msg[size-2-1], stuffed_msg[size-1-1]);

  int stuffed_idx = 0, unstuffed_idx = 0;

  for (; stuffed_idx < size; stuffed_idx++) {

    if (stuffed_msg[stuffed_idx] == ESCAPE) {
      stuffed_idx++;

      if (stuffed_msg[stuffed_idx] == ESCAPE_ESCAPE) {
        unstuffed_msg[unstuffed_idx++] = ESCAPE;
      } else if (stuffed_msg[stuffed_idx] == FLAG_ESCAPE) {
        unstuffed_msg[unstuffed_idx++] = FLAG;
      } else {
        return -1;
      }
    } else {
      unstuffed_msg[unstuffed_idx++] = stuffed_msg[stuffed_idx];
    }
  }
  printf("after unstuffing:\n");
  printf("1018: %X\t1019: %X\t1020: %X\t 1021: %X\t1022: %X\t1023: %X\n", unstuffed_msg[size-6-1], unstuffed_msg[size-5-1], unstuffed_msg[size-4-1], unstuffed_msg[size-3-1], unstuffed_msg[size-2-1], unstuffed_msg[size-1-1]);

  return unstuffed_idx;
}
