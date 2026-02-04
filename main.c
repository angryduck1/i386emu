#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "types.h"
#include <tools.h>

#define MEMORY_REALMODE_SIZE 1024 * 1024

void execute_instructions(cpu_state_t *cpu, __uint8_t *memory) {
    while (true) {
        __uint8_t opcode = fetch_instruction_rmode(cpu, memory);

        switch (opcode) {
            case 0x88: { // MOV r/m8,r8
                modrm_t m = decode_modrm(cpu);
                __uint8_t *src = get_reg8(cpu, m.reg);

                if (m.mod == 3) {
                    __uint8_t *dst = get_reg8(cpu, m.rm);
                    set_reg8(dst, src);
                } else {
                    __uint16_t out_segment;
                    __uint32_t ea = effective_address(cpu, m, &out_segment);

                    write_byte(cpu, out_segment, ea, *src);
                }

                break;
            }
            case 0x8A: { // MOV r8, r/m8
                modrm_t m = decode_modrm(cpu);
                __uint8_t *dst = get_reg8(cpu, m.reg);

                if (m.mod == 3) {
                    __uint8_t *src = get_reg8(cpu, m.rm);
                    set_reg8(dst, src);
                } else {
                    __uint16_t out_segment;
                    __uint32_t ea = effective_address(cpu, m, &out_segment);

                    *dst = read_byte(cpu, out_segment, ea);
                }

                break;
            }

            case 0x89: { // MOV r/m16, r16 || MOV r/m32, r32
                modrm_t m = decode_modrm(cpu);
                bool op32 = (cpu->mode != REAL_MODE && !cpu->prefix.x66_mode) || (cpu->mode == REAL_MODE && cpu->prefix.x66_mode);

                if (m.mod == 3) {
                    if (op32) {
                       __uint32_t *src = get_reg32(cpu, m.reg);
                       __uint32_t *dst = get_reg32(cpu, m.rm);
                       set_reg32(dst, src);
                    } else {
                       __uint16_t *src = get_reg16(cpu, m.reg);
                       __uint16_t *dst = get_reg16(cpu, m.rm);
                       set_reg16(dst, src);
                    }
                } else {
                    __uint16_t out_segment;
                    __uint32_t ea = effective_address(cpu, m, &out_segment);

                    if (op32) {
                        __uint32_t *src = get_reg32(cpu, m.reg);
                        write_double_word(cpu, out_segment, ea, *src);
                    } else {
                        __uint16_t *src = get_reg16(cpu, m.reg);
                        write_word(cpu, out_segment, ea, *src);
                    }
                }

                break;
            }
            default:
                break;
        }
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

    execute_instructions(&cpu, cpu.memory);

    free(cpu.memory);
    cpu.memory = NULL;

    return 0;
}