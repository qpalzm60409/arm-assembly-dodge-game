/**************************************************************/
/*     File: C_func.c                                         */
/*  Purpose: Simple Application Subroutine                    */
/**************************************************************/

#include <stdio.h>

#ifdef __thumb
/* Define Angel Semihosting SWI to be Thumb one */
#define SemiSWI 0xAB
#else
/* Define Angel Semihosting SWI to be ARM one */
#define SemiSWI 0x123456
#endif

/* Write a string */
__swi(SemiSWI) void _Write0(unsigned op, char *string);
#define Write0(string) _Write0 (0x4,string)

extern ___weak void Load$$16bitRAM$$Base(void);
extern ___weak void Image$$16bitRAM$$Base(void);
extern ___weak void Image$$16bitRAM$$Length(void);
extern ___weak void Image$$16bitRAM$$ZI$$Base(void);
extern ___weak void Image$$16bitRAM$$ZI$$Length(void);

char buf1[40];  /* will occupy 40 bytes of ZI data */

int i = 1;      /* will occupy 4 bytes of RW data */

void C_func(void)
{
  sprintf(buf1, "Entered func%d\n", i);
  Write0(buf1);

  sprintf(buf1, "Load$$16bitRAM$$Base = %p\n", Load$$16bitRAM$$Base);
  Write0(buf1);
  sprintf(buf1, "Image$$16bitRAM$$Base = %p\n", Image$$16bitRAM$$Base);
  Write0(buf1);
  sprintf(buf1, "Image$$16bitRAM$$Length = %p\n", Image$$16bitRAM$$Length);
  Write0(buf1);
  sprintf(buf1, "Image$$16bitRAM$$ZI$$Base = %p\n", Image$$16bitRAM$$ZI$$Base);
  Write0(buf1);
  sprintf(buf1, "Image$$16bitRAM$$ZI$$Length = %p\n", Image$$16bitRAM$$ZI$$Length);
  Write0(buf1);
}

