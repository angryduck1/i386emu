#include <stdio.h>
#include <stdbool.h>

typedef char __int8_t;
typedef unsigned char __uint8_t;
typedef short __int16_t;
typedef unsigned short __uint16_t;
typedef int __int32_t;
typedef unsigned __uint32_t;

typedef union {
    __uint32_t dword;

    struct {
        __uint16_t low16;
        __uint16_t high16;
    };

    struct {
        __uint8_t low8;
        __uint8_t high8;
        __uint8_t high8_2;
        __uint8_t low8_3;
    };
} reg_32_t;

typedef enum {
    REAL_MODE,
    PROTECTED_MODE,
    V86_MODE
} cpu_mode_t;

typedef struct {
    bool x66_mode; // Operand size
    bool x67_mode; // Address size
} cpu_prefix;

typedef struct {
    __uint8_t mod;
    __uint8_t reg;
    __uint8_t rm;
} modrm_t;

typedef struct {
    __uint8_t scale;
    __uint8_t index;
    __uint8_t base;
} sib_t;

typedef struct {
    reg_32_t eax; // Accumulator
    reg_32_t ebx; // Base Register
    reg_32_t ecx; // Counter (cycles, offsets)
    reg_32_t edx; // I/O, arymphmetic
    reg_32_t esi; // Source Index (strings, memory)
    reg_32_t edi; // Destination index
    reg_32_t ebp; // Base Pointer
    reg_32_t esp; // Stack Pointer
} baseRegisters;

typedef struct {
    reg_32_t cs; // Code Segment
    reg_32_t ds; // Data Segment
    reg_32_t ss; // Stack Segment
    reg_32_t es; // Additional segment
    reg_32_t fs; // Additional TLS
    reg_32_t gs; // Additional TLS
} segmentRegisters;

typedef struct {
    baseRegisters gpr;
    reg_32_t eip;
    reg_32_t eflags;
    segmentRegisters seg;
    cpu_mode_t mode;
    cpu_prefix prefix;
    __uint8_t* memory;
} cpu_state_t;