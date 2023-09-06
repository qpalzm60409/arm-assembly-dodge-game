/* -*-C-*-
 *
 * $Revision: 1.18.2.6 $
 *   $Author: rivimey $
 *     $Date: 1998/10/23 16:00:54 $
 *
 * Copyright (c) 1996 Advanced RISC Machines Limited.
 * All Rights Reserved.
 *
 *   Project: ANGEL
 *
 *     Title: Implementation of serialiser module
 */

#include "devconf.h"
#include "serlock.h"
#include "logging.h"
#include "support.h"
#include "arm.h"
#include "devdriv.h"
#include "stacks.h"
#include "serial.h"

#if MINIMAL_ANGEL == 0

angel_TaskQueueItem *angel_TaskQueueHead;
angel_TaskQueueItem *angel_CurrentTask;
unsigned long angel_SWIReturnToApp;    /* if in appl SWI, 0 otherwise */
unsigned long angel_SWIDeferredBlock;    /* if in appl SWI, 0 otherwise */
unsigned long angel_stacksUsed = 0L;

/* This holds which of the pool entries are free and which used.
 * A set bit means that entry is FREE
 */
static unsigned angel_FreeBitMap;
angel_TaskQueueItem angel_TQ_Pool[POOLSIZE];
angel_RegBlock Angel_GlobalRegBlock[RB_NumRegblocks];

/* the externs here are defined in startrom.s */
extern unsigned long angel_GhostCount;
extern unsigned long angel_SWIReturnToApp;
extern unsigned long angel_ComplexSWILock;

#if DEBUG == 1

#define INC_STAT(x)  (x)++
#define DEC_STAT(x)  (x)--

#undef DEBUG_TASKS
#undef DEBUG_APPLINT

static unsigned long angel_NumCurrentTasks = 0;
static unsigned long angel_NumTasksHWM = 0;

extern unsigned long angel_InterruptCount;
extern unsigned long angel_DebugStartTaskCount;
extern unsigned long angel_DebugQueueTaskCount;
extern unsigned long angel_SysCallCount;
#else /* not DEBUG */

#define INC_STAT(x)
#define DEC_STAT(x)

#endif

#if DEBUG == 1
struct StatInfo intr_stat_info[] =
{
    "Interrupt Stats:\n"
    "  Total Interrupts       %8d\n", &angel_InterruptCount,
    "  Ghost Interrupts       %8d\n", &angel_GhostCount,
    NULL, NULL
};

struct StatInfo task_stat_info[] =
{
    "Task Stats:\n"
    "  Total Task Switches    %8d\n", &angel_DebugStartTaskCount,
    "  Tasks Interruped       %8d\n", &angel_DebugQueueTaskCount,
    "  System Calls           %8d\n", &angel_SysCallCount,
    "  Number of tasks (HWM)  %8d\n", &angel_NumTasksHWM,
    "\n"
    "Current State:\n"
    "  Number of tasks        %8d\n", &angel_NumCurrentTasks,
    "  Complex SWI Lock       %8d\n", &angel_ComplexSWILock,
    "  Current Task           %8x\n", (unsigned long *)&angel_CurrentTask,
    NULL, NULL
};
#endif

/**********************************************************************/

/*
 *        Function:  angel_StartTask
 *
 *         Purpose:  To execute a task.
 *
 *       Arguments:  regblock: a pointer to the angel_RegBlock structure
 *                             containing the registers to be instated
 *                             when the tasks executes
 *
 *    Special Note:  This is coded in serlasm.s, as it is not a normal
 *                   function.  However, the prototype is here because
 *                   it is needed by functions in this module.
 *
 *  Pre-conditions:  This code is called from SVC mode with IRQ and FIQ
 *                   disabled.
 *
 *          Effect:  The task described is executed - when that task
 *                   completes control will be returned to angel_NextTask.
 *                   This code just inststes all the registers precisely
 *                   as set up in regblock.  Thus for tasks which are being
 *                   started for the first time, sl, sp, fp, lr and cpsr
 *                   must have already been set up.
 *
 */

extern void angel_StartTask(angel_RegBlock * regblock);

/*
 *        Function:  angel_NextTask
 *
 *         Purpose:  Entered when any task completes
 *
 *       Arguments:  None
 *
 *    Special Note:  This is coded in serlasm.s, as it is not a normal
 *                   function.  However, the prototype is here because
 *                   it is needed by functions in this module.
 *
 *  Pre-conditions:  This code is called from either SVC mode or USR mode
 *                   with IRQ and FIQ enabled.
 *
 *          Effect:  SVC mode is entered and the SVC stack grabbed.
 *                   Interrupts are disabled, and then angel_SelectNextTask
 *                   is called.
 */

extern void angel_NextTask(void);


/**********************************************************************/


/*
 * The freeing and allocation of register blocks may be done by index only.
 */

#define ISFREE(n)  ((angel_FreeBitMap & (1 << (n))) != 0)
#define ISALLOC(n) ((angel_FreeBitMap & (1 << (n))) == 0)

#define FREE(n)    angel_FreeBitMap |= (1 << (n))
#define ALLOC(n)   angel_FreeBitMap &= ~(1 << (n))


/*
 *        Function:  Angel_GetBankedReg
 *
 *         Purpose:  Read the register 'regno' for mode 'mode' from
 *                   register block 'rb'.
 *
 *       Arguments:  rb    a pointer to the RegBlock to be updated.
 *                   regno the number of the register to read
 *                   mode  the CPU mode (as CPSR)
 *
 *                   Coding for 'regno': 13 => r13, 14 ->r14, 15 -> SPSR
 *
 *   Pre-conditions: None
 *
 *  Post-conditions: None
 */
   
extern unsigned char ContextLookuptable[3][16];

unsigned Angel_GetBankedReg(angel_RegBlock *rb, unsigned int mode, int regno)
{
    unsigned char rboffset = ContextLookuptable[regno - 13][mode & 0xf];
    unsigned char *rbptr = (unsigned char *)rb + rboffset;
    return *((unsigned int *)rbptr);
}


/*
 *        Function:  Angel_AdrBankedReg
 *
 *         Purpose:  Read the address of register 'regno' for mode 'mode'
 *                   in register block 'rb'.
 *
 *       Arguments:  rb    a pointer to the RegBlock to be updated.
 *                   regno the number of the register to read
 *                   mode  the CPU mode (as CPSR)
 *
 *                   Coding for 'regno': 13 => r13, 14 ->r14, 15 -> SPSR
 *
 *   Pre-conditions: None
 *
 *  Post-conditions: None
 */

unsigned *Angel_AdrBankedReg(angel_RegBlock *rb, unsigned int mode, int regno)
{
    unsigned char rboffset = ContextLookuptable[regno - 13][mode & 0xf];
    unsigned char *rbptr = (unsigned char *)rb + rboffset;
    return ((unsigned *)rbptr);
}

