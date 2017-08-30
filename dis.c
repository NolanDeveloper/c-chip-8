#include <stdio.h>

#include "chip8.h"

int main(int argc, char * argv[]) {
    if (argc < 2) {
        printf("Filename must be provided.\n");
        return -1;
    }
    if (disassemble(argv[1])) {
        printf("Disassembling failed.\n");
        return -1;
    }
}
