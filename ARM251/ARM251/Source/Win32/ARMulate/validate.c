
/* validate.c - ARM validation suite co-processxor and OS. (see also trickbox)
 *
 * Copyright (C) Advanced RISC Machines Limited, 1995. All rights reserved.
 *
 * RCS $Revision: 1.32.2.2 $
 * Checkin $Date: 1999/05/20 10:33:38 $
 * Revising $Author: dcleland $
 */

/*#include "armdefsp.h"*/
#include "armdefs.h"
#include "armmem.h"
 
#include "cycles.h"
#include "armcnf.h"

 
 
/*
 * What follows is the Validation Suite Coprocessor.  It uses two
 * co-processor numbers (4 and 5) and has the follwing functionality.
 * Sixteen registers.
 * Both co-processor nuimbers can be used in an MCR and MRC to access
 *   these registers.
 * CP 4 can LDC and STC to and from the registers.
 * CP 4 and CP 5 CDP 0 will busy wait for the number of cycles
 *   specified by a CP register.
 * CP 5 CDP 1 issues a FIQ after a number of cycles (specified in a CP
 *   register),
 * CDP 2 issues an IRQW in the same way, CDP 3 and 4 turn of the FIQ
 *   and IRQ source, and CDP 5 stores a 32 bit time value in a CP
 *   register (actually it's the total number of N, S, I, C and F
 *   cyles)
 */

#define CORE_TYPE 0
#define NON_CORE_TYPE 1
 
typedef struct {
      unsigned long ulBusyWaitStart;
      int       iBusyWaitDuration;
      int       iBusyWaiting;
    } CPBusyWait;

CPBusyWait CP4Busy={0l,0};
CPBusyWait CP5Busy={0l,0};
CPBusyWait CP8Busy={0l,0}; 


/* CP8 takes 2 cycles for LDC access - 2 registers accessed per LDC */

static ARMword ValCP4Reg[16] ;
static ARMword ValCP5Reg[16] ;
static ARMword ValCP8Reg[16] ;




/*  Static function declarations  */
/* NOTE that there are further declarations in the OS section of this file */
static unsigned CheckBusyWait(unsigned type, CPBusyWait *CPBusy, unsigned long cycle_count, int * result);
static unsigned long GetCycleCount(void *handle, int cycle_type);

static unsigned CP4LDC(void *handle, unsigned type, ARMword instr, ARMword data);
static unsigned CP5LDC(void *handle, unsigned type, ARMword instr, ARMword data);
static unsigned CP8LDC(void *handle, unsigned type, ARMword instr, ARMword data);

static unsigned CP4STC(void *handle, unsigned type, ARMword instr, ARMword *data);
static unsigned CP5STC(void *handle, unsigned type, ARMword instr, ARMword *data);
static unsigned CP8STC(void *handle, unsigned type, ARMword instr, ARMword *data);

static unsigned CP4LDCCore(void *handle, unsigned type, ARMword instr, ARMword data);
static unsigned CP5LDCCore(void *handle, unsigned type, ARMword instr, ARMword data);
static unsigned CP8LDCCore(void *handle, unsigned type, ARMword instr, ARMword data);

static unsigned CP4STCCore(void *handle, unsigned type, ARMword instr, ARMword *data);
static unsigned CP5STCCore(void *handle, unsigned type, ARMword instr, ARMword *data);
static unsigned CP8STCCore(void *handle, unsigned type, ARMword instr, ARMword *data);

static unsigned DoCP4LDC(unsigned type, ARMword instr, ARMword data, unsigned long cycle_count);
static unsigned DoCP4STC(unsigned type, ARMword instr, ARMword *data, unsigned long cycle_count);
static unsigned DoCP5LDC(unsigned type, ARMword instr, ARMword data, unsigned long cycle_count);
static unsigned DoCP5STC(unsigned type, ARMword instr, ARMword *data, unsigned long cycle_count);
static unsigned DoCP8LDC(unsigned type, ARMword instr, ARMword data, unsigned long cycle_count);
static unsigned DoCP8STC(unsigned type, ARMword instr, ARMword *data, unsigned long cycle_count);


static unsigned DoCDP(unsigned type, ARMword instr, unsigned long cycle_count, ARMword howlong, CPBusyWait *CPBusy);

static unsigned CP4CDP(void *handle, unsigned type, ARMword instr);
static unsigned CP5CDP(void *handle, unsigned type, ARMword instr);
static unsigned CP8CDP(void *handle, unsigned type, ARMword instr);
static unsigned CP4CDPCore(void *handle, unsigned type, ARMword instr);
static unsigned CP5CDPCore(void *handle, unsigned type, ARMword instr);
static unsigned CP8CDPCore(void *handle, unsigned type, ARMword instr);


static unsigned DoMRC(unsigned type, ARMword instr, ARMword *value, ARMword reg_val, CPBusyWait *CPBusy, unsigned long cycle_count);
static unsigned DoMCR(unsigned type, ARMword instr, ARMword value, ARMword *reg_val, CPBusyWait *CPBusy, unsigned long cycle_count);

static unsigned CP4MRC(void *handle, unsigned type, ARMword instr,ARMword *value);
static unsigned CP5MRC(void *handle, unsigned type, ARMword instr,ARMword *value);
static unsigned CP8MRC(void *handle, unsigned type, ARMword instr,ARMword *value);

static unsigned CP4MCR(void *handle, unsigned type, ARMword instr, ARMword value);
static unsigned CP5MCR(void *handle, unsigned type, ARMword instr, ARMword value);
static unsigned CP8MCR(void *handle, unsigned type, ARMword instr, ARMword value);
 
static unsigned CP4MRCCore(void *handle, unsigned type, ARMword instr,ARMword *value);
static unsigned CP5MRCCore(void *handle, unsigned type, ARMword instr,ARMword *value);
static unsigned CP8MRCCore(void *handle, unsigned type, ARMword instr,ARMword *value);

static unsigned CP4MCRCore(void *handle, unsigned type, ARMword instr, ARMword value);
static unsigned CP5MCRCore(void *handle, unsigned type, ARMword instr, ARMword value);
static unsigned CP8MCRCore(void *handle, unsigned type, ARMword instr, ARMword value);


extern unsigned DoAFIQ(void *handle);
extern unsigned DoAIRQ(void *handle);
  
static ARMul_Error Val1Init(ARMul_State *state,
                            unsigned num,
                            ARMul_CPInterface *interf,
                            toolconf config,
                            void *sibling);

static ARMul_Error Val2Init(ARMul_State *state,
                            unsigned num,
                            ARMul_CPInterface *interf,
                            toolconf config,
                            void *sibling);

static ARMul_Error Val3Init(ARMul_State *state,
                            unsigned num,
                            ARMul_CPInterface *interf,
                            toolconf config,
                            void *sibling);



/* Checks whether the copro is waiting for a previous instruction to complete, and also whether there is 
   an interrupt */
static unsigned CheckBusyWait(unsigned type, CPBusyWait *CPBusy, unsigned long cycle_count, int * result)
{
  unsigned long finish;

  /* If we get an interrupt, regardless of busy wait status then exit */
  if ( type == ARMul_INTERRUPT) {
     CPBusy->iBusyWaiting=0; /* Clear busy waiting flag */
     *result = ARMul_DONE;/* We've been interrupted */
     return TRUE; 
  }

  /* Is this CP currently busy waiting? */
  if (CPBusy->iBusyWaiting) {

    /* Yes it might be, check times */
    finish = CPBusy->ulBusyWaitStart + CPBusy->iBusyWaitDuration;
    if ( cycle_count <= finish ) {
        /* Yes it IS busy waiting still, so return ARMul_BUSY*/
        *result = ARMul_BUSY;
        return TRUE;
    }
    else {
        CPBusy->iBusyWaiting=0; /* Assert that we aren't busy waiting any more */
    }
  }
  return FALSE;

}

/* Code to get the current cycle count total, either for Core cycle type armulators (ARM9)
   or others */
static unsigned long GetCycleCount(void *handle, int cycle_type)
{
  ARMul_State *state=(ARMul_State *)handle;
  const ARMul_Cycles *cycles = ARMul_ReadCycles(state); /* Get a handle to cycles structure */

if (cycle_type == CORE_TYPE) {
    return cycles->CoreCycles;
  }
  else {
    return cycles->Total;
  }
}


static unsigned CP4LDC(void *handle, unsigned type,
                       ARMword instr, ARMword data)
{
  unsigned long cycle_count = GetCycleCount(handle, NON_CORE_TYPE);
  return DoCP4LDC(type, instr, data, cycle_count);
}

