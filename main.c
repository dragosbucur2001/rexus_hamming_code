#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "./hamming.h"

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
const uint32_t CHECKSUM_SIZE = 1;

const uint32_t PACKAGE_SIZE = HEADER_SEGMENT_SIZE + DATA_SEGMENT_SIZE + CHECKSUM_SIZE;
const uint32_t PACKAGE_COUNT = 2000;
const uint32_t DATA_SIZE = PACKAGE_SIZE * PACKAGE_COUNT;
const uint32_t DATA_SIZE_ENCODED = DATA_SIZE * 2;

//
// ==== Error Parameters ====
//

/*const float BIT_ERROR_P = 1 / 16.0;*/

// 1/512 represents the first threshold you might see actual corrupt data
const float BIT_ERROR_P = 1.0 / (1 << 9);
const float BYTE_DROP_P = 0.01;

int main() {
    ran_ctx generator;
    ran_init(&generator, SEED);

    uint8_t data[DATA_SIZE];
    uint32_t data_idx = 0;

    for (uint32_t i = 0; i < PACKAGE_COUNT; i++) {
        for (uint32_t j = 0; j < HEADER_SEGMENT_SIZE; j++) {
            data[data_idx] = HEADER[j];
            data_idx++;
        }

        uint8_t checksum = 0;
        for (uint32_t j = 0; j < DATA_SEGMENT_SIZE; j++) {
            data[data_idx] = ran_val(&generator);
            checksum ^= data[data_idx];
            data_idx++;
        }

        data[data_idx] = checksum;
        data_idx++;
    }
    assert(DATA_SIZE == data_idx);

    buffer_hamming data_original = {
        .buf = data,
        .used = data_idx,
        .allocated = DATA_SIZE,
    };

    uint8_t encoded_buffer[DATA_SIZE_ENCODED];
    buffer_hamming data_encoded = {
        .buf = encoded_buffer,
        .used = 0,
        .allocated = DATA_SIZE_ENCODED,
    };
    encode_hamming(&data_original, &data_encoded);

    uint8_t bit_error_data[DATA_SIZE_ENCODED];
    uint8_t packet_error_counter = 0;
    for (uint32_t i = 0; i < data_encoded.used; i++) {
        bit_error_data[i] = data_encoded.buf[i];

        uint8_t byte_error_counter = 0;
        for (int j = 7; j > 0; j--) {
            if (roll_dice(&generator, BIT_ERROR_P)) {
                byte_error_counter++;
                bit_error_data[i] ^= 1 << j;
            }
        }

        if (byte_error_counter > 1) {
            packet_error_counter++;
        }
        if (i % 30 == 0) {
            printf("Unrecoverable errors within packet: %d\n", packet_error_counter);
            packet_error_counter = 0;
        }

        printf("Errors within byte: %d\n", byte_error_counter);
    }

    FILE *clean_data_f = fopen("clean.bin", "wb");
    fwrite(data_original.buf, sizeof(uint8_t), DATA_SIZE, clean_data_f);
    fclose(clean_data_f);

    FILE *encoded_data_f = fopen("encoded.bin", "wb");
    fwrite(data_encoded.buf, sizeof(uint8_t), data_encoded.used, encoded_data_f);
    fclose(encoded_data_f);

    FILE *bit_error_data_f = fopen("bit_error.bin", "wb");
    fwrite(bit_error_data, sizeof(uint8_t), data_encoded.used, bit_error_data_f);
    fclose(bit_error_data_f);

    /*uint8_t byte_error_data[DATA_SIZE];*/
    /*uint32_t byte_error_size = DATA_SIZE;*/
    /*uint32_t byte_error_idx = 0;*/
    /*for (uint32_t i = 0; i < DATA_SIZE; i++) {*/
    /*    if (roll_dice(&generator, BYTE_DROP_P)) {*/
    /*        byte_error_size--;*/
    /*        continue;*/
    /*    }*/
    /**/
    /*    byte_error_data[byte_error_idx++] = data[i];*/
    /*}*/

    /*FILE *byte_error_data_f = fopen("byte_drop.bin", "wb");*/
    /*assert(byte_error_size == byte_error_idx);*/
    /*fwrite(byte_error_data, sizeof(uint8_t), byte_error_size, byte_error_data_f);*/
    /*fclose(byte_error_data_f);*/

    return 0;
}
