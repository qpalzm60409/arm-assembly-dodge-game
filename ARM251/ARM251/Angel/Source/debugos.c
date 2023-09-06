/* -*-C-*-
 *
 * $Revision: 1.23.2.4 $
 *   $Author: rivimey $
 *     $Date: 1998/10/19 12:21:59 $
 *
 * Copyright (c) 1995-96 Advanced RISC Machines Limited
 * All Rights Reserved.
 *
 * debugos.c: Operating system debug support functions.
 */
#include "angel.h"
#include "devconf.h"
#include "adp.h"
#include "debug.h"
#include "debugos.h"
#include "endian.h"
#include "debughwi.h"
#include "logging.h"
#include "serlock.h"
#include "stacks.h"
#include "devclnt.h"
#include "support.h"

/* use the memset in suppasm.s */
#define memset __rt_memset

struct AngelOSInfo OSinfo_struct;

/* Breakpoint stuff */
static struct BreakPoint *head_bp;
struct BreakPoint bp[32];
word free_bps;

int memory_is_being_accessed = 0;
volatile int memory_access_aborted = 0;

#if defined(SYSTEM_COPROCESSOR_SUPPORTED) && SYSTEM_COPROCESSOR_SUPPORTED > 0
/* Define the coprocessor data structures */
#define SYS_COPROC_MAX_REGS 32
static int sys_coproc_reg_entries;
static CP_CoProRegDesc sys_coproc_regdesc[SYS_COPROC_MAX_REGS];

#endif

unsigned int angel_debugger_endianess;

int angelOS_SemiHostingEnabled;

/*
 *  Function: 
 *   Purpose: 
 *
 *    Params: 
 *       Input: 
 *
 *   Returns: 
 */
#if DEBUG == 1
int angelOS_PrintState(word OSinfo1, word OSinfo2)
{
    int c;
    angel_RegBlock *rb;
    unsigned *r;

    rb = Angel_AccessApplicationRegBlock();

    LogInfo(LOG_DEBUGOS, ( "State:  OSinfo1: 0x%x.  OSinfo2: 0x%x.\n", OSinfo1, OSinfo2));
    LogInfo(LOG_DEBUGOS, ( "Registers r0 - r14:-\n"));

    /* r0 is separate, r1-7 are in a single block */
    LogInfo(LOG_DEBUGOS, ( " r0 0x%08x ", rb->r0));
    r = &rb->r1;
    for (c = 1; c < 8; c++, r++)
    {
        LogInfo(LOG_DEBUGOS, ( " r%d 0x%08x ", c, *r));
        if (c == 3 || c == 7)
            LogInfo(LOG_DEBUGOS, ("\n"));
    }
    
    if ((rb->cpsr & 0x1f) == FIQmode)
        r = &rb->r8fiq;
    else
        r = &rb->r8usr;
    
    for (; c < 13; c++, r++)
    {
        LogInfo(LOG_DEBUGOS, ( "%sr%d 0x%08x ", (c < 10)? " ":"", c, *r));
        if (c == 11)
            LogInfo(LOG_DEBUGOS, ("\n"));
    }

    /* now do r13, r14... banked across most modes */
    LogInfo(LOG_DEBUGOS, ( "r13 0x%08x ", Angel_GetBankedReg(rb, rb->cpsr, 13)));
    LogInfo(LOG_DEBUGOS, ( "r14 0x%08x\n", Angel_GetBankedReg(rb, rb->cpsr, 14)));
    
    LogInfo(LOG_DEBUGOS, ( " pc 0x%08x cpsr 0x%08x\n", rb->pc, rb->cpsr));

    /* cannot do banked modes any more! */

    return RDIError_NoError;
}
#else
#define angelOS_PrintState(OSinfo1, OSinfo2)
#endif


/*
 *  Function: 
 *   Purpose: 
 *
 *    Params: 
 *       Input: 
 *
 *   Returns: 
 */
int angelOS_Initialise(void)
{
    LogInfo(LOG_DEBUGOS, ( "Entered angelOS_Initialise.\n"));

    /* Initialise OS info structure. */
    OSinfo_struct.infoBitset = ADP_Info_Target_HW
        | ADP_Info_Target_CanInquireBufferSize
        | ADP_Info_Target_CanReloadAgent
#if defined(THUMB_SUPPORT) && THUMB_SUPPORT!=0
        | ADP_Info_Target_Thumb
#endif
#if PROFILE_SUPPORTED
        | ADP_Info_Target_Profiling
#endif
        ;

    OSinfo_struct.infoPoints = 0;
    OSinfo_struct.infoStep = 0;

    /* Initialise breakpoints. */
    head_bp = NULL;
    free_bps = 0xFFFFFFFF;

    /* semihosting on by default */
    angelOS_SemiHostingEnabled = 1;

#if defined(SYSTEM_COPROCESSOR_SUPPORTED) && SYSTEM_COPROCESSOR_SUPPORTED > 0
    /* Initialise system coprocessor description array; the first part
     * of this is correct for any processor with a system co-processor
     */

    /* Reg 0 is read only */
    sys_coproc_regdesc[0].rmin = 0;
    sys_coproc_regdesc[0].rmax = 0;
    sys_coproc_regdesc[0].nbytes = 4;
    sys_coproc_regdesc[0].access = 0x1;  /* Read only, use register access */
    sys_coproc_regdesc[0].accessinst.cprt.read_b0 = 0;
    sys_coproc_regdesc[0].accessinst.cprt.read_b1 = 0;
    sys_coproc_regdesc[0].accessinst.cprt.write_b0 = 0;
    sys_coproc_regdesc[0].accessinst.cprt.write_b1 = 0;

    /* Registers 1 to 3 are simple control registers */
    sys_coproc_regdesc[1].rmin = 1;
    sys_coproc_regdesc[1].rmax = 3;
    sys_coproc_regdesc[1].nbytes = 4;
    sys_coproc_regdesc[1].access = 0x3;  /* Read/Write, use register access */
    sys_coproc_regdesc[1].accessinst.cprt.read_b0 = 0;
    sys_coproc_regdesc[1].accessinst.cprt.read_b1 = 0;
    sys_coproc_regdesc[1].accessinst.cprt.write_b0 = 0;
    sys_coproc_regdesc[1].accessinst.cprt.write_b1 = 0;

    /* Register 4 is undefined */

    /* Registers 5 and 6 are fault registers, similar to 1 to 3 */
    sys_coproc_regdesc[2].rmin = 5;
    sys_coproc_regdesc[2].rmax = 6;
    sys_coproc_regdesc[2].nbytes = 4;
    sys_coproc_regdesc[2].access = 0x3;  /* Read/Write, use register access */
    sys_coproc_regdesc[2].accessinst.cprt.read_b0 = 0;
    sys_coproc_regdesc[2].accessinst.cprt.read_b1 = 0;
    sys_coproc_regdesc[2].accessinst.cprt.write_b0 = 0;
    sys_coproc_regdesc[2].accessinst.cprt.write_b1 = 0;

    /* Registers 7 and 8 provide cache and writebuffer operations; since
     * these need a variety of different opcode_2 and CRm values it does
     * not make sense to provide a default for these
     */

    /* StrongARM only - Register 15 provides some additional clock
     * control functions. Again there are no sensible defaults for
     * opcode_2 and CRm
     */

    sys_coproc_reg_entries = 3;
#endif

    memory_is_being_accessed = 0;
    memory_access_aborted = 0;

    return RDIError_NoError;
}

