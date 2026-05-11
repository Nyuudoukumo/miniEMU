#ifndef __miniEMU_h__
#define __miniEMU_h__

#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#define MEM_SIZE (1024 * 1024) // 1MB
#define R(idx) read_reg(idx)

typedef uint32_t XLEN;

extern XLEN PC;

// RV32E has 16 rigisters, and the rigisters x0 is hardwired with bits equal to 0
extern XLEN R[16];
extern int terminate;

extern uint8_t M[MEM_SIZE];
extern uint8_t screen[256*256*4];

extern uint8_t PC_UPDATE;

typedef struct {
    XLEN inst;
    uint8_t opcode;
    uint8_t rd;
    uint8_t rs1;
    uint8_t rs2;
    uint8_t funct3;
    uint8_t funct7;
    uint32_t imm;
    enum {
        R_TYPE,
        I_TYPE,
        S_TYPE,
        U_TYPE
    } format;
} decoded_inst_t;

void inst_cycle();

#endif // !__miniEMU_h__
