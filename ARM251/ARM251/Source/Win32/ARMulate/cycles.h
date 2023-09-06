/* cycles.h
 * Copyright (c) 1997 Advanced RISC Machines Limited
 * All Rights Reserved
 *
 * RCS $Revision: 1.1 $
 * Checkin $Date: 1997/07/23 13:57:57 $
 * Revising $Author: clavende $
 */

#include                "bron.h"

/* Application specific includes */
#include        "armdefs.h"

/* Local definitions*/
#define UL unsigned long


/* external data */


/* external function prototypes */
extern void eventQueueShutdown(void);
extern void eventQueueInit(void);
extern void coreEvents(ARMul_State *state);
extern void ARMul_ScheduleEventCore(ARMul_State *state, armul_EventProc *func,
        void *handle, unsigned long coreCycleCount);


/* OBJECT struct support */
/* ===================== */
typedef struct EVENT_QUEUE eventQueue;
typedef struct CORE_CYCLE_EVENT coreCycleEvent;

/* OBJECT type declarations */
/* ======================== */

struct EVENT_QUEUE {
        dnode   *coreCycleEvents;
                };

struct CORE_CYCLE_EVENT {
        I_DNODE( CORE_CYCLE_EVENT );
        eventQueue *myEventQueue;
        armul_EventProc *func;
        void    *handle;
        UL      count;
                };

/* Access Macros */
/* ============= */
#define         eventQueuesFirstCoreCycleEvent(val)     \
        ((coreCycleEvent*)(SUCC(val->coreCycleEvents)))
#define         eventQueuesLastCoreCycleEvent(val)      \
        ((coreCycleEvent*)(PRED(val->coreCycleEvents)))
#define         eventQueuesCoreCycleEventListIsEmpty(val)       \
        ((val)?(SUCC(val->coreCycleEvents)==val->coreCycleEvents):0)


/* OBJECT function prototypes */
/* ========================== */


/*      eventQueue Function Prototypes  */
/*      ==============================  */
eventQueue *newEventQueue();
void freeEventQueue(eventQueue *theEventQueue);


/*      coreCycleEvent Function Prototypes      */
/*      ==================================      */
coreCycleEvent *newCoreCycleEvent(armul_EventProc *func,void *handle,UL count);
void freeCoreCycleEvent(coreCycleEvent *theCoreCycleEvent);
coreCycleEvent *linkCoreCycleEvent(eventQueue *parent,coreCycleEvent *theCoreCycleEvent);
void unlinkCoreCycleEvent(coreCycleEvent *theCoreCycleEvent);
coreCycleEvent *linkNewCoreCycleEvent(eventQueue *parent,armul_EventProc *func,void *handle,UL count);
void unlinkFreeCoreCycleEvent(coreCycleEvent *theCoreCycleEvent);
