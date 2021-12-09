#include <stdio.h>

#include "state.h"
#include "datalink-defines.h"

static state_machine state;

state_t get_state() { return state.current_st; }

unsigned char get_addr() { return state.addr; }

unsigned char get_ctrl() { return state.ctrl; }

void set_state(state_t st) { state.current_st = st; }

void set_addr(unsigned char addr) { state.addr = addr; }

void set_ctrl(unsigned char ctrl) { state.ctrl = ctrl; }

static void handle_start(unsigned char byte) {
  switch (byte) {
  case FLAG:
    set_state(FLAG_RCV);
    break;
  default:
    break;
  }
}

static void handle_flag_rcv(unsigned char byte) {
  switch (byte) {
  case A_SEND_CMD:
  case A_RECV_CMD:
    set_state(A_RCV);
    set_addr(byte);
    break;
  case FLAG:
    break;
  default:
    set_state(START);
    break;
  }
}

static void handle_a_rcv(unsigned char byte) {
  switch (byte) {
  case C_SET:
  case C_UA:
  case C_DISC:
  case C_RR0:
  case C_RR1:
  case C_REJ0:
  case C_REJ1:
    set_state(C_RCV);
    set_ctrl(byte);
    break;
  case C_S0:
  case C_S1:
    set_state(I_MSG);
    set_ctrl(byte);
    break;
  case FLAG:
    set_state(FLAG_RCV);
    break;
  default:
    set_state(START);
    break;
  }
}

static void handle_c_recv(unsigned char byte) {
  switch (byte) {
  case FLAG:
    set_state(FLAG_RCV);
    break;
  default:
    if (byte == (BCC1(get_addr(), get_ctrl()))) {
      set_state(STOP);
    } else {
      set_state(START);
    }
    break;
  }
}

static void handle_bcc_ok(unsigned char byte) {
  switch (byte) {
  case FLAG:
    set_state(STOP);
    break;
  default:
    set_state(START);
    break;
  }
}

static void handle_i_msg(unsigned char byte) {
  switch (byte) {
  case FLAG:
    set_state(FLAG_RCV);
    break;
  default:
    if (byte == (BCC1(get_addr(), get_ctrl()))) {
      set_state(DATA_RCV);
    } else {
      set_state(START);
    }
    break;
  }
}

static void handle_data_rcv(unsigned char byte) {
  switch (byte) {
  case FLAG:
    set_state(STOP);
    break;
  default:
    break;
  }
}

void handleState(unsigned char byte) {

  switch (get_state()) {
  case START:
    handle_start(byte);
    break;
  case FLAG_RCV:
    handle_flag_rcv(byte);
    break;
  case A_RCV:
    handle_a_rcv(byte);
    break;
  case C_RCV:
    handle_c_recv(byte);
    break;
  case BCC_OK:
    handle_bcc_ok(byte);
    break;
  case I_MSG:
    handle_i_msg(byte);
    break;
  case DATA_RCV:
    handle_data_rcv(byte);
    break;
  case STOP:
    break;
  }
}
