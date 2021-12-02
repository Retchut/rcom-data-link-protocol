#ifndef STATE_H_
#define STATE_H_

typedef enum {
  START,    // Start of the input
  FLAG_RCV, // Received a flag
  A_RCV,    // Received Sender or Receiver Address
  C_RCV,    // Received ctrl byte that is not from an I frame
  I_MSG,    // Switch state for I_MSG
  BCC_OK,   // Receives the BCC for a supervision message
  DATA_RCV, // Receiving data, "ignore" these bytes for now
  STOP      // Success state
} state_t;

typedef struct {
  state_t current_st;
  unsigned char addr;
  unsigned char ctrl;
} state_machine;

/**
 * @brief Handles the state of the program, according to the input given
 *
 * @param byte Byte received from serial port
 */
void handleState(unsigned char byte);

/**
 * @brief Retrieves the current state of the program
 *
 * @return The current state of the program
 */
state_t get_state();

void set_state(state_t state);

unsigned char get_addr();

void set_addr(unsigned char addr);

unsigned char get_ctrl();

void set_ctrl(unsigned char ctrl);

#endif // STATE_H_