#ifdef DEBUG_TASKS
void angel_PrintTaskQueue()
{
    angel_TaskQueueItem *tqi;

    for(tqi = angel_TaskQueueHead; tqi != NULL; tqi = tqi->next)
    {
        LogInfo(LOG_SERLOCK, ("Task Queue %d: (%08lx) %s\n"
                                 "Next = %08lx, Type = %d, State = %d, Priority = %u\n",
                                 tqi->index, tqi, (tqi->name) ? tqi->name : "", tqi->next,
                                 tqi->type, tqi->state, tqi->priority));
        LogInfo(LOG_SERLOCK, ("Stack Base = %08lx, Limit = %08lx, Entrypoint = %08lx\n",
                                 tqi->stackBase, tqi->stackLimit, tqi->entryPoint));
        LogInfo(LOG_SERLOCK, ("Signal Wait = %08lx, Signal Bits = %08lx\n",
                                 tqi->signalWaiting, tqi->signalReceived));
        LogInfo(LOG_SERLOCK, ("      pc = %08lx cpsr = %08lx\n",
                                 tqi->rb.pc, tqi->rb.cpsr));
        LogInfo(LOG_SERLOCK, ("      r0 = %08lx   r1 = %08lx   r2 = %08lx   r3 = %08lx\n",
                                 tqi->rb.r0, tqi->rb.r1, tqi->rb.r2, tqi->rb.r3));

        if (tqi->next)
            LogInfo(LOG_SERLOCK, ("\n"));
    }
}
#else
#define angel_PrintTaskQueue()
#endif

#ifdef DEBUG_TASKS
void angel_PrintTaskQueueShort()
{
    angel_TaskQueueItem *tqi;
    
    LogInfo(LOG_SERLOCK, ("Task Queue: "));
    for(tqi = angel_TaskQueueHead; tqi != NULL; tqi = tqi->next)
    {
        LogInfo(LOG_SERLOCK, ("%d (%d %c)",
                              tqi->index, tqi->priority,
                              (tqi->state == TS_Blocked) ? 'B' : 'R'));
        if (tqi->next)
            LogInfo(LOG_SERLOCK, (" -> "));
    }
    LogInfo(LOG_SERLOCK, ("\n"));
}
#else
#define angel_PrintTaskQueueShort()
#endif

/* angel_FreeToPool
 *
 * This function is passed the index of a pool entry to be freed.
 *
 * It will modify the queue pointers appropriately as well as marking
 * this pool element as free.
 */

static void 
angel_FreeToPool(int n)
{
    angel_TaskQueueItem *my_tqi = &angel_TQ_Pool[n];
    angel_TaskQueueItem *tqi, *l_tqi;

    if (n < 0 || n >= POOLSIZE)
        LogFatalError(LOG_SERLOCK, ("Attempt to free invalid task entry %d.", n));
    
    if (ISFREE(n))
        LogFatalError(LOG_SERLOCK, ( "Attempt to free free task %d.\n", n));

    FREE(n);
    
    DEC_STAT(angel_NumCurrentTasks);

    /* Now we find it on the queue and remove it */
    for (tqi = angel_TaskQueueHead, l_tqi = NULL;
         (tqi != my_tqi && tqi != NULL);)
    {
        l_tqi = tqi;
        tqi = tqi->next;
    }

    tqi->state = TS_Undefined;
    
    if (tqi == NULL)
    {
        LogError(LOG_SERLOCK, ( "Task could not be dequeued (%d).\n", n));
        return;
    }

    if (l_tqi == NULL)
        angel_TaskQueueHead = my_tqi->next;
    else
        l_tqi->next = my_tqi->next;
    my_tqi->next = NULL;
}


/*
 *        Function:  angel_EnqueueTask
 *
 *         Purpose:  To include the given task in the scheduler's task
 *                   list, in the correct order as determined by ;pri'.
 *
 *       Arguments:  my_tqi    a pointer to the TaskQueueItem to be included.
 *                   pri       the priority level at which to insert the 
 *                             new task.
 *                   
 *
 *   Pre-conditions: angel_TaskQueueHead initialised.
 *
 *  Post-conditions: Task queue will be one task longer.
 */

void angel_EnqueueTask(angel_TaskQueueItem *my_tqi, unsigned pri)
{
    angel_TaskQueueItem *tqi, *l_tqi;
    
    LogInfo(LOG_SERLOCK, ("angel_EnqueueTask: task %d pri %d\n",
                          (unsigned)my_tqi->index, (int)pri));
    
    for (tqi = angel_TaskQueueHead, l_tqi = NULL;
         (tqi != NULL) && (tqi->priority > pri);)
    {
        l_tqi = tqi;
        tqi = tqi->next;
    }
    
    /* We may have reached the end of the list, or we may not have */
    my_tqi->next = tqi;
    if (l_tqi == NULL)
        angel_TaskQueueHead = my_tqi;
    else
        l_tqi->next = my_tqi;
    
#ifdef DEBUG_TASKS
    angel_PrintTaskQueueShort();
#endif
}

/* angel_AllocateFromPool
 *
 * This function is passed a priority and will return a pointer
 * to the pool entry which has been set up.
 *
 * It will modify the queue pointers appropriately, and sets up the
 * priority member.  It does not set up the regblock in any way.
 *
 * A task is put on the front of the part of the queue of the
 * appropriate priority only if it is neither a new task nor a yielded
 * task (ie. a real interrupted task).  All others are put an the end
 * of the part of the queue with the appropriate priority
 */

static angel_TaskQueueItem * 
angel_AllocateFromPool(angel_TaskType type, bool yielded)
{
    int i;

    for (i = 0; i < POOLSIZE; i++)
    {
        if (ISFREE(i))
        {
            angel_TaskQueueItem *my_tqi = &angel_TQ_Pool[i];
            unsigned pri;

            ALLOC(i);

#if DEBUG == 1
            INC_STAT(angel_NumCurrentTasks);
            if (angel_NumTasksHWM < angel_NumCurrentTasks)
                angel_NumTasksHWM = angel_NumCurrentTasks;
#endif
            
            /* Set up the type and priority */
            my_tqi->type = type;
            my_tqi->index = i;
            
            /* the initial task priority is determined by the type */
            pri = TaskPriorityOf(type);
            my_tqi->priority = pri;
            
            my_tqi->signalReceived = 0;
            my_tqi->signalWaiting = 0;
            my_tqi->state = TS_Defined;
            my_tqi->name = 0;

            /* And insert the item into the queue.
             * which first involves finding a task of same priority and
             * inserting this task before it if not new, or after it if new.
             */

            /* if new or yielded, ensure it goes after any exisiting entry at pri */
            if (yielded)
                pri = pri - 1;

            angel_EnqueueTask(my_tqi, pri);
            
            /* LogInfo(LOG_SERLOCK, ( "Allocated RegBlock %d\n",i)); */

            return my_tqi;
        }
    }
    LogError(LOG_SERLOCK, ( "AllocateFromPool: No free RegBlock\n"));
    
#if DEBUG == 1
    angel_PrintTaskQueue();
#endif
    
    return NULL;
}