static unsigned CP4LDCCore(void *handle, unsigned type,
                       ARMword instr, ARMword data)
{
  unsigned long cycle_count = GetCycleCount(handle, CORE_TYPE);
  return DoCP4LDC(type, instr, data, cycle_count);
}

static unsigned DoCP4LDC (unsigned type, ARMword instr,
                      ARMword data, unsigned long cycle_count)
{
  static int regNum;
  static unsigned words;
  static unsigned wordsToGet;
  int result=0;
 
  if (CheckBusyWait(type, &CP4Busy, cycle_count, &result))
    return result; /* Either busy waiting or interrupted */

  /*ARMul_ConsolePrint(handle,"CP4LDC type = %d, data=%08x, reg=%08x\n"
              ,type,data,regNum);*/
  if ( type == ARMul_FIRST)
    {
    regNum = BITS(12,15);  /* Get the register number */
    words = 0;       /* Reset the word fetch counter */
    wordsToGet = 1;  /* just to be safe */
    /*ARMul_ConsolePrint(handle,"First access\n");*/
    return(ARMul_INC);
    }

  if ( type == ARMul_TRANSFER)
    {
    /*ARMul_ConsolePrint(handle,"ARMul_Transfer\n");*/
    return(ARMul_INC);
    }

  if (BIT(22)) 
     {       
     /* Its a CPDTL instruction so 4 words to fetch */  
     wordsToGet=4;
     /*ARMul_ConsolePrint(handle,"LONG fetch - 4 words \n");*/

     /* Do the data access,  incrementing the register as we go */
     ValCP4Reg[regNum++] = data ;
     regNum &= 0xf;

     /* If we've got enough words then return done, else request more ! */

     /*ARMul_ConsolePrint(handle,"Words done so far = %d\n",words+1);*/
     if (++words >= wordsToGet)
        return(ARMul_DONE) ;
     else
        return(ARMul_INC) ;
     }
  else 
     { 
     /* NOT a long access so get just one word */ 
     /*ARMul_ConsolePrint(handle,"Single word access LDC reg = %d, data = %08x\n"
      ,BITS(12,15),data);*/
     ValCP4Reg[BITS(12,15)] = data ;
     return(ARMul_DONE) ;
     }
}


static unsigned CP4STC(void *handle, unsigned type,
                       ARMword instr, ARMword *data)
{
  unsigned long cycle_count = GetCycleCount(handle, NON_CORE_TYPE);
  return DoCP4STC(type, instr, data, cycle_count);
}

static unsigned CP4STCCore(void *handle, unsigned type,
                       ARMword instr, ARMword *data)
{
  unsigned long cycle_count = GetCycleCount(handle, CORE_TYPE);
  return DoCP4STC(type, instr, data, cycle_count);
}

static unsigned DoCP4STC(unsigned type, ARMword instr,
                      ARMword *data, unsigned long cycle_count)
{ 
  static int regNum;
  static unsigned words;
  static unsigned wordsToSend;
  
  
   if ( type == ARMul_INTERRUPT)
     {
     CP4Busy.iBusyWaiting=0; /* Clear busy waiting flag */
     return ARMul_DONE; /* We've been interrupted */
     }

 /* Is this CP currently busy waiting? */

 if (CP4Busy.iBusyWaiting)
    {
    unsigned long finish;

 if ( type == ARMul_FIRST)
    {
    regNum = BITS(12,15);  /* Get the register number */
    words = 0;       /* Reset the word fetch counter */
    wordsToSend = 1;  /* just to be safe */
    /*ARMul_ConsolePrint(handle,"First access - busy waiting\n");*/
     }

    /* Yes it might be, check times */
    finish = CP4Busy.ulBusyWaitStart + CP4Busy.iBusyWaitDuration;
    if ( cycle_count <= finish )
        {
        /* Yes it IS busy waiting still, so return ARMul_BUSY*/

        return(ARMul_BUSY);
        }
    else
        {
        CP4Busy.iBusyWaiting=0; /* Assert that we aren't busy waiting any more */
        return (ARMul_INC);
        }
    }
  

  /*ARMul_ConsolePrint(handle,"CP4STC type = %d, reg=%02x\n"
              ,type,regNum);*/
  if ( type == ARMul_FIRST)
    {
    regNum = BITS(12,15);  /* Get the register number */
    words = 0;       /* Reset the word fetch counter */
    wordsToSend = 1;  /* just to be safe */
    /*ARMul_ConsolePrint(handle,"First access\n");*/
    return(ARMul_INC);
    }

  if (BIT(22)) 
    { 
     /* it's a long access*/
     /* Do we want 2 or 4 words transferred? */
      
     /* Its a CPDTL instruction so 4 words */   
      /* ARMul_ConsolePrint(handle,"CP4STCL instruction 4 words\n");*/ 
     wordsToSend = 4;


     /* Now do the data access */
     *data = ValCP4Reg[regNum++];
     /*ARMul_ConsolePrint(handle,"Data = %08x\n",ValCP4Reg[regNum-1]);*/

     /* Store the data in Register[regNum] and increment regNum for the
        next transfer */

     /* If we've got enough words then return done, else request more ! */

     /*ARMul_ConsolePrint(handle,"Words transferred so far = %d\n",words+1);*/
     if (++words >= wordsToSend)
       return(ARMul_DONE) ;
     else
       return(ARMul_INC) ;
    }
  else
    { 
    /* get just one word */
    *data = ValCP4Reg[regNum] ; 
    return(ARMul_DONE) ;
    }
 }


static unsigned CP5LDC(void *handle, unsigned type,
                       ARMword instr, ARMword data)
{
  unsigned long cycle_count = GetCycleCount(handle, NON_CORE_TYPE);
  return DoCP5LDC(type, instr, data, cycle_count);
}

static unsigned CP5LDCCore(void *handle, unsigned type,
                       ARMword instr, ARMword data)
{
  unsigned long cycle_count = GetCycleCount(handle, CORE_TYPE);
  return DoCP5LDC(type, instr, data, cycle_count);
}

/* CP5 LDC instruction. Does 2 word accesses */
static unsigned DoCP5LDC(unsigned type, ARMword instr,
                      ARMword data, unsigned long cycle_count)
{
  static int regNum;
  static unsigned words;
  static unsigned wordsToGet;
  unsigned long finish;
  

 /* If we get an interrupt, regardless of busy wait status then exit */
  if ( type == ARMul_INTERRUPT)
     {
     CP5Busy.iBusyWaiting=0; /* Clear busy waiting flag */
     return ARMul_DONE; /* We've been interrupted */
     }

 /* Is this CP currently busy waiting? */

 if (CP5Busy.iBusyWaiting)
    {
     if ( type == ARMul_FIRST)
        {
        regNum = BITS(12,15);  /* Get the register number */
        words = 0;       /* Reset the word fetch counter */
        wordsToGet = 1;  /* just to be safe */
        /*ARMul_ConsolePrint(handle,"First access - busy waiting\n");*/
        }

    /* Yes it might be, check times */
    finish = CP5Busy.ulBusyWaitStart + CP5Busy.iBusyWaitDuration;
    if ( cycle_count <= finish )
        {
        /* Yes it IS busy waiting still, so return ARMul_BUSY*/
        return(ARMul_BUSY);
        }
    else
        {
        CP5Busy.iBusyWaiting=0; /* Assert that we aren't busy waiting any more */
       /* return (ARMul_INC);*/
        }
    }

/* If we get here then we aren't busy waiting */

/* ARMul_ConsolePrint(handle,"CP5LDC type = %d, data=%08x, reg=%08x\n"
              ,type,data,regNum);*/

 
 if ( type == ARMul_FIRST)
    {
    regNum = BITS(12,15);  /* Get the register number */
    words = 0;       /* Reset the word fetch counter */
    wordsToGet = 1;  /* just to be safe */
    /*ARMul_ConsolePrint(handle,"First access\n");*/
    return(ARMul_INC);

    }

 if ( type == ARMul_TRANSFER)
    {
     return(ARMul_INC); 
    }

 if (BIT(22)) 
     {       
     /* Its a LDCL instruction so 2 words to fetch */  
     wordsToGet=2;
     /*ARMul_ConsolePrint(handle,"LONG fetch - 2 words \n"); */
 
     /* Do the data access,  incrementing the register as we go */
     ValCP5Reg[regNum++] = data ;
     regNum &= 0xf;

     /* If we've got enough words then return done, else request more ! */

     /*ARMul_ConsolePrint(handle,"%d of %d Words done so far\n",words+1, wordsToGet);*/
     if (++words >= wordsToGet)
        {
        /*ARMul_ConsolePrint(handle,"CP5LDC Fetched all words required - returning ARMul_Done\n");*/
        return(ARMul_DONE) ;
        }
     else
        return(ARMul_INC) ;
     }
 else 
     { 
     /* NOT a long access so get just one word */ 
     /*ARMul_ConsolePrint(handle,"Single word access LDC reg = %d, data = %08x\n"
      ,BITS(12,15),data);*/
     ValCP5Reg[BITS(12,15)] = data ;
     return(ARMul_DONE) ;
     }
    
 }

