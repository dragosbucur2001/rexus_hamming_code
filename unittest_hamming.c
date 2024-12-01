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
  uint8_t data_out[SIZE];
  buffer_hamming buf_out = {
      .buf = data_out,
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

  uint8_t data_in[SIZE];
  buffer_hamming buf_in = {
      .buf = data_in,
      .used = 0,
      .allocated = SIZE,
  };

  // manually chosen values
  buf_in.buf[0] = 4 << 4 | 3;
  buf_in.buf[1] = 2 << 4 | 11;
  buf_in.buf[2] = 7 << 4 | 15;
  buf_in.buf[3] = 9 << 4 | 12;
  buf_in.used = 4;

  encode_hamming(&buf_in, &buf_out);
  assert(buf_out.used == 8);
  assert(buf_out.buf[0] == mappings[4]);
  assert(buf_out.buf[1] == mappings[3]);
  assert(buf_out.buf[2] == mappings[2]);
  assert(buf_out.buf[3] == mappings[11]);
  assert(buf_out.buf[4] == mappings[7]);
  assert(buf_out.buf[5] == mappings[15]);
  assert(buf_out.buf[6] == mappings[9]);
  assert(buf_out.buf[7] == mappings[12]);

  // use the buf values so that they will not be optimised away
  printf("%d %d\n", buf_out.used, buf_out.buf[buf_out.used - 1]);

  return 0;
}
