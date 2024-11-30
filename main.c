#include <stdint.h>

// small RNG: https://www.pcg-random.org/posts/bob-jenkins-small-prng-passes-practrand.html

typedef struct { uint32_t a, b, c, d; } ran_ctx ;

#define rot32(x,k) (((x)<<(k))|((x)>>(32-(k))))

uint32_t ran_val( ran_ctx *gen ) {
    uint32_t e = gen->a - rot32(gen->b, 27);
    gen->a = gen->b ^ rot32(gen->c, 17);
    gen->b = gen->c + gen->d;
    gen->c = gen->d + e;
    gen->d = e + gen->a;
    return gen->d;
}

void ran_init( ran_ctx *gen, uint32_t seed ) {
    gen->a = 0xf1ea5eed;
    gen->b = gen->c = gen->d = seed;

    for (uint32_t i=0; i<20; ++i) {
        (void)ran_val(gen);
    }
}

// end RNG

const uint32_t SEED = 0;

#define HEADER_SEGMENT_SIZE  2
const char HEADER[HEADER_SEGMENT_SIZE] = {0b10101010, 0b10101010};
const uint32_t DATA_SEGMENT_SIZE = 12;

const uint32_t PACKAGE_SIZE = HEADER_SEGMENT_SIZE + DATA_SEGMENT_SIZE;
const uint32_t PACKAGE_COUNT = 200;
const uint32_t DATA_SIZE = PACKAGE_SIZE * PACKAGE_COUNT;

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



    return 0;
}
