#include "minunit.h"
#include "miniEMU.h"
#include <assert.h>
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>

static void reset_emu(void)
{
    PC = 0;
    PC_UPDATE = 1;
    terminate = 0;
    memset(R, 0, sizeof(R));
    memset(M, 0, sizeof(M));
}

char *test_addi_jalr()
{
    uint8_t program[] = {
        0x13, 0x05, 0x40, 0x01,
        0xe7, 0x00, 0x00, 0x01,
        0xe7, 0x00, 0xc0, 0x00,
        0x73, 0x00, 0x10, 0x00,
        0x13, 0x05, 0x00, 0x00,
        0x67, 0x80, 0x00, 0x00,
    };

    reset_emu();
    memcpy(M, program, sizeof(program));

    while (!terminate) {
        inst_cycle();
    }

    mu_assert(R[10] == 0, "Hit bad trap!\n");
    return NULL;
}

char *test_sum()
{
    reset_emu();

    FILE *fp = fopen("logisim-bin/sum.bin", "r");
    assert(fp != NULL);

    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    rewind(fp);
    fread(M, 1, size, fp);
    fclose(fp);

    uint8_t inst1[] = { 0x13, 0x05, 0x00, 0x00 }; // Addi a0 zero 0
    uint8_t inst2[] = { 0x73, 0x00, 0x10, 0x00 }; // EBREAK

    memcpy(M+0x244, inst1, 4);
    memcpy(M+0x248, inst2, 4);

    while (!terminate) {
        inst_cycle();
    }

    log_info("PC is 0x%x", PC);
    log_info("a0 is %u", R[10]);
    mu_assert(R[10] == 0, "a0 is NOT 0");
    return NULL;
}

char *test_mem()
{
    reset_emu();

    FILE *fp = fopen("logisim-bin/mem.bin", "r");
    assert(fp != NULL);

    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    rewind(fp);
    fread(M, 1, size, fp);
    fclose(fp);
    uint8_t inst1[] = { 0x13, 0x05, 0x00, 0x00 }; // Addi a0 zero 0
    uint8_t inst2[] = { 0x73, 0x00, 0x10, 0x00 }; // EBREAK

    memcpy(M+0x1218, inst1, 4);
    memcpy(M+0x121c, inst2, 4);
    while (!terminate) {
        inst_cycle();
    }

    log_info("PC is 0x%x", PC);
    log_info("a0 is %u", R[10]);
    mu_assert(R[10] == 0, "a0 is NOT 0");
    return NULL;
}

char *all_tests()
{
    mu_suite_start();
    mu_run_test(test_addi_jalr);
    mu_run_test(test_sum);
    mu_run_test(test_mem);
    return NULL;
}

RUN_TESTS(all_tests);
