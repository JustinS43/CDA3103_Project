#include "spimcore.h"


/* ALU */
/* 10 Points */
void ALU(unsigned A,unsigned B,char ALUControl,unsigned *ALUresult,char *Zero){
    if (ALUControl == 0) {
        // ADD
        *ALUresult = A + B;
    }
    else if (ALUControl == 1) {
        // SUB
        *ALUresult = A - B;
    }
    else if (ALUControl == 2) {
        // SLT
        *ALUresult = (A < B) ? 1 : 0;
    }
    else if (ALUControl == 3) {
        // SHIFT LEFT B by 16
        *ALUresult = B << 16;
    }
    else if (ALUControl == 4) {
        // AND
        *ALUresult = A & B;
    }
    else if (ALUControl == 5) {
        // OR
        *ALUresult = A | B;
    }
    else if (ALUControl == 6) {
        // NOR
        *ALUresult = ~(A | B);
    }
    else if (ALUControl == 7) {
        // MULTIPLY
        *ALUresult = A * B;
    }
    else {
        // Invalid ALUControl
        *ALUresult = 0;
    }

    // Zero flag logic
    *Zero = (*ALUresult == 0) ? 1 : 0

}

/* instruction fetch */
/* 10 Points */
int instruction_fetch(unsigned PC,unsigned *Mem,unsigned *instruction){
    // Halt if PC is out of memory range (64 KB = 0x0000 to 0xFFFF)
    if (PC > 0xFFFF)
        return 1;

    // PC must be word aligned
    if (PC % 4 != 0)
        return 1;

    // Convert byte address to word index
    unsigned index = PC >> 2;

    // Fetch the instruction
    *instruction = Mem[index];

    return 0; // no halt

}


/* instruction partition */
/* 10 Points */
void instruction_partition(unsigned instruction, unsigned *op, unsigned *r1,unsigned *r2, unsigned *r3, unsigned *funct, unsigned *offset, unsigned *jsec){
    *op     = (instruction >> 26) & 0x3F;      // 6 bits
    *r1     = (instruction >> 21) & 0x1F;      // 5 bits
    *r2     = (instruction >> 16) & 0x1F;      // 5 bits
    *r3     = (instruction >> 11) & 0x1F;      // 5 bits
    *funct  =  instruction        & 0x3F;      // 6 bits
    *offset =  instruction        & 0xFFFF;    // 16 bits
    *jsec   =  instruction        & 0x3FFFFFF; // 26 bits
}



/* instruction decode */
/* 15 Points */
int instruction_decode(unsigned op,struct_controls *controls)
{

}

/* Read Register */
/* 5 Points */
void read_register(unsigned r1,unsigned r2,unsigned *Reg,unsigned *data1,unsigned *data2)
{

}


/* Sign Extend */
/* 10 Points */
void sign_extend(unsigned offset,unsigned *extended_value)
{

}

/* ALU operations */
/* 10 Points */
int ALU_operations(unsigned data1,unsigned data2,unsigned extended_value,unsigned funct,char ALUOp,char ALUSrc,unsigned *ALUresult,char *Zero)
{

}

/* Read / Write Memory */
/* 10 Points */
int rw_memory(unsigned ALUresult,unsigned data2,char MemWrite,char MemRead,unsigned *memdata,unsigned *Mem)
{

}


/* Write Register */
/* 10 Points */
void write_register(unsigned r2,unsigned r3,unsigned memdata,unsigned ALUresult,char RegWrite,char RegDst,char MemtoReg,unsigned *Reg)
{

}

/* PC update */
/* 10 Points */
void PC_update(unsigned jsec,unsigned extended_value,char Branch,char Jump,char Zero,unsigned *PC)
{

}

