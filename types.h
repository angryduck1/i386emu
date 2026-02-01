#include <stdio.h>
#include <stdbool.h>

typedef enum {
    REAL_MODE,
    PROTECTED_MODE,
    V86_MODE
} cpu_mode_t;

typedef struct {
    bool x66_mode;
    bool x67_mode;
} cpu_prefix;

typedef struct {
    __uint8_t mod;
    __uint8_t reg;
    __uint8_t rm;
} modrm_t;

typedef struct {
    __uint32_t eax; // Accumulator
    __uint32_t ebx; // Base Register
    __uint32_t ecx; // Counter (cycles, offsets)
    __uint32_t edx; // I/O, arymphmetic
    __uint32_t esi; // Source Index (strings, memory)
    __uint32_t edi; // Destination index
    __uint32_t ebp; // Base Pointer
    __uint32_t esp; // Stack Pointer
} baseRegisters;

typedef struct {
    __uint32_t cs; // Code Segment
    __uint32_t ds; // Data Segment
    __uint32_t ss; // Stack Segment
    __uint32_t es; // Additional segment
    __uint32_t fs; // Additional TLS
    __uint32_t gs; // Additional TLS
} segmentRegisters;

typedef struct {
    baseRegisters gpr;
    __uint32_t eip;
    __uint32_t eflags;
    segmentRegisters seg;
    cpu_mode_t mode;
    cpu_prefix prefix;
} cpu_state_t;