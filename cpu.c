#include "cpu.h"
#include "gameboy.h"
#include "memory.h"


/* Helper function spam */
char get_a(registers *regs) {
    return regs->a;
}

char get_b(registers *regs) {
    return regs->b;
}

char get_c(registers *regs) {
    return regs->c;
}

char get_d(registers *regs) {
    return regs->d;
}

char get_e(registers *regs) {
    return regs->e;
}

char get_f(registers *regs) {
    return regs->f;
}

char get_h(registers *regs) {
    return regs->h;
}

char get_l(registers *regs) {
    return regs->l;
}

short get_af(registers *regs) {
    return (regs->a << 8) | (regs->f);
}

short get_bc(registers *regs) {
    return (regs->b << 8) | (regs->c);
}

short get_de(registers *regs) {
    return (regs->d << 8) | (regs->e);
}

short get_hl(registers *regs) {
    return (regs->h << 8) | (regs->l);
}

short get_sp(registers *regs) {
    return regs->sp;
}

short get_pc(registers *regs) {
    return regs->pc;
}

short is_z(registers *regs) {
    return regs->f & ZERO_FLAG;
}

void set_z(registers *regs) {
    regs->f |= ZERO_FLAG;
}

void clear_z(registers *regs) {
    regs->f &= ~ZERO_FLAG;
}

short is_n(registers *regs) {
    return regs->f & SUB_FLAG;
}

void set_n(registers *regs) {
    regs->f |= SUB_FLAG;
}

void clear_n(registers *regs) {
    regs->f &= ~SUB_FLAG;
}

short is_h(registers *regs) {
    return regs->f & HALF_CARRY_FLAG;
}

void set_h(registers *regs) {
    regs->f |= HALF_CARRY_FLAG;
}

void clear_h(registers *regs) {
    regs->f &= ~HALF_CARRY_FLAG;
}

short is_c(registers *regs) {
    return regs->f & CARRY_FLAG;
}

void set_c(registers *regs) {
    regs->f |= CARRY_FLAG;
}

void clear_c(registers *regs) {
    regs->f &= ~CARRY_FLAG;
}

// Fetch-Decode-Execute instruction
char fetch_instruction(memory_map *mem, registers *regs) {
    return read_memory(mem, regs->pc++);
}

