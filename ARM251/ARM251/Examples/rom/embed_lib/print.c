/* 1.1.2.2 */
#include <stdio.h>

#ifdef __thumb
/* Define Angel Semihosting SWI to be Thumb one */
#define SemiSWI 0xAB
#else
/* Define Angel Semihosting SWI to be ARM one */
#define SemiSWI 0x123456
#endif

/* We use the following Debug Monitor SWIs in this example */

/* Write a string */
__swi(SemiSWI) void _Write0(unsigned op, char *string);
#define Write0(string) _Write0 (0x4,string)

/* Exit */
__swi(SemiSWI) void _Exit(unsigned op, unsigned except);
#define Exit() _Exit (0x18,0x20026)

void C_Entry(void)
{
   int i;
   char buf[20];

   for (i = 0; i < 10; i++) {
     sprintf(buf, "Hello, World %d\n", i);
     Write0(buf);
   }
   Exit();
}
