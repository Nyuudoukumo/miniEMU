#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include "miniEMU.h"
#include "dbg.h"

XLEN PC = 0;

// RV32E has 16 rigisters, and the rigisters x0 is hardwired with bits equal to 0
XLEN R[16] = {0};

uint8_t M[MEM_SIZE] = {
    0x13,0x05,0x40,0x01,
    0xe7,0x00,0x00,0x01,
    0xe7,0x00,0xc0,0x00,
    0x67,0x00,0xc0,0x00,
    0x13,0x05,0xa5,0x00,
    0x67,0x80,0x00,0x00
};

uint8_t PC_UPDATE = 1;

decoded_inst_t decode(XLEN inst)
{
    decoded_inst_t decInst = {0};
    decInst.inst = inst;
    decInst.opcode = inst & 0x7F;

    switch(decInst.opcode) {
        // I_TYPE
        case 0b0010011: // OP_IMM
        case 0b1100111: // JALR
        case 0b0000011: // LOAD
        {
            decInst.format = I_TYPE;
            decInst.rd = (inst >> 7) & 0x1F;
            decInst.rs1 = (inst >> 15) & 0x1F;
            decInst.funct3 = (inst >> 12) & 0x7;
            uint32_t imm = inst >> 20;
            decInst.imm = (imm & 0x800) ? (imm | 0xFFFFF000) : imm;
            break;
        }
        // S_TYPE
        case 0b0100011: // STORE
        {
            decInst.format = S_TYPE;
            decInst.rs1 = (inst >> 15) & 0x1F;
            decInst.rs2 = (inst >> 20) & 0x1F;
            decInst.funct3 = (inst >> 12) & 0x7;
            uint32_t imm = ((inst >> 7) & 0x1F) | (((inst >> 25) & 0x7F) << 5);
            decInst.imm = (imm & 0x800) ? (imm | 0xFFFFF000) : imm;
            break;
        }
        // R_TYPE
        case 0b0110011: // OP
        {
            decInst.format = R_TYPE;
            decInst.rd = (inst >> 7) & 0x1F;
            decInst.rs1 = (inst >> 15) & 0x1F;
            decInst.rs2 = (inst >> 20) & 0x1F;
            decInst.funct3 = (inst >> 12) & 0x7;
            decInst.funct7 = (inst >> 25) & 0x7F;
            break;
        }
        // U_TYPE
        case 0b0110111: // LUI
        {
            decInst.format = U_TYPE;
            decInst.rd = (inst >> 7) & 0x1F;
            decInst.imm = inst & 0xFFFFF000;
            break;
        }
        default:
            sentinel("Error when decoding");
    }

return decInst;

error:
    exit(1);
}

static inline uint32_t read_reg(uint8_t idx) {
    return (idx == 0) ? 0 : R[idx];
}

static inline void write_reg(uint8_t idx, uint32_t val) {
    if (idx != 0) R[idx] = val;
}

static void ADDI(decoded_inst_t decInst)
{
    uint32_t val = decInst.imm + R(decInst.rs1);
    write_reg(decInst.rd, val);
}

static void JALR(decoded_inst_t decInst)
{
    uint32_t next_pc = PC + 4;
    // `& ~1` 用于字节对齐（测试程序不会体现）.
    PC = ((uint32_t)decInst.imm + R(decInst.rs1)) & ~1;
    write_reg(decInst.rd, next_pc);
    PC_UPDATE = 0;
}

static void LW(decoded_inst_t decInst)
{
    uint32_t addr = decInst.imm + R(decInst.rs1);
    uint32_t val = (uint32_t)M[addr] | (uint32_t)M[addr + 1] << 8 | (uint32_t)M[addr + 2] << 16 | (uint32_t)M[addr + 3] << 24;
    write_reg(decInst.rd, val);
}

static void LBU(decoded_inst_t decInst)
{
    uint32_t addr = decInst.imm + R(decInst.rs1);
    uint32_t val = 0 | (uint32_t)M[addr];
    write_reg(decInst.rd, val);
}

