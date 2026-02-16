#include "tools.h"

typedef void (*Opcodes)(cpu_state_t*, __uint8_t opcode);

void mov_rm8_r8(cpu_state_t *cpu, __uint8_t opcode) { // MOV r/m8, r8
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
}

void mov_r8_rm8(cpu_state_t *cpu, __uint8_t opcode) { // MOV r8, r/m8
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
}

void mov_rm16or32_r16or32(cpu_state_t *cpu, __uint8_t opcode) { // MOV r/m16, r16 || MOV r/m32, r32
    modrm_t m = decode_modrm(cpu);
    bool op32 = (cpu->mode != REAL_MODE && !cpu->prefix.x66_mode) || (cpu->mode == REAL_MODE && cpu->prefix.x66_mode);

    if (m.mod == 3) {
        if (op32) {
            __uint32_t *src = get_reg32(cpu, m.reg);
            __uint32_t *dst = get_reg32(cpu, m.rm);
            *dst = *src;
        } else {
            __uint16_t *src = get_reg16(cpu, m.reg);
            __uint16_t *dst = get_reg16(cpu, m.rm);
            *dst = *src;
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

void mov_r16or32_rm16or32(cpu_state_t *cpu, __uint8_t opcode) { // MOV r16, r/m16 || MOV r32, r/m32
    modrm_t m = decode_modrm(cpu);
    bool op32 = (cpu->mode != REAL_MODE && !cpu->prefix.x66_mode) || (cpu->mode == REAL_MODE && cpu->prefix.x66_mode);

    if (m.mod == 3) {
        if (op32) {
            __uint32_t *dst = get_reg32(cpu, m.reg);
            __uint32_t *src = get_reg32(cpu, m.rm);
            *dst = *src;
        } else {
            __uint16_t *dst = get_reg16(cpu, m.reg);
            __uint16_t *src = get_reg16(cpu, m.rm);
            *dst = *src;
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

void mov_rm16_sreg(cpu_state_t *cpu, __uint8_t opcode) { // MOV r/m16, Sreg
    modrm_t m = decode_modrm(cpu);

    __uint16_t *src = get_sreg(cpu, m.reg);

    if (m.mod == 3) {
        __uint16_t *dst = get_reg16(cpu, m.rm);
        *dst = *src;
    } else {
        __uint16_t out_segment;
        __uint32_t ea = effective_address(cpu, m, &out_segment);

        write_word(cpu, out_segment, ea, *src);
    }
}

void mov_sreg_rm16(cpu_state_t *cpu, __uint8_t opcode) { // MOV Sreg, r/m16
    modrm_t m = decode_modrm(cpu);
    if (m.reg == 1) {
        fprintf(stderr, "#UD Exception, while not released");
        abort();
        // #UD
    }

    __uint16_t *dst = get_sreg(cpu, m.reg);

    if (m.mod == 3) {
        __uint16_t *src = get_reg16(cpu, m.rm);
        *dst = *src;
    } else {
        __uint16_t out_segment;
        __uint32_t ea = effective_address(cpu, m, &out_segment);

        *dst = read_word(cpu, out_segment, ea);
    }
}

void mov_al_moffs8(cpu_state_t *cpu, __uint8_t opcode) { // MOV AL, moffs8
    bool addr32 = (cpu->mode != REAL_MODE) || cpu->prefix.x67_mode;

    __uint32_t offset;

    if (addr32) {
        offset = read_double_word(cpu, cpu->seg.cs.dword, cpu->eip.dword);
        cpu->eip.dword += 4;
    } else {
        offset = read_word(cpu, cpu->seg.cs.dword, cpu->eip.dword);
        cpu->eip.dword += 2;
    }

    __uint8_t value = read_byte(cpu, cpu->seg.cs.dword, offset);

    __uint8_t *dst = get_reg8(cpu, 0);
    *dst = value;
}

void mov_axoreax_moffs16or32(cpu_state_t *cpu, __uint8_t opcode) { // MOV AX/EAX, moffs16/32
    bool op32 = (cpu->mode != REAL_MODE && !cpu->prefix.x66_mode) || (cpu->mode == REAL_MODE && cpu->prefix.x66_mode);
    bool addr32 = (cpu->mode != REAL_MODE) || cpu->prefix.x67_mode;

    __uint32_t offset;

    if (addr32) {
        offset = read_double_word(cpu, cpu->seg.cs.dword, cpu->eip.dword);
        cpu->eip.dword += 4;
    } else {
        offset = read_word(cpu, cpu->seg.cs.dword, cpu->eip.dword);
        cpu->eip.dword += 2;
    }

    if (op32) {
        cpu->gpr.eax.dword = read_double_word(cpu, cpu->seg.ds.dword, offset);
    } else {
        __uint16_t value;
        value = read_word(cpu, cpu->seg.ds.dword, offset);
        cpu->gpr.eax.dword = (cpu->gpr.eax.dword & 0xFFFF0000) | value;
    }
}

void mov_moffs8_al(cpu_state_t *cpu, __uint8_t opcode) { // MOV moffs8, AL
    bool addr32 = (cpu->mode != REAL_MODE) || cpu->prefix.x67_mode;

    __uint32_t offset;

    if (addr32) {
        offset = read_double_word(cpu, cpu->seg.cs.dword, cpu->eip.dword);
        cpu->eip.dword += 4;
    } else {
        offset = read_word(cpu, cpu->seg.cs.dword, cpu->eip.dword);
        cpu->eip.dword += 2;
    }

    __uint8_t value = cpu->gpr.eax.dword & 0xFF;
    write_byte(cpu, cpu->seg.ds.dword, offset, value);
}

void mov_moffs16or32_axoreax(cpu_state_t *cpu, __uint8_t opcode) { // moffs16/32, MOV AX/EAX
    bool op32 = (cpu->mode != REAL_MODE && !cpu->prefix.x66_mode) || (cpu->mode == REAL_MODE && cpu->prefix.x66_mode);
    bool addr32 = (cpu->mode != REAL_MODE) || cpu->prefix.x67_mode;

    __uint32_t offset;

    if (addr32) {
        offset = read_double_word(cpu, cpu->seg.cs.dword, cpu->eip.dword);
        cpu->eip.dword += 4;
    } else {
        offset = read_word(cpu, cpu->seg.cs.dword, cpu->eip.dword);
        cpu->eip.dword += 2;
    }

    if (op32) {
        __uint32_t value = cpu->gpr.eax.dword;
        write_double_word(cpu, cpu->seg.ds.dword, offset, value);
    } else {
        __uint16_t value = cpu->gpr.eax.dword & 0xFFFF;
        write_word(cpu, cpu->seg.ds.dword, offset, value);
    }
}

void mov_reg8_imm8(cpu_state_t *cpu, __uint8_t opcode) { // MOV reg8, imm8
    __uint8_t reg = opcode & 0x07;
    __uint8_t* dst = get_reg8(cpu, reg);

    __uint8_t imm = read_byte(cpu, cpu->seg.cs.dword, cpu->eip.dword++);

    *dst = imm;
}

void mov_reg16or32_imm16or32(cpu_state_t *cpu, __uint8_t opcode) { // MOV reg16/32, imm16/32
    bool op32 = (cpu->mode != REAL_MODE && !cpu->prefix.x66_mode) || (cpu->mode == REAL_MODE && cpu->prefix.x66_mode);
    __uint8_t reg = opcode & 0x07;

    if (op32) {
        __uint32_t* dst = get_reg32(cpu, reg);

        __uint32_t imm = read_double_word(cpu, cpu->seg.cs.dword, cpu->eip.dword);
        cpu->eip.dword += 4;

        *dst = imm;
    } else {
       __uint16_t* dst = get_reg16(cpu, reg);

        __uint16_t imm = read_word(cpu, cpu->seg.cs.dword, cpu->eip.dword);
        cpu->eip.dword += 2;

        *dst = imm;
    }
}

void mov_rm8_imm8(cpu_state_t *cpu, __uint8_t opcode) { // MOV r/m8, imm8
    modrm_t m = decode_modrm(cpu);

    if (m.reg != 0) {
        fprintf(stderr, "#UD Exception, while not released");
        abort();
        // #UD
    }

    __uint8_t imm = read_byte(cpu, cpu->seg.cs.dword, cpu->eip.dword++);

    if (m.mod == 3) {
        __uint8_t *dst = get_reg8(cpu, m.rm);
        *dst = imm;
    } else {
        __uint16_t out_segment;
        __uint32_t ea = effective_address(cpu, m, &out_segment);

        write_byte(cpu, out_segment, ea, imm);
    }
}

void mov_rm16or32_imm16or32(cpu_state_t *cpu, __uint8_t opcode) { // MOV r/m16/32, imm16/32
    modrm_t m = decode_modrm(cpu);

    if (m.reg != 0) {
        fprintf(stderr, "#UD Exception, while not released");
        abort();
        // #UD
    }

    bool op32 = (cpu->mode != REAL_MODE && !cpu->prefix.x66_mode) || (cpu->mode == REAL_MODE && cpu->prefix.x66_mode);

    if (m.mod == 3) {
        if (op32) {
            __uint32_t imm = read_double_word(cpu, cpu->seg.cs.dword, cpu->eip.dword);
            cpu->eip.dword += 4;
            __uint32_t *dst = get_reg32(cpu, m.rm);
            *dst = imm;
        } else {
            __uint16_t imm = read_word(cpu, cpu->seg.cs.dword, cpu->eip.dword);
            cpu->eip.dword += 2;
            __uint16_t *dst = get_reg16(cpu, m.rm);
            *dst = imm;
        }
    } else {
        __uint16_t out_segment;
        __uint32_t ea = effective_address(cpu, m, &out_segment);

        if (op32) {
            __uint32_t imm = read_double_word(cpu, cpu->seg.cs.dword, cpu->eip.dword);
            cpu->eip.dword += 4;

            write_double_word(cpu, out_segment, ea, imm);
        } else {
            __uint16_t imm = read_word(cpu, cpu->seg.cs.dword, cpu->eip.dword);
            cpu->eip.dword += 2;

            write_word(cpu, out_segment, ea, imm);
        }
    }
}

void push_m16or32(cpu_state_t *cpu, __uint8_t opcode) { // PUSH, m16/32
    bool op32 = (cpu->mode != REAL_MODE && !cpu->prefix.x66_mode) || (cpu->mode == REAL_MODE && cpu->prefix.x66_mode);
    modrm_t m = decode_modrm(cpu);

    if (m.reg != 6) {
        fprintf(stderr, "#UD Exception, while not released");
        abort();
        // #UD
    }

    if (m.mod == 3) {
        if (op32) {
            __uint32_t* value = get_reg32(cpu, m.rm);
            double_word_to_stack(cpu, *value);
        } else {
            __uint16_t* value = get_reg16(cpu, m.rm);
            word_to_stack(cpu, *value);
        }
    } else {
        __uint16_t out_segment;
        __uint32_t ea = effective_address(cpu, m, &out_segment);

        if (op32) {
            __uint32_t value = read_double_word(cpu, out_segment, ea);
            double_word_to_stack(cpu, value);
        } else {
            __uint16_t value = read_word(cpu, out_segment, ea);
            word_to_stack(cpu, value);
        }
    }
}

void init_opcodes(Opcodes* opcodes) {
    opcodes[0x88] = mov_rm8_r8; // MOV r/m8, r8
    opcodes[0x8A] = mov_r8_rm8; // MOV r8, r/m8
    opcodes[0x89] = mov_rm16or32_r16or32; // MOV r/m16, r16 || MOV r/m32, r32
    opcodes[0x8B] = mov_r16or32_rm16or32; // MOV r16, r/m16 || MOV r32, r/m32
    opcodes[0x8C] = mov_rm16_sreg; // MOV r/m16, Sreg
    opcodes[0x8E] = mov_sreg_rm16; // MOV Sreg, r/m16
    opcodes[0xA0] = mov_al_moffs8; // MOV AL, moffs8
    opcodes[0xA1] = mov_axoreax_moffs16or32; // MOV AX/EAX, moffs16/32
    opcodes[0xA2] = mov_moffs8_al; // MOV moffs8, AL
    opcodes[0xA3] = mov_moffs16or32_axoreax; // moffs16/32, MOV AX/EAX
    for (int i = 0; i < 8; i++) {
        opcodes[0xB0 + i] = mov_reg8_imm8; // MOV reg8, imm8
    }
    opcodes[0xB8] = mov_reg16or32_imm16or32; // MOV reg16/32, imm16/32
    opcodes[0xC6] = mov_rm8_imm8; // MOV r/m8, imm8
    opcodes[0xC7] = mov_rm16or32_imm16or32; // MOV r/m16/32, imm16/32
    opcodes[0xFF] = push_m16or32; // PUSH, m16/32
}