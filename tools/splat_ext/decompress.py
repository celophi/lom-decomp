import sys

def decompress(src):
    dst = bytearray()
    src_ptr = 0
    while src_ptr < len(src):
        opcode = src[src_ptr]
        if opcode == 0xF0:
            param1 = src[src_ptr + 1]
            src_ptr += 2
            iterations = (param1 & 0xF) + 3
            param1 = param1 >> 4
            for _ in range(iterations):
                dst.append(param1)
        elif opcode == 0xF1:
            param1 = src[src_ptr + 2]
            next_byte = src[src_ptr + 1]
            src_ptr += 3
            iterations = next_byte + 4
            for _ in range(iterations):
                dst.append(param1)
        elif opcode == 0xF2:
            param1 = src[src_ptr + 2]
            next_byte = src[src_ptr + 1]
            src_ptr += 3
            iterations = next_byte + 2
            param2 = param1 >> 4
            param1 = param1 & 0xF
            for _ in range(iterations):
                dst.append(param1)
                dst.append(param2)
        elif opcode == 0xF3:
            param1 = src[src_ptr + 2]
            param0 = src[src_ptr + 3]
            next_byte = src[src_ptr + 1]
            src_ptr += 4
            iterations = next_byte + 2
            for _ in range(iterations):
                dst.append(param1)
                dst.append(param0)
        elif opcode == 0xF4:
            param1 = src[src_ptr + 2]
            param0 = src[src_ptr + 3]
            param3 = src[src_ptr + 4]
            next_byte = src[src_ptr + 1]
            src_ptr += 5
            iterations = next_byte + 2
            for _ in range(iterations):
                dst.append(param1)
                dst.append(param0)
                dst.append(param3)
        elif opcode == 0xF5:
            param1 = src[src_ptr + 2]
            next_byte = src[src_ptr + 1]
            src_ptr += 3
            iterations = next_byte + 4
            for _ in range(iterations):
                dst.append(param1)
                dst.append(src[src_ptr])
                src_ptr += 1
        elif opcode == 0xF6:
            param1 = src[src_ptr + 2]
            param0 = src[src_ptr + 3]
            next_byte = src[src_ptr + 1]
            src_ptr += 4
            iterations = next_byte + 3
            for _ in range(iterations):
                dst.append(param1)
                dst.append(param0)
                next_byte = src[src_ptr]
                src_ptr += 1
                dst.append(next_byte)
        elif opcode == 0xF7:
            param1 = src[src_ptr + 2]
            param0 = src[src_ptr + 3]
            param3 = src[src_ptr + 4]
            next_byte = src[src_ptr + 1]
            src_ptr += 5
            iterations = next_byte + 2
            for _ in range(iterations):
                dst.append(param1)
                dst.append(param0)
                dst.append(param3)
                dst.append(src[src_ptr])
                src_ptr += 1
        elif opcode == 0xF8:
            param1 = src[src_ptr + 2]
            next_byte = src[src_ptr + 1]
            src_ptr += 3
            iterations = next_byte + 4
            for _ in range(iterations):
                dst.append(param1)
                param1 = (param1 + 1) & 0xFF
        elif opcode == 0xF9:
            param1 = src[src_ptr + 2]
            next_byte = src[src_ptr + 1]
            src_ptr += 3
            iterations = next_byte + 4
            for _ in range(iterations):
                dst.append(param1)
                param1 = (param1 - 1) & 0xFF
        elif opcode == 0xFA:
            param1 = src[src_ptr + 2]
            param0 = src[src_ptr + 3]
            next_byte = src[src_ptr + 1]
            src_ptr += 4
            iterations = next_byte + 5
            for _ in range(iterations):
                dst.append(param1)
                param1 = (param1 + param0) & 0xFF
        elif opcode == 0xFB:
            param2 = src[src_ptr + 2]
            something = src[src_ptr + 3]
            next_byte = src[src_ptr + 1]
            param3 = src[src_ptr + 4]
            src_ptr += 5
            iterations = next_byte + 3
            s8 = param3 if param3 < 128 else param3 - 256
            for _ in range(iterations):
                dst.append(param2)
                dst.append(something)
                next_val = (((something << 8) | param2) + s8) & 0xFFFF
                param2 = next_val & 0xFF
                something = (next_val >> 8) & 0xFF
        elif opcode == 0xFC:
            param1 = src[src_ptr + 1]
            offset_low = src[src_ptr + 2]
            src_ptr += 3
            iterations = (offset_low >> 4) + 4
            offset = param1 | ((offset_low & 0xF) << 8)
            temp_ptr = len(dst) - offset
            for _ in range(iterations):
                dst.append(dst[temp_ptr - 1])
                temp_ptr += 1
        elif opcode == 0xFD:
            param1 = src[src_ptr + 1]
            next_byte = src[src_ptr + 2]
            src_ptr += 3
            iterations = next_byte + 0x14
            temp_ptr = len(dst) - param1
            for _ in range(iterations):
                dst.append(dst[temp_ptr - 1])
                temp_ptr += 1
        elif opcode == 0xFE:
            param1 = src[src_ptr + 1]
            src_ptr += 2
            iterations = (param1 & 0xF) + 3
            offset = (param1 & 0xF0) >> 1
            temp_ptr = len(dst) - offset
            for _ in range(iterations):
                offset_low = dst[temp_ptr - 8]
                dst.append(offset_low)
                temp_ptr += 1
        elif opcode == 0xFF:
            src_ptr += 1
            break
        else:
            src_ptr += 1
            iterations = opcode + 1
            for _ in range(iterations):
                dst.append(src[src_ptr])
                src_ptr += 1
    return dst

if __name__ == "__main__":
    if len(sys.argv) != 5:
        print("Usage: python script.py input_file offset length output_file")
        sys.exit(1)
    input_file = sys.argv[1]
    offset = int(sys.argv[2])
    length = int(sys.argv[3])
    output_file = sys.argv[4]
    with open(input_file, 'rb') as f:
        f.seek(offset)
        compressed = f.read(length)
    decompressed = decompress(compressed)
    with open(output_file, 'wb') as f:
        f.write(decompressed)