static unsigned CP5STC(void *handle, unsigned type,
                       ARMword instr, ARMword *data)
{
  unsigned long cycle_count = GetCycleCount(handle, NON_CORE_TYPE);
  return DoCP5STC(type, instr, data, cycle_count);
}

static unsigned CP5STCCore(void *handle, unsigned type,
                       ARMword instr, ARMword *data)
{
  unsigned long cycle_count = GetCycleCount(handle, CORE_TYPE);
  return DoCP5STC(type, instr, data, cycle_count);
}

static unsigned DoCP5STC(unsigned type, ARMword instr,
                      ARMword *data, unsigned long cycle_count)
{ 
  static int regNum;
  static unsigned words;
  static unsigned wordsToSend;
  int result;

  if (CheckBusyWait(type, &CP5Busy, cycle_count, &result))
    return result; /* Either busy waiting or interrupted */

   /* ARMul_ConsolePrint(handle,"CP5STC type = %d, reg=%08x\n"
              ,type,regNum); */ 
  if ( type == ARMul_FIRST)
    {
    regNum = BITS(12,15);  /* Get the register number */
    words = 0;       /* Reset the word fetch counter */
    wordsToSend = 1;  /* just to be safe */
    /*ARMul_ConsolePrint(handle,"First access\n");*/
    return(ARMul_INC);

    }

  if (BIT(22)) 
     { 
     /* it's a long access*/
      
     /* Its a CPDTL instruction so 2 words */   
     /*ARMul_ConsolePrint(handle,"CP5STCL instruction 2 words\n");*/
     wordsToSend = 2;


     /* Now do the data access */
     *data = ValCP5Reg[regNum++];
     /* Store the data in Register[regNum] and increment regNum for the
        next transfer */

     /* If we've got enough words then return done, else request more ! */

     /*ARMul_ConsolePrint(handle,"Words transferred so far = %d\n",words+1);*/
     if (++words >= wordsToSend)
       return(ARMul_DONE) ;
     else
       return(ARMul_INC) ;
    }
  else
    { 
    /* get just one word */
    *data = ValCP5Reg[regNum] ; 
    return(ARMul_DONE) ;
    }
 }


static unsigned CP8LDC(void *handle, unsigned type,
                       ARMword instr, ARMword data)
{
  unsigned long cycle_count = GetCycleCount(handle, NON_CORE_TYPE);
  return DoCP8LDC(type, instr, data, cycle_count);
}

static unsigned CP8LDCCore(void *handle, unsigned type,
                       ARMword instr, ARMword data)
{
  unsigned long cycle_count = GetCycleCount(handle, CORE_TYPE);
  return DoCP8LDC(type, instr, data, cycle_count);
}

/* CP8 LDC instruction. Does 2 word accesses */
static unsigned DoCP8LDC(unsigned type, ARMword instr,
                      ARMword data, unsigned long cycle_count)
{
  static int regNum;
  static unsigned words;
  static unsigned wordsToGet;
  int result;

  if (CheckBusyWait(type, &CP8Busy, cycle_count, &result))
    return result; /* Either busy waiting or interrupted */

  /*ARMul_ConsolePrint(handle,"CP8LDC type = %d, data=%08x, reg=%08x\n"
              ,type,data,regNum);*/
  if ( type == ARMul_FIRST)
    {
    regNum = BITS(12,15);  /* Get the register number */
    words = 0;       /* Reset the word fetch counter */
    wordsToGet = 1;  /* just to be safe */
    /*ARMul_ConsolePrint(handle,"First access\n");*/
    return ARMul_BUSY;
    }

  if (BIT(22)) 
     {       
     /* Its a LDCL instruction so 2 words to fetch */  
     wordsToGet=2;
     /*ARMul_ConsolePrint(handle,"LONG fetch - 2 words \n");*/

     /* Do the data access,  incrementing the register as we go */
     ValCP8Reg[regNum++] = data ;
     regNum &= 0xf;

     /* If we've got enough words then return done, else request more ! */

     /*ARMul_ConsolePrint(handle,"Words done so far = %d\n",words+1);*/
     if (++words >= wordsToGet)
        return(ARMul_DONE) ;
     else
        return(ARMul_INC) ;
     }
  else 
     { 
     /* NOT a long access so get just one word */ 
     /*ARMul_ConsolePrint(handle,"Single word access LDC reg = %d, data = %08x\n"
      ,BITS(12,15),data);*/
     ValCP8Reg[BITS(12,15)] = data ;
     return(ARMul_DONE) ;
     }
}

static unsigned CP8STC(void *handle, unsigned type,
                       ARMword instr, ARMword *data)
{
  unsigned long cycle_count = GetCycleCount(handle, NON_CORE_TYPE);
  return DoCP8STC(type, instr, data, cycle_count);
}

static unsigned CP8STCCore(void *handle, unsigned type,
                       ARMword instr, ARMword *data)
{
  unsigned long cycle_count = GetCycleCount(handle, CORE_TYPE);
  return DoCP8STC(type, instr, data, cycle_count);
}

static unsigned DoCP8STC(unsigned type, ARMword instr,
                      ARMword *data, unsigned long cycle_count)
{ 
  static int regNum;
  static unsigned words;
  static unsigned wordsToSend;
  int result;
  
  if (CheckBusyWait(type, &CP8Busy, cycle_count, &result))
    return result; /* Either busy waiting or interrupted */

  /*ARMul_ConsolePrint(handle,"CP8STC type = %d, reg=%08x\n"
              ,type,regNum);*/
  if ( type == ARMul_FIRST)
    {
    regNum = BITS(12,15);  /* Get the register number */
    words = 0;       /* Reset the word fetch counter */
    wordsToSend = 1;  /* just to be safe */
    /*ARMul_ConsolePrint(handle,"First access\n");*/
    return ARMul_INC;
    }

  if (BIT(22)) 
     { 
     /* it's a long access*/
      
     /* Its a CPDTL instruction so 2 words */   
     /*ARMul_ConsolePrint(handle,"CP8STCL instruction 2 words\n");*/
     wordsToSend = 2;


     /* Now do the data access */
     *data = ValCP8Reg[regNum++];
     /* Store the data in Register[regNum] and increment regNum for the
        next transfer */

     /* If we've got enough words then return done, else request more ! */

     /*ARMul_ConsolePrint(handle,"Words transferred so far = %d\n",words+1);*/
     if (++words >= wordsToSend)
       return(ARMul_DONE) ;
     else
       return(ARMul_INC) ;
    }
  else
    { 
    /* get just one word */
    *data = ValCP8Reg[regNum] ; 
    return(ARMul_DONE) ;
    }
}



/* MRC instructions for CP 4,5 and 8 - 
        there are 6 versions to allow variations between CP's, with a single function 
        (DoMRC) which does all the stuff common to all MRC's*/

static unsigned CP4MRC(void *handle, unsigned type, ARMword instr,ARMword *value)
{
  unsigned long cycle_count = GetCycleCount(handle, NON_CORE_TYPE);
  return DoMRC(type, instr, value, ValCP4Reg[BITS(16,19)], &CP4Busy, cycle_count);
}

static unsigned CP4MRCCore(void *handle, unsigned type, ARMword instr,ARMword *value)
{
  unsigned long cycle_count = GetCycleCount(handle, CORE_TYPE);
  return DoMRC(type, instr, value, ValCP4Reg[BITS(16,19)], &CP4Busy, cycle_count);
}

static unsigned CP5MRC(void *handle, unsigned type, ARMword instr,ARMword *value)
{
  unsigned long cycle_count = GetCycleCount(handle, NON_CORE_TYPE);
  IGNORE(type);
  /* CP5 MRCs only take 1 cycle - so force it by using type = ARMul_DATA */
  return DoMRC(ARMul_DATA, instr, value, ValCP5Reg[BITS(16,19)], &CP5Busy, cycle_count);
  /*return DoMRC(type, instr, value, ValCP5Reg[BITS(16,19)], &CP5Busy, cycle_count);*/
}

static unsigned CP5MRCCore(void *handle, unsigned type, ARMword instr,ARMword *value)
{
  unsigned long cycle_count = GetCycleCount(handle, CORE_TYPE);
  IGNORE(type);
  return DoMRC(ARMul_DATA, instr, value, ValCP5Reg[BITS(16,19)], &CP5Busy, cycle_count);
  /*return DoMRC(type, instr, value, ValCP5Reg[BITS(16,19)], &CP5Busy, cycle_count);*/
}