/*
 *  Function: angelOS_MemRead
 *
 *   Purpose: To transfer memory from the application memory space
 *         (that is, not Angel's memory) to an internal buffer in
 *         Angel, typically in response to a debugger request for 
 *         that memory. Because of this, it is important that halfword
 *         reads and word reads are used when transferring 2 or 4 
 *         bytes of data. Of course 1 bytes reads must use single byte
 *         transfers. For 3 or >4 bytes the limitations are less
 *         severe, but as implemented:
 *         
 *         - use word copy if addresses are word aligned and copying integral
 *           number of words
 *         
 *         - use halfword copy if addresses are halfword aligned but not
 *           word aligned and number of bytes is even
 *         
 *         - otherwise, copy byte-by-byte.
 *         
 *         These rules are fairly simple to understand and include the
 *         above requirements as a subset.
 *         
 *         Finally, the code must protect Angel from overwriting itself,
 *         and may thus refuse the request with a privilegde error if
 *         the requested address is in a protected region.
 *         
 *         This routine uses the volatile variables 
 *            memory_is_being_accessed   - flag to data abort handler
 *            memory_access_aborted      - count of data aborts caught
 *         
 *         The data abort handler is expected, when memory_is_being_accessed
 *         is non-zero, to increment the count of aborts found in
 *         memory_access_aborted whenever an abort occurs, returning to the
 *         *next* instruction. That is, this code requires that if data aborts
 *         are retried then when the handler returns the attempt *will*
 *         succeed; in this case, the aborted count should *not* be
 *         incremented.
 *         
 *    Params: 
 *       Input:
 *              OsInfo[12]  - unused; would be used for thread context etc
 *              Address     - the address in the target to read
 *              Nbytes      - the number of bytes to read
 *              DataArea    - where to put the data once read
 *              
 *      Output: 
 *              BytesRead   - return of how many bytes were transferred
 *
 *   Returns: Error code: OK if memort transferred, DataAbort or Priviledge
 *                        error otherwise.
 */
int
angelOS_MemRead(word OSinfo1, word OSinfo2, word Address, word Nbytes,
                byte * DataArea, word * BytesRead)
{
    int all_permitted;

    IGNORE(OSinfo1);
    IGNORE(OSinfo2);

    /*
     * If we detect that either the start or end of the area is not
     * a permitted read area then give up immediately.
     * Otherwise transfer the whole lot a byte at a time.
     * This is preferable to transferring words due to a number of
     * problems: endianess and non alignment of data in target memory
     * and/or the datablock.
     */

    all_permitted = (READ_PERMITTED(Address) &&
                     READ_PERMITTED(Address + Nbytes - 1));

    if (!all_permitted)
    {
        *BytesRead = 0;
        return RDIError_InsufficientPrivilege;
    }

    memory_is_being_accessed = 1;
    memory_access_aborted = 0;

    /*
     *
     */

    if ( (((int)DataArea | Address | Nbytes) & 0x3) == 0)
    {
        word *swAddress = (word *)Address;
        word *dwAddress = (word *)DataArea;
        word Nwords = Nbytes >> 2;
        
        while (Nwords >= 4)
        {
            *dwAddress++ = *swAddress++;
            *dwAddress++ = *swAddress++;
            *dwAddress++ = *swAddress++;
            *dwAddress++ = *swAddress++;

            Nwords -= 4;

            /* check to see access was ok */
            if (memory_access_aborted)
                break;
        }
        switch(Nwords)
        {
            case 3:
                *dwAddress++ = *swAddress++;
                /*fallthrough*/
            case 2:
                *dwAddress++ = *swAddress++;
                /*fallthrough*/
            case 1:
                *dwAddress++ = *swAddress++;
                break;
                
            default:
                /* could have 0 or > 3 words here... either way, do nothing */
                break;
        }
        /* modify ssAddress by the number of aborts; this should
         * get us back to the real abort address
         */
        swAddress -= memory_access_aborted;
        
        *BytesRead = (int)swAddress - (int)Address;
    }
    else if ((((int)DataArea | (int)Address | Nbytes) & 0x1) == 0)
    {
        short *ssAddress = (short *)Address;
        short *dsAddress = (short *)DataArea;
        word Nshorts = Nbytes >> 1;

        while (Nshorts >= 4)
        {
            *dsAddress++ = *ssAddress++;
            *dsAddress++ = *ssAddress++;
            *dsAddress++ = *ssAddress++;
            *dsAddress++ = *ssAddress++;

            Nshorts -= 4;

            /* check to see access was ok */
            if (memory_access_aborted)
                break;
        }
        switch(Nshorts)
        {
            case 3:
                *dsAddress++ = *ssAddress++;
                /*fallthrough*/
            case 2:
                *dsAddress++ = *ssAddress++;
                /*fallthrough*/
            case 1:
                *dsAddress++ = *ssAddress++;
                break;
                
            default:
                /* could have 0 or > 3 words here... either way, do nothing */
                break;
        }

        /* modify ssAddress by the number of aborts; this should
         * get us back to the real abort address
         */
        ssAddress -= memory_access_aborted;
        *BytesRead = (int)ssAddress - (int)Address;
    }
    else
    {
        char *sbAddress = (char *)Address;
        char *dbAddress = (char *)DataArea;

        while (Nbytes >= 4)
        {
            *dbAddress++ = *sbAddress++;
            *dbAddress++ = *sbAddress++;
            *dbAddress++ = *sbAddress++;
            *dbAddress++ = *sbAddress++;

            Nbytes -= 4;

            /* check to see access was ok */
            if (memory_access_aborted)
                break;
        }
        switch(Nbytes)
        {
            case 3:
                *dbAddress++ = *sbAddress++;
                /*fallthrough*/
            case 2:
                *dbAddress++ = *sbAddress++;
                /*fallthrough*/
            case 1:
                *dbAddress++ = *sbAddress++;
                break;
                
            default:
                /* could have 0 or > 3 words here... either way, do nothing */
                break;
        }

        /* modify ssAddress by the number of aborts; this should
         * get us back to the real abort address
         */
        sbAddress -= memory_access_aborted;
        *BytesRead = (int)sbAddress - (int)Address;
    }


    memory_is_being_accessed = 0;

    return memory_access_aborted ? RDIError_DataAbort : RDIError_NoError;
}

