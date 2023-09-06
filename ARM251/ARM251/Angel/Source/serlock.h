/* -*-C-*-
 *
 * $Revision: 1.12.2.3 $
 *   $Author: rivimey $
 *     $Date: 1998/10/19 12:22:02 $
 *
 * Copyright (c) 1996 Advanced RISC Machines Limited.
 * All Rights Reserved.
 *
 *   Project: ANGEL
 *
 *     Title: Definitions for the serial kernel of Angel.
 *
 */

#ifndef angel_serlock_h
#define angel_serlock_h

#ifdef TARGET
# include "angel.h"
#else
# include "host.h"
#endif

/****************************************************************************/

/*
 * This definition is central to the way in which the serial core of Angel
 * keeps track of interrupted and schedulable states.
 *
 * The following structure is capable of holding the complete set of
 * registers, sufficient to resume an instruction stream in exactly the same
 * state as it was in when it was interrupted (or interrupted itself). The .r
 * field holds the nominal registers r0 to r15 (= pc), while the .cpsr and
 * .spsr fields hold the Current Program Status register and the Saved
 * Program Status Register respectively.
 *
 * The identity of banked registers is implied by the mode held in the .cpsr
 * field.
 *
 * Note: This structure is duplicated in assembler terms in taskmacs.s. The
 * code assumes this order.
 * 
 */

typedef struct
{
    unsigned r0, pc;
    
    unsigned spsrfiq, r8fiq, r9fiq, r10fiq, r11fiq, r12fiq, r13fiq, r14fiq;
    
    unsigned spsrirq, r13irq, r14irq;
    
    unsigned spsrsvc, r13svc, r14svc;
    
    unsigned spsrabt, r13abt, r14abt;
    
    unsigned spsrund, r13und, r14und;
    
    unsigned cpsr;
    
    unsigned r1, r2, r3, r4, r5, r6, r7;
    
    unsigned r8usr, r9usr, r10usr, r11usr, r12usr, r13usr, r14usr;

} angel_RegBlock;


typedef enum angel_TaskType
{
    TP_IdleLoop = 0,
    TP_AngelInit = 1,
    TP_Application = 2,
    TP_ApplCallBack = 3,
    TP_AngelCallBack = 4,
    TP_AngelWantLock = 5
} angel_TaskType;


/* ---------------------------------------------------------------------
 * Angel Interrupt Usage
 * ---------------------
 *
 * These two mask definitions are used by Angel to manipulate the
 * interrupt mask bits in the CPSR. Note: there *are* cases when it is
 * valid for Angel to modify both bits, irrespective of these
 * definitions. These are:
 * 
 * a. On initial boot, when initialising the system. Angel cannot cope
 *      with interrupts at this stage, and the devices and interrupt
 *      system are not necessarily configured properly yet.
 * 
 * b. ??When Angel_EnterSVC is called. The user has asked for interrupts
 *      to be masked; Angel doesn't assume only Angel's interrupts are
 *      relevant. TBD Is this correct? Should it include the SWI or
 *      just the function call?
 * 
 * c. When performing a register dump following an exception, where if
 *      another exception occurred required data might be lost.
 * 
 * d. When disabling interrupts prior to entering Deadloop. There's no
 *      point in DeadLoop if it can be interrupted.
 * 
 * Typically, these places use the constant InterruptMask.
 * 
 * 
 * The definitions:
 * 
 *       AngelInterruptMask is the bits to set in order to disable, or
 *                         which can be cleared to enable, interrupts
 *                         in Angel
 * 
 *       NotAngelInterruptMask is the bits to set in order to disable,
 *                         or which can be cleared to enable,
 *                         interrupts in the application.
 *
 */

/* see also comments in taskmacs.s. */
#if (HANDLE_INTERRUPTS_ON_FIQ == 1)
#if (HANDLE_INTERRUPTS_ON_IRQ == 1)
#define AngelInterruptMask      IRQDisable + FIQDisable
#define NotAngelInterruptMask   0
#else
#define AngelInterruptMask      FIQDisable
#define NotAngelInterruptMask   IRQDisable
#endif    
#else
#if (HANDLE_INTERRUPTS_ON_IRQ == 1)
#define AngelInterruptMask      IRQDisable
#define NotAngelInterruptMask   FIQDisable
#else
#error "Angel needs an interrupt source!"
#endif
#endif


