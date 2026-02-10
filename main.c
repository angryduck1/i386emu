#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "functions.h"
#include "tools.h"

#define MEMORY_REALMODE_SIZE 1024 * 1024

void execute_instructions(cpu_state_t *cpu, __uint8_t *memory, Opcodes* opcodes) {
    while (true) {
        __uint8_t opcode = fetch_instruction_rmode(cpu, memory);

        opcodes[opcode](cpu, opcode);
    }
}

int main() {
    FILE *f = fopen("bootloader.bin", "rb");
    if (!f) { perror("Connot open file"); return 1; };

    cpu_state_t cpu;
    cpu.memory = (__uint8_t*)malloc(MEMORY_REALMODE_SIZE * sizeof(__uint8_t));
    if (!cpu.memory) {
        perror("Memory allocating failed");
        fclose(f);
        return -1;
    }

    fread(&cpu.memory[0x7C00], 1, 512, f);
    fclose(f);

    cpu.gpr.eax = 0;
    cpu.gpr.ebx = 0;
    cpu.gpr.ecx = 0;
    cpu.gpr.edx = 0;
    cpu.gpr.esi = 0;
    cpu.gpr.edi = 0;
    cpu.gpr.ebp = 0;
    cpu.gpr.esp = 0x7C00;

    cpu.seg.cs = 0x0000;
    cpu.seg.ds = 0x0000;
    cpu.seg.es = 0x0000;
    cpu.seg.ss = 0x0000;
    cpu.seg.fs = 0x0000;
    cpu.seg.gs = 0x0000;

    cpu.eip = 0x7C00;
    cpu.eflags = 0x0002;

    cpu.mode = REAL_MODE;

    Opcodes opcodes[256] = {NULL};
    init_opcodes(opcodes);

    execute_instructions(&cpu, cpu.memory, opcodes);

    free(cpu.memory);
    cpu.memory = NULL;

    return 0;
}