/*
 *  Function: angelOS_MemWrite
 *   Purpose: To transfer memory from an internal buffer in Angel, to
 *         the application memory space (that is, not Angel's memory)
 *         typically in response to a debugger request to write that
 *         memory. Because of this, it is important that halfword
 *         reads and word reads are used when transferring 2 or 4 
 *         bytes of data. Of course 1 bytes reads must use single byte
 *         transfers. For 3 or >4 bytes the limitations are less
 *         severe, but as implemented:
 *         
 *         - use word copy if addresses are word aligned and copying integral
 *           number of words
 *         
 *         - use halfword copy if addresses are halfword aligned but not
 *           word aligned and number of bytes is even
 *         
 *         - otherwise, copy byte-by-byte.
 *         
 *         These rules are fairly simple to understand and include the
 *         above requirements as a subset.
 *         
 *         Finally, the code must protect Angel from overwriting itself,
 *         and may thus refuse the request with a privilegde error if
 *         the requested address is in a protected region.
 *         
 *         This routine uses the volatile variables 
 *            memory_is_being_accessed   - flag to data abort handler
 *            memory_access_aborted      - count of data aborts caught
 *         
 *         The data abort handler is expected, when memory_is_being_accessed
 *         is non-zero, to increment the count of aborts found in
 *         memory_access_aborted whenever an abort occurs, returning to the
 *         *next* instruction. That is, this code requires that if data aborts
 *         are retried then when the handler returns the attempt *will*
 *         succeed; in this case, the aborted count should *not* be
 *         incremented.
 *         
 *    Params: 
 *       Input:
 *              OsInfo[12]  - unused; would be used for thread context etc
 *              Address     - the address in the target to write the data
 *              Nbytes      - the number of bytes to write
 *              DataArea    - where to get the data from
 *              
 *      Output: 
 *              BytesWritten- return of how many bytes were transferred
 *
 *   Returns: Error code: OK if memort transferred, DataAbort or Priviledge
 *                        error otherwise.
 */
int
angelOS_MemWrite(word OSinfo1, word OSinfo2, word Address, word Nbytes,
                 byte * DataArea, word * BytesWritten)
{
    int all_permitted;

    IGNORE(OSinfo1);
    IGNORE(OSinfo2);

    /*
     * If we detect that either the start or end of the area is not
     * a permitted write area then give up immediately.
     * Otherwise transfer the whole lot a byte (or word) at a time.
     * This is preferable to transferring words due to a number of
     * problesm: endianess and non alignment of data in target memory
     * and/or the datablock.
     */

    all_permitted = (WRITE_PERMITTED(Address) &&
                     WRITE_PERMITTED(Address + Nbytes - 1));

    if (!all_permitted)
    {
        *BytesWritten = 0;
        return RDIError_InsufficientPrivilege;
    }

    memory_is_being_accessed = 1;
    memory_access_aborted = 0;

    /*
     * use halfword copy if addresses are halfword aligned but not
     * word aligned and number of bytes is not odd
     *
     * use word copy if addresses are word aligned and copying integral number
     * of words
     *      *(((char *)Address) + c) = DataArea[c];
     *
     * otherwise, copy byte-by-byte.
     *
     */
    if ((((int)DataArea | (int)Address | Nbytes) & 0x3) == 0)
    {
        int *swAddress = (int*)DataArea;
        int *dwAddress = (int*)Address;
        word Nwords = Nbytes >> 2;
        
        while (Nwords >= 4)
        {
            *dwAddress++ = *swAddress++;
            *dwAddress++ = *swAddress++;
            *dwAddress++ = *swAddress++;
            *dwAddress++ = *swAddress++;
            Nwords -= 4;
            /* check to see access was ok */
            if (memory_access_aborted)
                break;
        }
        
        switch(Nwords)
        {
            case 3:
                *dwAddress++ = *swAddress++;
                /*fallthrough*/
            case 2:
                *dwAddress++ = *swAddress++;
                /*fallthrough*/
            case 1:
                *dwAddress++ = *swAddress++;
                break;
                
            default:
                /* could have 0 or > 3 words here... either way, do nothing */
                break;
        }
        dwAddress -= memory_access_aborted;
        *BytesWritten = (int)dwAddress - (int)Address;
    }
    else if ((((int)DataArea | (int)Address | Nbytes) & 0x1) == 0)
    {
        short *ssAddress = (short *)DataArea;
        short *dsAddress = (short *)Address;
        word Nshorts = Nbytes >> 1;

        while (Nshorts >= 4)
        {
            *dsAddress++ = *ssAddress++;
            *dsAddress++ = *ssAddress++;
            *dsAddress++ = *ssAddress++;
            *dsAddress++ = *ssAddress++;
            Nshorts -= 4;
            /* check to see access was ok */
            if (memory_access_aborted)
                break;
        }
        switch(Nshorts)
        {
            case 3:
                *dsAddress++ = *ssAddress++;
                /*fallthrough*/
            case 2:
                *dsAddress++ = *ssAddress++;
                /*fallthrough*/
            case 1:
                *dsAddress++ = *ssAddress++;
                break;
                
            default:
                /* could have 0 or > 3 halfwords here... either way, do nothing */
                break;
        }
        dsAddress -= memory_access_aborted;
        *BytesWritten = (int)dsAddress - (int)Address;
    }
    else
    {
        char *sbAddress = (char *)DataArea;
        char *dbAddress = (char *)Address;

        while (Nbytes >= 4)
        {
            *dbAddress++ = *sbAddress++;
            *dbAddress++ = *sbAddress++;
            *dbAddress++ = *sbAddress++;
            *dbAddress++ = *sbAddress++;
            Nbytes -= 4;
            /* check to see access was ok */
            if (memory_access_aborted)
                break;
        }
        switch(Nbytes)
        {
            case 3:
                *dbAddress++ = *sbAddress++;
                /*fallthrough*/
            case 2:
                *dbAddress++ = *sbAddress++;
                /*fallthrough*/
            case 1:
                *dbAddress++ = *sbAddress++;
                break;
                
            default:
                /* could have 0 or > 3 bytes here... either way, do nothing */
                break;
        }
        dbAddress -= memory_access_aborted;
        *BytesWritten = (int)dbAddress - (int)Address;
    }

    memory_is_being_accessed = 0;


#if CACHE_SUPPORTED
    Cache_IBR((char *)Address, (char *)(Address + Nbytes - 1));
#endif

    return memory_access_aborted ? RDIError_DataAbort : RDIError_NoError;
}