/**********************************************************************/

/*
 *        Function:  angel_AccessQueuedTask
 *
 *         Purpose:  This routine looks for a task on the task queue
 *                   with the given type.
 *
 *       Arguments:  type      the type of task being searched for
 *
 *          Return:  tqi       a pointer to the task queue for the
 *                             required task, or NULL if no task found.
 *
 *   Pre-conditions: angel_TaskQueueHead initialised.
 *
 *  Post-conditions: none
 */
angel_TaskQueueItem *
angel_AccessQueuedTask(angel_TaskType type)
{
    angel_TaskQueueItem *tqi;

    /* Find a task with the required type, not marked as new */
    for (tqi = angel_TaskQueueHead;
         (tqi != NULL) && !(tqi->type == type);
        )
    {
        tqi = tqi->next;
    }
    
    return tqi;
}

/*
 *        Function:  Angel_AccessApplicationTask
 *
 *         Purpose:  This routine looks for the application task 
 *                   queue item.
 *
 *       Arguments:  none
 *
 *          Return:  tqi       a pointer to the task queue for the
 *                             appl task, or NULL if no task found.
 *
 *   Pre-conditions: angel_TaskQueueHead initialised.
 *
 *  Post-conditions: none
 */
angel_TaskQueueItem *
Angel_AccessApplicationTask(void)
{
    angel_TaskQueueItem *t;

    /* Find a task with the Application priority */
    t = angel_AccessQueuedTask(TP_Application);

#if DEBUG == 1
    if (t == NULL)
    {
        LogWarning(LOG_SERLOCK, ( "Could not find application task.\n"));
        return NULL;
    }
#endif

    return t;
}

/*
 *        Function:  angel_AccessQueuedRegBlock
 *
 *         Purpose:  This routine looks for a task on the task queue
 *                   with the given type.
 *
 *       Arguments:  type      the type of task being searched for
 *
 *          Return:  rb        a pointer to the regblock for the
 *                             required task, or NULL if no task found.
 *
 *   Pre-conditions: angel_TaskQueueHead initialised.
 *
 *  Post-conditions: none
 */
angel_RegBlock *
angel_AccessQueuedRegBlock(angel_TaskType type)
{
    angel_TaskQueueItem *t;
    
    /* Find a task with the requested priority */
    t = angel_AccessQueuedTask(type);

#if DEBUG == 1
    if (t == NULL)
    {
        LogWarning(LOG_SERLOCK, ( "Could not find application task.\n"));
        return NULL;
    }
#endif

    return &(t->rb);
}

/*
 *        Function:  Angel_AccessApplicationRegBlock
 *
 *         Purpose:  This routine looks for the application task's 
 *                   regblock.
 *
 *       Arguments:  none
 *
 *          Return:  rb        a pointer to the regblock for the
 *                             appl task, or NULL if no task found.
 *
 *   Pre-conditions: angel_TaskQueueHead initialised.
 *
 *  Post-conditions: none
 */
angel_RegBlock *
Angel_AccessApplicationRegBlock(void)
{
    angel_TaskQueueItem *t;

    /* Find a task with the Application priority */
    t = angel_AccessQueuedTask(TP_Application);

#if DEBUG == 1
    if (t == NULL)
    {
        LogWarning(LOG_SERLOCK, ( "Could not find application task.\n"));
        return NULL;
    }
#endif

    return &(t->rb);
}

/*
 *        Function:  Angel_BlockApplication
 *
 *         Purpose:  Block or unblock the application task; that is
 *                   either allow or prevent it from executing any
 *                   further. If Angel is currently executing a SWI
 *                   on behalf of the application, the SWI is allowed
 *                   to complete, but the application is not resumed.
 *
 *       Arguments:  value     TRUE to block, FALSE to unblock appl
 *
 *          Return:  none.
 *
 *   Pre-conditions: angel_TaskQueueHead initialised.
 *
 *  Post-conditions: none
 */
void 
Angel_BlockApplication(int value)
{
    angel_TaskQueueItem *tqi;
    LogInfo(LOG_SERLOCK, ( "angel_BlockApplication - set to %d.\n", value));

    if (angel_SWIReturnToApp)
    {
        angel_SWIDeferredBlock = 1;
    }
    else
    {
        tqi = Angel_AccessApplicationTask();
        tqi->signalWaiting = value;
        tqi->state = (value == 0) ? TS_Runnable : TS_Blocked;
    }
}

int 
Angel_IsApplicationBlocked(void)
{
    angel_TaskQueueItem *tqi;
    int blocked;

    if (angel_SWIDeferredBlock)
        blocked = 1;
    else
    {
        tqi = Angel_AccessApplicationTask();
        blocked = (tqi->state == TS_Blocked);
    }

    LogInfo(LOG_SERLOCK, ("angel_IsApplicationBlocked - returns %d.\n",
                          blocked));
    
    return blocked;
}

/*
 *        Function:  Angel_IsApplicationRunning
 *
 *         Purpose:  To return whether the application task is actually
 *                   running or not; this includes any Application callback
 *                   tasks. Note this will only be true if called from
 *                   the application.
 *
 *       Arguments:  none
 *
 *          Return:  boolean - TRUE indicates application running
 *                             FALSE indicates some other task.
 *
 *  Pre-conditions:  Scheduler must have been initialised so that
 *                   angel_Currenttask is valid.
 *
 *          Effect:  none.
 */

int 
Angel_IsApplicationRunning(void)
{
    int running = (angel_CurrentTask->type == TP_Application) ||
        (angel_CurrentTask->type == TP_ApplCallBack);

    LogInfo(LOG_SERLOCK, ( "angel_IsApplicationRunning - returns %d.\n", running));
    return running;
}

/*
 *        Function:  Angel_FlushApplicationCallbacks
 *
 *         Purpose:  To remove any outstanding application callbacks from the
 *                   task queue.
 *
 *       Arguments:  none.
 *
 *          Return:  none
 *
 *    Special Note:  This routine merely removes the tasks, it will not free
 *                   any resources those tasks had locked (e.g. packet buffers,
 *                   device write locks). Use with care!
 *
 *  Pre-conditions:  Scheduler must have been initialised so that
 *                   angel_Currenttask is valid.
 *
 *          Effect:  All tasks of type TP_ApplCallback removed from the task
 *                   queue.
 *
 * Post-conditions:
 *
 */

void 
Angel_FlushApplicationCallbacks(void)
{
    angel_TaskQueueItem *tqi;
    int found;

    LogInfo(LOG_SERLOCK, ( "angel_FlushApplicationCallBakcs entered\n"));

    /* Find all tasks with the ApplCallback tasks.
     * We have to start the loop from scratch each time we find one
     * because angel_FreeToPool changes all the pointers !
     */
    do
    {
        for (found = 0, tqi = angel_TaskQueueHead;
             (tqi != NULL && tqi->type != TP_ApplCallBack);)
        {
            tqi = tqi->next;
        }

        if (tqi != NULL)
        {
            LogInfo(LOG_SERLOCK,
                    ( "angel_FlushApplicationCallbacks - found task to flush\n"));
            angel_DeleteTask(tqi);
            found = 1;
        }
    } while (found == 1);
}

