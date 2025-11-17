#include "spimcore.h"


/* ALU */
/* 10 Points */
void ALU(unsigned A,unsigned B,char ALUControl,unsigned *ALUresult,char *Zero)
{

}

/* instruction fetch */
/* 10 Points */
int instruction_fetch(unsigned PC,unsigned *Mem,unsigned *instruction)
{

}


/* instruction partition */
/* 10 Points */
void instruction_partition(unsigned instruction, unsigned *op, unsigned *r1,unsigned *r2, unsigned *r3, unsigned *funct, unsigned *offset, unsigned *jsec)
{

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
int rw_memory(unsigned ALUresult,unsigned data2,char MemWrite,char MemRead,unsigned *memdata,unsigned *Mem)
{
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
void write_register(unsigned r2,unsigned r3,unsigned memdata,unsigned ALUresult,char RegWrite,char RegDst,char MemtoReg,unsigned *Reg)
{
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