static unsigned CP8MRC(void *handle, unsigned type, ARMword instr,ARMword *value)
{
  unsigned long cycle_count = GetCycleCount(handle, NON_CORE_TYPE);
  return DoMRC(type, instr, value, ValCP8Reg[BITS(16,19)], &CP8Busy, cycle_count);
}

static unsigned CP8MRCCore(void *handle, unsigned type, ARMword instr,ARMword *value)
{
  unsigned long cycle_count = GetCycleCount(handle, CORE_TYPE);
  return DoMRC(type, instr, value, ValCP8Reg[BITS(16,19)], &CP8Busy, cycle_count);
}

static unsigned DoMRC(unsigned type, ARMword instr, ARMword *value, ARMword reg_val, CPBusyWait *CPBusy, unsigned long cycle_count)
{
  int result = 0;
    
  IGNORE(instr);
  
  if (CheckBusyWait(type, CPBusy, cycle_count, &result))
    return result; /* Either busy waiting or interrupted */

/* Note this next section isn't in an else clause ( although it is one )for 
  if (CPBusy.iBusyWaiting)   because I split the check into two to save having 
  to work out the time unnecessarily - without relying on lazy expression evaluation! */

  if (type == ARMul_FIRST)
    {
    *value = reg_val;  /* Don't set data first time?? */
    return ARMul_INC; /* Say that there is more data to be sent!*/
    }
  else
    {
    /* Return the value*/
    *value = reg_val;  
    return ARMul_DONE; /* Complete */
    } 
}





/* MCR instructions for CP 4,5 and 8 - 
        there are 6 versions to allow variations between CP's, with a single function 
        (DoMCR) which does all the stuff common to all MCRs*/

static unsigned CP4MCR(void *handle, unsigned type, ARMword instr, ARMword value)
{
  unsigned long cycle_count = GetCycleCount(handle, NON_CORE_TYPE);
  return DoMCR(type, instr, value, &(ValCP4Reg[BITS(16,19)]), &CP4Busy, cycle_count);
}

static unsigned CP4MCRCore(void *handle, unsigned type, ARMword instr, ARMword value)
{
  unsigned long cycle_count = GetCycleCount(handle, CORE_TYPE);
  return DoMCR(type, instr, value, &(ValCP4Reg[BITS(16,19)]), &CP4Busy, cycle_count);
}

static unsigned CP5MCR(void *handle, unsigned type, ARMword instr, ARMword value)
{
  unsigned long cycle_count = GetCycleCount(handle, NON_CORE_TYPE);
  IGNORE(type);
  /* CP5 MCRs only take 1 cycle - so force it by using type = ARMul_DATA */
  return DoMCR(ARMul_DATA, instr, value, &(ValCP5Reg[BITS(16,19)]), &CP5Busy, cycle_count);
  /*return DoMCR(type, instr, value, &(ValCP5Reg[BITS(16,19)]), &CP5Busy, cycle_count);*/
}

static unsigned CP5MCRCore(void *handle, unsigned type, ARMword instr, ARMword value)
{
  unsigned long cycle_count = GetCycleCount(handle, CORE_TYPE);
  IGNORE(type);
  /* CP5 MCRs only take 1 cycle - so force it by using type = ARMul_DATA */
  return DoMCR(ARMul_DATA, instr, value, &(ValCP5Reg[BITS(16,19)]), &CP5Busy, cycle_count);
  /*return DoMCR(type, instr, value, &(ValCP5Reg[BITS(16,19)]), &CP5Busy, cycle_count);*/
}

static unsigned CP8MCR(void *handle, unsigned type, ARMword instr, ARMword value)
{
  unsigned long cycle_count = GetCycleCount(handle, NON_CORE_TYPE);
  return DoMCR(type, instr, value, &(ValCP8Reg[BITS(16,19)]), &CP8Busy, cycle_count);
}

static unsigned CP8MCRCore(void *handle, unsigned type, ARMword instr, ARMword value)
{
  unsigned long cycle_count = GetCycleCount(handle, CORE_TYPE);
  return DoMCR(type, instr, value, &(ValCP8Reg[BITS(16,19)]), &CP8Busy, cycle_count);
}

static unsigned DoMCR(unsigned type, ARMword instr, ARMword value, ARMword *reg_val, CPBusyWait *CPBusy, unsigned long cycle_count)
{
  unsigned long finish;

  IGNORE(instr);
   /* Is this CP currently busy waiting? */

  /* If we get an interrupt, regardless of busy wait status then exit */
  if ( type == ARMul_INTERRUPT)
     {
     CPBusy->iBusyWaiting=0; /* Clear busy waiting flag */
     return ARMul_DONE; /* We've been interrupted */
     }

 /* Is this CP currently busy waiting? */

 if (CPBusy->iBusyWaiting)
    {
    /* Yes it might be, check times */
    finish = CPBusy->ulBusyWaitStart + CPBusy->iBusyWaitDuration;
    if ( cycle_count <= finish )
        {
        /* Yes it IS busy waiting still, so return ARMul_BUSY*/
        return(ARMul_BUSY);
        }
    }
 
/* Note this next section isn't in an else clause ( although it is one )for 
  if (CP4Busy.iBusyWaiting)   because I split the check into two to save having 
  to work out the time unnecessarily - without relying on lazy expression evaluation! */

  if (type == ARMul_FIRST)
    {
     /*ValCP4Reg[BITS(16,19)] = value;*/
     *reg_val = value;
     return ARMul_INC; /* Say that there is more data to be sent!*/
    }
 else
    {
     /* No it isn't busy waiting, so store the value*/
     *reg_val = value;
     return ARMul_DONE; /* Complete */
    } 

}




 

/* CDP co-processor instructions for CP 4,5 and 8 */

/* Note that ARM9 uses coreCycle counts and therefore
   we need two versions - one for early cores and one for cores requiring
   coreCycle counting. */

/* The core functionality of CDP instructions has now been placed in DoCDP, which
   is used by CP4 and CP8. CP5 is sufficiently different to not use this.*/

static unsigned CP4CDPCore(void *handle, unsigned type, ARMword instr)
{
  ARMword howlong = ValCP4Reg[BITS(12,13)]; /* See how long we have to busy wait for - if we can */
  unsigned long cycle_count = GetCycleCount(handle, CORE_TYPE);

  return DoCDP(type, instr, cycle_count, howlong, &CP4Busy);
}

static unsigned CP4CDP(void *handle, unsigned type, ARMword instr)
{
  ARMword howlong = ValCP4Reg[BITS(12,13)]; /* See how long we have to busy wait for - if we can */
  unsigned long cycle_count = GetCycleCount(handle, NON_CORE_TYPE);

  return DoCDP(type, instr, cycle_count, howlong, &CP4Busy);
}
 

