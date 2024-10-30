#ifndef CPU_H
#define CPU_H

#define ZERO_FLAG (1 << 7)
#define SUB_FLAG (1 << 6)
#define HALF_CARRY_FLAG (1 << 5)
#define CARRY_FLAG (1 << 4)


typedef struct {
    unsigned char a;
    unsigned char b;
    unsigned char c;
    unsigned char d;
    unsigned char e;
    unsigned char f;
    unsigned char h;
    unsigned char l;
    unsigned short sp;
    unsigned short pc;
} registers;


#endif