/*
 *        Function:  angel_AllocNewStack
 *
 *         Purpose:  Allocate a new stack region on the Angel USR stack.
 *                   There are several stack regions used within Angel: the
 *                   privileged mode stacks (e.g. IRQ) are allocated to only
 *                   one thing ata time, but the USR mode stack area is used
 *                   by all Angel Callback tasks. This code manages the area
 *                   by maintaining a bitmap of allocated areas, an area being
 *                   defined by the #define Angel_AngelStackFreeSpace.
 *                   
 *                   The StackLimit variable in the task structure is
 *                   initialised here but only used by the debug code.
 *                   
 *                   The new stack area is set into the task block but not into
 *                   the register block; it is up to the caller to do this.
 *
 *       Arguments:  newtask - a pointer to the task requiring a stack area.
 *
 *          Return:  bool - TRUE if stack area found, FALSE otherwise.
 *                   
 *  Pre-conditions:  angel_InitialiseOneOff called to initialise angel_stacksUsed
 *                   Angel_StackBase initialised to the base of the Angel stack area
 *
 *
 * Post-conditions:
 *
 */

static 
bool angel_AllocNewStack(angel_TaskQueueItem *newtask)
{
    /* stackBase is the highest address used by the descending stack */
    unsigned stackBase = Angel_StackBase + Angel_AngelStackOffset;
    unsigned stackLimit = Angel_StackBase + Angel_AngelStackLimitOffset;
    int c;
    
    for (c = 0; stackBase > stackLimit; c++)
    {
        if ((angel_stacksUsed & (0x1 << c)) == 0)
        {
            angel_stacksUsed |= (0x1 << c);
            /* LogInfo(LOG_BUFFER, ("Angel_StackAlloc: %d (%08x)\n",
                                 c, stackBase)); */

            newtask->stackBase = stackBase;
            newtask->stackLimit = stackBase - Angel_AngelStackFreeSpace;
            return TRUE;
        }
        stackBase -= Angel_AngelStackFreeSpace;
    }
    
    return FALSE;
}

/*
 *        Function:  angel_DeleteTask
 *
 *         Purpose:  To remove the task pointed to by oldtask, freeing up
 *                   its stack and other resources and removing the TQI
 *                   from the queue.
 *
 *       Arguments:  oldtask - a pointer to the TQI for the task to remove
 *
 *          Return:  none.
 *                   
 *  Pre-conditions:  Serialiser initialised; oldtask should not point to the
 *                   idle task, and must point to a valid TQI structure.
 *
 *
 * Post-conditions:
 *
 */

void angel_DeleteTask(angel_TaskQueueItem *oldtask)
{
    unsigned stackBase;
    unsigned stackLimit;
    
#ifdef DEBUG_TASKS
    LogInfo(LOG_SERLOCK, ("DeleteTask: task %d\n", oldtask->index ));
#endif
    
    if (oldtask == NULL)
        return;
    
#if DEBUG != 0
    Angel_DebugLog(0, oldtask, 10);
#endif

    /* stackBase is the highest address used by the descending stack */
    stackBase = Angel_StackBase + Angel_AngelStackOffset;
    stackLimit = Angel_StackBase + Angel_AngelStackLimitOffset;

    if ((oldtask->stackBase <= stackBase) &&
        (oldtask->stackBase > stackLimit))
    {
        /* this stack was in the Angel USR stack area -- remove it */
        int c = (stackBase - oldtask->stackBase) / Angel_AngelStackFreeSpace;
        /* LogInfo(LOG_BUFFER, ("Angel_StackFree: %d (%08x)\n",
                             c, oldtask->stackBase)); */
        angel_stacksUsed &= ~(0x1 << c);
    }

    /*
     * this call makes the TQI available for allocation again
     */
    angel_FreeToPool(oldtask->index);
}


/*
 *        Function:  angel_SetPriority
 *
 *         Purpose:  To change the scheduler priority of a task on the task
 *                   queue. As the priority is implicit in the position in the
 *                   task list it is not possible to just poke the priority
 *                   value in the TQI; the task must be requeued as well.
 *                   
 *                   The task is located in the task queue, which ensures it has
 *                   been initialised and to find the next higher priority task.
 *                   'olstask' is then removed from the list, the priority in
 *                   its TQI modifined, and the task enqueue routine called to
 *                   place it in the correct place.
 *
 *                   This routine will not cause a task switch to 'oldtask'
 *                   if it is made higher priority than the current task, nor
 *                   can it cause the current task to be suspended if it is
 *                   set to a lower priority than another runnable task.
 *
 *       Arguments:  oldtask - the task whose priority is to be changed
 *                   priority - the new task priority
 *
 *          Return:  none.
 *
 *  Pre-conditions:  Serialiser initialised; the task pointed to by 'oldtask'
 *                   must have previously been added with Angel_NewTask; the
 *                   routine must be called with interrupts disabled.
 *
 *          Effect:  'oldtask' is requeued to reflect the assigned priority.
 *
 * Post-conditions:
 *
 */

void angel_SetPriority(angel_TaskQueueItem *oldtask, unsigned priority)
{
    angel_TaskQueueItem *tqi = angel_TaskQueueHead;
    
    if (tqi != oldtask)
    {
        while(tqi != NULL && tqi->next != oldtask)
            tqi = tqi->next;
    }
    
    if (tqi == NULL)
    {
        LogFatalError(LOG_SERLOCK, ("SetPriority: task not found\n"));
        return;
    }

    if (tqi == oldtask)
    {
        angel_TaskQueueHead = tqi->next;
    }
    else
    {
        /* remove task from priority list */
        tqi->next = tqi->next->next;
        oldtask->priority = priority;
    }
    
    angel_EnqueueTask(oldtask, priority);
}

/*
 *        Function:  angel_EnterSWI
 *
 *         Purpose:  Routine called when an Angel Complex SWI is entered.
 *                   Used to perform some housekeeping, including raising
 *                   the priority of the application task to that of Angel
 *                   callbacks. This is necessary so that an incoming packet
 *                   cannot interrupt the processing of a SWI; if this were
 *                   allowed, deadlock could occur if the incoming packet
 *                   needed to send a reply while the SWI request packet was
 *                   still being processed.
 *
 *                   To enable Angel to tell that Angel is executing code on
 *                   behalf of the application task, the flag SWIReturnToApp
 *                   is set. This is used in the SWI return code and within the
 *                   BlockApplication routine.
 *
 *       Arguments:  none.
 *
 *          Return:  none.
 *
 *  Pre-conditions:  Must be called with interrupts disabled (because SetPriority
 *                   must be). angel_CurrentTask must point to an Application task
 *
 * Post-conditions:  The application task temporarily has a higher priority;
 *                   the Application-is-executing-SWI flag is set.
 *
 */
