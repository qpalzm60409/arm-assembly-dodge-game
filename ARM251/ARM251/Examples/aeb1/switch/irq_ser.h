/**************************************************************************
 *
 * ARM - Strategic Support Group
 *
 ***************************************************************************/

/***************************************************************************
 *
 * Module       : irq.h
 * Description  :
 *
 *      irq handles the counter timer provided by the Sharp 77790
 *      microcontroller. Setup the button interrupt...
 *    
 * Status       : Complete
 * Platform     : AEB-1
 * History      :
 *
 *      980329 ASloss
 *      
 *              Added header information
 *
 * Copyright (C) 1998 ARM Ltd. All rights reserved.
 *
 *
 * RCS $Revision: 1.2 $
 * Checkin $Date: 1998/08/06 18:49:55 $
 * Revising $Author: swoodall $
 *
 **************************************************************************/

/**************************************************************************
 * IMPORTS
 **************************************************************************/

/* none... */

/**************************************************************************
 * MACROS
 **************************************************************************/

/* none... */

/**************************************************************************
 * DATATYPES
 **************************************************************************/

/**************************************************************************
 * STATICS
 **************************************************************************/

/* Number of times the CT has completed its count */

extern volatile unsigned int AuxCT_repetitions; 

/**************************************************************************
 * PROTOTYPES
 **************************************************************************/

/* -- irq_init -------------------------------------------------------------
 *
 * Description  : Call only once, at system initialization. Initializes
 *                internal variables.
 *
 * Parameters   : none...
 * Return       : none...
 * Notes        : none...
 *
 */

extern void irq_init(void);     

/* -- AuxCT_Start ------------------------------------------------------------
 *
 * Description  : Call immediately before sequence to time
 *
 *
 * Parameters   : none...
 * Return       : none...
 * Notes        : internal time variable set to 0
 *
 */

extern void irq_auxctstart(void);

/* -- irq_auxctstop -------------------------------------------------------------
 *
 * Description  : Call after sequence completes, return value is 
 *                approximately the number of clock cycles measured from 
 *                start time. Valid for 32 bits, accurate minus interrupt 
 *                handling time (if target sequence is single threaded
 *
 * Parameters   : none...
 * Return       : unsigned int - number of clock cycles
 * Notes        : none...
 *
 */

extern unsigned int irq_auxctstop(void); 

/**************************************************************************
 * End of irq.h
 **************************************************************************/
