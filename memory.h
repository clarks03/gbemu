#ifndef MEMORY_H
#define MEMORY_H

typedef struct {
    char rom_bank_00[0x4000];  // 16 KiB ROM bank 00
    char rom_bank_nn[0x4000];  // 16 KiB ROM bank 01-NN
    char vram[0x2000];         // 8 KiB VRAM
    char eram[0x2000];         // 8 KiB External RAM
    char wram_1[0x1000];       // 4 KiB Work RAM (WRAM)
    char wram_2[0x1000];       // 4 KiB Work RAM (WRAM)
    char echo_ram[0x1D00];     // echo ram
    char oam[0xA0];            // object attribute memory (oam)
    char unusable[0x60];       // not usable
    char io[0x80];             // I/O Registers
    char hram[0x7F];           // High RAM (HRAM)
    char ie;                   // Interrupt enable register
} memory_map;

char read_memory(memory_map *mem, unsigned short address);
void write_memory(memory_map *mem, int address, char value);

#endif
