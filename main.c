#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <memory.h>
#include "headers/functions.h"
#include "headers/tools.h"

#define MEMORY_REALMODE_SIZE 1024 * 1024

void execute_instructions(cpu_state_t *cpu, __uint8_t *memory, Opcodes* opcodes) {
    while (true) {
        __uint8_t opcode = fetch_instruction_rmode(cpu, memory);

        if (opcode == 0x00) {
            break;
        }

        opcodes[opcode](cpu, opcode);
    }
}

int main() {
    FILE *f = fopen("test.bin", "rb");
    if (!f) { perror("Connot open file"); return 1; };

    cpu_state_t cpu;
    cpu.memory = (__uint8_t*)malloc(MEMORY_REALMODE_SIZE * sizeof(__uint8_t));
    if (!cpu.memory) {
        perror("Memory allocating failed");
        fclose(f);
        return -1;
    }
    
    memset(cpu.memory, 0, MEMORY_REALMODE_SIZE * sizeof(__uint8_t));

    size_t n = fread(&cpu.memory[0x00], 1, 512, f);
    fclose(f);

    cpu.gpr.eax.dword = 0;
    cpu.gpr.ebx.dword = 0;
    cpu.gpr.ecx.dword = 0;
    cpu.gpr.edx.dword = 0;
    cpu.gpr.esi.dword = 0;
    cpu.gpr.edi.dword = 0;
    cpu.gpr.ebp.dword = 0;
    cpu.gpr.esp.dword = 0x7C00;

    cpu.seg.cs.dword = 0x0000;
    cpu.seg.ds.dword = 0x0000;
    cpu.seg.es.dword = 0x0000;
    cpu.seg.ss.dword = 0xFFFF;
    cpu.seg.fs.dword = 0x0000;
    cpu.seg.gs.dword = 0x0000;

    cpu.eip.dword = 0x00;
    cpu.eflags.dword = 0x0002;

    cpu.mode = REAL_MODE;

    Opcodes opcodes[256] = {NULL};
    init_opcodes(opcodes);

    execute_instructions(&cpu, cpu.memory, opcodes);

    free(cpu.memory);
    cpu.memory = NULL;

    printf("EAX: %d \n EBX: %d \n ECX: %d \n EDX: %d \n ESI: %d \n EDI: %d \n EBP: %d \n ESP: %d \n", cpu.gpr.eax, cpu.gpr.ebx, cpu.gpr.ecx, cpu.gpr.edx, cpu.gpr.esi, cpu.gpr.edi, cpu.gpr.ebp, cpu.gpr.esp);

    return 0;
}