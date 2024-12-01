#include "./hamming.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#define SIZE 1000

// encodings of 4 bit numbers to Hamming (8,4)
// manually crafted
uint8_t mappings[16] = {
    0b00000000, 0b00001111, 0b00110011, 0b00111100, 0b01010101, 0b01011010,
    0b01100110, 0b01101001, 0b10010110, 0b10011001, 0b10100101, 0b10101010,
    0b11000011, 0b11001100, 0b11110000, 0b11111111,
};

int main() {
  uint8_t buffer[SIZE];
  buffer_hamming buf_out = {
      .buf = buffer,
      .used = 0,
      .allocated = SIZE,
  };

  for (int i = 0; i < 16; i++) {
    encode_4_bits(&buf_out, i);
    assert(buf_out.buf[buf_out.used - 1] == mappings[i]);
  }

  for (int i = 0; i < 16; i++) {
    encode_4_bits(&buf_out, i);
    assert(buf_out.buf[buf_out.used - 1] == mappings[i]);
  }

  // use the buf values so that they will not be optimised away
  printf("%d %d\n", buf_out.used, buf_out.buf[buf_out.used - 1]);

  return 0;
}
