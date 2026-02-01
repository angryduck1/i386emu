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

__uint8_t* get_reg8(cpu_state_t *cpu, __uint8_t reg) {
    switch (reg) {
        case 0: return (__uint8_t*)&cpu->gpr.eax; // AL
        case 1: return (__uint8_t*)&cpu->gpr.ecx; // CL
        case 2: return (__uint8_t*)&cpu->gpr.edx; // DL
        case 3: return (__uint8_t*)&cpu->gpr.ebx; // BL
        case 4: return ((__uint8_t*)&cpu->gpr.eax) + 1; // AH
        case 5: return ((__uint8_t*)&cpu->gpr.ecx) + 1; // CH
        case 6: return ((__uint8_t*)&cpu->gpr.edx) + 1; // DH
        case 7: return ((__uint8_t*)&cpu->gpr.ebx) + 1; // BH
    }

    return NULL;
}

__uint16_t* get_reg16(cpu_state_t *cpu, __uint8_t reg) {
    switch (reg) {
        case 0: return (__uint16_t*)&cpu->gpr.eax; // AX
        case 1: return (__uint16_t*)&cpu->gpr.ecx; // CX
        case 2: return (__uint16_t*)&cpu->gpr.edx; // DX
        case 3: return (__uint16_t*)&cpu->gpr.ebx; // BX
        case 4: return (__uint16_t*)&cpu->gpr.esp; // SP
        case 5: return (__uint16_t*)&cpu->gpr.ebp; // BP
        case 6: return (__uint16_t*)&cpu->gpr.esi; // SI
        case 7: return (__uint16_t*)&cpu->gpr.edi; // DI
    }

    return NULL;
}

modrm_t decode_modrm(cpu_state_t* cpu) {
    __uint8_t byte = read_byte(cpu, cpu->seg.cs, cpu->eip++);
    modrm_t m;
    m.mod = (byte >> 6) & 0x03;
    m.reg = (byte >> 3) & 0x07;
    m.rm = byte & 0x07;

    return m;
}

__uint32_t effective_address(cpu_state_t *cpu, modrm_t m, __uint16_t* out_segment) {
    __int32_t base = 0;

    __uint32_t disp = (m.mod == 0x01) ? read_byte(cpu, cpu->seg.cs, cpu->eip++) : (m.mod == 0x02) ? read_word(cpu, cpu->seg.cs, cpu->eip += 2) : 0;

    switch (m.rm) {
        case 0: base = cpu->gpr.ebx + cpu->gpr.esi; *out_segment = cpu->seg.ds; break;
        case 1: base = cpu->gpr.ebx + cpu->gpr.edi; *out_segment = cpu->seg.ds; break;
        case 2: base = cpu->gpr.ebp + cpu->gpr.esi; *out_segment = cpu->seg.ss; break;
        case 3: base = cpu->gpr.ebp + cpu->gpr.edi; *out_segment = cpu->seg.ss; break;
        case 4: base = cpu->gpr.esi; *out_segment = cpu->seg.ds; break;
        case 5: base = cpu->gpr.edi; *out_segment = cpu->seg.ds; break;
        case 6: {
            if (m.mod == 0) {
                base = read_word(cpu, cpu->seg.cs, cpu->eip);
                cpu->eip += 2;
                *out_segment = cpu->seg.ds;
            } else {
                base = cpu->gpr.ebp;
                *out_segment = cpu->seg.ds;
            }
            break;
        }
        case 7: base = cpu->gpr.ebx; *out_segment = cpu->seg.ds; break;
    }

    return base + disp;
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

void execute_instructions(cpu_state_t *cpu, __uint8_t *memory) {
    while (true) {
        __uint8_t opcode = fetch_instruction_rmode(cpu, memory);

        switch (opcode) {
            case 0x88: { // MOV r/m8,r8
                modrm_t m = decode_modrm(cpu);
                __uint8_t *src = get_reg8(cpu, m.reg);

                if (m.mod == 3) {
                    __uint8_t *dst = get_reg8(cpu, m.rm);
                    *dst = *src;
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
                    *dst = *src;
                } else {
                    __uint16_t out_segment;
                    __uint32_t ea = effective_address(cpu, m, &out_segment);

                    *dst = read_byte(cpu, out_segment, ea);
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

    execute_instructions(&cpu, memory);
}