static void SW(decoded_inst_t decInst)
{
    uint32_t addr = decInst.imm + R(decInst.rs1);
    M[addr] = R(decInst.rs2) & 0xFF;
    M[addr + 1] = (R(decInst.rs2) >> 8) & 0xFF;
    M[addr + 2] = (R(decInst.rs2) >> 16) & 0xFF;
    M[addr + 3] = (R(decInst.rs2) >> 24) & 0xFF;
}

static void SB(decoded_inst_t decInst)
{
    uint32_t addr = decInst.imm + R(decInst.rs1);
    M[addr] = R(decInst.rs2) & 0xFF;
}

static void ADD(decoded_inst_t decInst)
{
    write_reg(decInst.rd, R(decInst.rs1) + R(decInst.rs2));
}

static void LUI(decoded_inst_t decInst)
{
    write_reg(decInst.rd, decInst.imm);
}

static void ex_I(decoded_inst_t decInst)
{
    if(decInst.opcode == 0b0010011){ // OP_IMM
        switch(decInst.funct3) {
            case 0x0: // ADDI
                ADDI(decInst);
                break;
            default:
                sentinel("Error instruction!\n");
        }
    } else if(decInst.opcode == 0b1100111) {
        if(decInst.funct3 == 0x0) {
            // JALR
            JALR(decInst);
        } else {
            sentinel("Error instruction!\n");
        }
    } else if(decInst.opcode == 0b0000011) {
        switch(decInst.funct3) {
            case 0x2: // lw
                LW(decInst);
                break;
            case 0x4: // lbu
                LBU(decInst);
                break;
            default:
                sentinel("Error instruction!\n");
        }
    } else {
        sentinel("Error opcode!\n");
    }
    return;
error:
    exit(1);
}

static void ex_S(decoded_inst_t decInst)
{
    if(decInst.opcode == 0b0100011) {
        switch(decInst.funct3) {
            case 0x0:
                SB(decInst);
                break;
            case 0x2:
                SW(decInst);
                break;
            default:
                sentinel("Error instruction!\n");
        }
    } else {
            sentinel("Error opcode!\n");
    }
    return;
error:
    exit(1);
}

static void ex_R(decoded_inst_t decInst)
{
    switch(decInst.opcode) {
        case 0b0110011:
            if(decInst.funct3 == 0x0 && decInst.funct7 == 0x00) { // ADD
                ADD(decInst);
            } else {
                sentinel("Error instruction!\n");
            }
            break;
        default:
            sentinel("Error opcode!\n");
    }
    return;
error:
    exit(1);
}

static void ex_U(decoded_inst_t decInst)
{
    switch(decInst.opcode){
        case 0b0110111: // LUI
            LUI(decInst);
            break;
        default:
            sentinel("Error opcode!\n");
    }
    return;
error:
    exit(1);
}

void inst_cycle()
{
    // FI
    const XLEN inst = (uint32_t)M[PC] | (uint32_t)M[PC + 1] << 8 | (uint32_t)M[PC + 2] << 16 | (uint32_t)M[PC + 3] << 24;
    decoded_inst_t decInst =  decode(inst);
    PC_UPDATE = 1;

    switch(decInst.format) {
        case I_TYPE:
            ex_I(decInst);
            break;
        case S_TYPE:
            ex_S(decInst);
            break;
        case R_TYPE:
            ex_R(decInst);
            break;
        case U_TYPE:
            ex_U(decInst);
            break;
        default:
            sentinel("Error instruction type! \n");
    }
    // update pc
    if(PC_UPDATE) {
        PC = PC + 4;
    }

    return;
error:
    exit(1);
}



// int main(int argc, char* argv[])
// {
//     int i = 0;
//     // check(argc == 2, "Use: sEMU num");
//     // int r = atoi(argv[1]);
//     // R[0] = r;
//     while (i < 100) {
//         inst_cycle();
//         i++;
//     }
//     printf("x10: %d\n", R[10]);
//     return 0;
// }
