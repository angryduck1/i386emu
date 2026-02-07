#pragma once

#include <stdio.h>
#include "types.h"

__uint32_t translate_address(cpu_state_t *cpu, __uint16_t segment, __uint32_t offset) {
    switch (cpu->mode) {
        case REAL_MODE:
            return ((segment << 4) + offset) & 0xFFFFF;
        case PROTECTED_MODE:
            fprintf(stderr, "PROTECTED_MODE not implemented\n");
            abort();
        default:
            abort();
    }
}

__uint8_t read_byte(cpu_state_t *cpu, __uint16_t segment, __uint32_t offset) {
    __uint32_t phys = translate_address(cpu, segment, offset);
    return cpu->memory[(phys) & 0xFFFFF];
}

__uint16_t read_word(cpu_state_t *cpu, __uint16_t segment, __uint32_t offset) {
    __uint32_t phys = translate_address(cpu, segment, offset);
    return cpu->memory[(phys) & 0xFFFFF] | (cpu->memory[(phys+1) & 0xFFFFF] << 8);
}

__uint32_t read_double_word(cpu_state_t *cpu, __uint16_t segment, __uint32_t offset) {
    __uint32_t phys = translate_address(cpu, segment, offset);
    return cpu->memory[(phys) & 0xFFFFF] | (cpu->memory[(phys+1) & 0xFFFFF] << 8) | (cpu->memory[(phys+2) & 0xFFFFF] << 16) | (cpu->memory[(phys+3) & 0xFFFFF] << 24);
}

void write_byte(cpu_state_t *cpu, __uint16_t segment, __uint32_t offset, __uint8_t value) {
    __uint32_t phys = translate_address(cpu, segment, offset);
    cpu->memory[(phys) & 0xFFFFF] = value;
}

void write_word(cpu_state_t *cpu, __uint16_t segment, __uint32_t offset, __uint16_t value) {
    __uint32_t phys = translate_address(cpu, segment, offset);
    cpu->memory[(phys) & 0xFFFFF] = value & 0xFF;
    cpu->memory[(phys+1) & 0xFFFFF] = (value) >> 8 & 0xFF;
}

