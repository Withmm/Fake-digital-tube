#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
int hi2c1;

void set_terminal_mode() {
  struct termios term;
  tcgetattr(STDIN_FILENO, &term);
  term.c_lflag &= ~(ICANON);
  tcsetattr(STDIN_FILENO, TCSANOW, &term);
}

uint8_t DIGITAL[8] = {0};

void print_DIGITAL() {
  printf("DIGITAL: { ");
  for (uint8_t i = 0; i < 8; ++i) {
    printf("%x ", DIGITAL[i]);
  }
  printf("}\n");
}

void MX_I2C1_Init() { set_terminal_mode(); }

void I2C_ZLG7290_Read(int *fake, uint8_t dev_and_mode, uint8_t where,
                      uint8_t *buffer, uint8_t size) {
  if (dev_and_mode != 0x71) {
    printf("I2C_ZLG7290_Read arguments error");
    exit(0);
  }

  if (where == 0x01) {
    for (uint8_t i = 0; i < size; ++i) {
      uint8_t ch = getchar();
      if (ch == 0x31) {
        buffer[i] = 0x1C;
      } else if (ch == 0x32) {
        buffer[i] = 0x1B;
      } else if (ch == 0x33) {
        buffer[i] = 0x1A;
      } else if (ch == 0x34) {
        buffer[i] = 0x14;
      } else if (ch == 0x35) {
        buffer[i] = 0x13;
      } else if (ch == 0x36) {
        buffer[i] = 0x12;
      } else if (ch == 0x37) {
        buffer[i] = 0x0C;
      } else if (ch == 0x38) {
        buffer[i] = 0x0B;
      } else if (ch == 0x39) {
        buffer[i] = 0x0A;
      } else if (ch == 'q') {
        buffer[i] = 0x19;
      } else if (ch == 'w') {
        buffer[i] = 0x11;
      } else if (ch == 'e') {
        buffer[i] = 0x09;
      } else if (ch == 'r') {
        buffer[i] = 0x01;
      } else if (ch == 0x30) {
        buffer[i] = 0x03;
      } else if (ch=='t'){
        buffer[i] = 0x04;
      } else if (ch=='*'){
        buffer[i] = 0x02;
      }
    }
  } else if (where == 0x10) {
    for (uint8_t i = 0; i < size; ++i) {
      buffer[i] = DIGITAL[i];
    }
  }
}

void I2C_ZLG7290_Write(int *fake, uint8_t dev_and_mode, uint8_t where,
                       uint8_t *buffer, uint8_t size) {
  if (dev_and_mode != 0x70) {
    printf("I2C_ZLG7290_Write arguments error");
    exit(0);
  }

  for (uint8_t i = 0; i < size; ++i) {
    uint8_t offset = where - 0x10;
    if (offset + i >= 0 && offset + i < 8) {
      DIGITAL[offset + i] = buffer[i];
    }
  }
  print_DIGITAL();
  fflush(stdout);
}
