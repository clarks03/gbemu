#ifndef GAMEBOY_H
#define GAMEBOY_H

#include "memory.h"
#include "cpu.h"

typedef struct {
    memory_map *memory;
    registers *regs;
} gameboy;

#endif
