#include "tools.h"

typedef void (*Opcodes)(cpu_state_t*);

void mov_rm8_r8(cpu_state_t *cpu) { // MOV r/m8, r8
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
}

void mov_r8_rm8(cpu_state_t *cpu) { // MOV r8, r/m8
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
}

void mov_rm16or32_r16or32(cpu_state_t *cpu) { // MOV r/m16, r16 || MOV r/m32, r32
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
}

void mov_r16or32_rm16or32(cpu_state_t *cpu) { // MOV r16, r/m16 || MOV r32, r/m32
    modrm_t m = decode_modrm(cpu);
    bool op32 = (cpu->mode != REAL_MODE && !cpu->prefix.x66_mode) || (cpu->mode == REAL_MODE && cpu->prefix.x66_mode);

    if (m.mod == 3) {
        if (op32) {
            __uint32_t *dst = get_reg32(cpu, m.reg);
            __uint32_t *src = get_reg32(cpu, m.rm);
            set_reg32(dst, src);
        } else {
            __uint16_t *dst = get_reg16(cpu, m.reg);
            __uint16_t *src = get_reg16(cpu, m.rm);
            set_reg16(dst, src);
        }
    } else {
        __uint16_t out_segment;
        __uint32_t ea = effective_address(cpu, m, &out_segment);

        if (op32) {
            __uint32_t *dst = get_reg32(cpu, m.reg);
            *dst = read_double_word(cpu, out_segment, ea);
        } else {
            __uint16_t *dst = get_reg16(cpu, m.reg);
            *dst = read_word(cpu, out_segment, ea);
        }
    }
}

void mov_rm16_sreg(cpu_state_t *cpu) { // MOV r/m16, Sreg
    modrm_t m = decode_modrm(cpu);

    __uint16_t *src = get_sreg(cpu, m.reg);

    if (m.mod == 3) {
        __uint16_t *dst = get_reg16(cpu, m.rm);
        set_reg16(dst, src);
    } else {
        __uint16_t out_segment;
        __uint32_t ea = effective_address(cpu, m, &out_segment);

        write_word(cpu, out_segment, ea, *src);
    }
}

void mov_sreg_rm16(cpu_state_t *cpu) { // MOV Sreg, r/m16
    modrm_t m = decode_modrm(cpu);
    if (m.reg == 1) {
        fprintf(stderr, "#UD Exception, while not released");
        abort();
        // #UD
    }

    __uint16_t *dst = get_sreg(cpu, m.reg);

    if (m.mod == 3) {
        __uint16_t *src = get_reg16(cpu, m.rm);
        set_reg16(dst, src);
    } else {
        __uint16_t out_segment;
        __uint32_t ea = effective_address(cpu, m, &out_segment);

        dst = read_word(cpu, out_segment, ea);
    }
}

void init_opcodes(Opcodes* opcodes) {
    opcodes[0x88] = mov_rm8_r8; // MOV r/m8, r8
    opcodes[0x8A] = mov_r8_rm8; // MOV r8, r/m8
    opcodes[0x89] = mov_rm16or32_r16or32; // MOV r/m16, r16 || MOV r/m32, r32
    opcodes[0x8B] = mov_r16or32_rm16or32; // MOV r16, r/m16 || MOV r32, r/m32
    opcodes[0x8C] = mov_rm16_sreg; // MOV r/m16, Sreg
    opcodes[0x8D] = mov_rm16_sreg; // MOV Sreg, r/m16
}