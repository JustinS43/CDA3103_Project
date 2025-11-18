#include "spimcore.h"

// ALU
void ALU(unsigned A,unsigned B,char ALUControl,unsigned *ALUresult,char *Zero){

    switch (ALUControl){
        case 0:  // add
            *ALUresult = A + B;
            break;
        case 1:  // sub
            *ALUresult = A - B;
            break;
        case 2:  // slt (signed)
            *ALUresult = ((int)A < (int)B) ? 1 : 0;
            break;
        case 3:  // sltu (unsigned)
            *ALUresult = (A < B) ? 1 : 0;
            break;
        case 4:  // and
            *ALUresult = A & B;
            break;
        case 5:  // or
            *ALUresult = A | B;
            break;
        case 6:  // shift left B by 16 bits
            *ALUresult = B << 16;
            break;
        case 7:  // not A
            *ALUresult = ~A;
            break;
        default: // invalid control
            *ALUresult = 0;
            break;
    }

    // Zero flag
    *Zero = (*ALUresult == 0) ? 1 : 0;
}

// instruction fetch
int instruction_fetch(unsigned PC,unsigned *Mem,unsigned *instruction){
    // Halt if PC is beyond memory (0x0000–0xFFFF)
    if (PC > 0xFFFF)
        return 1;

    // PC must be word-aligned (multiple of 4)
    if (PC % 4 != 0)
        return 1;

    // Convert byte address to word index
    unsigned index = PC >> 2;

    // Fetch instruction
    *instruction = Mem[index];

    return 0;
}

// instruction partition
void instruction_partition(unsigned instruction, unsigned *op, unsigned *r1,unsigned *r2, unsigned *r3, unsigned *funct,
unsigned *offset, unsigned *jsec){
    *op     = (instruction >> 26) & 0x3F;
    *r1     = (instruction >> 21) & 0x1F;
    *r2     = (instruction >> 16) & 0x1F;
    *r3     = (instruction >> 11) & 0x1F;
    *funct  =  instruction & 0x3F;
    *offset =  instruction & 0xFFFF;
    *jsec   =  instruction & 0x3FFFFFF;
}

// instruction decode - Justin Suriel
int instruction_decode(unsigned op, struct_controls *controls){
    // default
    controls->RegDst   = 2;
    controls->Jump     = 0;
    controls->Branch   = 0;
    controls->MemRead  = 0;
    controls->MemtoReg = 2;
    controls->ALUOp    = 0;
    controls->MemWrite = 0;
    controls->ALUSrc   = 0;
    controls->RegWrite = 0;

    switch (op){
        case 0x00:  // R-type
            controls->RegDst   = 1;
            controls->RegWrite = 1;
            controls->ALUOp    = 7;   // R-type, ALU_operations uses funct
            break;

        case 0x23:  // lw
            controls->RegDst   = 0;   // rt
            controls->MemRead  = 1;
            controls->MemtoReg = 1;   // from memory
            controls->ALUOp    = 0;   // add
            controls->ALUSrc   = 1;   // immediate
            controls->RegWrite = 1;
            break;

        case 0x2B:  // sw
            controls->MemWrite = 1;
            controls->ALUOp    = 0;   // add
            controls->ALUSrc   = 1;   // immediate
            controls->RegDst   = 2;
            controls->MemtoReg = 2;
            controls->RegWrite = 0;
            break;

        case 0x04:  // beq
            controls->Branch   = 1;
            controls->ALUOp    = 1;   // subtraction
            controls->ALUSrc   = 0;   // register
            controls->RegDst   = 2;
            controls->MemtoReg = 2;
            controls->RegWrite = 0;
            break;

        case 0x02:  // j
            controls->Jump     = 1;
            controls->RegDst   = 2;
            controls->MemtoReg = 2;
            controls->ALUOp    = 0;
            controls->ALUSrc   = 2;
            controls->RegWrite = 0;
            break;

        case 0x08:  // addi
            controls->RegDst   = 0;
            controls->MemtoReg = 0;   // from ALU
            controls->ALUOp    = 0;   // add
            controls->ALUSrc   = 1;   // immediate
            controls->RegWrite = 1;
            break;

        case 0x0A:  // slti
            controls->RegDst   = 0;
            controls->MemtoReg = 0;
            controls->ALUOp    = 2;   // slt
            controls->ALUSrc   = 1;
            controls->RegWrite = 1;
            break;

        case 0x0B:  // sltiu
            controls->RegDst   = 0;
            controls->MemtoReg = 0;
            controls->ALUOp    = 3;   // sltu
            controls->ALUSrc   = 1;
            controls->RegWrite = 1;
            break;

        case 0x0C:  // andi
            controls->RegDst   = 0;
            controls->MemtoReg = 0;
            controls->ALUOp    = 4;   // and
            controls->ALUSrc   = 1;
            controls->RegWrite = 1;
            break;

        case 0x0D:  // ori
            controls->RegDst   = 0;
            controls->MemtoReg = 0;
            controls->ALUOp    = 5;   // or
            controls->ALUSrc   = 1;
            controls->RegWrite = 1;
            break;

        case 0x0F:  // lui
            controls->RegDst   = 0;
            controls->MemtoReg = 0;
            controls->ALUOp    = 6;   // shift left 16
            controls->ALUSrc   = 1;
            controls->RegWrite = 1;
            break;

        default:
            return 1;
    }

    return 0;
}