/* CP5 CDP instruction - behaviour not checked*/ 
static unsigned CP5CDPCore(void *handle, unsigned type, ARMword instr)
{
  ARMul_State *state=(ARMul_State *)handle;
  static unsigned long FRTimer = 0;
  unsigned long timeNow;
  ARMword howlong  = ValCP5Reg[BITS(12,13)] ; /* Only allow 8 bits of reg value*/
  unsigned long cycle_count = GetCycleCount(handle, CORE_TYPE);
  int result = 0;

/* howlong is used to store the number of cycles in advance of now
   the required action happens - See Regme in the VHDL */

 /*ARMul_ConsolePrint(state,"CP5CDP, howlong = %d, reg = %d\n",howlong, BITS(12,13));*/

  if (CheckBusyWait(type, &CP4Busy, cycle_count, &result))
    return result; /* Either busy waiting or interrupted */

  /* Bits 23..20 is correct for opcode_1*/
  switch((int)BITS(20,23)) {

  case 0 : 
    {
      /* No it isn't, so set up the new busy wait and return ARMul_DONE */
      /*ARMul_ConsolePrint(state,"BUSY WAITING CP5 for %d cycles.\n",howlong);*/
      CP5Busy.iBusyWaiting=1;
      CP5Busy.ulBusyWaitStart = cycle_count;
      CP5Busy.iBusyWaitDuration = howlong;
      return ARMul_DONE; 
    }
  case 1 : if (howlong == 0)
        {
        ARMul_SetNfiq(state,LOW);
        }
      else
        {
        /*ARMul_ConsolePrint(state,"schedule a FIQ %d cycles from now (at %08x)\n",howlong,cycle_count+howlong);*/
#ifndef OLD_ARM9_CORE 
        /* New core used for 920T model has different interrupt scheduling */
        ARMul_ScheduleEventCore(state,DoAFIQ, state, cycle_count + howlong + 3); /* a fixed up value */
#else
        ARMul_ScheduleEventCore(state,DoAFIQ, state, cycle_count + howlong + 2); /* a fixed up value */
#endif
        }


     return(ARMul_DONE);
  case 2 : if (howlong == 0)
        {
        ARMul_SetNirq(state,LOW);
        }
     else
        {
#ifndef OLD_ARM9_CORE
        /* New core used for 920T model has different interrupt scheduling */
        ARMul_ScheduleEventCore(state,DoAIRQ,state, cycle_count + howlong + 3); /* see above */
#else
        ARMul_ScheduleEventCore(state,DoAIRQ,state, cycle_count + howlong + 2); /* see above */
#endif
        }
      return(ARMul_DONE);
  case 3 : ARMul_SetNfiq(state,HIGH);
       return(ARMul_DONE);
  case 4 : ARMul_SetNirq(state,HIGH);
       return(ARMul_DONE);
  case 5 : /* Reset Timer */
      FRTimer = cycle_count;
      /*ARMul_ConsolePrint(state,"Reset Trickbox timer to %08x\n",FRTimer);
        ARMul_ConsolePrint(state,"Stored in C%d\n",BITS(12,13));*/
      return(ARMul_DONE);
  case 6 : timeNow =  cycle_count;
      if ( FRTimer < timeNow )
        {
        ValCP5Reg[BITS(12,13)] = ((timeNow - FRTimer) &  0xFF) - 1;
        /*ARMul_ConsolePrint(state,"CP5, read timer - count is %08x, in C%d.\n",ValCP5Reg[BITS(12,13)],BITS(12,13));*/
        }
      else
        {
        /* ARMul time has rolled over */
        /*ValReg[BITS(0,3)] = (((2^32) - FRTimer ) + timeNow) & 0xFF ;
                bits 12..13 for this co-processor */
        ValCP5Reg[BITS(12,13)] = (((2^32) - FRTimer ) + timeNow) & 0xFF ;
        }
      /* QQQ Check Register id and number for writeback*/
      /* Return current time - stored time & 0xFF to give 8 bits. */
      return(ARMul_DONE);

     }
 return(ARMul_CANT) ;
 }


 
/* CP5 CDP instruction - behaviour not checked*/ 
static unsigned CP5CDP(void *handle, unsigned type, ARMword instr)
{
  ARMul_State *state=(ARMul_State *)handle;
  ARMword howlong  = ValCP5Reg[BITS(12,13)] ; /* Only allow 8 bits of reg value*/
  unsigned long cycle_count = GetCycleCount(handle, NON_CORE_TYPE);
  int result = 0;

  /* howlong is used to store the number of cycles in advance of now
     the required action happens - See Regme in the VHDL */

  if (CheckBusyWait(type, &CP4Busy, cycle_count, &result))
    return result; /* Either busy waiting or interrupted */

  /*ARMul_ConsolePrint(state,"CP5CDP, howlong = %d, reg = %d\n",howlong, BITS(12,13));*/


  switch((int)BITS(20,23)) {

  case 0: 
    {
      /* Set up the new busy wait and return ARMul_DONE */
      /*ARMul_ConsolePrint(state,"BUSY WAITING CP5 for %d cycles.\n",howlong);*/
      CP5Busy.iBusyWaiting=1;
      CP5Busy.ulBusyWaitStart = cycle_count;
      CP5Busy.iBusyWaitDuration = howlong;
      return(ARMul_DONE); 
    }
  case 1 : if (howlong == 0)
                {
                ARMul_SetNfiq(state,LOW);
                }
              else
                {
                  /* ARMul_ConsolePrint(state,"schedule a FIQ %d cycles from now (at %08x)\n",howlong,cycle_count+howlong);*/
                ARMul_ScheduleEvent(state,howlong,DoAFIQ,state);
                }

 
             return(ARMul_DONE);
  case 2 : if (howlong == 0)
                {
                ARMul_SetNirq(state,LOW);
                }
             else
                {
                /*ARMul_ConsolePrint(state,"schedule an IRQ %d cycles from now (at %08x)\n",howlong,cycle_count+howlong);*/
                ARMul_ScheduleEvent(state,howlong,DoAIRQ,state);
                }
              return(ARMul_DONE);
  case 3 : ARMul_SetNfiq(state,HIGH);
             return(ARMul_DONE);
  case 4 : ARMul_SetNirq(state,HIGH);
             return(ARMul_DONE);
  case 5 : /* Read Timer */
                 ValCP5Reg[BITS(12,13)] = cycle_count;
          /*ARMul_ConsolePrint(state,"Read Trickbox timer - %08x\n",cycle_count);
          ARMul_ConsolePrint(state,"Stored in C%d\n",BITS(12,13));*/
             return(ARMul_DONE);
     }
 return(ARMul_CANT) ;
 }

 


/* CP8 CDP instruction - behaviour not checked*/ 
static unsigned CP8CDPCore(void *handle, unsigned type, ARMword instr)
{
  ARMword howlong = ValCP8Reg[BITS(12,13)]; /* See how long we have to busy wait for - if we can */
  unsigned long cycle_count = GetCycleCount(handle, CORE_TYPE);

  return DoCDP(type, instr, cycle_count, howlong, &CP8Busy);
}

static unsigned CP8CDP(void *handle, unsigned type, ARMword instr)
{
  ARMword howlong = ValCP8Reg[BITS(12,13)]; /* See how long we have to busy wait for - if we can */
  unsigned long cycle_count = GetCycleCount(handle, NON_CORE_TYPE);

  return DoCDP(type, instr, cycle_count, howlong, &CP8Busy);
}

static unsigned DoCDP(unsigned type, ARMword instr, unsigned long cycle_count, ARMword howlong, CPBusyWait *CPBusy)
{
  int result = 0;

  if (BITS(20,23)==0) {
    if (CheckBusyWait(type, CPBusy, cycle_count, &result))
      return result; /* Either busy waiting or interrupted */

    /* No it isn't, so set up the new busy wait and return ARMul_DONE */
    /*ARMul_ConsolePrint(state,"BUSY WAITING CP for %d cycles.\n",howlong);*/
    CPBusy->iBusyWaiting=1;
    CPBusy->ulBusyWaitStart = cycle_count;
    CPBusy->iBusyWaitDuration = howlong;
    return(ARMul_DONE);
  }

  /* If we get here then we received an instruction we couldn't process */
  return ARMul_CANT;
}



 




extern unsigned DoAFIQ(void *handle)
{
const ARMul_Cycles *cycles;
 
 cycles = ARMul_ReadCycles(handle);

  /*ARMul_ConsolePrint((ARMul_State*)handle,"In DoAFIQ at %08x\n",cycles->Total);*/
  ARMul_SetNfiq((ARMul_State *)handle,LOW);
  return(0) ;
}

extern unsigned DoAIRQ(void *handle)
{
 /* ARMul_ConsolePrint((ARMul_State*)handle,"In DoAIRQ\n"); */
  ARMul_SetNirq((ARMul_State *)handle,LOW);
  return(0) ;
}




/* CP Register Read write access for RDI */

static unsigned CP4Read(void *handle,unsigned reg,ARMword *value)
{
 IGNORE(handle);
 *value = ValCP4Reg[reg] ;  
 return ARMul_DONE;
}

   
static unsigned CP5Read(void *handle,unsigned reg,ARMword *value)
{
 IGNORE(handle);
 *value = ValCP5Reg[reg] ;  
 return ARMul_DONE;
}
 
static unsigned CP8Read(void *handle,unsigned reg,ARMword *value)
{
 IGNORE(handle);
 *value = ValCP8Reg[reg] ;  
 return ARMul_DONE;
}

static unsigned CP4Write(void *handle,unsigned reg,ARMword const *value)
{
  IGNORE(handle);
  ValCP4Reg[reg] = *value;
 
  return 1;
}

static unsigned CP5Write(void *handle,unsigned reg,ARMword const *value)
{
  IGNORE(handle);
  ValCP4Reg[reg] = *value;
 
  return 1;
}

static unsigned CP8Write(void *handle,unsigned reg,ARMword const *value)
{
  IGNORE(handle);
  ValCP4Reg[reg] = *value;
 
  return 1;
}





/* Register CP4 co-processor */