void angel_EnterSWI(void)
{
    if (angel_CurrentTask->type != TP_Application)
    {
        LogFatalError(LOG_SERLOCK,
                      ("Complex SWI called from within Angel: stack use conflict\n"));
    }

    angel_SetPriority(angel_CurrentTask, TaskPriorityOf(TP_AngelCallBack));
    angel_SWIReturnToApp = 1;
}


/*
 *        Function:  angel_ExitSWI
 *
 *         Purpose:  Undo the effects of angel_EnterSWI. Return the application
 *                   to it's normal priority and reset the in-swi flag. This routine
 *                   is called by return-from-complex-SWI routine.
 *
 *       Arguments:  none.
 *
 *          Return:  none.
 *
 *  Pre-conditions:  Must be called with interrupts disabled (because SetPriority
 *                   must be). angel_CurrentTask must point to an Application task
 *                   angel_SWIReturnToApp should be set.
 *
 * Post-conditions:  The application task priority is returned to normal; the in-swi
 *                   flag is reset.
 *
 */
void angel_ExitSWI(void)
{
#if ASSERT_ENABLED
    if ((Angel_GetCPSR() & 0xf) == 0)
        LogFatalError(LOG_SERLOCK, ("ExitSWI: in USR mode\n"));
#endif

    if (angel_SWIReturnToApp != 1)
    {
        LogFatalError(LOG_SERLOCK,
                      ("System Call Return with no Call -- what happened?\n"));
    }

    angel_SetPriority(angel_CurrentTask, TaskPriorityOf(TP_Application));
    angel_SWIReturnToApp = 0;
}

/*
 *        Function:  Angel_QueueCallback
 *
 *         Purpose:  The external API for the NewTask function, called by routines
 *                   within Angel wishing to create callback tasks.
 *
 *       Arguments:  fn     - the function to call
 *                   type   - the task type (e.g. TP_AngelCallback) to create
 *                   a1.. a4 - (up to) 4 parameters for the new task.
 *
 *          Return:  none.
 *                   
 *  Pre-conditions:  The serialiser must have been initialised.
 *
 * Post-conditions:  The new task will have been added to the task queue as a
 *                   runnable task, unless there was no TQI or stack available for
 *                   it.
 *
 */
void 
Angel_QueueCallback(angel_CallbackFn fn, angel_TaskType type,
                    void *a1, void *a2, void *a3, void *a4)
{
    Angel_NewTask((unsigned)fn, type, FALSE, a1, a2, a3, a4);
}


/*
 *        Function:  angel_SetupExceptionRegisters
 *
 *         Purpose:  
 *                   
 *
 *       Arguments:  
 *                   
 *
 *          Return:  none.
 *                   
 *  Pre-conditions:  The serialiser must have been initialised.
 *
 * Post-conditions:  
 *                   
 *
 */
void Angel_SetupExceptionRegisters(angel_TaskQueueItem * tqi)
{
    /* set up the new task's exception registers. */
    tqi->rb.r13fiq = Angel_StackBase + Angel_FIQStackOffset;
    tqi->rb.r14fiq = 0;
    
    tqi->rb.r13irq = Angel_StackBase + Angel_IRQStackOffset;
    tqi->rb.r14irq = 0;
    
    tqi->rb.r13und = Angel_StackBase + Angel_UNDStackOffset;
    tqi->rb.r14und = 0;
    
    tqi->rb.r13abt = Angel_StackBase + Angel_ABTStackOffset;
    tqi->rb.r14abt = 0;

}

/*
 *        Function:  Angel_NewTask
 *
 *         Purpose:  To create a new task context. This is the only way to create
 *                   a new task, and may be executed in USR or Supervisor mode. The
 *                   new task will not be executed immediately, but will be queued
 *                   according to it's priority and may be executed as soon as 
 *                   SelectNextTask is run again.
 *
 *                   The new task context's mode will depend on it's type: all tasks
 *                   other than AngelWantLock are given a USR mode context, with a
 *                   stack allocated in the AngelAngelStack area; AngelWantLock tasks
 *                   (which are only created by SerialiseTaskCore) are run in SVC mode
 *                   with a statically allocated SVC stack.
 *
 *                   The processor registers pc, r0 - r3 are set using the arguments
 *                   'fn' and 'a1'-'a4' respectively; lr is set to angel_NextTask, which
 *                   will kill the task and enter the scheduler on it's exit. The
 *                   sp of the exception modes is set to their stack top, and their lr
 *                   registers to a routine which will cause a fatal error (the LR should
 *                   be overwritten on entry to the handler by the processor; therefore,
 *                   if the value written here the program has already gone wrong.
 *                   
 *                   The SVC stack is set up for USR mode tasks, but not vice versa;
 *                   the SVC task is not expected to use or need r13usr to be set. (The
 *                   reverse is also true, but SVC sp can be set up easily).
 *
 *       Arguments:  fn     - the function to call
 *                   type   - the task type (e.g. TP_AngelCallback) to create
 *                   a1.. a4 - (up to) 4 parameters for the new task.
 *
 *          Return:  a pointer to the TQI allocated to the task or NULL if task not
 *                   initialised (an error).
 *                   
 *  Pre-conditions:  The serialiser, and exception subsystems must have been set up.
 *
 * Post-conditions:  The new task added to the task queue, or NULL returned.
 *
 */
