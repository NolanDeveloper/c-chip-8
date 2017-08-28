#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "chip8.h"

static const unsigned char GLYPHS[16][5] = {
    { 0xF0, 0x90, 0x90, 0x90, 0xF0 }, { 0x20, 0x60, 0x20, 0x20, 0x70 },
    { 0xF0, 0x10, 0xF0, 0x80, 0xF0 }, { 0xF0, 0x10, 0xF0, 0x10, 0xF0 },
    { 0x90, 0x90, 0xF0, 0x10, 0x10 }, { 0xF0, 0x80, 0xF0, 0x10, 0xF0 },
    { 0xF0, 0x80, 0xF0, 0x90, 0xF0 }, { 0xF0, 0x10, 0x20, 0x40, 0x40 },
    { 0xF0, 0x90, 0xF0, 0x90, 0xF0 }, { 0xF0, 0x90, 0xF0, 0x10, 0xF0 },
    { 0xF0, 0x90, 0xF0, 0x90, 0x90 }, { 0xE0, 0x90, 0xE0, 0x90, 0xE0 },
    { 0xF0, 0x80, 0x80, 0x80, 0xF0 }, { 0xE0, 0x90, 0x90, 0x90, 0xE0 },
    { 0xF0, 0x80, 0xF0, 0x80, 0xF0 }, { 0xF0, 0x80, 0xF0, 0x80, 0x80 }
};

static union {
    unsigned char bytes[4096];
    struct {
        unsigned char V[16];
        unsigned short I;
        unsigned char DT;
        unsigned char ST;
        unsigned short PC;
        unsigned char SP;
        unsigned short stack[16];
        unsigned char sprites[16][5];
        unsigned char display[DISPLAY_HEIGHT][DISPLAY_WIDTH / 8];
        unsigned short key_state;
        unsigned char waiting;
        unsigned char store_key;
    };
} ram;

extern int
get_pixel(int row, int col) {
    return (ram.display[row][col / 8] >> (col % 8u)) & 0x1u;
}

extern void
reset_ram() {
    memset(&ram, 0, sizeof(ram));
    ram.PC = 0x200;
    ram.SP = (char *)&ram.stack - (char*)&ram;
    memcpy(&ram.sprites, &GLYPHS, sizeof(GLYPHS));
}

static void
clear_display() {
    memset(ram.display, 0, sizeof(ram.display));
}

static void
return_from_subroutine() {
    ram.SP -= 2;
    ram.PC = (ram.bytes[ram.SP] | (ram.bytes[ram.SP + 1] << 8));
}

static void
call(unsigned nnn) {
    ram.bytes[ram.SP] = ram.PC & 0xff;
    ram.bytes[ram.SP + 1] = (ram.PC & 0xff00) >> 8;
    ram.SP += 2;
    ram.PC = nnn - 2;
}

static void
skip_next_instruction() {
    ram.PC += 2;
}

static void
display_sprite(unsigned vx, unsigned vy, unsigned nibble) {
    ram.V[0xf] = 0;
    unsigned x = ram.V[vx] %= DISPLAY_WIDTH;
    unsigned y = ram.V[vy] %= DISPLAY_HEIGHT;
    unsigned hmax = DISPLAY_HEIGHT < y + nibble ? DISPLAY_HEIGHT - y : nibble;
    for (unsigned h = 0; h < hmax; ++h) {
        unsigned char byte = ram.bytes[ram.I + h];
        unsigned yy = y + h;
        unsigned vmax = DISPLAY_WIDTH < x + 8 ? DISPLAY_WIDTH - x : 8;
        for (unsigned v = 0; v < vmax; ++v) {
            unsigned xx = x + v;
            if (!((byte >> (7 - v)) & 0x1u)) continue;
            if ((ram.display[yy][xx / 8] >> (xx % 8)) & 0x1u) {
                ram.V[0xf] = 1;
            }
            ram.display[yy][xx / 8] ^= 1u << (xx % 8);
        }
    }
}

static int
is_key_pressed(unsigned x) {
    if (ram.V[x] > 0xf) return 0;
    return (ram.key_state >> ram.V[x]) & 0x1;
}

static void
wait_for_key_press(unsigned x) {
    ram.waiting = 1;
    ram.store_key = (unsigned char)x;
}

static void
store_bcd(unsigned x) {
    unsigned char vx = ram.V[x];
    ram.bytes[ram.I] = vx / 100u;
    ram.bytes[ram.I + 1] = (vx / 10u) % 10u;
    ram.bytes[ram.I + 2] = vx % 10u;
}