/*
 *  Function: cpuread
 *   Purpose: Convenient function to read the next word from a byte buffer and
 *            then increment a counter appropriately
 *
 *    Params: 
 *       Input: 
 *
 *   Returns: 
 */

static word
cpuread(byte * wd, int *c)
{
    word w;

    w = GET32LE(wd + *c);
    *c += 4;
    return (w);
}


/*
 *  Function: angelOS_MemWrite
 *   Purpose: 
 *
 *    Params: 
 *       Input: 
 *
 *   Returns: 
 */
int
angelOS_CPUWrite(word OSinfo1, word OSinfo2, word Mode, word Mask,
                 byte * WriteData)
{
    int i, c = 0;
    angel_RegBlock *rb;
    unsigned *r, cpsr;

    IGNORE(OSinfo1);
    IGNORE(OSinfo2);

    LogInfo(LOG_DEBUGOS, ( "angelOS_CPUWrite: Mode = 0x%x, Mask = 0x%x.\n", Mode, Mask));

    rb = Angel_AccessApplicationRegBlock();

    if (rb == NULL)
        return RDIError_Error;

    cpsr = rb->cpsr;
    
    if (Mode == ADP_CPUmode_Current)
        Mode = (cpsr & ModeMask);

    if (Mode == SYS32mode)
        Mode = USR32mode;

    if ((Mode & ModeMask) != Mode)
    {
        LogWarning(LOG_DEBUGOS, ( "ERROR: Unrecognised mode in angelOS_CPUWrite.\n"));
        return RDIError_BadCPUStateSetting;
    }

    /*
     * requested mode must match application mode  - if not then refuse
     * to do it.
     */
    if (Mode != (cpsr & ModeMask))
        return RDIError_BadCPUStateSetting;

    if ((Mask & 0x7FFFF) != Mask)
    {
        LogWarning(LOG_DEBUGOS, ( "ERROR: Mask invalid in angelOS_CPUWrite.\n"));
        return RDIError_Error;
    }

    /* deal first with r0 */
    if (Mask & 1L)
        rb->r0 = cpuread(WriteData, &c);

    /* now r1 -- r7 */
    r = &rb->r1;
    for (i = 1; i < 8; i++, r++)
        if ((1L << i) & Mask)
            *r = cpuread(WriteData, &c);

    /* now r8 - r12, banked FIQ / NotFIQ */
    if ((cpsr & ModeMask) == FIQmode)
        r = &rb->r8fiq;
    else
        r = &rb->r8usr;
    
    for (; i < 13; i++, r++)
        if ((1L << i) & Mask)
            *r = cpuread(WriteData, &c);

    /* finally, r13, r14 */
    if (Mask & (1L << 13))
        *Angel_AdrBankedReg(rb, Mode, 13) = cpuread(WriteData, &c);
    
    if (Mask & (1L << 14))
        *Angel_AdrBankedReg(rb, Mode, 14) = cpuread(WriteData, &c);

    /* now r15 -- pc */
    if (ADP_CPUread_PCmode & Mask)
        rb->pc = cpuread(WriteData, &c);

    if (ADP_CPUread_PCnomode & Mask)
        rb->pc = cpuread(WriteData, &c);

    if (ADP_CPUread_CPSR & Mask)
        rb->cpsr = cpuread(WriteData, &c);

    /* banked register 15 is the SPSR for the mode */
    if (Mode != USR32mode) /* and Mode != SYS32mode -- see above */
        if (ADP_CPUread_SPSR & Mask)
            *Angel_AdrBankedReg(rb, Mode, 15) = cpuread(WriteData, &c);

    return RDIError_NoError;
}

/*
 *  Function: cpuwrite
 *   Purpose: 
 *            Convenient function to write the next word to a byte buffer and
 *            then increment a counter appropriately
 *
 *    Params: 
 *       Input: 
 *
 *   Returns: 
 */
static void
cpuwrite(word w, byte * wd, int *c)
{
    PUT32LE(wd + *c, w);
    *c += 4;
}

/*
 *  Function: angelOS_CPURead
 *   Purpose: 
 *
 *    Params: 
 *       Input: 
 *
 *   Returns: 
 */
int
angelOS_CPURead(word OSinfo1, word OSinfo2, word Mode, word Mask,
                byte * ReadData)
{
    int i, c = 0;
    angel_RegBlock *rb;
    unsigned *r, cpsr;

    IGNORE(OSinfo1);
    IGNORE(OSinfo2);

    LogInfo(LOG_DEBUGOS, ( "angelOS_CPURead: Mode = 0x%x, Mask = 0x%x.\n", Mode, Mask));

    rb = Angel_AccessApplicationRegBlock();

    if (rb == NULL)
        return RDIError_Error;

    cpsr = rb->cpsr;
    
    if (Mode == ADP_CPUmode_Current)
        Mode = (rb->cpsr & ModeMask);

    if (Mode == SYS32mode)
        Mode = USR32mode;

    /* If we have made a request for a 26 bit mode, change it to the
     * 32 bit equivalent mode - this allows such requests to work
     * properly on the StrongARM.
     */
    Mode |= 0x10;

    if ((Mode & ModeMask) != Mode)
    {
        LogWarning(LOG_DEBUGOS, ( "ERROR: Unrecognised mode in angelOS_CPURead.\n"));
        return RDIError_BadCPUStateSetting;
    }

    if ((Mask & 0x7FFFF) != Mask)
    {
        LogWarning(LOG_DEBUGOS, ( "ERROR: Mask invalid in angelOS_CPURead.\n"));
        return RDIError_Error;
    }


    /* deal first with r0 */
    if (Mask & 1L)
        cpuwrite(rb->r0, ReadData, &c);

    /* now r1 -- r7 */
    r = &rb->r1;
    for (i = 1; i < 8; i++, r++)
        if ((1L << i) & Mask)
            cpuwrite(*r, ReadData, &c);

    /* now r8 - r12, banked FIQ / NotFIQ */
    if ((cpsr & ModeMask) == FIQmode)
        r = &rb->r8fiq;
    else
        r = &rb->r8usr;
    
    for (; i < 13; i++, r++)
        if ((1L << i) & Mask)
            cpuwrite(*r, ReadData, &c);

    /* finally, r13, r14 */
    if (Mask & (1L << 13))
        cpuwrite(Angel_GetBankedReg(rb, Mode, 13), ReadData, &c);
    
    if (Mask & (1L << 14))
        cpuwrite(Angel_GetBankedReg(rb, Mode, 14), ReadData, &c);

    /* now r15 -- pc */
    if (ADP_CPUread_PCmode & Mask)
        cpuwrite(rb->pc, ReadData, &c);

    if (ADP_CPUread_PCnomode & Mask)
    {
        if (!(cpsr) & (1 << 4))
        {
            /* if in 26 bit mode */
            cpuwrite(rb->pc & 0x0FFFFFFb, ReadData, &c);
        }
        else
        {
            /* 32 bit, return full PC */
            cpuwrite(rb->pc, ReadData, &c);
        }
    }
    
    if (ADP_CPUread_CPSR & Mask)
        cpuwrite(rb->cpsr, ReadData, &c);

    /* banked register 15 is the SPSR for the mode */
    if (Mode != USR32mode) /* and Mode != SYS32mode -- see above */
        if (ADP_CPUread_SPSR & Mask)
            cpuwrite(Angel_GetBankedReg(rb, Mode, 15), ReadData, &c);    

    return RDIError_NoError;
}

