#include <assert.h>
#include <stdint.h>
#include <stdio.h>

// small RNG:
// https://www.pcg-random.org/posts/bob-jenkins-small-prng-passes-practrand.html

typedef struct {
  uint32_t a, b, c, d;
} ran_ctx;

#define rot32(x, k) (((x) << (k)) | ((x) >> (32 - (k))))

uint32_t ran_val(ran_ctx *gen) {
  uint32_t e = gen->a - rot32(gen->b, 27);
  gen->a = gen->b ^ rot32(gen->c, 17);
  gen->b = gen->c + gen->d;
  gen->c = gen->d + e;
  gen->d = e + gen->a;
  return gen->d;
}

uint8_t roll_dice(ran_ctx *gen, float probability) {
  float roll = (float)ran_val(gen) / (float)UINT32_MAX;
  return roll < probability;
}

void ran_init(ran_ctx *gen, uint32_t seed) {
  gen->a = 0xf1ea5eed;
  gen->b = gen->c = gen->d = seed;

  for (uint32_t i = 0; i < 20; ++i) {
    (void)ran_val(gen);
  }
}

const uint32_t SEED = 0;

// end RNG

//
// ==== Packet Parameters ====
//

// this needs to be a define because of some gcc error
#define HEADER_SEGMENT_SIZE 2
const char HEADER[HEADER_SEGMENT_SIZE] = {0b10101010, 0b10101010};
const uint32_t DATA_SEGMENT_SIZE = 12;

const uint32_t PACKAGE_SIZE = HEADER_SEGMENT_SIZE + DATA_SEGMENT_SIZE;
const uint32_t PACKAGE_COUNT = 2000;
const uint32_t DATA_SIZE = PACKAGE_SIZE * PACKAGE_COUNT;

//
// ==== Error Parameters ====
//

const float BIT_ERROR_P = 0.01;
const float BYTE_DROP_P = 0.01;

int main() {
  ran_ctx generator;
  ran_init(&generator, SEED);

  char data[DATA_SIZE];
  uint32_t data_idx = 0;

  for (uint32_t i = 0; i < PACKAGE_COUNT; i++) {
    for (uint32_t j = 0; j < HEADER_SEGMENT_SIZE; j++) {
      data[data_idx++] = HEADER[j];
    }

    for (uint32_t j = 0; j < DATA_SEGMENT_SIZE; j++) {
      data[data_idx] = ran_val(&generator);
    }
  }

  char bit_error_data[DATA_SIZE];
  for (uint32_t i = 0; i < DATA_SIZE; i++) {
    bit_error_data[i] = 0;

    for (int j = 7; j > 0; j--) {
      char data_bit = (data[i] & (1 << j)) >> j;

      if (roll_dice(&generator, BIT_ERROR_P)) {
        data_bit ^= 1;
      }

      bit_error_data[i] |= data_bit << j;
    }
  }

  char byte_error_data[DATA_SIZE];
  uint32_t byte_error_size = DATA_SIZE;
  uint32_t byte_error_idx = 0;
  for (uint32_t i = 0; i < DATA_SIZE; i++) {
    if (roll_dice(&generator, BYTE_DROP_P)) {
      byte_error_size--;
      continue;
    }

    byte_error_data[byte_error_idx++] = data[i];
  }

  FILE *original_data_f = fopen("original.bin", "wb");
  fwrite(data, sizeof(char), DATA_SIZE, original_data_f);
  fclose(original_data_f);

  FILE *bit_error_data_f = fopen("bit_error.bin", "wb");
  fwrite(bit_error_data, sizeof(char), DATA_SIZE, bit_error_data_f);
  fclose(bit_error_data_f);

  FILE *byte_error_data_f = fopen("byte_drop.bin", "wb");
  assert(byte_error_size == byte_error_idx);
  fwrite(byte_error_data, sizeof(char), byte_error_size, byte_error_data_f);
  fclose(byte_error_data_f);

  return 0;
}