angel_TaskQueueItem * 
Angel_NewTask(unsigned fn, angel_TaskType type, bool yield,
                    void *a1, void *a2, void *a3, void *a4)
{
    unsigned int cpsr;
    angel_TaskQueueItem *tqi;

    LogInfo(LOG_SERLOCK, ("Angel_NewTask: func: 0x%08lx, type: %d\n",
                          (unsigned)fn, (int)type));

    /* enter critical section */
    cpsr = Angel_GetCPSR();
    Angel_EnterCriticalSection();

    tqi = angel_AllocateFromPool(type, yield);
    if (tqi == NULL) /* sorry, can't do it! */
    {
        Angel_LeaveCriticalSection();
        LogFatalError(LOG_SERLOCK, ("Angel_NewTask: Cannot create new task.\n"));
        return NULL;
    }
    
    /* set up the registers we want for this call */
    tqi->rb.r0 = (unsigned)a1;
    tqi->rb.r1 = (unsigned)a2;
    tqi->rb.r2 = (unsigned)a3;
    tqi->rb.r3 = (unsigned)a4;
    tqi->rb.pc = (unsigned)fn;
    /* tqi->rb.r11usr = 0; */
    tqi->entryPoint = (unsigned)fn;
    tqi->name = NULL;

    /* set up the new task's exception registers. */
    Angel_SetupExceptionRegisters(tqi);

    tqi->rb.r13svc = Angel_StackBase + Angel_SVCStackOffset;
    tqi->rb.r14svc = 0;

    if (type == TP_AngelWantLock)
    {
        tqi->stackBase = tqi->rb.r13svc;
        tqi->stackLimit = Angel_StackBase + Angel_SVCStackLimitOffset;
        /* copy the state of the non-Angel interrupt bit to the new task */
        tqi->rb.cpsr = SVCmode | (cpsr & NotAngelInterruptMask);
        tqi->rb.r14svc = (unsigned)angel_NextTask;
        
#ifdef R10_IS_SL
        /* The stack limit is not used in standard Angel: this code is here to
         * show where and how to set up SL.
         */
        tqi->rb.r10usr = tqi->stackLimit;
#endif
    }
    else
    {
        /* try to allocate some stack space for the new task */
        if (!angel_AllocNewStack(tqi)) /* sorry, can't do it! */
        {
            angel_FreeToPool(tqi->index);
            Angel_LeaveCriticalSection();
            LogError(LOG_SERLOCK, ("Angel_NewTask: Cannot allocate new stack.\n"));
            return NULL;
        }
        
        /* copy the state of the non-Angel interrupt bit to the new task */
        tqi->rb.cpsr = USRmode | (cpsr & NotAngelInterruptMask);
        tqi->rb.r13usr = tqi->stackBase;
        
#ifdef R10_IS_SL
        /* The stack limit is not used in standard Angel: this code is here to
         * show where and how to set up SL.
         */
        tqi->rb.r10usr = tqi->stackLimit;
#endif
        tqi->rb.r14usr = (unsigned)angel_NextTask;
    }

    /* now we've set up the task, change it's state to runnable. */
    tqi->state = TS_Runnable;
    
    /* ok now... leave critical section */
    Angel_LeaveCriticalSection();

    LogInfo(LOG_SERLOCK, ("Angel_NewTask: New task %d returning to %lx.\n",
                          tqi->index, tqi->rb.pc));
    return tqi;
}

/*
 *        Function:  angel_QueueTask
 *
 *         Purpose:  To copy a regblock context into the task queue item, used
 *                   when saving a task context for later execution.
 *                   
 *                   The routine modifies the run state; if the task was running
 *                   it patently isn't anymore, and the state is set to runnable.
 *                   As this must not happen if state was blocked, the initial
 *                   state must be verified as Running first.
 *                   
 *       Arguments:  regblock - the regblock from which to take the processor
 *                              context.
 *                   tqi      - the tqi into which the context must be saved
 *
 *          Return:  none.
 *                   
 *  Pre-conditions:  the context in regblock must be the context for the task
 *                   referenced in tqi.
 *
 * Post-conditions:  the TQI is up-to-date w.r.t. it's task.
 *
 */
void
angel_QueueTask(angel_RegBlock * regblock,
                angel_TaskQueueItem *tqi)
{
    /* LogInfo(LOG_SERLOCK, ("QueueTask: rb %x, tq %x\n", regblock, tqi )); */
    
    __rt_memcpy((void *)&(tqi->rb), (const void *)regblock,
                sizeof(angel_RegBlock));
    
#if DEBUG != 0
    Angel_DebugLog(regblock, tqi, 2) ;
#endif

    /* if this task was running, it isn't any longer. Mark it so. */
    if (tqi->state == TS_Running)
    {
        tqi->state = TS_Runnable;
    }
    
    return;
}

/*
 *        Function:  angel_WaitCore
 *
 *         Purpose:  The core routine for Angel_Wait (see serlasm.s), which
 *                   overall deschedules the calling task waiting for an event
 *                   of some sort. By the time we get here, we have:
 *                   
 *                   a. entered SVC mode with interrupts disabled 
 *                   
 *                   b. saved the task context in RB_Yield
 *                   
 *                   c. saved the wait parameter (the value to wait on) in
 *                      the regblock r0 value.
 *                   
 *      Description  The event value is a bitmask; each bit is significant, but
 *      -----------  the meaning of the bits must be pre-arranged bu the tasks
 *                   (although this can happen at run-time if needed). A task
 *                   may validaly wait for a number of events by setting more than
 *                   one bit in the parameter to wait. If any one of these events
 *                   has happened, wait will return immediately with a bitmask
 *                   indicating which events out of those requested have happened.
 *                   If no events have yet happened, wait causes the task to sleep
 *                   until something Signal()s the task. Note that as Signal does
 *                   not cause a reschedule, more than one signal may occur before
 *                   wait finally returns; again, all the events signalled will be
 *                   returned to the waiting task.
 *                   
 *      Action       The core code must thus check whether the event to be waited
 *      ------       for has already happened (by examining signalsReceived), and
 *                   if it has to return, modifying signalsReceived to remove the
 *                   events this wait was waiting for. If the event has yet to
 *                   happen, the routine records the event in the TQI for this
 *                   task and deschedule itself.
 *                   
 *                   The anomalous case when a task waits for no events results
 *                   in Angel_Wait returning a result of 0 immediately.
 *
 *       Arguments:  rb - the register block into which the task context was saved
 *                   tqi - the task queue referring to the task being inspected.
 *
 *          Return:  none (returns via StartTask on rb, or via serialiser).
 *                   
 *  Pre-conditions:  serialiser must be initialised. Only validly called via Angel_Wait
 *
 * Post-conditions:  (none).
 *
 */
void angel_WaitCore(angel_RegBlock *rb, angel_TaskQueueItem *tqi)
{
    int b;

#if ASSERT_ENABLED
    if ((Angel_GetCPSR() & 0xf) == 0)
        LogFatalError(LOG_SERLOCK, ("WaitCore: in USR mode\n"));
#endif

    /* if we ask for no events happening, just return 0 to caller */
    if (rb->r0 == 0)
    {
        LogInfo(LOG_SERLOCK, ("WaitCore: task %d returning: r0 is 0\n",
                              Angel_TaskID()));
        angel_StartTask(rb);
    }

    /* find out if (at least one of) the events has already happened */
    b = rb->r0 & tqi->signalReceived;

    /* reset the events in received which we have now accepted */
    tqi->signalReceived &= ~b;
    if (b)
    {
#ifdef DEBUG_TASKS
        LogInfo(LOG_SERLOCK, ("WaitCore: task %d returning: signalReceived matched\n",
                              Angel_TaskID()));
#endif        
        /* one of the events has happened; return which event in r0
          * and return now.
          */ 
        rb->r0 = b;
        angel_StartTask(rb);
    }
    else
    {
#ifdef DEBUG_TASKS
        LogInfo(LOG_SERLOCK, ("WaitCore: task %d waiting for signal %x...\n",
                              Angel_TaskID(), rb->r0));
#endif        
        /* no event we are interested in has happened yet. Tell the
          * scheduler to deschedule us until they do.
          */
        tqi->signalWaiting = rb->r0;
        tqi->state = TS_Blocked;

        /* copy the register context to the appl TQI */
        angel_QueueTask(rb, tqi);

        tqi->state = TS_Blocked;
        
        /* jump to scheduler to find a new task */
        angel_SelectNextTask();
    }
}