#if defined(SYSTEM_COPROCESSOR_SUPPORTED) && SYSTEM_COPROCESSOR_SUPPORTED > 0
static int
angelOS_CPSysWriteReg(word regnum, word * Data)
{
    /* This function writes a single system coprocessor register. It
     * does so as defined by the system coprocessor register
     * description.  To do this we must build an instruction and then
     * execute it.
     */
    int i;
    word WriteInstrs[2] =
    {0xEE000F10,                /* MCR p15, 0, r0, c0, c0, 0 */
     0xE1A0F00E                 /* MOV pc,lr (return) */
    };
    void (*WriteFunction) (word);

    /* Find the entry */
    for (i = 0; i < sys_coproc_reg_entries; i++)
    {
        if (regnum >= sys_coproc_regdesc[i].rmin &&
            regnum <= sys_coproc_regdesc[i].rmax)
        {
            /* Found the description - patch up the instruction */
            WriteInstrs[0] |= (sys_coproc_regdesc[i].accessinst.cprt.write_b0
                               & 0xff);
            WriteInstrs[0] |= (sys_coproc_regdesc[i].accessinst.cprt.write_b1
                               & 0xff) << 16;
            if ((sys_coproc_regdesc[i].accessinst.cprt.write_b1 & 0x0f) == 0)
            {
                /* Use the coprocessor reg number from the call in the instruction */
                WriteInstrs[0] |= regnum << 16;
            }
#if CACHE_SUPPORTED
            Cache_IBR(&WriteInstrs[0], &WriteInstrs[1]);
#endif
            WriteFunction = (void (*)(word))WriteInstrs;

            Angel_EnterSVC();

            WriteFunction(*Data);

            Angel_ExitToUSR();

            return RDIError_NoError;
        }
    }
    /* Not found */
    return RDIError_UnknownCoProState;
}
#endif

int
angelOS_CPWrite(word OSinfo1, word OSinfo2, word CPnum, word Mask,
                word * Data, word * nbytes)
{
#if defined(SYSTEM_COPROCESSOR_SUPPORTED) && SYSTEM_COPROCESSOR_SUPPORTED > 0
    int regno;
    word status = RDIError_NoError;
    word *dp = Data;

#else
    IGNORE(Mask);
    IGNORE(CPnum);
    IGNORE(Data);
#endif

    IGNORE(OSinfo1);
    IGNORE(OSinfo2);
    IGNORE(nbytes);

#if defined(SYSTEM_COPROCESSOR_SUPPORTED) && SYSTEM_COPROCESSOR_SUPPORTED > 0
    if (CPnum != 15)
#endif
        return RDIError_UnknownCoPro;

#if defined(SYSTEM_COPROCESSOR_SUPPORTED) && SYSTEM_COPROCESSOR_SUPPORTED > 0
    for (regno = 0; regno < 32; regno++)
    {
        if ((Mask >> regno) & 0x1)
        {
            status = angelOS_CPSysWriteReg(regno, dp);
            if (status != RDIError_NoError)
            {
                return status;
            }
            dp++;
        }
    }
    return status;
#endif
}

#if defined(SYSTEM_COPROCESSOR_SUPPORTED) && SYSTEM_COPROCESSOR_SUPPORTED > 0
  /* This function writes a single system coprocessor register. It
   * does so as defined by the system coprocessor register
   * description.  To do this we must build an instruction and then
   * execute it.
   */
static int
angelOS_CPSysReadReg(word regnum, word * Data)
{
    int i;
    word ReadInstrs[2] =
    {0xEE100F10,                /* MRC p15, 0, r0, c0, c0, 0 */
     0xE1A0F00E                 /* MOV pc,lr (return) */
    };

    word(*ReadFunction) (void);
    /* Find the entry */
    for (i = 0; i < sys_coproc_reg_entries; i++)
    {
        if (regnum >= sys_coproc_regdesc[i].rmin &&
            regnum <= sys_coproc_regdesc[i].rmax)
        {
            /* Found the description - patch up the instruction */
            ReadInstrs[0] |= (sys_coproc_regdesc[i].accessinst.cprt.read_b0
                              & 0xff);
            ReadInstrs[0] |= (sys_coproc_regdesc[i].accessinst.cprt.read_b1
                              & 0xff) << 16;
            if ((sys_coproc_regdesc[i].accessinst.cprt.read_b1 & 0x0f) == 0)
            {
                /* Use the coprocessor reg number from the call in the
                 *  instruction
                 */
                ReadInstrs[0] |= regnum << 16;
            }
#if CACHE_SUPPORTED
            Cache_IBR(&ReadInstrs[0], &ReadInstrs[1]);
#endif
            ReadFunction = (word(*)(void))ReadInstrs;

            Angel_EnterSVC();

            *Data = ReadFunction();

            Angel_ExitToUSR();

            return RDIError_NoError;
        }
        /* Not found */
        return RDIError_UnknownCoProState;
    }
}
#endif

int 
angelOS_CPRead(word OSinfo1, word OSinfo2, word CPnum, word Mask,
               word * Data, word * nbytes)
{
#if defined(SYSTEM_COPROCESSOR_SUPPORTED) && SYSTEM_COPROCESSOR_SUPPORTED > 0
    int regno;
    word status = RDIError_NoError;
    word *dp = Data;

#else
    IGNORE(CPnum);
    IGNORE(Mask);
    IGNORE(Data);
#endif

    IGNORE(OSinfo1);
    IGNORE(OSinfo2);

    *nbytes = 0;

#if defined(SYSTEM_COPROCESSOR_SUPPORTED) && SYSTEM_COPROCESSOR_SUPPORTED > 0
    if (CPnum != 15)
#endif
        return RDIError_UnknownCoPro;

#if defined(SYSTEM_COPROCESSOR_SUPPORTED) && SYSTEM_COPROCESSOR_SUPPORTED > 0
    for (regno = 0; regno < 32; regno++)
    {
        if ((Mask >> regno) & 0x1)
        {
            status = angelOS_CPSysReadReg(regno, dp);
            if (status != RDIError_NoError)
                return status;
            *nbytes += 4;
            dp++;
        }
    }
    return status;
#endif
}

