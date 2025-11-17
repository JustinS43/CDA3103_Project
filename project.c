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
    *Zero = (*ALUresult == 0) ? 1 : 0;

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
int instruction_decode(unsigned op,struct_controls *controls){
    controls->RegDst = 2;
    controls->Jump = 0;
    controls->Branch   = 0;
    controls->MemRead  = 0;
    controls->MemtoReg = 2;
    controls->ALUOp    = 0;   
    controls->MemWrite = 0;
    controls->ALUSrc   = 0;
    controls->RegWrite = 0;

    switch(op) {

    case 0x00:
        controls->RegDst =1;
        controls->RegWrite = 1;
        controls->ALUOp=7;
        break;
    case 0x23:
        controls->RegDst   = 0;
        controls->MemRead  = 1;
        controls->MemtoReg = 1;
        controls->ALUOp    = 0;
        controls->ALUSrc   = 1;
        controls->RegWrite = 1;
        break;
    case 0x2B:  /* sw */
        controls->MemWrite = 1;
        controls->ALUOp    = 0;
        controls->ALUSrc   = 1;
        controls->RegDst   = 2;
        controls->MemtoReg = 2;
        controls->RegWrite = 0;
        break;
    case 0x04:  /* beq */
        controls->Branch   = 1;
        controls->ALUOp    = 1;
        controls->ALUSrc   = 0;
        controls->RegDst   = 2;
        controls->MemtoReg = 2;
        controls->RegWrite = 0;
        break;
    case 0x02:  /* j */
        controls->Jump     = 1;
        controls->RegDst   = 2;
        controls->MemtoReg = 2;
        controls->ALUOp    = 0;
        controls->ALUSrc   = 2;
        controls->RegWrite = 0;
        break;
    case 0x08:  /* addi */
        controls->RegDst   = 0;
        controls->MemtoReg = 0;
        controls->ALUOp    = 0;
        controls->ALUSrc   = 1;
        controls->RegWrite = 1;
        break;
    case 0x0A:  /* slti */
        controls->RegDst   = 0;
        controls->MemtoReg = 0;
        controls->ALUOp    = 2;
        controls->ALUSrc   = 1;
        controls->RegWrite = 1;
        break;
    case 0x0B:  /* sltiu */
        controls->RegDst   = 0;
        controls->MemtoReg = 0;
        controls->ALUOp    = 3;
        controls->ALUSrc   = 1;
        controls->RegWrite = 1;
        break;
    case 0x0C:  /* andi */
        controls->RegDst   = 0;
        controls->MemtoReg = 0;
        controls->ALUOp    = 4;
        controls->ALUSrc   = 1;
        controls->RegWrite = 1;
        break;
    case 0x0D:  /* ori */
        controls->RegDst   = 0;
        controls->MemtoReg = 0;
        controls->ALUOp    = 5;
        controls->ALUSrc   = 1;
        controls->RegWrite = 1;
        break;
    case 0x0F:  /* lui */
        controls->RegDst   = 0;
        controls->MemtoReg = 0;
        controls->ALUOp    = 6;
        controls->ALUSrc   = 1;
        controls->RegWrite = 1;
        break;
    default:
        return 1;
    }
    return 0;
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
    unsigned operand2;
    char control = 0;

    /* choose second operand based on ALUSrc */
    operand2 = (ALUSrc) ? extended_value : data2;

    if (ALUOp == 7)
    {
        /* R-type – decode based on funct field */
        switch (funct)
        {
            case 0x20: /* add */
                control = 0;
                break;
            case 0x22: /* sub */
                control = 1;
                break;
            case 0x2A: /* slt (signed) */
                control = 2;
                break;
            case 0x2B: /* sltu (unsigned) */
                control = 3;
                break;
            case 0x24: /* and */
                control = 4;
                break;
            case 0x25: /* or */
                control = 5;
                break;
            default:
                /* unsupported R-type instruction */
                return 1;
        }
    }
    else
    {
        /* directly use ALUOp encoding */
        control = ALUOp;
    }

    ALU(data1, operand2, control, ALUresult, Zero);
    return 0;
}

/* Read / Write Memory */
/* 10 Points */
int rw_memory(unsigned ALUresult,unsigned data2,char MemWrite,char MemRead,unsigned *memdata,unsigned *Mem){
    if (MemRead)
    {
        /* address must be aligned and in range */
        if ((ALUresult & 0x3) != 0 || ALUresult > 0xFFFF)
            return 1;

        *memdata = MEM(ALUresult);
    }

    if (MemWrite)
    {
        if ((ALUresult & 0x3) != 0 || ALUresult > 0xFFFF)
            return 1;

        MEM(ALUresult) = data2;
}


/* Write Register */
/* 10 Points */
void write_register(unsigned r2,unsigned r3,unsigned memdata,unsigned ALUresult,char RegWrite,char RegDst,char MemtoReg,unsigned *Reg){
    if (!RegWrite)
            return;

        unsigned dest = (RegDst) ? r3 : r2;
        unsigned value = (MemtoReg) ? memdata : ALUresult;

        Reg[dest] = value;
}

/* PC update */
/* 10 Points */
void PC_update(unsigned jsec,unsigned extended_value,char Branch,char Jump,char Zero,unsigned *PC)
{
/* default: move to next sequential instruction */
    *PC += 4;

    /* branch: BEQ using Zero flag */
    if (Branch && Zero)
    {
        /* offset is in words – shift left by 2 to convert to bytes */
        *PC += (extended_value << 2);
    }

    /* jump */
    if (Jump)
    {
        unsigned upper  = (*PC) & 0xF0000000;
        unsigned target = jsec << 2;
        *PC = upper | target;
    }
}

