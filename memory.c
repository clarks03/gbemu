#include "memory.h"


char read_memory(memory_map *mem, unsigned short address) {
    // Handle different memory regions based on address ranges
    if (address < 0x4000)
        return mem->rom_bank_00[address];
    else if (address < 0x8000)
        return mem->rom_bank_nn[address];
    // Add other cases
    
    return 0;
}

void write_memory(memory_map *mem, int address, char value) {
    if (address < 0x4000)
        mem->rom_bank_00[address] = value;
    else if (address < 0x8000)
        mem->rom_bank_nn[address] = value;
    // Add other cases
    
    return;
}