static struct BreakPoint *
getfreebp(void)
{
    int c;
    struct BreakPoint *b;

    LogInfo(LOG_DEBUGOS, ( "getfreebp: free_bps = %x.\n", (word) free_bps));

    for (c = 0; c < 32; c++)
    {
        if ((free_bps & (1 << c)) != 0)
        {
            free_bps &= ~(1 << c);

            LogInfo(LOG_DEBUGOS, ( "Allocating breakpoint %d.\n", c));

            b = &(bp[c]);
            b->bpno = c;
            return (b);
        }
    }

    return NULL;
}

int 
angelOS_SetBreak(word OSinfo1, word OSinfo2,
                 struct SetPointValue *SetData,
                 struct PointValueReturn *ReturnValue)
{
    struct BreakPoint *b = NULL, **pp, **p;
    BreakPointSize size;

    LogInfo(LOG_DEBUGOS, ( "angelOS_SetBreak:\n"));

    size = (SetData->pointType & ADP_SetBreak_Thumb) ? bs_16bit : bs_32bit;

    SetData->pointType &= ~ADP_SetBreak_Thumb;
    if (SetData->pointType != ADP_SetBreak_EqualsAddress)
        return RDIError_UnimplementedType;

    for (p = NULL, pp = &head_bp; (b = *pp) != NULL; p = pp, pp = &b->next)
        if (b->address == SetData->pointAddress)
            break;

    if (b != NULL)
    {
        if (size != b->size)
            return RDIError_ConflictingPoint;

        b->count++;
    }
    else
    {
        word n;
        word status;

        /* We have to be careful with these as they can hold 2 or 4 bytes */
        word data;
        word testRead;

        if (free_bps == 0)
            return RDIError_NoMorePoints;

        b = getfreebp();
        *pp = b;
        b->next = NULL;
        b->count = 1;
        b->size = size;
        b->address = SetData->pointAddress;

        status = angelOS_MemRead(OSinfo1, OSinfo2, b->address,
                                 size == bs_32bit ? 4 : 2,
                                 (byte *) (&b->inst), &n);

        if (status != RDIError_NoError)
        {
            free_bps |= (1 << b->bpno);
            if (p != NULL)
                *p = NULL;
            return status;
        }

        if (size == bs_32bit)
        {
            *((word *) & data) = angel_BreakPointInstruction_ARM;
        }
        else
        {
            *((short *)&data) = (short)angel_BreakPointInstruction_THUMB;
        }
        status = angelOS_MemWrite(OSinfo1, OSinfo2, b->address,
                                  size == bs_32bit ? 4 : 2,
                                  (byte *) (&data), &n);

        if (status != RDIError_NoError)
        {
            free_bps |= (1 << b->bpno);
            if (p != NULL)
                *p = NULL;
            return status;
        }

        /* Check that the break point has been written. */
        status = angelOS_MemRead(OSinfo1, OSinfo2, b->address,
                                 size == bs_32bit ? 4 : 2,
                                 (byte *) (&testRead), &n);
        if (status != RDIError_NoError ||
            __rt_memcmp((void *)&testRead, (void *)&data, size == bs_32bit ? 4 : 2))
        {
            LogWarning(LOG_DEBUGOS, ("Error setting breakpoint at 0x%x: orig data 0x%x current 0x%x.\n", b->address, b->inst, testRead));
            free_bps |= (1 << b->bpno);
            if (p != NULL)
                *p = NULL;
            return RDIError_CantSetPoint;
        }
    }

    ReturnValue->pointHandle = (word) b;

    LogInfo(LOG_DEBUGOS, ("handle = %x head_bp = %x free_bps = %x.\n",
              ReturnValue->pointHandle, (word) head_bp, (word) free_bps));

    return RDIError_NoError;
}


int 
angelOS_ClearBreak(word OSinfo1, word OSinfo2, word Handle)
{
    struct BreakPoint *b, *p, **pp;
    word status, n;

    LogInfo(LOG_DEBUGOS, ( "angelOS_ClearBreak: Handle = %x.\n", Handle));

    b = (struct BreakPoint *)Handle;
    if (b == NULL)
        return RDIError_NoSuchPoint;

    for (pp = &head_bp; (p = *pp) != b; pp = &p->next)
    {
        LogInfo(LOG_DEBUGOS, ( "Current bp address = %x.\n", (word) p));

        if (p == NULL)
            return RDIError_NoSuchPoint;
    }

    if (--b->count != 0)
        return RDIError_NoError;

    *pp = b->next;

    LogInfo(LOG_DEBUGOS, ( "Freeing breakpoint %d.\n", b->bpno));

    free_bps |= (1 << b->bpno);
    status = angelOS_MemWrite(OSinfo1, OSinfo2, b->address,
                              b->size == bs_32bit ? 4 : 2,
                              (byte *) (&(b->inst)), &n);

    if (status != RDIError_NoError)
    {
        LogWarning(LOG_DEBUGOS, ( "Memwrite error in CB.\n"));
        return status;
    }

    LogInfo(LOG_DEBUGOS, ( "Leaving angelOS_ClearBreak function.\n"));
    return RDIError_NoError;
}

int 
angelOS_SetWatch(word OSinfo1, word OSinfo2,
                 struct SetPointValue *SetData,
                 struct PointValueReturn *ReturnValue)
{
    IGNORE(OSinfo1);
    IGNORE(OSinfo2);
    IGNORE(ReturnValue);
    IGNORE(SetData);
    return RDIError_UnimplementedMessage;
}

int 
angelOS_ClearWatch(word OSinfo1, word OSinfo2, word Handle)
{
    IGNORE(OSinfo1);
    IGNORE(OSinfo2);
    IGNORE(Handle);
    return RDIError_UnimplementedMessage;
}

int 
angelOS_BreakPointStatus(word OSinfo1, word OSinfo2, word Handle,
                         word * Hw, word * Type)
{
    IGNORE(OSinfo1);
    IGNORE(OSinfo2);
    IGNORE(Handle);
    IGNORE(Hw);
    IGNORE(Type);
    return RDIError_UnimplementedMessage;
}

int 
angelOS_WatchPointStatus(word OSinfo1, word OSinfo2, word Handle,
                         word * Hw, word * Type)
{
    IGNORE(OSinfo1);
    IGNORE(OSinfo2);
    IGNORE(Handle);
    IGNORE(Hw);
    IGNORE(Type);
    return RDIError_UnimplementedMessage;
}

int 
angelOS_SetupStep(word OSinfo1, word OSinfo2, word StepCount,
                  void **handle)
{
    IGNORE(OSinfo1);
    IGNORE(OSinfo2);
    IGNORE(StepCount);
    IGNORE(handle);
    return RDIError_UnimplementedMessage;
}

