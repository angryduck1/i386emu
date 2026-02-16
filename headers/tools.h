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
        case 0: return &cpu->gpr.eax.low8; // AL
        case 1: return &cpu->gpr.ecx.low8; // CL
        case 2: return &cpu->gpr.edx.low8; // DL
        case 3: return &cpu->gpr.ebx.low8; // BL
        case 4: return &cpu->gpr.eax.high8; // AH
        case 5: return &cpu->gpr.ecx.high8; // CH
        case 6: return &cpu->gpr.edx.high8; // DH
        case 7: return &cpu->gpr.ebx.high8; // BH
        default:
            fprintf(stderr, "Invalid Reg8 index: %u", reg);
            abort();
    }
}

__uint16_t* get_reg16(cpu_state_t *cpu, __uint8_t reg) {
    switch (reg) {
        case 0: return &cpu->gpr.eax.low16; // AX
        case 1: return &cpu->gpr.ecx.low16; // CX
        case 2: return &cpu->gpr.edx.low16; // DX
        case 3: return &cpu->gpr.ebx.low16; // BX
        case 4: return &cpu->gpr.esp.low16; // SP
        case 5: return &cpu->gpr.ebp.low16; // BP
        case 6: return &cpu->gpr.esi.low16; // SI
        case 7: return &cpu->gpr.edi.low16; // DI
        default:
            fprintf(stderr, "Invalid Reg16 index: %u", reg);
            abort();
    }
}

__uint32_t* get_reg32(cpu_state_t *cpu, __uint8_t reg) {
    switch (reg) {
        case 0: return &cpu->gpr.eax.dword; // EAX
        case 1: return &cpu->gpr.ecx.dword; // ECX
        case 2: return &cpu->gpr.edx.dword; // EDX
        case 3: return &cpu->gpr.ebx.dword; // EBX
        case 4: return &cpu->gpr.esp.dword; // ESP
        case 5: return &cpu->gpr.ebp.dword; // EBP
        case 6: return &cpu->gpr.esi.dword; // ESI
        case 7: return &cpu->gpr.edi.dword; // EDI
        default:
            fprintf(stderr, "Invalid Reg32 index: %u", reg);
            abort();
    }
}

__uint16_t* get_sreg(cpu_state_t *cpu, __uint8_t reg) {
    switch(reg) {
        case 0: return &cpu->seg.es.low16;
        case 1: return &cpu->seg.cs.low16;
        case 2: return &cpu->seg.ss.low16;
        case 3: return &cpu->seg.ds.low16;
        case 4: return &cpu->seg.fs.low16;
        case 5: return &cpu->seg.gs.low16;
        default:
            fprintf(stderr, "Invalid Sreg index: %u", reg);
            abort();
    }
}


void word_to_stack(cpu_state_t *cpu, __uint16_t value) {
    if (cpu->mode == REAL_MODE) {
        cpu->gpr.esp.low16 -= 2;
        
        __uint32_t linear_addr = translate_address(cpu, cpu->seg.ss.low16, cpu->gpr.esp.low16);

        cpu->memory[linear_addr] = value & 0xFF;
        cpu->memory[linear_addr + 1] = (value >> 8) & 0xFF;
    } else if (cpu->mode == PROTECTED_MODE) {
        cpu->gpr.esp.dword -= 2;

        __uint32_t linear_addr = translate_address(cpu, cpu->seg.ss.low16, cpu->gpr.esp.dword);

        cpu->memory[linear_addr] = value & 0xFF;
        cpu->memory[linear_addr + 1] = (value >> 8) & 0xFF;
    }
}

void double_word_to_stack(cpu_state_t *cpu, __uint32_t value) {
    if (cpu->mode == REAL_MODE) {
        cpu->gpr.esp.low16 -= 4;

        __uint32_t linear_addr = translate_address(cpu, cpu->seg.ss.low16, cpu->gpr.esp.low16);
    
        cpu->memory[linear_addr] = value & 0xFF;
        cpu->memory[linear_addr + 1] = (value >> 8) & 0xFF;
        cpu->memory[linear_addr + 2] = (value >> 16) & 0xFF;
        cpu->memory[linear_addr + 3] = (value >> 24) & 0xFF;
    } else if (cpu->mode == PROTECTED_MODE) {
        cpu->gpr.esp.dword -= 4;

        __uint32_t linear_addr = translate_address(cpu, cpu->seg.ss.low16, cpu->gpr.esp.dword);
    
        cpu->memory[linear_addr] = value & 0xFF;
        cpu->memory[linear_addr + 1] = (value >> 8) & 0xFF;
        cpu->memory[linear_addr + 2] = (value >> 16) & 0xFF;
        cpu->memory[linear_addr + 3] = (value >> 24) & 0xFF;
    }
}

