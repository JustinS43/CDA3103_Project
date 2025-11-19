#include "spimcore.h"

/* ALU */
void ALU(unsigned A,unsigned B,char ALUControl,unsigned *ALUresult,char *Zero)
{
    switch (ALUControl)
    {
        case 0: /* A + B */
            *ALUresult = A + B;
            break;
        case 1: /* A - B */
            *ALUresult = A - B;
            break;
        case 2: /* signed set less than */
            *ALUresult = ((int)A < (int)B) ? 1u : 0u;
            break;
        case 3: /* unsigned set less than */
            *ALUresult = (A < B) ? 1u : 0u;
            break;
        case 4: /* A AND B */
            *ALUresult = A & B;
            break;
        case 5: /* A OR B */
            *ALUresult = A | B;
            break;
        case 6: /* B << 16 (used for LUI) */
            *ALUresult = B << 16;
            break;
        case 7: /* NOT A */
            *ALUresult = ~A;
            break;
        default: /* undefined control – safe no-op */
            *ALUresult = 0;
            break;
    }

    /* Zero flag */
    *Zero = (*ALUresult == 0) ? 1 : 0;
}

/* instruction fetch */
int instruction_fetch(unsigned PC,unsigned *Mem,unsigned *instruction)
{
    /* halt if address is out of range or not word aligned */
    if ((PC & 0x3) != 0 || PC > 0xFFFF)
        return 1;

    *instruction = MEM(PC);

    /* halt on illegal 0x00000000 instruction */
    if (*instruction == 0x00000000)
        return 1;

    return 0;
}

/* instruction partition */
void instruction_partition(unsigned instruction,
                           unsigned *op,
                           unsigned *r1,
                           unsigned *r2,
                           unsigned *r3,
                           unsigned *funct,
                           unsigned *offset,
                           unsigned *jsec)
{
    *op     = (instruction >> 26) & 0x3F;      /* bits 31-26 */
    *r1     = (instruction >> 21) & 0x1F;      /* bits 25-21 (rs) */
    *r2     = (instruction >> 16) & 0x1F;      /* bits 20-16 (rt) */
    *r3     = (instruction >> 11) & 0x1F;      /* bits 15-11 (rd) */
    *funct  =  instruction        & 0x3F;      /* bits 5-0   */
    *offset =  instruction        & 0xFFFF;    /* bits 15-0  */
    *jsec   =  instruction        & 0x3FFFFFF; /* bits 25-0  */
}

/* instruction decode */
int instruction_decode(unsigned op,struct_controls *controls)
{
    /* default everything to 0 (safe "do nothing") */
    controls->RegDst   = 0;
    controls->Jump     = 0;
    controls->Branch   = 0;
    controls->MemRead  = 0;
    controls->MemtoReg = 0;
    controls->ALUOp    = 0;
    controls->MemWrite = 0;
    controls->ALUSrc   = 0;
    controls->RegWrite = 0;

    switch (op)
    {
        case 0x00: /* R-type: add, sub, and, or, slt, sltu */
            controls->RegDst   = 1;
            controls->RegWrite = 1;
            controls->ALUSrc   = 0;
            controls->ALUOp    = 7;   /* decode by funct in ALU_operations */
            break;

        case 0x23: /* lw */
            controls->RegDst   = 0;
            controls->MemRead  = 1;
            controls->MemtoReg = 1;
            controls->ALUOp    = 0;   /* add base + offset */
            controls->MemWrite = 0;
            controls->ALUSrc   = 1;   /* immediate */
            controls->RegWrite = 1;
            break;

        case 0x2B: /* sw */
            controls->MemRead  = 0;
            controls->MemWrite = 1;
            controls->ALUSrc   = 1;
            controls->ALUOp    = 0;   /* add base + offset */
            controls->RegWrite = 0;
            break;

        case 0x04: /* beq */
            controls->Branch   = 1;
            controls->ALUSrc   = 0;
            controls->ALUOp    = 1;   /* subtraction to set Zero */
            controls->RegWrite = 0;
            break;

        case 0x08: /* addi */
            controls->RegDst   = 0;
            controls->RegWrite = 1;
            controls->ALUSrc   = 1;
            controls->ALUOp    = 0;   /* add */
            break;

        case 0x0A: /* slti */
            controls->RegDst   = 0;
            controls->RegWrite = 1;
            controls->ALUSrc   = 1;
            controls->ALUOp    = 2;   /* set less than signed */
            break;

        case 0x0B: /* sltiu */
            controls->RegDst   = 0;
            controls->RegWrite = 1;
            controls->ALUSrc   = 1;
            controls->ALUOp    = 3;   /* set less than unsigned */
            break;

        case 0x0C: /* andi */
            controls->RegDst   = 0;
            controls->RegWrite = 1;
            controls->ALUSrc   = 1;
            controls->ALUOp    = 4;   /* AND */
            break;

        case 0x0D: /* ori */
            controls->RegDst   = 0;
            controls->RegWrite = 1;
            controls->ALUSrc   = 1;
            controls->ALUOp    = 5;   /* OR */
            break;

        case 0x0F: /* lui */
            controls->RegDst   = 0;
            controls->RegWrite = 1;
            controls->ALUSrc   = 1;
            controls->ALUOp    = 6;   /* shift left 16 */
            break;

        case 0x02: /* j */
            controls->Jump     = 1;
            /* all other controls remain 0 */
            break;

        default:
            /* illegal / unsupported instruction */
            return 1;
    }

    return 0;
}

/* Read Register */
void read_register(unsigned r1,unsigned r2,unsigned *Reg,unsigned *data1,unsigned *data2)
{
    *data1 = Reg[r1];
    *data2 = Reg[r2];
}

/* Sign Extend */
void sign_extend(unsigned offset,unsigned *extended_value)
{
    if (offset & 0x8000)  /* negative number */
        *extended_value = offset | 0xFFFF0000;
    else
        *extended_value = offset & 0x0000FFFF;
}

/* ALU operations */
int ALU_operations(unsigned data1,unsigned data2,unsigned extended_value,
                   unsigned funct,char ALUOp,char ALUSrc,
                   unsigned *ALUresult,char *Zero)
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
int rw_memory(unsigned ALUresult,unsigned data2,char MemWrite,char MemRead,
              unsigned *memdata,unsigned *Mem)
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

    return 0;
}

/* Write Register */
void write_register(unsigned r2,unsigned r3,unsigned memdata,unsigned ALUresult,
                    char RegWrite,char RegDst,char MemtoReg,unsigned *Reg)
{
    if (!RegWrite)
        return;

    unsigned dest = (RegDst) ? r3 : r2;
    unsigned value = (MemtoReg) ? memdata : ALUresult;

    Reg[dest] = value;
}

/* PC update */
void PC_update(unsigned jsec,unsigned extended_value,char Branch,char Jump,
               char Zero,unsigned *PC)
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