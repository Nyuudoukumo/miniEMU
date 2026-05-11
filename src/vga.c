#include <am.h>
#include <klib-macros.h>
#include <stdlib.h>
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

void test_vga()
{
    reset_emu();

    FILE *fp = fopen("../logisim-bin/vga.bin", "r");
    assert(fp != NULL);

    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    rewind(fp);
    fread(M, 1, size, fp);
    fclose(fp);

    int i = 0;
    while (i<=630000) {
        inst_cycle();
        i++;
    }

    int w = 256;
    int h = 256;
    uint32_t color_buf[256*256];
    int j = 0;
    for (int i = 0; i < 256*256; i++) {
        j = i * 4;
        color_buf[i] = screen[j]| screen[j + 1] << 8 | screen[j + 2] << 16 | screen[j + 3] << 24;
    }
    io_write(AM_GPU_FBDRAW, 0, 0, color_buf, w, h, true);
    while(1) {}

    return;
}

int main()
{
    ioe_init();
    test_vga();
}