/*
 *        Function:  Angel_Signal
 *
 *         Purpose:  To cause an event to be sent to a task which is now, or will
 *                   in the future Wait() for it. In itself, Signal() does nothing
 *                   useful.
 *                   
 *                   Signal takes a task id (a small integer) identifying a current
 *                   task and marks that task as having received the passed event
 *                   bits (see the description for Angel_Wait). If the task is already
 *                   blocked, it is assumed that the task is Wait()ing and, having
 *                   subtracted the new signals from the signals the task was waiting
 *                   for it (potentially) wakes up that task.
 *                   
 *                   If the task was running, it is assumed that sometime it will call
 *                   wait(), and the signals are just marked in the task signalWaiting
 *                   member.
 *
 *                   Task ID's are returned by the call Angel_TaskID(), which returns
 *                   a tasks own id. It must use other means to communicate this id
 *                   to whatever needs to signal it.
 *                   
 *       Arguments:  task - the task id (NOT TQI pointer) for the task to signal.
 *                   signals - one or more bits set in a word corresponding to events.
 *
 *          Return:  none.
 *                   
 *  Pre-conditions:  serialiser initialised; task identified by 'task' is a current
 *                   task.
 *
 * Post-conditions:  task state modified.
 *
 */
void Angel_Signal(unsigned task, unsigned signals)
{
    Angel_EnterCriticalSection();
            
    if (task > 0 && task < POOLSIZE)
    {
        angel_TaskQueueItem *tqi = &angel_TQ_Pool[task];
        
        /* if current task is already waiting, then signal it directly */
        if (tqi->state == TS_Blocked)
        {
#ifdef DEBUG_TASKS
            LogInfo(LOG_SERLOCK, ("Signal: from task %d, waiting task %d; signal %04x, "
                                  "(old) signalReceived %04x, signalWaiting %04x.\n",
                                  Angel_TaskID(), task, signals, tqi->signalReceived,
                                  tqi->signalWaiting));
#endif
            /* clear bits set in 'signals' in task's signalbits */
            tqi->signalWaiting &= ~(signals);
            tqi->state = (tqi->signalWaiting == 0) ? TS_Runnable : TS_Blocked;
            
            /*
             * poke r0 in task RB so when task resumes the return value
             * is the signals we have just changed
             */
            tqi->rb.r0 |= signals;
        }
        else if (tqi->state == TS_Runnable || tqi->state == TS_Running)
        {
#ifdef DEBUG_TASKS
            LogInfo(LOG_SERLOCK, ("Signal: from task %d, running task %d; signal %04x,"
                                  "(old) signalReceived %04x.\n",
                                  Angel_TaskID(), task, signals, tqi->signalReceived));
#endif            
            /* current task is not waiting for us. Assume that it will
              * sometime in the future...
              */
            tqi->signalReceived |= signals;
        }
        else
            LogError(LOG_SERLOCK, ("Signal: task in strange state %d\n", tqi->state));
    }
    else
        LogError(LOG_SERLOCK, ("Signal: invalid task id %d\n", task));
    
    Angel_LeaveCriticalSection();
}


/*
 *        Function:  angel_SerialiseTaskCore
 *
 *         Purpose:  To queue a new task for the scheduler. This calls NewTask
 *                   to create a new SVC mode task and if possible, executes the
 *                   it (it may not be possible due to a current SVC task in
 *                   progress.
 *
 *       Arguments:  called_by_yield     - 1 if called by Angel_Yield
 *                                         0 otherwise.
 *                   interrupted_regblock- a regblock pointer to the context
 *                                         for the task which was interrupted
 *                   
 *                   desired_regblock    - a regblock pointer containing the
 *                                         PC and R0 for the new task
 *                                         
 *
 *   Pre-conditions: This function must be called from SVC mode with the
 *                   SVC stack set up for use.  On entry IRQ's and
 *                   FIQ's will be disabled.  It must only ever be
 *                   called from the assembler veneer code in
 *                   Angel_SerialiseTask
 *
 *  Post-conditions: An SVC task will be executing.
 */

extern void angel_SerialiseTaskCore(int, angel_RegBlock *, angel_RegBlock *);

void 
angel_SerialiseTaskCore(int called_by_yield,
                        angel_RegBlock *desired_regblock,
                        angel_RegBlock *interrupted_regblock)
{
    angel_TaskQueueItem *tqi;
    
    /* LogInfo(LOG_SERLOCK, ("SerialiseTaskCore: drb %x, irb %x, "
                          "old pc %8X, new pc %8X yield? %d\n",
                          desired_regblock, interrupted_regblock,
                          interrupted_regblock->pc, desired_regblock->pc,
                          called_by_yield)); */
#ifdef DEBUG_APPLINT
    if (angel_CurrentTask->type == TP_Application)
        LogInfo(LOG_SERLOCK, ("Interrupting Application, PC = %08lx, CPSR = %08lx\n",
                                  interrupted_regblock->pc, interrupted_regblock->cpsr));
#endif
        
#if ASSERT_ENABLED
    if ((Angel_GetCPSR() & 0xf) == 0)
        LogFatalError(LOG_SERLOCK, ("SerialiseTaskCore: in USR mode\n"));
#endif

    /*
     * first off, we need a tqi for this new task. Create one from
     * the info in desired_regblock
     */
    tqi = Angel_NewTask(desired_regblock->pc, TP_AngelWantLock, called_by_yield,
                        (void*)desired_regblock->r0, 0, 0, 0);
    tqi->name = "Handler";

    /*
     * interrupted_regblock represents the state of the current task,
     * which might be the one we continue with. If we are already running
     * a task with WantLock priority, then we just continue with it.
     * If not, then use interrupted_regblock to update the memory context
     * for the interrupted task and enter the scheduler, which will pick
     * the current highest-priority task and run it.
     */
    if (angel_CurrentTask->type == TP_AngelWantLock)
        angel_StartTask(interrupted_regblock);
    else
    {
        angel_QueueTask(interrupted_regblock, angel_CurrentTask);
        angel_SelectNextTask();
    }
}


/*
 *        Function:  angel_IdleLoop
 *
 *         Purpose:  The idle loop just marks time. It runs (in user mode)
 *                   when the serialiser finds no other task can run.
 *
 *                   The task starts out with the Angel interrupt(s) enabled,
 *                   and the non-Angel interrupt (if any) disabled. The
 *                   state of the non-Angel interrupt can change if the
 *                   serialiser "pokes" the CPSR when a interrupts happen,
 *                   which it would do if it was "following" the application
 *                   interrupt state.
 *
 *                   The ifdef's surround a code chunk which will flash the
 *                   LED managed by the LED_DEVICE at approximately 1s
 *                   intervals (on a PIE) when Angel is idling, and slower
 *                   when it is performing "real" work. If heartbeats are
 *                   disabled and an application which doesn't use
 *                   semihosting is running, the led will stop flashing.
 * 
 *                   THIS ROUTINE NEVER RETURNS.
 * 
 *       Arguments:  none.
 *
 *          Return:  none.
 *                   
 *  Pre-conditions:  Devices initialised.
 *
 * Post-conditions:  Device poll carried out; interrupts serviced.
 *
 */