static ARMul_Error Val1Init(ARMul_State *state,
                            unsigned num,
                            ARMul_CPInterface *interf,
                            toolconf config,
                            void *sibling)
{
 /* Check config to see what type of event scheduling to use */
 int iCoreCycleEvents = (ARMul_Properties(state) & ARM_ARM9_Prop);



  IGNORE(num); IGNORE(config); IGNORE(sibling);

  interf->handle=(void *)state;
 
  if (iCoreCycleEvents)
      {
        ARMul_PrettyPrint(state,", Core cycle event scheduling");
       interf->cdp=CP4CDPCore;
       interf->ldc=CP4LDCCore;
       interf->stc=CP4STCCore;
       interf->mrc=CP4MRCCore;
       interf->mcr=CP4MCRCore;
      }
  else
      {
        ARMul_PrettyPrint(state,", System cycle event scheduling");
       interf->cdp=CP4CDP;
       interf->ldc=CP4LDC;
       interf->stc=CP4STC;
       interf->mrc=CP4MRC;
       interf->mcr=CP4MCR;
      }
  interf->read=CP4Read;
  interf->write=CP4Write;

  return ARMulErr_NoError;
}


/* Register CP5 co-processor */
static ARMul_Error Val2Init(ARMul_State *state,
                            unsigned num,
                            ARMul_CPInterface *interf,
                            toolconf config,
                            void *sibling)
{
 /* Check config to see what type of event scheduling to use */
 int iCoreCycleEvents = (ARMul_Properties(state) & ARM_ARM9_Prop);

 IGNORE(num); IGNORE(config); IGNORE(sibling);

  interf->handle=(void *)state;
 
  if (iCoreCycleEvents)
      {
       interf->cdp=CP5CDPCore;
       interf->ldc=CP5LDCCore;
       interf->stc=CP5STCCore;
       interf->mrc=CP5MRCCore;
       interf->mcr=CP5MCRCore;
      }
  else
      {
       interf->cdp=CP5CDP;
       interf->ldc=CP5LDC;
       interf->stc=CP5STC;
       interf->mrc=CP5MRC;
       interf->mcr=CP5MCR;
      }
  interf->read=CP5Read;
  interf->write=CP5Write;

  return ARMulErr_NoError;
}


/* Register CP8 co-processor */
static ARMul_Error Val3Init(ARMul_State *state,
                            unsigned num,
                            ARMul_CPInterface *interf,
                            toolconf config,
                            void *sibling)
{
 /* Check config to see what type of event scheduling to use */
 int iCoreCycleEvents = (ARMul_Properties(state) & ARM_ARM9_Prop);

  IGNORE(num); IGNORE(config); IGNORE(sibling);


  interf->handle=(void *)state;
  if (iCoreCycleEvents)
      {
       interf->cdp=CP8CDPCore;
       interf->ldc=CP8LDCCore;
       interf->stc=CP8STCCore;
       interf->mrc=CP8MRCCore;
       interf->mcr=CP8MCRCore;
      }
  else
      {
       interf->cdp=CP8CDP;
       interf->ldc=CP8LDC;
       interf->stc=CP8STC;
       interf->mrc=CP8MRC;
       interf->mcr=CP8MCR;
      }
  interf->read=CP8Read;
  interf->write=CP8Write;

  return ARMulErr_NoError;
}



/* Validation suite co-processor model */

#include <time.h>
#include <errno.h>
#include <string.h>

#include "armdefs.h"
#include "rdi_hif.h"

/*
 * Time for the Operating System to initialise itself.
 */

static ARMul_Error OSInit(ARMul_State *state,
#ifndef NEW_OS_INTERFACE
                          ARMul_OSInterface *interf,
#endif
                          toolconf config);

#ifdef NEW_OS_INTERFACE
const ARMul_ModelStub ARMul_ValidateOS = {
  OSInit,
  "ValidateOS"
};
static unsigned OSHandleSWI(
   void *handle, ARMword vector, ARMword pc, ARMword instr);
#else
const ARMul_OSStub ARMul_ValidateOS = {
  OSInit,
  (tag_t)"Validate"
  };
static unsigned OSHandleSWI(void *handle,ARMword number);
static unsigned OSException(void *handle, ARMword vector, ARMword pc);
#endif

static ARMul_Error OSInit(ARMul_State *state,
#ifndef NEW_OS_INTERFACE
                          ARMul_OSInterface *interf,
#endif
                          toolconf config)
{
  ARMul_Error err;

  ARMul_PrettyPrint(state, ", ARM Validation OS");

  err=ARMul_CoProAttach(state,4,Val1Init,config,state);
  if (err==ARMulErr_NoError) 
        {
        err=ARMul_CoProAttach(state,5,Val2Init,config,state);
        }
  if (err==ARMulErr_NoError) 
        {
        err=ARMul_CoProAttach(state,8,Val3Init,config,state);
        }  
  if (err!=ARMulErr_NoError) return err;

#ifdef NEW_OS_INTERFACE
  ARMul_InstallExceptionHandler(state, OSHandleSWI, (void *)state);
#else
  interf->handle_swi=OSHandleSWI;
  interf->exception=OSException;
  interf->handle=(void *)state;
#endif

  return ARMulErr_NoError;
}

/*
 * The emulator calls this routine when a SWI instruction is encuntered. The
 * parameter passed is the SWI number (lower 24 bits of the instruction).
 */

static unsigned OSHandleSWI(
#ifdef NEW_OS_INTERFACE
    void *handle, ARMword vector, ARMword pc, ARMword instr
#else
    void *handle,ARMword number
#endif
)
{
  ARMul_State *state=(ARMul_State *)handle;
#ifdef NEW_OS_INTERFACE
  ARMword number;
  IGNORE(pc);
  if (vector != ARM_V_SWI) return FALSE;
  number = instr & 0xffffff;
#endif
  switch (number) {
  case 0x11:
    ARMul_HaltEmulation(state,FALSE);
    return(TRUE) ;
  case 0x01:
    if (ARMul_GetMode(state)>USER32MODE) /* Stay in entry (ARM/THUMB) state */
      ARMul_SetCPSR(state, (ARMul_GetCPSR(state) & 0xffffffe0) | 0x13) ;
    else
      ARMul_SetCPSR(state, (ARMul_GetCPSR(state) & 0xffffffc0) | 0x3) ;
    return(TRUE) ;
  default:
    return(FALSE) ;
  }
}

#ifndef NEW_OS_INTERFACE
/*
 * The emulator calls this routine when an Exception occurs.  The second
 * parameter is the address of the relevant exception vector.  Returning
 * FALSE from this routine causes the trap to be taken, TRUE causes it to
 * be ignored (so set state->Emulate to FALSE!).
 */

static unsigned OSException(void *handle, ARMword vector, ARMword pc)
{ /* don't use this here */
  IGNORE(handle); IGNORE(vector); IGNORE(pc);
  return(FALSE) ;
}
#endif

/*
 * install ourselves as a Coprocessor, without claiming to be an OS model
 */
static ARMul_Error CPInit(ARMul_State *state,
                          toolconf config)
{
    ARMul_Error err;

    ARMul_PrettyPrint(state, ", ARM Validation system");

    err = ARMul_CoProAttach(state, 4, Val1Init, config, state);

    if (err == ARMulErr_NoError)
        {
        err = ARMul_CoProAttach(state, 5, Val2Init, config, state);
        }
    if (err == ARMulErr_NoError)
        {
        err = ARMul_CoProAttach(state, 8, Val3Init, config, state);
        }

    if (err != ARMulErr_NoError)
        return err;

    return ARMulErr_NoError;
}

const ARMul_ModelStub ARMul_ValidateCP =
{
    CPInit,
    (tag_t)"ValidateCP"
};

/* EOF validate.c */