void write_double_word(cpu_state_t *cpu, __uint16_t segment, __uint32_t offset, __uint32_t value) {
    __uint32_t phys = translate_address(cpu, segment, offset);
    cpu->memory[(phys) & 0xFFFFF] = value & 0xFF;
    cpu->memory[(phys+1) & 0xFFFFF] = (value >> 8) & 0xFF;
    cpu->memory[(phys+2) & 0xFFFFF] = (value >> 16) & 0xFF;
    cpu->memory[(phys+3) & 0xFFFFF] = (value >> 24) & 0xFF;
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

__uint32_t* get_reg32(cpu_state_t *cpu, __uint8_t reg) {
    switch (reg) {
        case 0: return &cpu->gpr.eax; // EAX
        case 1: return &cpu->gpr.ecx; // ECX
        case 2: return &cpu->gpr.edx; // EDX
        case 3: return &cpu->gpr.ebx; // EBX
        case 4: return &cpu->gpr.esp; // ESP
        case 5: return &cpu->gpr.ebp; // EBP
        case 6: return &cpu->gpr.esi; // ESI
        case 7: return &cpu->gpr.edi; // EDI
    }

    return NULL;
}

__uint16_t* get_sreg(cpu_state_t *cpu, __uint8_t reg) {
    switch(reg) {
        case 0: return &cpu->seg.es;
        case 1: return &cpu->seg.cs;
        case 2: return &cpu->seg.ss;
        case 3: return &cpu->seg.ds;
        case 4: return &cpu->seg.fs;
        case 5: return &cpu->seg.gs;
        default:
            fprintf(stderr, "Invalid Sreg index: %u", reg);
            abort();
    }
}

void set_reg8(__uint8_t* dst, __uint8_t* src) {
    *dst = *src;
}

void set_reg16(__uint16_t* dst, __uint16_t* src) {
    *dst = *src;
}

void set_reg32(__uint32_t* dst, __uint32_t* src) {
    *dst = *src;
}

modrm_t decode_modrm(cpu_state_t* cpu) {
    __uint8_t byte = read_byte(cpu, cpu->seg.cs, cpu->eip++);
    modrm_t m;
    m.mod = (byte >> 6) & 0x03;
    m.reg = (byte >> 3) & 0x07;
    m.rm = byte & 0x07;

    return m;
}

sib_t decode_sib(cpu_state_t *cpu) {
    __uint8_t byte = read_byte(cpu, cpu->seg.cs, cpu->eip++);
    sib_t s;
    s.scale = (byte >> 6) & 0x03;
    s.index = (byte >> 3) & 0x07;
    s.base = byte & 0x07;
    return s;
}

__uint32_t effective_sib_address(cpu_state_t *cpu, modrm_t m, sib_t s, __uint16_t* out_segment) {
    __uint32_t index = 0;
    __uint32_t base = 0;

    if (s.index != 4) {
        index = *get_reg32(cpu, s.index) << s.scale;
    }

    if (s.base == 5 && m.mod == 0) {
        base = read_double_word(cpu, cpu->seg.cs, cpu->eip);
        cpu->eip += 4;
        *out_segment = cpu->seg.ds;
    } else {
        base = *get_reg32(cpu, s.base);

        if (s.base == 4 || s.base == 5) {
            *out_segment = cpu->seg.ss;
        }
        else {
            *out_segment = cpu->seg.ds;
        }
    }

    return base + index;
}

__uint32_t effective_address(cpu_state_t *cpu, modrm_t m, __uint16_t* out_segment) {
    __int32_t base = 0;
    __int32_t disp = 0;

    bool addr32 = (cpu->mode != REAL_MODE) || (cpu->mode == REAL_MODE && cpu->prefix.x67_mode);

    if (m.mod == 1) {
        disp = (__int8_t)read_byte(cpu, cpu->seg.cs, cpu->eip++);
    } else if (m.mod == 2) {
        if (addr32) {
            disp = read_double_word(cpu, cpu->seg.cs, cpu->eip);
            cpu->eip+=4;
        } else {
            disp = read_word(cpu, cpu->seg.cs, cpu->eip);
            cpu->eip+=2;
        }
    }

    if (!addr32) {
        switch (m.rm) {
            case 0: base = (cpu->gpr.ebx & 0xFFFF) + (cpu->gpr.esi & 0xFFFF); *out_segment = cpu->seg.ds; break;
            case 1: base = (cpu->gpr.ebx & 0xFFFF) + (cpu->gpr.edi & 0xFFFF); *out_segment = cpu->seg.ds; break;
            case 2: base = (cpu->gpr.ebp & 0xFFFF) + (cpu->gpr.esi & 0xFFFF); *out_segment = cpu->seg.ss; break;
            case 3: base = (cpu->gpr.ebp & 0xFFFF) + (cpu->gpr.edi & 0xFFFF); *out_segment = cpu->seg.ss; break;
            case 4: base = (cpu->gpr.esi & 0xFFFF); *out_segment = cpu->seg.ds; break;
            case 5: base = (cpu->gpr.edi & 0xFFFF); *out_segment = cpu->seg.ds; break;
            case 6: {
                if (m.mod == 0) {
                    base = read_word(cpu, cpu->seg.cs, cpu->eip);
                    cpu->eip += 2;
                    *out_segment = cpu->seg.ds;
                } else {
                    base = cpu->gpr.ebp & 0xFFFF;
                    *out_segment = cpu->seg.ss;
                }
                break;
            }
            case 7: base = cpu->gpr.ebx & 0xFFFF; *out_segment = cpu->seg.ds; break;
        }

        return (base + disp) & 0xFFFF;
    } else if (addr32) {
        switch (m.rm) {
        case 0: base = cpu->gpr.eax; *out_segment = cpu->seg.ds; break;
        case 1: base = cpu->gpr.ecx; *out_segment = cpu->seg.ds; break;
        case 2: base = cpu->gpr.edx; *out_segment = cpu->seg.ds; break;
        case 3: base = cpu->gpr.ebx; *out_segment = cpu->seg.ds; break;
        case 6: base = cpu->gpr.esi; *out_segment = cpu->seg.ds; break;
        case 7: base = cpu->gpr.edi; *out_segment = cpu->seg.ds; break;
        case 4: 
            sib_t s = decode_sib(cpu); 
            base = effective_sib_address(cpu, m, s, out_segment); 
            break;
        case 5:
            if (m.mod == 0) {
                base = read_double_word(cpu, cpu->seg.cs, cpu->eip);
                cpu->eip += 4;
                *out_segment = cpu->seg.ds;
            } else {
                base = cpu->gpr.ebp;
                *out_segment = cpu->seg.ss;
            }
        break;
    }

    if (cpu->mode == REAL_MODE) {
        return (base + disp) & 0xFFFF;
    }
    else {
        return base + disp;
    }
}
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