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
    //
    // TEST: encode_4_bits
    //
    printf("Testing encode_4_bits...");
    uint8_t data_encoded[SIZE];
    buffer_hamming buf_encoded = {
        .buf = data_encoded,
        .used = 0,
        .allocated = SIZE,
    };

    for (int i = 0; i < 16; i++) {
        encode_4_bits(&buf_encoded, i);
        assert(buf_encoded.buf[buf_encoded.used - 1] == mappings[i]);
    }

    for (int i = 0; i < 16; i++) {
        encode_4_bits(&buf_encoded, i);
        assert(buf_encoded.buf[buf_encoded.used - 1] == mappings[i]);
    }
    printf("OK\n");

    //
    // TEST: encode_hamming
    //
    printf("Testing encode_hamming...");
    uint8_t data_plain[SIZE];
    buffer_hamming buf_plain = {
        .buf = data_plain,
        .used = 0,
        .allocated = SIZE,
    };

    buf_plain.buf[0] = 4 << 4 | 3;
    buf_plain.buf[1] = 2 << 4 | 11;
    buf_plain.buf[2] = 7 << 4 | 15;
    buf_plain.buf[3] = 9 << 4 | 12;
    buf_plain.buf[4] = 6 << 4 | 7;
    buf_plain.buf[5] = 9 << 4 | 10;
    buf_plain.used = 6;

    encode_hamming(&buf_plain, &buf_encoded);
    assert(buf_encoded.used == 12);
    assert(buf_encoded.buf[0] == mappings[4]);
    assert(buf_encoded.buf[1] == mappings[3]);
    assert(buf_encoded.buf[2] == mappings[2]);
    assert(buf_encoded.buf[3] == mappings[11]);
    assert(buf_encoded.buf[4] == mappings[7]);
    assert(buf_encoded.buf[5] == mappings[15]);
    assert(buf_encoded.buf[6] == mappings[9]);
    assert(buf_encoded.buf[7] == mappings[12]);
    assert(buf_encoded.buf[8] == mappings[6]);
    assert(buf_encoded.buf[9] == mappings[7]);
    assert(buf_encoded.buf[10] == mappings[9]);
    assert(buf_encoded.buf[11] == mappings[10]);
    printf("OK\n");

    //
    // TEST: decode_hamming
    //
    printf("Testing decode_hamming...");
    uint8_t data_decode[SIZE];
    buffer_hamming buf_decode = {
        .buf = data_decode,
        .used = 0,
        .allocated = SIZE,
    };

    decode_hamming(&buf_encoded, &buf_decode);
    assert(buf_plain.used == buf_decode.used);

    for (int i = 0; i < buf_plain.used; i++) {
        assert(buf_plain.buf[i] == buf_decode.buf[i]);

        if (buf_plain.buf[i] != buf_decode.buf[i]) {
            printf("WRONG DECODE: %d\n", i);
        }
    }
    printf("OK\n");

    //
    // TEST: decode_hamming with bit flips
    //
    printf("Testing decode_hamming with flips...");
    for (int i = 0; i < buf_encoded.used; i++) {
        uint8_t flip = i % 8;
        buf_encoded.buf[i] ^= 1 << flip;
    }

    decode_hamming(&buf_encoded, &buf_decode);
    assert(buf_plain.used == buf_decode.used);

    for (int i = 0; i < buf_plain.used; i++) {
        assert(buf_plain.buf[i] == buf_decode.buf[i]);

        if (buf_plain.buf[i] != buf_decode.buf[i]) {
            printf("WRONG DECODE: %d\n", i);
        }
    }
    printf("OK\n");

    //
    // TEST: decode_hamming with more bit flips
    //
    printf("Testing decode_hamming with more flips...");
    for (int i = 0; i < buf_encoded.used; i++) {
        // flip other bits besides the bits flipped in the previous test
        uint8_t flip_1 = (i + 3) % 8;
        uint8_t flip_2 = (i + 5) % 8;
        buf_encoded.buf[i] ^= (1 << flip_1) | (1 << flip_2);
    }

    decode_hamming(&buf_encoded, &buf_decode);
    assert(buf_plain.used == buf_decode.used);

    int errors = 0;
    for (int i = 0; i < buf_plain.used; i++) {
        errors += buf_plain.buf[i] != buf_decode.buf[i];
    }
    assert(errors > 0);

    printf("CORRECTLY ERRORED\n");

    return 0;
}
