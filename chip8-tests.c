#include <assert.h>

#include "chip8.c"

static void
test_ret() {
    reset_ram();
    ram.stack[0] = 0x300;
    ram.SP = (char*)&ram.stack[1] - (char*)ram.bytes;
    execute_instruction(0x00ee);
    assert(ram.PC == 0x302);
    assert(ram.SP == (char*)&ram.stack[0] - (char*)ram.bytes);
}

static void
test_jp0() {
    reset_ram();
    execute_instruction(0x1300);
    assert(ram.PC == 0x300);
}

static void
test_call() {
    reset_ram();
    execute_instruction(0x2300);
    assert(ram.PC == 0x300);
    assert(ram.stack[0] == 0x200);
    assert(ram.SP == (char*)&ram.stack[1] - (char*)ram.bytes);
}

static void
test_se0() {
    reset_ram();
    execute_instruction(0x3000);
    assert(ram.PC == 0x204);
    execute_instruction(0x3001);
    assert(ram.PC == 0x206);
    execute_instruction(0x3100);
    assert(ram.PC == 0x20a);
    execute_instruction(0x3101);
    assert(ram.PC == 0x20c);
}

static void
test_sne0() {
    reset_ram();
    execute_instruction(0x4001);
    assert(ram.PC == 0x204);
    execute_instruction(0x4000);
    assert(ram.PC == 0x206);
    execute_instruction(0x4101);
    assert(ram.PC == 0x20a);
    execute_instruction(0x4100);
    assert(ram.PC == 0x20c);
}

static void
test_se1() {
    reset_ram();
    execute_instruction(0x5010);
    assert(ram.PC == 0x204);
    ram.V[0] = 1;
    execute_instruction(0x3010);
    assert(ram.PC == 0x206);
}

static void
test_ld0() {
    reset_ram();
    execute_instruction(0x6001);
    assert(ram.V[0] == 1);
}

static void
test_add0() {
    reset_ram();
    execute_instruction(0x7001);
    assert(ram.V[0] == 1);
}

static void
test_arithm() {
    reset_ram();
    ram.V[1] = 42;
    execute_instruction(0x8010);
    assert(ram.V[0] == 42);
    ram.V[0] = 1;
    ram.V[1] = 2;
    execute_instruction(0x8011);
    assert(ram.V[0] == 3);
    execute_instruction(0x8012);
    assert(ram.V[0] == 2);
    execute_instruction(0x8013);
    assert(ram.V[0] == 0);
    execute_instruction(0x8014);
    assert(ram.V[0] == 2);
    execute_instruction(0x8015);
    assert(ram.V[0] == 0);
    ram.V[0] = 2;
    execute_instruction(0x8016);
    assert(ram.V[0] == 1);
    ram.V[1] = 3;
    ram.V[0] = 2;
    execute_instruction(0x8017);
    assert(ram.V[0] == 1);
    assert(ram.V[0xf] == 1);
    ram.V[1] = 2;
    ram.V[0] = 3;
    execute_instruction(0x8017);
    assert(ram.V[0] == 0xff);
    assert(ram.V[0xf] == 0);
    ram.V[0] = 1;
    execute_instruction(0x801e);
    assert(ram.V[0] == 2);
    ram.V[0] = 255;
    ram.V[1] = 1;
    ram.V[0xf] = 0;
    execute_instruction(0x8014);
    assert(ram.V[0] == 0);
    assert(ram.V[0xf] == 1);
    ram.V[0] = 100;
    ram.V[1] = 101;
    ram.V[0xf] = 0;
    execute_instruction(0x8015);
    assert(ram.V[0] == 255);
    assert(ram.V[0xf] == 0);
    ram.V[0] = 100;
    ram.V[1] = 101;
    ram.V[0xf] = 0;
}

static void
test_sne1() {
    reset_ram();
    execute_instruction(0x9010);
    assert(ram.PC == 0x202);
    ram.V[0] = 1;
    execute_instruction(0x9010);
    assert(ram.PC == 0x206);
}

static void
test_ld1() {
    reset_ram();
    execute_instruction(0xa123);
    assert(ram.I == 0x123);
}

static void
test_jp1() {
    reset_ram();
    ram.V[0] = 0x42;
    execute_instruction(0xb311);
    assert(ram.PC == 0x353);
}

static void
test_drw() {
    reset_ram();
    ram.V[0] = 0;
    ram.V[1] = 0;
    ram.bytes[0x200] = 170;
    ram.bytes[0x201] = 85;
    ram.bytes[0x202] = 170;
    ram.bytes[0x203] = 85;
    ram.bytes[0x204] = 170;
    ram.I = 0x200;
    execute_instruction(0xd015);
    assert(ram.display[0][0] == 85);
    assert(ram.display[1][0] == 170);
    assert(ram.display[2][0] == 85);
    assert(ram.display[3][0] == 170);
    assert(ram.display[4][0] == 85);

    clear_display();
    memset(&ram.bytes[0x200], 0xff, 8);
    execute_instruction(0xd018);
    assert(ram.display[0][0] == 0xff);
    assert(ram.display[1][0] == 0xff);
    assert(ram.display[2][0] == 0xff);
    assert(ram.display[3][0] == 0xff);
    assert(ram.display[4][0] == 0xff);
    assert(ram.display[5][0] == 0xff);
    assert(ram.display[6][0] == 0xff);
    assert(ram.display[7][0] == 0xff);
    execute_instruction(0xd018);
    assert(ram.display[0][0] == 0);
    assert(ram.display[1][0] == 0);
    assert(ram.display[2][0] == 0);
    assert(ram.display[3][0] == 0);
    assert(ram.display[4][0] == 0);
    assert(ram.display[5][0] == 0);
    assert(ram.display[6][0] == 0);
    assert(ram.display[7][0] == 0);
}

static void
test_skp() {
    reset_ram();
    ram.V[0] = 0;
    execute_instruction(0xe09e);
    assert(ram.PC == 0x202);
    set_key_state(0, 1);
    execute_instruction(0xe09e);
    assert(ram.PC == 0x206);
}

static void
test_sknp() {
    reset_ram();
    ram.V[0] = 0;
    execute_instruction(0xe0a1);
    assert(ram.PC == 0x204);
    set_key_state(0, 1);
    execute_instruction(0xe0a1);
    assert(ram.PC == 0x206);
}

static void
test_ld2() {
    reset_ram();
    ram.DT = 42;
    execute_instruction(0xf007);
    assert(ram.V[0] == 42);
    execute_instruction(0xf00a);
    assert(ram.waiting);
    assert(ram.store_key == 0);
    set_key_state(3, 1);
    assert(!ram.waiting);
    assert(ram.V[0] == 3);
    ram.V[0] = 43;
    execute_instruction(0xf015);
    assert(ram.DT == 43);
    execute_instruction(0xf018);
    assert(ram.ST == 43);
}

static void
test_add1() {
    reset_ram();
    ram.I = 0x300;
    ram.V[1] = 0x42;
    execute_instruction(0xf11e);
    assert(ram.I == 0x342);
}

static void
test_ld3() {
    reset_ram();
    ram.V[0] = 5;
    execute_instruction(0xf029);
    assert(ram.I == (char*)&ram.sprites[5] - (char*)&ram);
}

int main() {
    test_ret();
    test_jp0();
    test_call();
    test_se0();
    test_sne0();
    test_se1();
    test_ld0();
    test_add0();
    test_arithm();
    test_sne1();
    test_ld1();
    test_jp1();
    test_drw();
    test_skp();
    test_sknp();
    test_ld2();
    test_add1();
    test_ld3();
}