static void
store_registers(unsigned x) {
    memcpy(ram.bytes + ram.I, ram.V, x + 1);
}

static void
read_registers(unsigned x) {
    memcpy(ram.V, ram.bytes + ram.I, x + 1);
}

static void
show_instruction(unsigned short instruction) {
    unsigned nnn, addr;
    unsigned n, nibble;
    unsigned x, y;
    unsigned kk, byte;
    nnn = addr = instruction & 0xfff;
    n = nibble = instruction & 0xf;
    x = (instruction >> 8) & 0xf;
    y = (instruction >> 4) & 0xf;
    kk = byte = instruction & 0xff;
    switch ((instruction >> 12) & 0xf) {
    case 0x0:
        switch (nnn) {
        case 0x0e0: printf("CLS"); break;
        case 0x0ee: printf("RET"); break;
        default: printf("unknown instruction");
        }
        break;
    case 0x1: printf("JP 0x%03x", nnn); break;
    case 0x2: printf("CALL 0x%03x", nnn); break;
    case 0x3: printf("SE V%X, %d(0x%02x)", x, byte, byte); break;
    case 0x4: printf("SNE V%X, %d(0x%02x)", x, byte, byte); break;
    case 0x5: printf("SE V%X, V%X", x, y); break;
    case 0x6: printf("LD V%X, %d(0x%02x)", x, byte, byte); break;
    case 0x7: printf("ADD V%X, %d(0x%02x)", x, byte, byte); break;
    case 0x8:
        switch (n) {
        case 0x0: printf("LD V%X, V%X", x, y); break;
        case 0x1: printf("OR V%X, V%X", x, y); break;
        case 0x2: printf("AND V%X, V%X", x, y); break;
        case 0x3: printf("XOR V%X, V%X", x, y); break;
        case 0x4: printf("ADD V%X, V%X", x, y); break;
        case 0x5: printf("SUB V%X, V%X", x, y); break;
        case 0x6: printf("SHR V%X", x); break;
        case 0x7: printf("SUBN V%X, V%X", x, y); break;
        case 0xe: printf("SHL V%X", x); break;
        default: printf("unknown instruction");
        }
        break;
    case 0x9: printf("SNE V%X, V%X", x, y); break;
    case 0xa: printf("LD I, 0x%03x", addr); break;
    case 0xb: printf("JP V0, 0x%03x", addr); break;
    case 0xc: printf("RND V%X, %d(%02x)", x, byte, byte); break;
    case 0xd: printf("DRW V%X, V%X, %d", x, y, nibble); break;
    case 0xe:
        switch (kk) {
        case 0x9e: printf("SKP V%X", x); break;
        case 0xa1: printf("SKNP V%X", x); break;
        default: printf("unknown instruction");
        }
        break;
    case 0xf:
        switch (kk) {
        case 0x07: printf("LD V%X, DT", x); break;
        case 0x0a: printf("LD V%X, K", x); break;
        case 0x15: printf("LD DT, V%X", x); break;
        case 0x18: printf("LD ST, V%X", x); break;
        case 0x1e: printf("ADD I, V%X", x); break;
        case 0x29: printf("LD F, V%X", x); break;
        case 0x33: printf("LD B, V%X", x); break;
        case 0x55: printf("LD [I], V%X", x); break;
        case 0x65: printf("LD V%X, [I]", x); break;
        default: printf("unknown instruction");
        }
    }
}

