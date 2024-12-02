from enum import Enum
from typing import Tuple


def get_bit(byte, bit):
    return (byte & (1 << bit)) >> bit


def decode_4_bits(byte):
    parity = 0
    for i in range(1, 8):
        if (get_bit(byte, i) == 1):
            parity ^= i

    byte ^= 1 << parity

    result = (get_bit(byte, 7) << 3) | (get_bit(byte, 6) << 2) | (get_bit(byte, 5) << 1) | get_bit(byte, 3)
    return result

class DecodeResult(Enum):
    NO_SYNC = 1
    WRONG_CHECKSUM = 2
    GOOD = 3

def is_valid_msg(buffer: bytearray) -> DecodeResult:
    assert(len(buffer) == 15)

    if (buffer[0] != buffer[1] or buffer[0] != 0b10101010):
        return DecodeResult.NO_SYNC

    checksum = 0
    for i in range(2, 14):
        checksum ^= buffer[i]

    if (checksum != buffer[14]):
        return DecodeResult.WRONG_CHECKSUM

    return DecodeResult.GOOD


def try_decode(buffer: bytearray) -> Tuple[DecodeResult, bytearray]:
    assert(len(buffer) == 30)

    msg = bytearray()

    for i in range(30):
        half = decode_4_bits(buffer[i])
        if i % 2 == 0:
            half <<= 4
            msg.append(half)
        else:
            msg[-1] |= half

    is_valid = is_valid_msg(msg)
    return (is_valid, msg)


if __name__ == "__main__":
    f_out = open("python_decode.bin", "wb")

    with open("./bit_error.bin", "rb") as f_in:
        buffer = bytearray()

        while (byte := f_in.read(1)):
            assert(len(byte) == 1)
            buffer.append(byte[0])

            if (len(buffer) != 30):
                continue

            (error, msg) = try_decode(buffer)
            if (error == DecodeResult.GOOD):
                print("Good Message")
                f_out.write(msg)
                buffer = bytearray()
            else:
                buffer.pop(0)
                if (error == DecodeResult.WRONG_CHECKSUM):
                    print("Found a corrupted message")
                else:
                    # this is expected to appear a bunch of times in short bursts (like ~30 at a time)
                    # this is because once 1 byte within a packet is corrupted which would cause either
                    # the sync check or the checksum to fail, we can expect the following attempts to decode
                    # a packet to also fail. TODO: write a better explanation
                    print("Not Sync")