typedef enum angel_TaskState
{
    TS_Undefined,  /* TQI does not contain a valid task */
    TS_Defined,    /* TQI allocated but has never been started */
    TS_Runnable,   /* Task is runnable, but not running at the moment */
    TS_Running,    /* Task is in CPU and running (ints, etc excepted) */
    TS_Blocked     /* Task is not runnable; blocked by signalWaiting */
} angel_TaskState;


/*
 * Angel's Task Queue Item (process control block in other language).
 */
typedef struct angel_TaskQueueItem
{
    struct angel_TaskQueueItem *next;
    struct angel_TaskQueueItem *prev;

    /* index of this tqi into the TQ_Pool array */
    int index;
    
     /* if known -- otherwise NULL */
    char *name;

    /* where is the task's stack?  stackBase > stackLimit in FD stack */
    unsigned stackBase, stackLimit;
    
    /* where did we start. Don't believe it for the application tho */
    unsigned entryPoint;

    /* if this is non-zero, the task is not runnable. */
    angel_TaskState state;

    /* bits this task is currently waiting on. If nonzero task is blocked. */
    unsigned signalWaiting;

    /* bits that have been signalled since the last wait. */
    unsigned signalReceived;

    /* type of task. Esp used for finding the application */
    angel_TaskType type;

    /* task priority. more positive for higher execution priority */
    unsigned priority;

    /* The task's context. Not up-to-date in the case of a complex SWI,
     * or obviously when the task is running.
     */
    angel_RegBlock rb;
    
} angel_TaskQueueItem;

#define TP_MaxEnum  (TP_AngelWantLock)


/****************************************************************************/

/*
 * The following items are the (globally accessible) register blocks
 * used by interrupt handlers, the undefined instruction handler and
 * the SWI handler.  They are used to hold an interrupted_regblock
 * (element 0) and a desired_regblock (element 1) to be used by
 * SerialiseTask.
 */

#define RB_Interrupted  0
#define RB_Desired      1
#define RB_SWI          2
#define RB_UNDEF        3
#define RB_ABORT        4
#define RB_Yield        5

#if DEBUG == 1
# define RB_Fatal        6
# define RB_NumRegblocks 7
#else
# define RB_NumRegblocks 6
#endif

extern angel_RegBlock Angel_GlobalRegBlock[RB_NumRegblocks];

/*
 * This is the number of RegBlocks we support, and also the number of
 * entries we can cope with in the Task Queue.  If it is made to
 * exceed 31 then we must recode the bitmap manipulation stuff.
 */
#define POOLSIZE   12

/*
 * The Task priority is initialised by reference to the task type, but
 * is subsequently independent of it. This macro is used to convert
 * a task type into a task priority; multiplication by 4 is fast and good
 * enough to leave a few priority levels free.
 */
#define TaskPriorityOf(type)  (((unsigned)type) * 4)

/*
 * Return the Task ID of the current (calling) task. This is the way
 * that Angel_Signal() identifies the task to be signalled.
 */
#define Angel_TaskID()        (angel_CurrentTask->index)

/*
 * Return the interrupted PC value. This must be used within an interrupt
 * service routine; it means nothing outside of that context (including
 * a SerialiseTask chained from an ISR). The address is the address of the
 * next instruction which will be executed.
 */
#define Angel_InterruptedPC() (Angel_GlobalRegBlock[RB_Interrupted].pc)

/*
 * Convenience function to call a serialised task from an ISR.
 * Pass the task function and an optional arg (zero if not used).
 * See the docs on Angel_SerialiseTask for more info.
 */
#define Angel_SerialiseISR(routine, data) Angel_SerialiseTask( 0, routine, data, empty_stack, \
                                                        &Angel_GlobalRegBlock[RB_Interrupted])

/*
 * Convenience function to Serialise a call to Angel_Yield from an ISR.
 * See the docs on Angel_SerialiseTask and Angel_YieldCore for more info.
 */
#define Angel_SerialiseYieldFromISR()     Angel_SerialiseTask( 0, (angel_SerialisedFn)Angel_YieldCore, \
                                                        NULL, empty_stack, &Angel_GlobalRegBlock[RB_Interrupted])

