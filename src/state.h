#ifndef STATE_H_
#define STATE_H_

typedef enum {
  START, // Start of the input
  FLAG_RCV,
  A_RCV,
  C_RCV,
  I_MSG, // Switch state for I_MSG
  BCC_OK,
  DATA_RCV,
  STOP // Success state
} state_t;

typedef struct {
  state_t current_st;
  unsigned char addr;
  unsigned char ctrl;
} state_machine;

void handleState(unsigned char byte);

state_t get_state();

void set_state(state_t state);

unsigned char get_addr();

void set_addr(unsigned char addr);

unsigned char get_ctrl();

void set_ctrl(unsigned char ctrl);

#endif // STATE_H_