void decode_and_execute(gameboy *gb, char opcode) {
    switch (opcode & 0xC0) {  // Determining the block
        case 0x00:  // Block 0
            // There are 6 "sub-blocks" here
            // Block 1: 00000000
            // Block 2: last 4 bits 0001, 001, 1010, 1000
            // Block 3: last 4 bits 0011, 1011, 1001
            // Block 4: last 3 100, 101
            // Block 5: last 3 110
            // Block 6: last 3 111 and bits 5-3 iterating 000-111

            // We're gonna loop over the last 3 bits
            switch(opcode & 0x07) {
                case 0x00:  // 3 different options
                    // check bit[5]
                    switch ((opcode >> 5) & 1) {
                        case 0:
                            // check bit[4]
                            switch ((opcode >> 4) & 1) {
                                case 0:  // noop
                                break;

                                case 1:  // either jr imm8 or stop
                                    switch ((opcode >> 3) & 1) {
                                        case 0:  // stop
                                            // TODO: change this from noop
                                        break;

                                        case 1:  // jr imm8
                                            char imm8 = read_memory(gb->memory, gb->regs->pc);
                                            gb->regs->pc += imm8;
                                        break;
                                    }
                                break;
                            }
                        break;

                        case 1:  // jr cond, imm8
                            char imm8 = read_memory(gb->memory, gb->regs->pc);
                            // determine the value of cond
                            switch ((opcode >> 4) & 0x03) {
                                case 0:  // cond = nz
                                    if (!is_z(gb->regs)) 
                                        gb->regs->pc += imm8;
                                break;

                                case 1:  // cond = z
                                    if (is_z(gb->regs))
                                        gb->regs->pc += imm8;
                                break;

                                case 2:  // cond = nc
                                    if (!is_c(gb->regs))
                                        gb->regs->pc += imm8;
                                break;

                                case 3:  // cond = c
                                    if (is_c(gb->regs))
                                        gb->regs->pc += imm8;
                                break;
                            }
                        break;
                    }
                break;

                case 0x01:  // either ld r16, imm16 (if bit 3 == 0)
                            // or add hl, r16 (if bit 3 == 1)
                    switch ((opcode >> 3) & 1) {
                        case 0:  // ld r16, imm16
                            short imm16 = read_memory(gb->memory, gb->regs->pc);
                            gb->regs->pc++;
                            imm16 |= (read_memory(gb->memory, gb->regs->pc) << 8);
                            gb->regs->pc++;

                            // Now I need to choose the possible values of r16
                            switch ((opcode >> 4) & 0x03) {
                                case 0:  // bc
                                    gb->regs->b = (imm16 >> 8) & 0xFF;
                                    gb->regs->c = imm16 & 0xFF;
                                break;

                                case 1:  // de
                                    gb->regs->d = (imm16 >> 8) & 0xFF;
                                    gb->regs->e = imm16 & 0xFF;
                                break;

                                case 2:  // hl
                                    gb->regs->h = (imm16 >> 8) & 0xFF;
                                    gb->regs->l = imm16 & 0xFF;
                                break;

                                case 3:  // sp
                                    gb->regs->sp = imm16;
                                break;
                            }
                        break;
                        case 1:  // add hl, r16
                            // Choose possible values of r16
                            switch ((opcode >> 4) & 0x03) {
                                short low_sum, carry, high_sum;
                                case 0:  // r16 = bc
                                    low_sum = (gb->regs->l + gb->regs->c);
                                    gb->regs->l = low_sum & 0xFF;

                                    carry = (low_sum > 0xFF) ? 1 : 0;

                                    high_sum = (gb->regs-> h + gb->regs->b + carry);
                                    gb->regs->h = high_sum & 0xFF;

                                    if (gb->regs->h == 0 && gb->regs->l == 0)
                                        set_z(gb->regs);

                                    if (high_sum > 0xFF)
                                        set_c(gb->regs);
                                break;

                                case 1:  // r16 = de
                                    low_sum = (gb->regs->l + gb->regs->e);
                                    gb->regs->l = low_sum & 0xFF;

                                    carry = (low_sum > 0xFF) ? 1 : 0;

                                    high_sum = (gb->regs-> h + gb->regs->d + carry);
                                    gb->regs->h = high_sum & 0xFF;

                                    if (gb->regs->h == 0 && gb->regs->l == 0)
                                        set_z(gb->regs);

                                    if (high_sum > 0xFF)
                                        set_c(gb->regs);
                                break;

                                case 2:  // r16 = hl
                                    low_sum = (gb->regs->l + gb->regs->l);
                                    gb->regs->l = low_sum & 0xFF;

                                    carry = (low_sum > 0xFF) ? 1 : 0;

                                    high_sum = (gb->regs-> h + gb->regs->h + carry);
                                    gb->regs->h = high_sum & 0xFF;

                                    if (gb->regs->h == 0 && gb->regs->l == 0)
                                        set_z(gb->regs);

                                    if (high_sum > 0xFF)
                                        set_c(gb->regs);
                                break;

                                case 3:  // r16 = sp
                                    low_sum = (gb->regs->l + (gb->regs->sp & 0xFF));
                                    gb->regs->l = low_sum & 0xFF;

                                    carry = (low_sum > 0xFF) ? 1 : 0;

                                    high_sum = (gb->regs->h + (gb->regs->sp >> 8) + carry);
                                    gb->regs->h = high_sum & 0xFF;

                                    if (gb->regs->h == 0 && gb->regs->l == 0)
                                        set_z(gb->regs);

                                    if (high_sum > 0xFF)
                                        set_c(gb->regs);
                                break;
                            }
                        break;
                    }

                case 0x02:  // This can either be ld [r16mem] (if bit[3] == 0)
                            // or ld a, [r16mem] (if bit[3] == 1)
                    switch ((opcode >> 3) & 1) {
                        case 0:  // ld [r16mem], a
                            // Possible values of r16mem
                            switch ((opcode >> 4) & 0x03) {
                                case 0:  // r16mem = bc
                                    write_memory(gb->memory, get_bc(gb->regs), get_a(gb->regs));
                                break;

                                case 1:  // r16mem = de
                                    write_memory(gb->memory, get_de(gb->regs), get_a(gb->regs));
                                break;

                                case 2:  // r16mem = hl+
                                    write_memory(gb->memory, get_hl(gb->regs), get_a(gb->regs));
                                    gb->regs->l++;  // NOTE: not adjusting h
                                break;

                                case 3:  // r16mem = hl-
                                    write_memory(gb->memory, get_hl(gb->regs), get_a(gb->regs));
                                    gb->regs->l--;  // NOTE: not adjusting h
                                break;
                            }
                        break;

                        case 1:  // ld a, [r16mem]
                            // Possible values of r16mem
                            switch ((opcode >> 4) & 0x03) {
                                char val;
                                case 0:  // r16mem = bc
                                    val = read_memory(gb->memory, get_bc(gb->regs));
                                    gb->regs->a = val;
                                break;

                                case 1:  // r16mem = de
                                    val = read_memory(gb->memory, get_de(gb->regs));
                                    gb->regs->a = val;
                                break;

                                case 2:  // r16mem = hl
                                    val = read_memory(gb->memory, get_hl(gb->regs));
                                    gb->regs->a = val;
                                    gb->regs->l++;  // NOTE: not adjusting h
                                break;

                                case 3:
                                    val = read_memory(gb->memory, get_hl(gb->regs));
                                    gb->regs->a = val;
                                    gb->regs->l--;  // NOTE: not adjusting h
                                break;
                            }
                        break;
                    }
                break;

                case 0x03:  // This can either be inc r16 (if bit[3] == 0)
                            // or dec r16 (if bit[3] == 1)
                    switch ((opcode >> 3) & 1) {
                        case 0:  // inc r16
                            // Possible values of r16
                            switch ((opcode >> 4) & 0x03) {
                                case 0:  // r16 = bc
                                    gb->regs->c++;
                                    if (gb->regs->c == 0)
                                        gb->regs->b++;
                                break;

                                case 1:  // r16 = de
                                    gb->regs->e++;
                                    if (gb->regs->e == 0)
                                        gb->regs->d++;
                                break;

                                case 2:  // r16 = hl
                                    gb->regs->l++;
                                    if (gb->regs->l == 0)
                                        gb->regs->h++;
                                break;

                                case 3:  // r16 = sp
                                    gb->regs->sp++;
                                break;
                            }
                        break;

                        case 1:  // dec r16
                            // Possible values of r16
                            switch ((opcode >> 4) & 0x03) {
                                case 0:  // r16 = bc
                                    gb->regs->c--;
                                    if (gb->regs->c == 0xFF)
                                        gb->regs->b--;
                                break;

                                case 1:  // r16 = de
                                    gb->regs->e--;
                                    if (gb->regs->e == 0xFF)
                                        gb->regs->d--;
                                break;

                                case 2:  // r16 = hl
                                    gb->regs->l--;
                                    if (gb->regs->l == 0xFF)
                                        gb->regs->h--;
                                break;

                                case 3:  // r16 = sp
                                    gb->regs->sp--;
                                break;
                            }
                        break;
                    }
                break;

                case 0x04:  // inc r8
                    // Now I need to determine the value of r8 (another switch statement!!)
                    switch ((opcode >> 3) & 0x07) {
                        case 0:  // r8 = b register
                            gb->regs->b++;
                        break;

                        case 1:  // r8 = c register
                            gb->regs->c++;
                        break;

                        case 2:  // r8 = d register
                            gb->regs->d++;
                        break;

                        case 3:  // r8 = e register
                            gb->regs->e++;
                        break;

                        case 4:  // r8 = h register
                            gb->regs->h++;
                        break;

                        case 5:  // r8 = l register
                            gb->regs->l++;
                        break;

                        case 6:  // r8 = [hl] register
                            short hl = get_hl(gb->regs);
                            char val = read_memory(gb->memory, hl);
                            write_memory(gb->memory, hl, val + 1);
                        break;

                        case 7:  // r8 = a register
                            gb->regs->a++;
                        break;
                    }
                break;

                case 0x05:  // dec r8
                    // Now I need to determine the value of r8 (another switch statement!!)
                    switch ((opcode >> 3) & 0x07) {
                        case 0:  // r8 = b register
                            gb->regs->b--;
                        break;

                        case 1:  // r8 = c register
                            gb->regs->c--;
                        break;

                        case 2:  // r8 = d register
                            gb->regs->d--;
                        break;

                        case 3:  // r8 = e register
                            gb->regs->e--;
                        break;

                        case 4:  // r8 = h register
                            gb->regs->h--;
                        break;

                        case 5:  // r8 = l register
                            gb->regs->l--;
                        break;

                        case 6:  // r8 = [hl] register
                            short hl = get_hl(gb->regs);
                            char val = read_memory(gb->memory, hl);
                            write_memory(gb->memory, hl, val - 1);
                        break;

                        case 7:  // r8 = a register
                            gb->regs->a--;
                        break;
                    }
                break;
                
                case 0x06:  // ld r8, imm8
                    char imm8 = read_memory(gb->memory, gb->regs->pc);
                    gb->regs->pc++;
                    // Determine value of r8
                    switch ((opcode >> 3) & 0x07) {
                        case 0:  // r8 = b
                            gb->regs->b = imm8;
                        break;

                        case 1:  // r8 = c
                            gb->regs->c = imm8;
                        break;

                        case 2:  // r8 = d
                            gb->regs->d = imm8;
                        break;

                        case 3:  // r8 = e
                            gb->regs->e = imm8;
                        break;

                        case 4:  // r8 = h
                            gb->regs->h = imm8;
                        break;

                        case 5:  // r8 = l
                            gb->regs->l = imm8;
                        break;

                        case 6:  // r8 = [hl]
                            write_memory(gb->memory, gb->regs->h << 8 | gb->regs->l, imm8);
                        break;

                        case 7:  // r8 = a
                            gb->regs->a = imm8;
                        break;
                    }
                break;

                case 0x07:  // 8 possible values, based on bits 3-5
                    // NOTE: not 100% sure about these
                    char carry;
                    switch ((opcode >> 3) & 0x07) {
                        case 0:  // rlca
                            clear_c(gb->regs);
                            if ((gb->regs->a >> 7) & 1)
                                set_c(gb->regs);
                            gb->regs->a = (gb->regs->a << 1) | get_c(gb->regs);
                        break;

                        case 1:  // rrca
                            clear_c(gb->regs);
                            if (gb->regs->a & 0x01)
                                set_c(gb->regs) ;
                            gb->regs->a = (gb->regs->a >> 1) | (get_c(gb->regs) << 7);
                        break;

                        case 2:  // rla
                            carry = get_c(gb->regs);
                            clear_c(gb->regs);
                            if ((gb->regs->a >> 7) & 1)
                                set_c(gb->regs);
                            gb->regs->a = (gb->regs->a << 1) | carry;
                        break;

                        case 3:  // rra
                            carry = get_c(gb->regs);
                            clear_c(gb->regs);
                            if (gb->regs->a & 0x01)
                                set_c(gb->regs);
                            gb->regs->a = (carry << 7) | (gb->regs->a >> 1);
                        break;

                        case 4:  // daa
                            if ((gb->regs->a & 0x0F) || get_h(gb->regs))
                                gb->regs->a += 0x06;
                            if ((gb->regs->a > 0x99) || get_c(gb->regs))
                                gb->regs->a += 0x60;
                            if (gb->regs->a == 0)
                                set_c(gb->regs);
                            else
                                clear_c(gb->regs);
                        break;

                        case 5:  // cpl
                            gb->regs->a = ~gb->regs->a;
                            set_h(gb->regs);
                        break;

                        case 6:  // scf
                            set_c(gb->regs);
                            clear_h(gb->regs);
                        break;

                        case 7:  // ccf
                            if (get_c(gb->regs))
                                clear_c(gb->regs);
                            else
                                set_c(gb->regs);
                            clear_h(gb->regs);
                        break;
                    }
                break;
            }
        break;

        case 0x40:  // Block 1 (ld r8, r8)
            unsigned char r8_source;
            unsigned char *r8_dest;
            // Iterate over values of source r8
            switch (opcode & 0x07) {
                case 0:  // r8 = b
                    r8_source = gb->regs->b;
                break;

                case 1:  // r8 = c
                    r8_source = gb->regs->c;
                break;

                case 2:  // r8 = d
                    r8_source = gb->regs->d;
                break;

                case 3:  // r8 = e
                    r8_source = gb->regs->e;
                break;

                case 4:  // r8 = h
                    r8_source = gb->regs->h;
                break;

                case 5:  // r8 = l
                    r8_source = gb->regs->l;
                break;

                case 6:  // r8 = [hl]
                    r8_source = read_memory(gb->memory, get_hl(gb->regs));
                break;

                case 7:  // r8 = a
                    r8_source = gb->regs->a;
                break;
            }

            // Iterate over values of dest r8
            switch ((opcode >> 3) & 0x07) {
                case 0:  // r8_dest = b
                    gb->regs->b = r8_source;
                break;

                case 1:  // r8_dest = c
                    gb->regs->c = r8_source;
                break;

                case 2:  // r8_dest = d
                    gb->regs->d = r8_source;
                break;

                case 3:  // r8_dest = e
                    gb->regs->e = r8_source;
                break;

                case 4:  // r8_dest = h
                    gb->regs->h = r8_source;
                break;

                case 5:  // r8_dest = l
                    gb->regs->l = r8_source;
                break;

                case 6:  // r8_dest = [hl]
                    if ((opcode & 0x07) == 6) {
                        // TODO: HALT
                    } else {
                        write_memory(gb->memory, get_hl(gb->regs), r8_source);
                    }
                break;

                case 7:  // r8_dest = a
                    gb->regs->a = r8_source;
                break;
            }
        break;

        case 0x80:  // Block 2
            // We simply need to iterate over bits 3-5
            switch ((opcode >> 3) & 0x07) {
                case 0:  // add a, r8
                    // Iterate over possible values of r8
                    unsigned short val;
                    switch (opcode & 0x07) {
                        case 0:  // r8 = b
                            val = gb->regs->a + gb->regs->b;

                            // Update flags
                            clear_c(gb->regs);
                            clear_z(gb->regs);
                            clear_h(gb->regs);
                            clear_n(gb->regs);
                            
                            if ((val & 0xFF) == 0)
                                set_z(gb->regs);
                            if ((gb->regs->a & 0x0F) + (gb->regs->b & 0x0F) > 0x0F)
                                set_h(gb->regs);
                            if (val > 0xFF)
                                set_c(gb->regs);
                            gb->regs->a = val & 0xFF;
                        break;

                        case 1:  // r8 = c
                            val = gb->regs->a + gb->regs->c;

                            // Update flags
                            clear_c(gb->regs);
                            clear_z(gb->regs);
                            clear_h(gb->regs);
                            clear_n(gb->regs);
                            
                            if ((val & 0xFF) == 0)
                                set_z(gb->regs);
                            if ((gb->regs->a & 0x0F) + (gb->regs->c & 0x0F) > 0x0F)
                                set_h(gb->regs);
                            if (val > 0xFF)
                                set_c(gb->regs);
                            gb->regs->a = val & 0xFF;
                        break;

                        case 2:  // r8 = d
                            val = gb->regs->a + gb->regs->d;

                            // Update flags
                            clear_c(gb->regs);
                            clear_z(gb->regs);
                            clear_h(gb->regs);
                            clear_n(gb->regs);
                            
                            if ((val & 0xFF) == 0)
                                set_z(gb->regs);
                            if ((gb->regs->a & 0x0F) + (gb->regs->d & 0x0F) > 0x0F)
                                set_h(gb->regs);
                            if (val > 0xFF)
                                set_c(gb->regs);
                            gb->regs->a = val & 0xFF;
                        break;

                        case 3:  // r8 = e
                            val = gb->regs->a + gb->regs->e;

                            // Update flags
                            clear_c(gb->regs);
                            clear_z(gb->regs);
                            clear_h(gb->regs);
                            clear_n(gb->regs);
                            
                            if ((val & 0xFF) == 0)
                                set_z(gb->regs);
                            if ((gb->regs->a & 0x0F) + (gb->regs->e & 0x0F) > 0x0F)
                                set_h(gb->regs);
                            if (val > 0xFF)
                                set_c(gb->regs);
                            gb->regs->a = val & 0xFF;
                        break;

                        case 4:  // r8 = h
                            val = gb->regs->a + gb->regs->h;

                            // Update flags
                            clear_c(gb->regs);
                            clear_z(gb->regs);
                            clear_h(gb->regs);
                            clear_n(gb->regs);
                            
                            if ((val & 0xFF) == 0)
                                set_z(gb->regs);
                            if ((gb->regs->a & 0x0F) + (gb->regs->h & 0x0F) > 0x0F)
                                set_h(gb->regs);
                            if (val > 0xFF)
                                set_c(gb->regs);
                            gb->regs->a = val & 0xFF;
                        break;

                        case 5:  // r8 = l
                            val = gb->regs->a + gb->regs->l;

                            // Update flags
                            clear_c(gb->regs);
                            clear_z(gb->regs);
                            clear_h(gb->regs);
                            clear_n(gb->regs);
                            
                            if ((val & 0xFF) == 0)
                                set_z(gb->regs);
                            if ((gb->regs->a & 0x0F) + (gb->regs->l & 0x0F) > 0x0F)
                                set_h(gb->regs);
                            if (val > 0xFF)
                                set_c(gb->regs);
                            gb->regs->a = val & 0xFF;
                        break;

                        case 6:  // r8 = [hl]
                            val = gb->regs->a + read_memory(gb->memory, get_hl(gb->regs));

                            // Update flags
                            clear_c(gb->regs);
                            clear_z(gb->regs);
                            clear_h(gb->regs);
                            clear_n(gb->regs);
                            
                            if ((val & 0xFF) == 0)
                                set_z(gb->regs);
                            if ((gb->regs->a & 0x0F) + (read_memory(gb->memory, get_hl(gb->regs)) & 0x0F) > 0x0F)
                                set_h(gb->regs);
                            if (val > 0xFF)
                                set_c(gb->regs);
                            gb->regs->a = val & 0xFF;
                        break;

                        case 7:  // r8 = a
                            val = gb->regs->a + gb->regs->a;

                            // Update flags
                            clear_c(gb->regs);
                            clear_z(gb->regs);
                            clear_h(gb->regs);
                            clear_n(gb->regs);
                            
                            if ((val & 0xFF) == 0)
                                set_z(gb->regs);
                            if ((gb->regs->a & 0x0F) + (gb->regs->a & 0x0F) > 0x0F)
                                set_h(gb->regs);
                            if (val > 0xFF)
                                set_c(gb->regs);
                            gb->regs->a = val & 0xFF;
                        break;
                    }
                break;

                case 1:  // adc a, r8
                break;

                case 2:
                break;

                case 3:
                break;

                case 4:
                break;

                case 5:
                break;

                case 6:
                break;

                case 7:
                break;
            }

        break;

        case 0xC0:  // Block 3
        break;
    }
}