/* this is a magic number which works ok on PIE */
#define FLASH_LED_LOOP_COUNT 0x00040000

void angel_IdleLoop(void)
{    
#ifdef IDLE_FLASH_LEDS
    static int c = 0;
    static int s = 1;
    int y;
#endif

    while(TRUE)
    {
        angel_DeviceYield();

#ifdef IDLE_FLASH_LEDS
        if (++c > FLASH_LED_LOOP_COUNT )
        {
            s = ! s;
            c = 0;
            angel_DeviceControl((DeviceID)DI_LED_DEVICE, (DeviceControl)DC_SET_LED, (void*)s);
        }
#endif
    }
}


/*
 *        Function:  Angel_YieldCore
 *
 *         Purpose:  The core of Angel_Yield. This just calls the device Yield
 *                   routine which should poll any defined devices and return.
 *                   The main point of this routine outside of a poll is that
 *                   it is called with interrupts enabled, and thus any devices
 *                   with interrupts pending will be serviced as well.
 *
 *       Arguments:  none.
 *
 *          Return:  none.
 *                   
 *  Pre-conditions:  Devices initialised.
 *
 * Post-conditions:  Device poll carried out; interrupts serviced.
 *
 */
void 
Angel_YieldCore(void)
{
    angel_DeviceYield();
}

/*
 *       Function:  angel_SelectNextask
 *
 *        Purpose:  To select the first available task in the highest
 *                  priority queue which is not empty, and to effect it
 *                  by calling angel_StartTask.
 *
 *      Arguments:  None.
 *
 *  Pre-conditions: This routine must be called in SVC, with Angel interrupts
 *                  disabled.
 *
 *         Effect:  The queue is scanned in descending order
 *                  of priority. The first non blocked item in the 
 *                  queue is removed, and then effected.
 *                  If there is no task then go into an idle loop.
 */

extern void angel_SelectNextTask(void);

void 
angel_SelectNextTask(void)
{
    angel_TaskQueueItem *tqi;

#if ASSERT_ENABLED
    if ((Angel_GetCPSR() & 0xf) == 0)
        LogFatalError(LOG_SERLOCK, ("SelectNextTask: in USR mode\n"));
#endif

    /* Find the task with the highest priority which is not blocked */
    for (tqi = angel_TaskQueueHead; tqi != NULL; tqi = tqi->next)
    {
        /*
         * if task not runnable, look for another
         */
        if (tqi->state == TS_Blocked || tqi->state == TS_Undefined)
        {
            continue;
        }

        /* Otherwise this is the highest priority task so execute it ! */
#ifdef DEBUG_TASKS
        LogInfo(LOG_SERLOCK, ("SelectNextTask: Selecting task %d, pc 0x%lx, "
                              "usr lr 0x%lx, svc lr 0x%lx, type %d, pri %d\n",
                              tqi->index, tqi->rb.pc, tqi->rb.r14usr, tqi->rb.r14svc,
                              tqi->type, tqi->priority));
#endif
        
        angel_CurrentTask = tqi;
        tqi->state = TS_Running;

#ifdef DEBUG_APPLINT
        if (angel_CurrentTask->type == TP_Application)
            LogInfo(LOG_SERLOCK, ("Resuming Application, PC = %08lx, CPSR = %08lx\n",
                                  tqi->rb.pc, tqi->rb.cpsr));
#endif
        
        angel_StartTask(&(tqi->rb));
        /*NOTREACHED*/
    }

    if (tqi == NULL)
    {
        LogFatalError(LOG_SERLOCK, ("SelectNextTask: No task to run!"));
    }
}

/*
 *       Function:  angel_InitialiseOneOff
 *
 *        Purpose:  To initialise the scheduler and create an application
 *                  context which is linked onto the task queue, although
 *                  it is blocked from actually starting. It also sets the
 *                  scheduler in 'initialisaion' mode. This is one way of
 *                  setting a task running in Angel... 
 *
 *      Arguments:  None.
 *
 *  Pre-conditions: This routine must be called in SVC, with the I-bit
 *                  and F-bit set.
 *
 */
void 
angel_InitialiseOneOff(void)
{
    int i;
    angel_TaskQueueItem *tqi;

    LogInfo(LOG_SERLOCK, ( "angel_InitialiseOneOff entered\n"));

    angel_FreeBitMap = (1 << POOLSIZE) - 1;
    angel_stacksUsed = 0;
    angel_TaskQueueHead = NULL;

    for (i = 0; i < POOLSIZE; i++)
    {
        angel_TQ_Pool[i].next = NULL;
        angel_TQ_Pool[i].index = i;
        angel_TQ_Pool[i].type = TP_IdleLoop;
        angel_TQ_Pool[i].state = TS_Undefined;        
        angel_TQ_Pool[i].priority = 0;
    }
    
    /* The idle loop: this is needed! */
    tqi = Angel_NewTask((unsigned)angel_IdleLoop, TP_IdleLoop, FALSE, 0, 0, 0, 0);
    if (tqi == NULL) /* sorry, can't do it! */
    {
        LogFatalError(LOG_SERLOCK,
                      ("angel_InitialiseOneOff: CANNOT ALLOCATE IDLE TASK!\n"));
    }
    tqi->name = "Idle";
    angel_CurrentTask = tqi; /* must point at a valid task! */
    
    /* The application: always allocate one. */
    tqi = Angel_NewTask(0x8000, TP_Application, FALSE, 0, 0, 0, 0);
    if (tqi == NULL) /* sorry, can't do it! */
    {
        LogFatalError(LOG_SERLOCK,
                      ("angel_InitialiseOneOff: CANNOT ALLOCATE APPL TASK!\n"));
    }
    tqi->signalWaiting = 1;
    tqi->state = TS_Blocked;
    tqi->name = "Application";
}

/*
 *       Function:  angel_InitialiseTaskFinished
 *
 *        Purpose:  To signal that Angel initialisation has completed,
 *                  and the application task can be run. This is only
 *                  used in minimal angel systems, where no debugger
 *                  will step in and Execute the application.
 *
 *      Arguments:  None.
 *
 *  Pre-conditions: angel_InitialiseOneOff must have been called, and
 *                  no other task run since.
 *
 */
void 
Angel_InitialiseTaskFinished(void)
{
    LogInfo(LOG_SERLOCK, ( "Angel_InitialiseTaskFinished\n"));
}

#endif /* NOT MINIMAL_ANGEL */


/* EOF serlock.c */
