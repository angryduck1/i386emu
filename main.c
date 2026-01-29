#include <stdio.h>
#include <stdbool.h>
#include "types.h"

#define MEMORY_REALMODE_SIZE 1024 * 1024

__uint8_t memory[MEMORY_REALMODE_SIZE];

__uint32_t translate_address(cpu_state_t *cpu, __uint16_t segment, __uint32_t offset) {
    switch (cpu->mode) {
        case REAL_MODE:
            return ((__uint32_t)(segment << 4) + offset) & 0xFFFFF;
        case PROTECTED_MODE:
            fprintf(stderr, "PROTECTED_MODE not implemented\n");
            abort();
        default:
            abort();
    }
}

__uint8_t read_byte(cpu_state_t *cpu, __uint16_t segment, __uint32_t offset) {
    __uint32_t phys = translate_address(cpu, segment, offset);
    return memory[(phys) & 0xFFFFF];
}

__uint16_t read_word(cpu_state_t *cpu, __uint16_t segment, __uint32_t offset) {
    __uint32_t phys = translate_address(cpu, segment, offset);
    return memory[(phys) & 0xFFFFF] | (memory[(phys+1) & 0xFFFFF] << 8);
}

__uint32_t read_double_word(cpu_state_t *cpu, __uint16_t segment, __uint32_t offset) {
    __uint32_t phys = translate_address(cpu, segment, offset);
    return memory[(phys) & 0xFFFFF] | (memory[(phys+1) & 0xFFFFF] << 8) | (memory[(phys+2) & 0xFFFFF] << 16) | (memory[(phys+3) & 0xFFFFF] << 24);
}

void write_byte(cpu_state_t *cpu, __uint16_t segment, __uint32_t offset, __uint8_t value) {
    __uint32_t phys = translate_address(cpu, segment, offset);
    memory[(phys) & 0xFFFFF] = value;
}

void write_word(cpu_state_t *cpu, __uint16_t segment, __uint32_t offset, __uint16_t value) {
    __uint32_t phys = translate_address(cpu, segment, offset);
    memory[(phys) & 0xFFFFF] = value & 0xFF;
    memory[(phys+1) & 0xFFFFF] = (value) >> 8 & 0xFF;
}

void write_double_word(cpu_state_t *cpu, __uint16_t segment, __uint32_t offset, __uint32_t value) {
    __uint32_t phys = translate_address(cpu, segment, offset);
    memory[(phys) & 0xFFFFF] = value & 0xFF;
    memory[(phys+1) & 0xFFFFF] = (value >> 8) & 0xFF;
    memory[(phys+2) & 0xFFFFF] = (value >> 16) & 0xFF;
    memory[(phys+3) & 0xFFFFF] = (value >> 24) & 0xFF;
}

__uint8_t fetch_instruction_rmode(cpu_state_t *cpu, __uint8_t *memory) {
    __uint8_t byte;
    bool prefix_active = true;

    cpu->prefix.x66_mode = false;
    cpu->prefix.x67_mode = false;

    while (prefix_active) {
        byte = read_byte(cpu, cpu->seg.cs, cpu->eip++);
        switch (byte) {
            case 0x66:
                cpu->prefix.x66_mode = true;
                break;
            case 0x67:
                cpu->prefix.x67_mode = true;
                break;
            default:
                prefix_active = false;
                break;
        }
    }

    return byte;
}

void execute_instruction(cpu_state_t *cpu, __uint8_t *memory) {
    
}

int main() {
    FILE *f = fopen("bootloader.bin", "rb");
    if (!f) { perror("Connot open file"); return 1; };

    fread(&memory[0x7C00], 1, 512, f);
    fclose(f);

    cpu_state_t cpu;

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
}
