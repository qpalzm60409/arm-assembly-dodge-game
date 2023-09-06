
/****************************************************************
 *
 * ARM Strategic Support Group
 * 
 ****************************************************************/

/****************************************************************
 *
 *      Module          : pascal.c
 *      Description     : Writes out a Pascal's triangle using
 *                        semi-hosting and non-embedded libraries
 *      Platform        : AEB-1
 *      Status          : Complete
 *      History         : 980504 ASloss
 *
 *                         - started
 *
 *      Notes           :
 *
 *                        Limited to a depth of 6, since the
 *                        output routines only work with single 
 *                        digit numbers... 
 *
 * Copyright (C) 1998 ARM Ltd. All rights reserved.
 *
 *
 * RCS $Revision: 1.2 $
 * Checkin $Date: 1998/08/06 18:49:55 $
 * Revising $Author: swoodall $
 *
 ****************************************************************/

/****************************************************************
 * IMPORT
 ****************************************************************/

#include <stdio.h>
#include "led.h"

/****************************************************************
 * MACROS
 ****************************************************************/

// none...

/****************************************************************
 * DATATYPES
 ****************************************************************/

// none...

/****************************************************************
 * ROUTINES
 ****************************************************************/

/* -- pascal_generator ------------------------------------------
 *
 * Description  : Generates the PASCAL triangle numbers
 *
 * Parameters   : int n - level 
 *                int i - row
 * Return       : int - calculated PASCAL triangle number...
 * Notes        : A recursive function. Note recursive functions
 *                are dangerous in embedded systems which require
 *                deterministic stack...
 *
 */

int pascal_generator (int n, int i)
{
        LED_2_ON;
        LED_2_OFF;

        if (i == 1 || i == n) return 1;
        
        return pascal_generator(n-1,i-1) + pascal_generator(n-1,i);
}

/* -- pascal_triangle ------------------------------------------
 * 
 * Descriptions : Builds the complete PASCAL's triangle.
 *
 * Parameters   : int d - depth
 * Return       : none...
 * Notes        : 
 *
 */

void pascal_triangle (int depth)
{
int     i,j,s;

        LED_3_ON;
    
        for (j = 1; j < depth; j++) {
                printf("\n\t\t");

                for ( s = depth-j; s > 0; s-- ) {
                        putchar (' ');
                }

                for (i = 1; i <= j; i++) {
                printf("%d ", pascal_generator(j, i));
                }        
        }

        putchar ('\n');

        LED_3_OFF;
}
   
/* -- pascal_init ----------------------------------------------
 *
 * Description  : initialize the LED on the AEB-1 board
 *
 * Parameters   : none...
 * Return       : none...
 * Notes        : none...
 *
 */

void pascal_init (void)
{
        LED_1_OFF;
        LED_2_OFF;
        LED_3_OFF;
        LED_4_OFF;
}

/* -- main -----------------------------------------------------
 *
 * Description  : entry point
 * 
 * Parameters   : none...
 * Return       : int
 * Notes        : none...
 *
 */

int main (void)
{
        pascal_init();

        LED_4_ON;

        printf ("\n\t*** Pascal's Triangle *** \n\n");
        pascal_triangle (6);

        LED_4_OFF;

        return 1;
}

/****************************************************************
 * End of Pascal.c
 ****************************************************************/