#if 0
static unsigned CP4CDPCore(void *handle, unsigned type, ARMword instr)
{
 static unsigned long finish = 0 ;
 ARMword howlong ;
 ARMul_State *state=(ARMul_State *)handle;
 const ARMul_Cycles *cycles;


 cycles = ARMul_ReadCycles(state); /* Get a handle to cycles structure */

 howlong = ValCP4Reg[BITS(12,13)]; /* See how long we have to busy wait for - if we can */

 /*ARMul_ConsolePrint(state,"ARM8 CP4CDP howlong =%d from reg %d, cycles=%08x\n", howlong,BITS(12,13),cycles->CoreCycles);*/


 if (BITS(20,23)==0) 
    {
    if (type == ARMul_FIRST) 
       { 
        /* First time call */
        if (CP4Busy.iBusyWaiting)
            {
            /* The co-processor might be busy waiting*/ 
            /* See if it is busy waiting still - nb I've ignored the reset case for now!*/
            finish = CP4Busy.ulBusyWaitStart + CP4Busy.iBusyWaitDuration;
            if ( cycles->CoreCycles <= finish )
                {
                /* Yes it is. Return with ARMul_BUSY */
                /*ARMul_ConsolePrint(state,"CP4 still busy - wait..\n");*/
                return(ARMul_BUSY);
                }
            else
                {
                 CP4Busy.iBusyWaiting=0; /* Assert that we aren't busy waiting any more */
                }
            }
        /* No it isn't, so set up the new busy wait and return ARMul_DONE */
        /*ARMul_ConsolePrint(state,"BUSY WAITING CP4 for %d cycles.\n",howlong);*/
        CP4Busy.iBusyWaiting=1;
        CP4Busy.ulBusyWaitStart = cycles->CoreCycles;
        CP4Busy.iBusyWaitDuration = howlong+1;
        return(ARMul_DONE);
        }                                
    else if (type == ARMul_INTERRUPT)
        {
        /*ARMul_ConsolePrint(state,"Interrupt pending\n");*/
        return(ARMul_DONE);
        }
    else if (type == ARMul_BUSY) 
        {
        /* Lets check whether we're still busy waiting! */
         if (CP4Busy.iBusyWaiting)
            {
            /* The co-processor might be busy waiting*/ 
            /* See if it is busy waiting still - nb I've ignored the reset case for now!*/
            finish = CP4Busy.ulBusyWaitStart + CP4Busy.iBusyWaitDuration;
            if ( cycles->CoreCycles <= finish )
                {
                /* Yes it is. Return with ARMul_BUSY */
                /*ARMul_ConsolePrint(state,"CP4 still busy - wait..\n");*/
                return(ARMul_BUSY);
                }
            }
        /* AHA. We're no longer busy, so we can process the request */

        /*ARMul_ConsolePrint(state,"CP4 finished BUSY Waiting!\n");
        ARMul_ConsolePrint(state,"BUSY WAITING CP4 (again) for %d cycles.\n",howlong);*/
        CP4Busy.iBusyWaiting=1;
        CP4Busy.ulBusyWaitStart = cycles->CoreCycles;
        CP4Busy.iBusyWaitDuration = howlong+1;
        return(ARMul_DONE);
        }
    }
   /* If we get there then we received an instruction we couldn't process */
  return(ARMul_CANT) ;
}


/* Non ARM9 CP4CDP variant */
static unsigned CP4CDP(void *handle, unsigned type, ARMword instr)
{
 static unsigned long finish = 0 ;
 ARMword howlong ;
 ARMul_State *state=(ARMul_State *)handle;
 const ARMul_Cycles *cycles;


 cycles = ARMul_ReadCycles(state); /* Get a handle to cycles structure */

 howlong = ValCP4Reg[BITS(12,13)]; /* See how long we have to busy wait for - if we can */

 /*ARMul_ConsolePrint(state,"ARM8 CP4CDP howlong =%d from reg %d, cycles=%08x\n", howlong,BITS(12,13),cycles->Total); */


 if (BITS(20,23)==0) 
    {
    if (type == ARMul_FIRST) 
       { 
        /* First time call */
        if (CP4Busy.iBusyWaiting)
            {
            /* The co-processor might be busy waiting*/ 
            /* See if it is busy waiting still - nb I've ignored the reset case for now!*/
            finish = CP4Busy.ulBusyWaitStart + CP4Busy.iBusyWaitDuration;
            if ( cycles->Total <= finish )
                {
                /* Yes it is. Return with ARMul_BUSY */
                /*ARMul_ConsolePrint(state,"CP4 still busy - wait..\n");*/
                return(ARMul_BUSY);
                }
            else
                {
                 CP4Busy.iBusyWaiting=0; /* Assert that we aren't busy waiting any more */
                }
            }
        /* No it isn't, so set up the new busy wait and return ARMul_DONE */
        /*ARMul_ConsolePrint(state,"BUSY WAITING CP4 for %d cycles.\n",howlong);*/
        CP4Busy.iBusyWaiting=1;
        CP4Busy.ulBusyWaitStart = cycles->Total;
        CP4Busy.iBusyWaitDuration = howlong+1;
        return(ARMul_DONE);
        }                                
    else if (type == ARMul_INTERRUPT)
        {
        /*ARMul_ConsolePrint(state,"Interrupt pending\n");*/
        return(ARMul_DONE);
        }
    else if (type == ARMul_BUSY) 
        {
        /* Lets check whether we're still busy waiting! */
         if (CP4Busy.iBusyWaiting)
            {
            /* The co-processor might be busy waiting*/ 
            /* See if it is busy waiting still - nb I've ignored the reset case for now!*/
            finish = CP4Busy.ulBusyWaitStart + CP4Busy.iBusyWaitDuration;
            if ( cycles->Total <= finish )
                {
                /* Yes it is. Return with ARMul_BUSY */
                /*ARMul_ConsolePrint(state,"CP4 still busy - wait..\n");*/
                return(ARMul_BUSY);
                }
            }
        /* AHA. We're no longer busy, so we can process the request */

        /*ARMul_ConsolePrint(state,"CP4 finished BUSY Waiting!\n");
        ARMul_ConsolePrint(state,"BUSY WAITING CP4 (again) for %d cycles.\n",howlong);*/
        CP4Busy.iBusyWaiting=1;
        CP4Busy.ulBusyWaitStart = cycles->Total;
        CP4Busy.iBusyWaitDuration = howlong+1;
        return(ARMul_DONE);
        }
    }
   /* If we get there then we received an instruction we couldn't process */
  return(ARMul_CANT) ;
}


/* CP5 CDP instruction - behaviour not checked*/ 
static unsigned CP5CDP(void *handle, unsigned type, ARMword instr)
{
 static unsigned long finish ;
 
 ARMword howlong ;
 ARMul_State *state=(ARMul_State *)handle;
 const ARMul_Cycles *cycles;



/* howlong is used to store the number of cycles in advance of now
   the required action happens - See Regme in the VHDL */

 /* QQQ The busy wait case might not be used, if it is confirm
        that CRm ( bits 0..3 ) are used to pass the wait count */
 howlong = ValCP5Reg[BITS(12,13)] ; /* Only allow 8 bits of reg value*/
 cycles = ARMul_ReadCycles(state);

 /*ARMul_ConsolePrint(state,"CP5CDP, howlong = %d, reg = %d\n",howlong, BITS(12,13));*/
 


 if (BITS(20,23) == 0)
     {
      if (type == ARMul_FIRST) 
          { 
           /* First time call */
           if (CP5Busy.iBusyWaiting)
               {
                /* The co-processor might be busy waiting*/     
                /* See if it is busy waiting still - nb I've ignored the reset case for now!*/
                finish = CP5Busy.ulBusyWaitStart + CP5Busy.iBusyWaitDuration;
                if ( cycles->Total <= finish )
                    {
                     /* Yes it is. Return with ARMul_BUSY */
                     /*ARMul_ConsolePrint(state,"CP5 still busy - wait..\n");*/
                     return(ARMul_BUSY);
                    }
                else
                    {
                     CP8Busy.iBusyWaiting=0; /* Assert that we aren't busy waiting any more */
                    }

                }
            /* No it isn't, so set up the new busy wait and return ARMul_DONE */
            /*ARMul_ConsolePrint(state,"BUSY WAITING CP5 for %d cycles.\n",howlong);*/
            CP5Busy.iBusyWaiting=1;
            CP5Busy.ulBusyWaitStart = cycles->Total;
            CP5Busy.iBusyWaitDuration = howlong;
            return(ARMul_DONE); 
         }                                
      else if (type == ARMul_INTERRUPT)
         {
         /* ARMul_ConsolePrint(state,"Interrupt pending\n");*/
          return(ARMul_DONE);
         }
      else if (type == ARMul_BUSY) 
         {
          /* Lets check whether we're still busy waiting! */
          if (CP5Busy.iBusyWaiting)
              {
               /* The co-processor might be busy waiting*/      
               /* See if it is busy waiting still - nb I've ignored the reset case for now!*/
               finish = CP5Busy.ulBusyWaitStart + CP5Busy.iBusyWaitDuration;
               if ( cycles->Total <= finish )
                   {
                    /* Yes it is. Return with ARMul_BUSY */
                    /*ARMul_ConsolePrint(state,"CP5 still busy - wait..\n");*/
                    return(ARMul_BUSY);
                   }
              }
        /* AHA. We're no longer busy, so we can process the request */

        /*ARMul_ConsolePrint(state,"CP5 finished BUSY Waiting!\n");
        ARMul_ConsolePrint(state,"BUSY WAITING CP5 (again) for %d cycles.\n",howlong);*/
        CP5Busy.iBusyWaiting=1;
        CP5Busy.ulBusyWaitStart = cycles->Total;
        CP5Busy.iBusyWaitDuration = howlong;
        return(ARMul_DONE);
        }
    return(ARMul_CANT); /* catch all */
    }

/* If we get here then it wasn't opcode 0 - busy wait */

/* All the following instructions share a common busy wait behaviour - if we're
   busy waiting then we do nothing except return BUSY, otherwise we execute the
   instruction. */

 /* Are we busy waiting */
          
  if (CP5Busy.iBusyWaiting)
      {
       /* The co-processor might be busy waiting*/      
       /* See if it is busy waiting still - nb I've ignored the reset case for now!*/
       finish = CP5Busy.ulBusyWaitStart + CP5Busy.iBusyWaitDuration;
       if ( cycles->Total <= finish )
           {
            /* Yes it is. Return with ARMul_BUSY */
            /*ARMul_ConsolePrint(state,"CP5 still busy - wait..\n");*/
            return(ARMul_BUSY);
           }
      }

  /* AHA. We're no longer busy, so we can process the request */

  CP5Busy.iBusyWaiting=0; /* make it clear we're no longer busy waiting */
 
 /* Bits 23..20 is correct for opcode_1*/
  switch((int)BITS(20,23)) {

    case 1 : if (howlong == 0)
                {
                ARMul_SetNfiq(state,LOW);
                }
              else
                {
                /*ARMul_ConsolePrint(state,"schedule a FIQ %d cycles from now (at %08x)\n",howlong,cycles->Total+howlong);*/
                ARMul_ScheduleEvent(state,howlong,DoAFIQ,state);
                }

 
             return(ARMul_DONE);
    case 2 : if (howlong == 0)
                {
                ARMul_SetNirq(state,LOW);
                }
             else
                {
                ARMul_ScheduleEvent(state,howlong,DoAIRQ,state);
                }
              return(ARMul_DONE);
    case 3 : ARMul_SetNfiq(state,HIGH);
             return(ARMul_DONE);
    case 4 : ARMul_SetNirq(state,HIGH);
             return(ARMul_DONE);
    case 5 : /* Read Timer */
                 ValCP5Reg[BITS(12,13)] = cycles->Total;
          /*ARMul_ConsolePrint(state,"Read Trickbox timer - %08x\n",cycles->Total);
          ARMul_ConsolePrint(state,"Stored in C%d\n",BITS(12,13));*/
             return(ARMul_DONE);
     }
 return(ARMul_CANT) ;
 }

 