modrm_t decode_modrm(cpu_state_t* cpu) {
    __uint8_t byte = read_byte(cpu, cpu->seg.cs.dword, cpu->eip.dword++);
    modrm_t m;
    m.mod = (byte >> 6) & 0x03;
    m.reg = (byte >> 3) & 0x07;
    m.rm = byte & 0x07;

    return m;
}

sib_t decode_sib(cpu_state_t *cpu) {
    __uint8_t byte = read_byte(cpu, cpu->seg.cs.dword, cpu->eip.dword++);
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
        base = read_double_word(cpu, cpu->seg.cs.dword, cpu->eip.dword);
        cpu->eip.dword += 4;
        *out_segment = cpu->seg.ds.dword;
    } else {
        base = *get_reg32(cpu, s.base);

        if (s.base == 4 || s.base == 5) {
            *out_segment = cpu->seg.ss.dword;
        }
        else {
            *out_segment = cpu->seg.ds.dword;
        }
    }

    return base + index;
}

__uint32_t effective_address(cpu_state_t *cpu, modrm_t m, __uint16_t* out_segment) {
    __int32_t base = 0;
    __int32_t disp = 0;

    bool addr32 = (cpu->mode != REAL_MODE) || (cpu->mode == REAL_MODE && cpu->prefix.x67_mode);

    if (m.mod == 1) {
        disp = (__int8_t)read_byte(cpu, cpu->seg.cs.dword, cpu->eip.dword++);
    } else if (m.mod == 2) {
        if (addr32) {
            disp = read_double_word(cpu, cpu->seg.cs.dword, cpu->eip.dword);
            cpu->eip.dword+=4;
        } else {
            disp = read_word(cpu, cpu->seg.cs.dword, cpu->eip.dword);
            cpu->eip.dword+=2;
        }
    }

    if (!addr32) {
        switch (m.rm) {
            case 0: base = cpu->gpr.ebx.low16 + cpu->gpr.esi.low16; *out_segment = cpu->seg.ds.dword; break;
            case 1: base = cpu->gpr.ebx.low16 + cpu->gpr.edi.low16; *out_segment = cpu->seg.ds.dword; break;
            case 2: base = cpu->gpr.ebp.low16 + cpu->gpr.esi.low16; *out_segment = cpu->seg.ss.dword; break;
            case 3: base = cpu->gpr.ebp.low16 + cpu->gpr.edi.low16; *out_segment = cpu->seg.ss.dword; break;
            case 4: base = cpu->gpr.esi.low16; *out_segment = cpu->seg.ds.dword; break;
            case 5: base = cpu->gpr.edi.low16; *out_segment = cpu->seg.ds.dword; break;
            case 6: {
                if (m.mod == 0) {
                    base = read_word(cpu, cpu->seg.cs.dword, cpu->eip.dword);
                    cpu->eip.dword += 2;
                    *out_segment = cpu->seg.ds.dword;
                } else {
                    base = cpu->gpr.ebp.low16;
                    *out_segment = cpu->seg.ss.dword;
                }
                break;
            }
            case 7: base = cpu->gpr.ebx.low16; *out_segment = cpu->seg.ds.dword; break;
        }

        return (base + disp) & 0xFFFF;
    } else if (addr32) {
        switch (m.rm) {
        case 0: base = cpu->gpr.eax.dword; *out_segment = cpu->seg.ds.dword; break;
        case 1: base = cpu->gpr.ecx.dword; *out_segment = cpu->seg.ds.dword; break;
        case 2: base = cpu->gpr.edx.dword; *out_segment = cpu->seg.ds.dword; break;
        case 3: base = cpu->gpr.ebx.dword; *out_segment = cpu->seg.ds.dword; break;
        case 6: base = cpu->gpr.esi.dword; *out_segment = cpu->seg.ds.dword; break;
        case 7: base = cpu->gpr.edi.dword; *out_segment = cpu->seg.ds.dword; break;
        case 4: 
            sib_t s = decode_sib(cpu); 
            base = effective_sib_address(cpu, m, s, out_segment); 
            break;
        case 5:
            if (m.mod == 0) {
                base = read_double_word(cpu, cpu->seg.cs.dword, cpu->eip.dword);
                cpu->eip.dword += 4;
                *out_segment = cpu->seg.ds.dword;
            } else {
                base = cpu->gpr.ebp.dword;
                *out_segment = cpu->seg.ss.dword;
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
        byte = read_byte(cpu, cpu->seg.cs.dword, cpu->eip.dword++);
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