int 
angelOS_DoStep(void *handle)
{
    IGNORE(handle);
    return RDIError_UnimplementedMessage;
}

int 
angelOS_Execute(word OSinfo1, word OSinfo2)
{
    angel_RegBlock *rb;
    
    IGNORE(OSinfo1);
    IGNORE(OSinfo2);

    /* Find a task with the Application priority */
    rb = angel_AccessQueuedRegBlock(TP_Application);

    if (rb == NULL)
    {
        LogError(LOG_DEBUGOS, ( "angelOS_Execute: No application!\n"));
    }
    else
    {
        LogInfo(LOG_DEBUGOS, ( "angelOS_Execute: From PC = %08lx, CPSR = %08lx\n",
                               rb->pc, rb->cpsr));
    }
    
    Angel_BlockApplication(0);

    return RDIError_NoError;
}


int 
angelOS_InterruptExecution(word OSinfo1, word OSinfo2)
{
    LogInfo(LOG_DEBUGOS, ( "angelOS_InterruptExecution\n"));

    IGNORE(OSinfo1);
    IGNORE(OSinfo2);

    Angel_BlockApplication(1);

    return RDIError_NoError;
}

/* should this be in a header? */
extern void debug_ThreadStopped(word OSinfo1, word OSinfo2, unsigned int event,
                                unsigned int subcode);

extern void 
angelOS_ThreadStopped(word type)
{
    LogInfo(LOG_DEBUGOS, ( "Entered function angelOS_ThreadStopped.\n"));

    Angel_BlockApplication(1);

    angelOS_PrintState(ADP_HandleUnknown, ADP_HandleUnknown);

    Angel_QueueCallback((angel_CallbackFn) debug_ThreadStopped, TP_AngelCallBack,
                        (void *)ADP_HandleUnknown,
                        (void *)ADP_HandleUnknown,
                        (void *)type,
                        NULL);

    return;
}

int 
AngelOS_SetHandler(int VectorID, void (*handler) (),
                   void *(*oldHandler) ())
{
    IGNORE(VectorID);
    IGNORE(handler);
    IGNORE(oldHandler);
    return RDIError_UnimplementedMessage;
}

struct AngelOSInfo *
angelOS_ReturnInfo(void)
{
    return &OSinfo_struct;
}

/*
 *        Function:  angelOS_InitialiseApplication
 *
 *         Purpose:  Initialise the application context to 'default' values,
 *                   which are usable at least until the applicatio itself
 *                   can insert the correct ones. Note that in addition to
 *                   this initialisatin, the debugger may well use CPUwrite
 *                   to change other things, notably the CPU mode and PC
 *                   values.
 *                   
 *                   Rationale: The IRQ/FIQ/UND/ABT stacks are set up as for
 *                   Angel tasks; this will usually be ok, as you usually
 *                   won't be playing with these modes, and usually appl use
 *                   won't interfere with Angel use. However, if you intend
 *                   to use these modes you *should* initialise the stacks to
 *                   point at your own memory. Be aware though this may well
 *                   mean that Ange will use a few words of your stacks as it
 *                   enters it's own exception handler.
 *                   
 *                   The USR mode stack etc is set to the values obtained from
 *                   SWI GETHEAPDESC; this is what Angel has historically done,
 *                   and seems sane enough. To match this, the stack limit is
 *                   set to the appl stack limit. The SVC stack cannot be set
 *                   to the same value -- if in USR mode and you do a SWI then
 *                   Angel will use the base of your USR stack, overwriting
 *                   values which the program will reload later and usually
 *                   resulting in a crash.
 *                   
 *                   If the debugger starts the application in SVC mode, then
 *                   a better value for SL would have been the appl svc stack
 *                   limit, but as r10 is not banked btw. usr and svc we have
 *                   to choose. I have arbitrarily chosen to prefer USR.
 *                   
 *                   Lastly, the cpsr is set to interrupts disabled state for
 *                   those interrupts which are not shared with Angel, preventing
 *                   potential spurious interrupts.
 *                   
 *
 *       Arguments:  OSinfo1, OSinfo2 -- unused.
 *
 *         Returns:  RDIError_NoError - all ok
 *                   RDIError_Error - couldn't find the appl task!
 *
 *   Pre-conditions: Serialiser initialised, application task created.
 *
 *  Post-conditions: Serialiser initialised, application task created and set
 *                   up.
 *                   
 */

int 
angelOS_InitialiseApplication(word OSinfo1, word OSinfo2)
{
    angel_TaskQueueItem *appltqi;

    IGNORE(OSinfo1);
    IGNORE(OSinfo2);

    LogInfo(LOG_DEBUGOS, ( "angelOS_InitialiseApplication\n"));

    Angel_FlushApplicationCallbacks();

    appltqi = Angel_AccessApplicationTask();

    if (appltqi)
    {
        unsigned svcstacktop;

        LogInfo(LOG_DEBUGOS, ( "zeroing rb... \n"));

        /* clear out the register context, so we start in a known state */
        memset(&appltqi->rb, 0, sizeof(angel_RegBlock));

        LogInfo(LOG_DEBUGOS, ( "setting exception regs rb... \n"));

        /*
         * set up the non-SVC exception mode's SP/LR registers to the
         * Angel stack values, as they must be pointing somewhere
         * valid.
         */
        Angel_SetupExceptionRegisters(appltqi);

        /* set up SVC/USR mode state to point at the appl heap area.
         * these registers must also be valid from initialisation,
         * as you can't return from a SWI, including the GETHEAPDESC
         * SWI, unless you have a valid stack pointer.
         */
        svcstacktop   = Angel_StackBase + Angel_ApplSVCStackOffset;

        /* svc limit = "Angel_StackBase + Angel_ApplSVCStackLimitOffset", but
         * we must choose between USR and SVC limits -- prefer USR ATM
         */

        appltqi->rb.r13svc = svcstacktop;
        appltqi->rb.r13usr = (unsigned)angel_heapstackdesc.stacktop;
        appltqi->rb.r10usr = (unsigned)angel_heapstackdesc.stacklimit;
        appltqi->rb.r11usr = 0;
        appltqi->rb.cpsr = NotAngelInterruptMask | USR32mode;

        return RDIError_NoError;
    }
    else
        return RDIError_Error;
}

int 
angelOS_End(word debugID)
{
    /*
     * The host debugger is shutting down the debug session with ID debugID,
     * so reset the debug session. Currently we do nothing - the host
     * should renegotiate the baud rate back to the default if appropriate.
     */
    IGNORE(debugID);
    LogInfo(LOG_DEBUGOS, ( "angelOS_End\n"));
    return RDIError_NoError;
}


