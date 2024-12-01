#ifndef _HAMMING_H_
#define _HAMMING_H_

/*
 * AUTHOR: Alexandru-Dragos BUCUR
 *
 * Hamming code was chosen because it is easy to implement and is good enough
 * for the expected error rates.
 *
 * The *maximum* error rate you can set on the RXSM module is 2^-4, so 1 bit
 * error every 2 bytes. So a hamming code of at least (15,11) should be chosen
 * to cover these errors. A (15,11) Hamming Code encodes 11 bits of useful data
 * in 15 bits which will be transmitted, being able to fix 1 bit error within
 * the transmitted 15 bits.
 *
 * I chose (8,4) because it would be more reliable, and because we can
 * comfortably afford the 2x overhead. (8,4) Hamming codes can fix 1 bit error
 * every byte, for every 4 bits of useful data 8 bits are transmitted.
 *
 * NOTE: Why (8,4) instead of (7,4)?
 *
 * A (8,4) has more robust error *detection* compared to (7,4). However, both
 * (8,4) and (7,4) can reliably correct only 1 bit error, so using an additional
 * bit does not really improve anything for our case, because we will always
 * attempt to correct the message.
 *
 * The reason we use (8,4) is to handle *byte drops*. If we were to use (7,4)
 * the following could happen (note that the actual encoding of the parity bits
 * is not correct, and only for demonstration purposes):
 *
 * (Transmitted bits on top, amount of encoded useful data on bottom)
 *
 * 00000001 11111100 00000111 1111
 * ------||------||------||------|
 * 4 bits  4 bits  4 bits  4 bits
 * |-------------||--------------|
 *   1 data byte    1 data byte
 *
 * In the above example, if the RXSM module were to drop the 2nd byte
 * (11111100), we would lose at least 2 bytes of data (potentially more
 * depending on how the decoder is implemented), since the 2nd transmitted byte
 * contains decoding information for both the 1st and 2nd data byte. is a lot
 * less tedious. Theoretically, this could also corrupt 2 whole packets, since
 * the affected 2 bytes could be the final and first byte of 2 different
 * packets.
 *
 * *Most importantly*, this would have an cascading effect, where all of the
 * following encoded bits would be shifted, so the decoder would need to be
 * implemented as a sliding window over the received *bits*, in order to
 * eventually correct itself and not corrupt future correct packets.
 *
 * In comparison, a (8,4) code would do the following:
 *
 * 00000000 11111111 00000000 11111111
 * |------| |------| |------| |------|
 *  4 bits   4 bits   4 bits   4 bits
 * |---------------| |---------------|
 *    1 data byte       1 data byte
 *
 * Now, if we were to lose any transmitted byte, we would lose information about
 * a single data byte, which would corrupt only that specific packet and would
 * not cause a shift in the bits of the transmitted code. In this scenario, we
 * can implement the decoder as a sliding window over the received *bytes* while
 * still being able to skip over corrupt packets without influencing future
 * packets.
 *
 * This makes everything a lot less tedious and less error prone, so I decided
 * that the additional 1 bit is worth it.
 */

#include <assert.h>
#include <stdint.h>

typedef struct {
    uint8_t *buf;
    uint32_t used;
    uint32_t allocated;
} buffer_hamming;

#define GET_BIT(byte, bit) (((byte) & (1 << (bit))) >> (bit))

uint8_t get_1_count(uint8_t byte) {
    uint8_t count = 0;

    while (byte) {
        count += byte & 1;
        byte >>= 1;
    }

    return count;
}

void encode_4_bits(buffer_hamming *buf_out, uint8_t data) {
    assert(data <= 0b1111);

    char bits[4];
    for (int i = 3; i >= 0; i--) {
        bits[3 - i] = GET_BIT(data, i);
    }

    const char positions[4] = {0b111, 0b110, 0b101, 0b011};

    char parity = 0;
    for (int j = 0; j < 4; j++) {
        if (bits[j]) {
            parity ^= positions[j];
        }
    }

    char parity_bits[4] = {0};
    for (int j = 1; j < 4; j++) {
        parity_bits[j] = GET_BIT(parity, j - 1);
    }

    // encoding: a b c p3 d p2 p1 p0
    // p0 is global parity bit
    char encoded_byte = (bits[0] << 7) | (bits[1] << 6) | (bits[2] << 5) |
                        (parity_bits[3] << 4) | (bits[3] << 3) |
                        (parity_bits[2] << 2) | (parity_bits[1] << 1);
    encoded_byte |= get_1_count(encoded_byte) % 2;

    buf_out->buf[buf_out->used++] = encoded_byte;
}

void encode_hamming(const buffer_hamming *buf_in, buffer_hamming *buf_out) {
    // buf_out already needs to have enough allocated memory
    assert(buf_in->used <= buf_out->allocated / 2);

    buf_out->used = 0;
    for (uint32_t i = 0; i < buf_in->used; i++) {
        char byte = buf_in->buf[i];

        char first = (byte & 0xF0) >> 4;
        char second = (byte & 0x0F);

        encode_4_bits(buf_out, first);
        encode_4_bits(buf_out, second);
    }
}

#endif