/* CP8 CDP instruction - behaviour not checked*/ 
static unsigned CP8CDPCore(void *handle, unsigned type, ARMword instr)
{
 ARMword howlong = ValCP8Reg[BITS(12,13)]; /* See how long we have to busy wait for - if we can */
 ARMul_State *state=(ARMul_State *)handle;
 const ARMul_Cycles *cycles = ARMul_ReadCycles(state); /* Get a handle to cycles structure */

 /*ARMul_ConsolePrint(state,"ARM8 CP8CDP howlong =%d from reg %d, cycles=%08x\n", howlong,BITS(12,13),cycles->CoreCycles);*/

 if (BITS(20,23)==0) 
    {
    if (type == ARMul_FIRST) 
       { 
        /* First time call */
        if (CP8Busy.iBusyWaiting)
            {
            /* The co-processor might be busy waiting*/ 
            /* See if it is busy waiting still - nb I've ignored the reset case for now!*/
            finish = CP8Busy.ulBusyWaitStart + CP8Busy.iBusyWaitDuration;
            if ( cycles->CoreCycles <= finish )
                {
                /* Yes it is. Return with ARMul_BUSY */
                 /*ARMul_ConsolePrint(state,"CP8 still busy - wait..\n");*/
                return(ARMul_BUSY);
                }
            else
                {
                 CP8Busy.iBusyWaiting=0; /* Assert that we aren't busy waiting any more */
                }

            }
        /* No it isn't, so set up the new busy wait and return ARMul_DONE */
        /*ARMul_ConsolePrint(state,"BUSY WAITING CP8 for %d cycles.\n",howlong);*/
        CP8Busy.iBusyWaiting=1;
        CP8Busy.ulBusyWaitStart = cycles->CoreCycles;
        CP8Busy.iBusyWaitDuration = howlong;
        return(ARMul_DONE);
        }                                
    else if (type == ARMul_INTERRUPT)
        {
        /*ARMul_ConsolePrint(state,"Interrupt pending\n");*/
        return(ARMul_DONE);
        }
    else if (type == ARMul_BUSY) 
        {
        /* Lets check whether we're still busy waiting! */
         if (CP8Busy.iBusyWaiting)
            {
            /* The co-processor might be busy waiting*/ 
            /* See if it is busy waiting still - nb I've ignored the reset case for now!*/
            finish = CP8Busy.ulBusyWaitStart + CP8Busy.iBusyWaitDuration;
            if ( cycles->CoreCycles <= finish )
                {
                /* Yes it is. Return with ARMul_BUSY */
                /*ARMul_ConsolePrint(state,"CP8 still busy - wait..\n");*/
                return(ARMul_BUSY);
                }
            }
        /* AHA. We're no longer busy, so we can process the request */

        /*ARMul_ConsolePrint(state,"CP8 finished BUSY Waiting!\n");
        ARMul_ConsolePrint(state,"BUSY WAITING CP8 (again) for %d cycles.\n",howlong);*/
        CP8Busy.iBusyWaiting=1;
        CP8Busy.ulBusyWaitStart = cycles->CoreCycles;
        CP8Busy.iBusyWaitDuration = howlong;
        return(ARMul_DONE);
        }
    }
   /* If we get there then we received an instruction we couldn't process */
  return(ARMul_CANT) ;
 }





/* CP8 CDP instruction - behaviour not checked*/ 
static unsigned CP8CDP(void *handle, unsigned type, ARMword instr)
{
 static unsigned long finish = 0 ;
 ARMword howlong ;
 ARMul_State *state=(ARMul_State *)handle;
 const ARMul_Cycles *cycles;


 cycles = ARMul_ReadCycles(state); /* Get a handle to cycles structure */

 howlong = ValCP8Reg[BITS(12,13)]; /* See how long we have to busy wait for - if we can */

 /*ARMul_ConsolePrint(state,"ARM8 CP8CDP howlong =%d from reg %d, cycles=%08x\n", howlong,BITS(12,13),cycles->Total);*/


 if (BITS(20,23)==0) 
    {
    if (type == ARMul_FIRST) 
       { 
        /* First time call */
        if (CP8Busy.iBusyWaiting)
            {
            /* The co-processor might be busy waiting*/ 
            /* See if it is busy waiting still - nb I've ignored the reset case for now!*/
            finish = CP8Busy.ulBusyWaitStart + CP8Busy.iBusyWaitDuration;
            if ( cycles->Total <= finish )
                {
                /* Yes it is. Return with ARMul_BUSY */
                /*ARMul_ConsolePrint(state,"CP8 still busy - wait..\n");*/
                return(ARMul_BUSY);
                }
            else
                {
                 CP8Busy.iBusyWaiting=0; /* Assert that we aren't busy waiting any more */
                }

            }
        /* No it isn't, so set up the new busy wait and return ARMul_DONE */
        /*ARMul_ConsolePrint(state,"BUSY WAITING CP8 for %d cycles.\n",howlong);*/
        CP8Busy.iBusyWaiting=1;
        CP8Busy.ulBusyWaitStart = cycles->Total;
        CP8Busy.iBusyWaitDuration = howlong;
        return(ARMul_DONE);
        }                                
    else if (type == ARMul_INTERRUPT)
        {
        /*ARMul_ConsolePrint(state,"Interrupt pending\n");*/
        return(ARMul_DONE);
        }
    else if (type == ARMul_BUSY) 
        {
        /* Lets check whether we're still busy waiting! */
         if (CP8Busy.iBusyWaiting)
            {
            /* The co-processor might be busy waiting*/ 
            /* See if it is busy waiting still - nb I've ignored the reset case for now!*/
            finish = CP8Busy.ulBusyWaitStart + CP8Busy.iBusyWaitDuration;
            if ( cycles->Total <= finish )
                {
                /* Yes it is. Return with ARMul_BUSY */
                /*ARMul_ConsolePrint(state,"CP8 still busy - wait..\n");*/
                return(ARMul_BUSY);
                }
            }
        /* AHA. We're no longer busy, so we can process the request */

        /*ARMul_ConsolePrint(state,"CP8 finished BUSY Waiting!\n");
        ARMul_ConsolePrint(state,"BUSY WAITING CP8 (again) for %d cycles.\n",howlong);*/
        CP8Busy.iBusyWaiting=1;
        CP8Busy.ulBusyWaitStart = cycles->Total;
        CP8Busy.iBusyWaitDuration = howlong;
        return(ARMul_DONE);
        }
    }
   /* If we get there then we received an instruction we couldn't process */
  return(ARMul_CANT) ;
 }

#endif 