int 
angelOS_MemInfo(word * meminfo)
{
    /* TODO: Change this as required. */
    meminfo = 0;
    return RDIError_NoError;
}


int 
angelOS_DescribeCoPro(byte cpno, struct CP_CoProDesc *cpd)
{
    int ret_code = RDIError_Error;

    LogInfo(LOG_DEBUGOS, ( "angelOS_DescribeCoPro\n"));
#if defined(SYSTEM_COPROCESSOR_SUPPORTED) && SYSTEM_COPROCESSOR_SUPPORTED > 0
    /*
     * Check that this is for the system coprocessor - only one
     * supported by this code currently
     */
    if (cpno == 15)
    {
        int i;

        ret_code = RDIError_NoError;
        if (cpd->entries > SYS_COPROC_MAX_REGS)
        {
            sys_coproc_reg_entries = SYS_COPROC_MAX_REGS;
            ret_code = RDIError_OutOfStore;
        }
        else
            sys_coproc_reg_entries = cpd->entries;

        for (i = 0; i < sys_coproc_reg_entries; ++i)
            sys_coproc_regdesc[i] = cpd->regdesc[i];
    }
    else
        ret_code = RDIError_UnknownCoPro;
#else
    IGNORE(cpno);
    IGNORE(cpd);
    ret_code = RDIError_UnknownCoPro;
#endif

    return ret_code;
}


int 
angelOS_RequestCoProDesc(byte cpno, struct CP_CoProDesc *cpd)
{
    int ret_code;

    LogInfo(LOG_DEBUGOS, ( "angelOS_RequestCoProDesc\n"));
#if defined(SYSTEM_COPROCESSOR_SUPPORTED) && SYSTEM_COPROCESSOR_SUPPORTED > 0
    if (cpno == 15)
    {
        int i;

        ret_code = RDIError_NoError;

        /* cpd->entries is set to space available before call to this fn. */
        if (sys_coproc_reg_entries > cpd->entries)
            ret_code = RDIError_BufferFull;
        else
            cpd->entries = sys_coproc_reg_entries;

        for (i = 0; i < cpd->entries; ++i)
            cpd->regdesc[i] = sys_coproc_regdesc[i];
    }
    else
        ret_code = RDIError_UnknownCoPro;

#else
    IGNORE(cpno);
    IGNORE(cpd);
    ret_code = RDIError_UnknownCoPro;
#endif

    return ret_code;
}


int 
angelOS_VectorCatch(word OSinfo1, word OSinfo2, word VectorCatch)
{
    IGNORE(OSinfo1);
    IGNORE(OSinfo2);

    /* TODO: Do something with VectorCatch? */
    IGNORE(VectorCatch);

    return RDIError_NoError;
}


int 
angelOS_SemiHosting_SetState(word OSinfo1, word OSinfo2,
                             word SemiHosting_State)
{
    IGNORE(OSinfo1);
    IGNORE(OSinfo2);

    angelOS_SemiHostingEnabled = SemiHosting_State;
    return RDIError_NoError;
}


int 
angelOS_SemiHosting_GetState(word OSinfo1, word OSinfo2,
                             word * SemiHosting_State)
{
    IGNORE(OSinfo1);
    IGNORE(OSinfo2);

    *SemiHosting_State = angelOS_SemiHostingEnabled;
    return RDIError_NoError;
}


int 
angelOS_SemiHosting_SetVector(word OSinfo1, word OSinfo2,
                              word SemiHosting_Vector)
{
    IGNORE(OSinfo1);
    IGNORE(OSinfo2);

    LogWarning(LOG_DEBUGOS, ( "WARNING: SemiHosting_SetVector not implemented!\n"));
    IGNORE(SemiHosting_Vector);
    return RDIError_UnimplementedMessage;
}


int 
angelOS_SemiHosting_GetVector(word OSinfo1, word OSinfo2,
                              word * SemiHosting_Vector)
{
    IGNORE(OSinfo1);
    IGNORE(OSinfo2);

    LogInfo(LOG_DEBUGOS, ( "SemiHosting Vector always 8\n"));
    *SemiHosting_Vector = 8;
    return RDIError_NoError;
}


int 
angelOS_Ctrl_Log(word OSinfo1, word OSinfo2, word * LogSetting)
{
    IGNORE(OSinfo1);
    IGNORE(OSinfo2);

    /* Mask out logsetting from debug_Status  and shift into correct posn. */
    *LogSetting = (debug_Status & 0x6) >> 1;
    return RDIError_NoError;
}


int 
angelOS_Ctrl_SetLog(word OSinfo1, word OSinfo2, word LogSetting)
{
    IGNORE(OSinfo1);
    IGNORE(OSinfo2);

    debug_Status &= 0xFFF9;     /* Remove previous setting. */
    debug_Status |= (LogSetting << 1);  /* Set the new one. */
    return RDIError_NoError;
}


int 
angelOS_CanChangeSHSWI(void)
{
    /* Has to be done at compile-time for Angel */
    return RDIError_Error;
}


static word loadagent_address = (word) - 1;
static word loadagent_size;
static word loadagent_sofar;

int 
angelOS_LoadConfigData(word OSinfo1, word OSinfo2,
                       word nbytes, byte const *data)
{
    IGNORE(OSinfo1);
    IGNORE(OSinfo2);

    if (loadagent_address == (word) - 1)
        return RDIError_NotInDownload;

    if (nbytes + loadagent_sofar > loadagent_size)
        return RDIError_BadConfigData;

    __rt_memcpy((byte *) loadagent_address + loadagent_sofar, data, nbytes);
    loadagent_sofar += nbytes;

    return RDIError_NoError;
}


void 
angelOS_ExecuteNewAgent(word start_address)
{
    void (*exe) (void) = NULL;
    ansibodge a;

    IGNORE(start_address);

    a.w = loadagent_address;
    exe = a.vfn;

    LogInfo(LOG_DEBUGOS, ( "angelOS_ExecuteNewAgent: jumping to %08x\n", exe));

    Angel_EnterSVC();
    exe();
}


int 
angelOS_LoadAgent(word OSinfo1, word OSinfo2, word loadaddress,
                  word nbytes)
{
    IGNORE(OSinfo1);
    IGNORE(OSinfo2);

    LogInfo(LOG_DEBUGOS, ( "angelOS_LoadAgent: to %08x size %d.\n", loadaddress, nbytes));

    /* Do some checking and possible transformations on loadaddress */
    /* FOR NOW just force it to constant set in devconf.h */
    IGNORE(loadaddress);
    loadagent_address = Angel_DownloadAgentArea;

    loadagent_size = nbytes;
    loadagent_sofar = 0;

    return RDIError_NoError;
}


/* EOF debugos.c */