static void
execute_instruction(unsigned short instruction) {
    unsigned nnn, addr;
    unsigned n, nibble;
    unsigned x, y;
    unsigned kk, byte;
    nnn = addr = instruction & 0xfff;
    n = nibble = instruction & 0xf;
    x = (instruction >> 8) & 0xf;
    y = (instruction >> 4) & 0xf;
    kk = byte = instruction & 0xff;
    switch ((instruction >> 12) & 0xf) {
    case 0x0:
        switch (nnn) {
        case 0x0e0: clear_display(); break;
        case 0x0ee: return_from_subroutine(); break;
        }
        break;
    case 0x1: ram.PC = nnn - 2; break;
    case 0x2: call(nnn); break;
    case 0x3: if (ram.V[x] == kk) skip_next_instruction(); break;
    case 0x4: if (ram.V[x] != kk) skip_next_instruction(); break;
    case 0x5: if (ram.V[x] == ram.V[y]) skip_next_instruction(); break;
    case 0x6: ram.V[x] = kk; break;
    case 0x7: ram.V[x] += kk; break;
    case 0x8:
        switch (n) {
        case 0x0: ram.V[x] = ram.V[y]; break;
        case 0x1: ram.V[x] |= ram.V[y]; break;
        case 0x2: ram.V[x] &= ram.V[y]; break;
        case 0x3: ram.V[x] ^= ram.V[y]; break;
        case 0x4: ram.V[x] += ram.V[y];
                  ram.V[0xf] = ram.V[x] < ram.V[y] ? 1 : 0;
                  break;
        case 0x5: ram.V[0xf] = ram.V[x] >= ram.V[y] ? 1 : 0;
                  ram.V[x] -= ram.V[y];
                  break;
        case 0x6: ram.V[0xf] = ram.V[x] & 0x1u;
                  ram.V[x] >>= 1u;
                  break;
        case 0x7: ram.V[0xf] = ram.V[y] >= ram.V[x] ? 1 : 0;
                  ram.V[x] = ram.V[y] - ram.V[x];
                  break;
        case 0xe: ram.V[0xf] = (ram.V[x] >> 0x7) & 0x1;
                  ram.V[x] <<= 1u;
                  break;
        }
        break;
    case 0x9: if (ram.V[x] != ram.V[y]) skip_next_instruction(); break;
    case 0xa: ram.I = nnn; break;
    case 0xb: ram.PC = ram.V[0] + nnn - 2; break;
    case 0xc: ram.V[x] = (unsigned) rand() & kk; break;
    case 0xd: display_sprite(x, y, nibble); break;
    case 0xe:
        switch (kk) {
        case 0x9e: if (is_key_pressed(x)) skip_next_instruction(); break;
        case 0xa1: if (!is_key_pressed(x)) skip_next_instruction(); break;
        }
        break;
    case 0xf:
        switch (kk) {
        case 0x07: ram.V[x] = ram.DT; break;
        case 0x0a: wait_for_key_press(x); break;
        case 0x15: ram.DT = ram.V[x]; break;
        case 0x18: ram.ST = ram.V[x]; break;
        case 0x1e: ram.I += ram.V[x]; break;
        case 0x29: ram.I = (char *)&ram.sprites[ram.V[x]] - (char *)&ram;
                   break;
        case 0x33: store_bcd(x); break;
        case 0x55: store_registers(x); break;
        case 0x65: read_registers(x); break;
        }
    }
    ram.PC += 2;
}

extern int
load_file(char * filename) {
    FILE * file = fopen(filename, "r");
    if (!file) {
        perror("Can't open file");
        return 1;
    }
    unsigned pos = 0x200;
    int c;
    while (EOF != (c = getc(file))) {
        ram.bytes[pos++] = (unsigned char) c;
    }
    fclose(file);
    return 0;
}

static void
show_word(unsigned short word) {
    static char * blocks[] = { " ", "▀", "▄", "█" };
    for (unsigned i = 8; i-- > 0;) {
        unsigned short upper_bit = (word >> (i + 8u)) & 1u;
        unsigned short lower_bit = ((word >> i) & 1u) << 1u;
        fputs(blocks[upper_bit | lower_bit], stdout);
    }
}

extern int
disassemble(char * filename) {
    FILE * file = fopen(filename, "r");
    if (!file) {
        perror("Can't open file");
        return 1;
    }
    unsigned pos = 0x200;
    unsigned instruction;
    unsigned char lo = 0, hi = 0;
    while (1) {
        if (!fread(&lo, 1, 1, file)) break;
        if (!fread(&hi, 1, 1, file)) {
            instruction = lo << 8u;
            putchar('|');
            show_word(instruction);
            printf("|\n");
            break;
        }
        instruction = (lo << 8u) | hi;
        putchar('|');
        show_word(instruction);
        printf("|\t%03x\t%04x\t", pos, instruction);
        show_instruction(instruction);
        putchar('\n');
        pos += 2;
    }
    fclose(file);
    return 0;
}

extern void
set_key_state(unsigned n, unsigned state) {
    unsigned short mask = 0xffff ^ (1u << n);
    ram.key_state = (ram.key_state & mask) | (state << n);
    if (state && ram.waiting) {
        ram.waiting = 0;
        ram.V[ram.store_key] = n;
    }
}

extern void
tick() {
    static int ticks;
    if (ram.waiting) return;
    execute_instruction(ram.bytes[ram.PC] << 8 | ram.bytes[ram.PC + 1]);
    if (30 != ++ticks) return;
    if (ram.DT > 0) --ram.DT;
    if (ram.ST > 0) --ram.ST;
    ticks = 0;
}
