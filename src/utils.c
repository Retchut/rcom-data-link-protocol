#include "utils.h"
#include "defines.h"

unsigned char build_BCC2(unsigned char *data, size_t size) {
  unsigned char bcc2 = data[0];

  for (size_t p = 1; p < size; p++) {
    bcc2 ^= data[p];
  }

  return bcc2;
}

int unstuff_frame(unsigned char *stuffed_msg, size_t size,
                  unsigned char *unstuffed_msg) {

  int stuffed_idx = 0, unstuffed_idx = 0;
  for (; stuffed_idx < size; stuffed_idx++) {

    if (stuffed_msg[stuffed_idx] == ESCAPE) {
      stuffed_idx++;

      if (stuffed_msg[stuffed_idx] == 0x5d) {
        unstuffed_msg[unstuffed_idx++] = ESCAPE_ESCAPE;
      } else if (stuffed_msg[stuffed_idx] == FLAG_ESCAPE) {
        unstuffed_msg[unstuffed_idx++] = FLAG;
      } else {
        return -1;
      }
    } else {
      unstuffed_msg[unstuffed_idx++] = stuffed_msg[stuffed_idx];
    }
  }
  return unstuffed_idx;
}