// Read Register - Justin Suriel
void read_register(unsigned r1,unsigned r2,unsigned *Reg,unsigned *data1,unsigned *data2){
    // Read register r1 and r2 and output their values
    *data1 = Reg[r1];
    *data2 = Reg[r2];
}

// Sign Extend - Justin Suriel
void sign_extend(unsigned offset,unsigned *extended_value){
    // bit 15 is the sign bit of the 16-bit immediate
    unsigned signBit = 0x8000;

    if (offset & signBit){
        // negative: fill upper 16 bits with 1s
        *extended_value = offset | 0xFFFF0000;
    }else{
        // positive: keep lower 16 bits
        *extended_value = offset & 0x0000FFFF;
    }
}

// ALU operations - Alexander Walker
int ALU_operations(unsigned data1,unsigned data2,unsigned extended_value,unsigned funct,char ALUOp,char ALUSrc,
unsigned *ALUresult,char *Zero){
    unsigned operand2;
    char control = 0;

    // choose second operand based on ALUSrc
    if (ALUSrc == 0)
        operand2 = data2;
    else
        operand2 = extended_value;

    if (ALUOp == 7){
        // R-type – decode based on funct field
        switch (funct){
            case 0x20: // add
                control = 0;
                break;
            case 0x22: // sub
                control = 1;
                break;
            case 0x2A: // slt (signed)
                control = 2;
                break;
            case 0x2B: // sltu (unsigned)
                control = 3;
                break;
            case 0x24: // and
                control = 4;
                break;
            case 0x25: // or
                control = 5;
                break;
            default:
                // unsupported R-type instruction
                return 1;
        }
    }else{
        // Non–R-type: ALUOp directly selects ALU operation
        control = ALUOp;
    }

    ALU(data1, operand2, control, ALUresult, Zero);
    return 0;
}

// Read / Write Memory - Alexander Walker
int rw_memory(unsigned ALUresult,unsigned data2,char MemWrite,char MemRead,unsigned *memdata,unsigned *Mem){
    // Memory read
    if (MemRead){
        // address must be aligned and in range
        if ((ALUresult & 0x3) != 0 || ALUresult > 0xFFFF)
            return 1;

        // ALUresult is a byte address; convert to word index
        *memdata = Mem[ALUresult >> 2];
    }

    // Memory write
    if (MemWrite){
        if ((ALUresult & 0x3) != 0 || ALUresult > 0xFFFF)
            return 1;

        Mem[ALUresult >> 2] = data2;
    }
    return 0;
}

// Write Register - Alexander Walker
void write_register(unsigned r2, unsigned r3,unsigned memdata, unsigned ALUresult,char RegWrite, char RegDst, char MemtoReg,
unsigned *Reg){
    unsigned dest;
    unsigned value;

    // If RegWrite is 0, do not write anything
    if (RegWrite == 0){
        return;
    }

    /* Choose destination register:
       RegDst = 0 → r2 (rt)
       RegDst = 1 → r3 (rd)
    */

    if (RegDst == 0){
        dest = r2;
    }else{
        dest = r3;
    }

    /* Choose data source:
       MemtoReg = 0 → ALUresult
       MemtoReg = 1 → memdata
    */
    if (MemtoReg == 0){
        value = ALUresult;
    }else{
        value = memdata;
    }

    // Write the value into the destination register
    Reg[dest] = value;
}

// PC update - Alexander Walker
void PC_update(unsigned jsec,unsigned extended_value,char Branch,char Jump,char Zero,unsigned *PC){
    // Default: move to next sequential instruction
    *PC += 4;

    // Branch: BEQ using Zero flag
    if (Branch && Zero){
        // offset in words → shift left by 2 to convert to bytes
        *PC += (extended_value << 2);
    }

    // Jump
    if (Jump){
        unsigned upper  = (*PC) & 0xF0000000;
        unsigned target = (jsec << 2);
        *PC = upper | target;
    }
}