/*
 * These two macros are used in various places to indicate the nature 
 * of the call to EnterSVC/ExitToUSR. In most places, Angel uses them
 * not to get into SVC mode, but to disable interrupts around a critical
 * section.
 */
#define Angel_EnterCriticalSection()  Angel_EnterSVC()
#define Angel_LeaveCriticalSection()  Angel_ExitToUSR()


/****************************************************************************/

typedef void (*angel_SerialisedFn)(void *);

void Angel_SerialiseTask(bool called_by_yield, angel_SerialisedFn fn,
                         void *state, unsigned empty_stack,
                         angel_RegBlock *regblock);


typedef void (*angel_CallbackFn)(void *a1, void *a2, void *a3, void *a4);

void Angel_QueueCallback(angel_CallbackFn fn, angel_TaskType type,
                         void *a1, void *a2, void *a3, void *a4);

void Angel_BlockApplication(int value);

int Angel_IsApplicationBlocked(void);

int Angel_IsApplicationRunning(void);

angel_RegBlock *Angel_AccessApplicationRegBlock(void);

angel_TaskQueueItem *Angel_AccessApplicationTask(void);

angel_RegBlock *angel_AccessQueuedRegBlock(angel_TaskType);

angel_TaskQueueItem *angel_AccessQueuedTask(angel_TaskType);

void Angel_FlushApplicationCallbacks(void);

void Angel_Yield(void);

extern void Angel_YieldCore(void);

void Angel_EnterSVC(void);

void Angel_ExitToUSR(void);

unsigned int Angel_DisableInterruptsFromSVC(void);

unsigned int Angel_EnableInterruptsFromSVC(void);

void Angel_RestoreInterruptsFromSVC(unsigned int state);

void angel_InitialiseOneOff(void);

void Angel_InitialiseTaskFinished(void);

void angel_SaveTask(angel_RegBlock *rb);

unsigned int Angel_GetBankedReg(angel_RegBlock *rb, unsigned mode, int regno);

unsigned int* Angel_AdrBankedReg(angel_RegBlock *rb, unsigned mode, int regno);

extern void Angel_DebugLog(angel_RegBlock *r, angel_TaskQueueItem *t, int id);

extern void angel_DeleteTask(angel_TaskQueueItem *oldtask);

angel_TaskQueueItem *
Angel_NewTask(unsigned fn, angel_TaskType priority, bool yield,
              void *a1, void *a2, void *a3, void *a4);

extern void angel_SelectNextTask(void);

void Angel_Signal(unsigned task, unsigned signals);

unsigned Angel_Wait(unsigned signals);

void angel_ExitSWI(void);

void angel_EnterSWI(void);

unsigned Angel_GetCPSR(void);

void angel_WaitCore(angel_RegBlock *r, angel_TaskQueueItem *t);

void angel_SetPriority(angel_TaskQueueItem *oldtask, unsigned priority);

void Angel_SetupExceptionRegisters(angel_TaskQueueItem * tqi);

void angel_EnqueueTask(angel_TaskQueueItem *my_tqi, unsigned pri);

void angel_QueueTask(angel_RegBlock * regblock, angel_TaskQueueItem *tqi);


extern angel_TaskQueueItem angel_TQ_Pool[POOLSIZE];
extern angel_RegBlock Angel_GlobalRegBlock[RB_NumRegblocks];
extern unsigned long angel_SWIReturnToApp;    /* if in appl SWI, 0 otherwise */
extern unsigned long angel_SWIDeferredBlock;    /* if in appl SWI, 0 otherwise */
extern unsigned long angel_stacksUsed;
extern angel_TaskQueueItem *angel_TaskQueueHead;
extern angel_TaskQueueItem *angel_CurrentTask;

/* Signal bit used for signalling read and write callbacks to task */
#define SIGBIT_PAUSE    0x1   /* (application) stop-start */
#define SIGBIT_READCB   0x2   /* channel read callback */
#define SIGBIT_WRITECB  0x4   /* channel write callback */
#define SIGBIT_THRSTOP  0x8   /* thread stopped ack received */

#endif /* ndef angel_serlock_h */

/* EOF serlock.h */
