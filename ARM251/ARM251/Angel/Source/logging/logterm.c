/* -*-C-*-
 *
 * $Revision: 1.3.2.2 $
 *   $Author: rivimey $
 *     $Date: 1998/10/19 12:22:38 $
 *
 * Copyright (c) 1997 Advanced RISC Machines Limited.
 * All Rights Reserved.
 *
 *   Project: ANGEL
 *
 *     Title: Debug interface via Serial writes to 16c552 serial port B.
 */


#include <string.h>

#if DEBUG == 1 && (!defined(MINIMAL_ANGEL) || MINIMAL_ANGEL == 0)

#include "channels.h"
#include "devconf.h"
#include "platform.h"
#include "logging.h"
#include "logging/logterm.h"
#include "serlock.h"
#include "serring.h"
#include "debug.h"
#include "support.h"
#include "disass.h"


#define SAVEBUFSIZE     2048    /* max #words in save buffer -- min 6 words/message */
#define MAXARGS         32      /* max number of distinct args on cmd line */
#define CMDBUFSIZE      128     /* max number of characters on command line */
#define OUTPUTBUFSIZE   64      /* number of buffered characters from message before flush */

#ifndef UNUSED
#define UNUSED(x)       (0 ? (x) = (x) : 0)
#endif

static unsigned long msgsavebuf[SAVEBUFSIZE];
static struct LogSaveBuffer savebuf;

static char log_commandbuf[CMDBUFSIZE];
static int log_cursor = 0;

static WarnLevel logterm_level;
static char  logterm_buf[OUTPUTBUFSIZE];
static char *logterm_pos;
static char *logterm_end;
static int log_tracing = TRUE;
static int log_deferredprompt = FALSE;
static int log_buflock = FALSE;
static int log_cmdlock = FALSE;

/* static void setupsave(void); */
void log_emitch(char ch);
void log_emitstr(char *str);
static void log_output(int enable);
static void log_processchar(char ch, unsigned int empty_stack);

static int log_dump(int argc, char **argv);
static int log_echo(int argc, char **argv);
static int log_ver(int argc, char **argv);
static int log_help(int argc, char **argv);
static int log_trace(int argc, char **argv);
static int log_go(int argc, char **argv);
static int log_restart(int argc, char **argv);
int log_pause(int argc, char **argv);
static int log_stat(int argc, char **argv);
static int log_task(int argc, char **argv);
static int log_level(int argc, char **argv);
#ifdef HAVE_DISASS
static int log_list(int argc, char **argv);
#endif
static int log_format(int argc, char **argv);

static struct 
{
    char *str;
    int (*pfn)(int argc, char **argv);
    char *desc, *helpstr;
} log_cmds[] =
{   /* These must be kept in sorted order by name */
    { "dump",  log_dump,  "Dump\n",
                          "syntax: dump <start addr> [ <end addr> | +<len> ]\n"
    },
    { "echo",  log_echo,  "Echo\n",
                          "syntax: echo <words>\n"
    },
    { "format",log_format,"Show / set the per-line format string\n",
                          "syntax: format [<new string>]\n"
    },
    { "go",    log_go,    "Undo pause; reenable ints\n",
                          "syntax: go\n"
    },
    { "help",  log_help,  "Command help\n",
                          "syntax: help [command]\n"
    },
    { "level", log_level,   "Set the minimum log level displayed.\n",
                            "syntax: level [info|warn|err]\n"
    },
#ifdef HAVE_DISASS
    { "list",  log_list,    "Disassemble instructions from memory.\n",
                            "syntax: list addr1 [addr2 | +len]\n"
    },
#endif
    { "pause", log_pause, "Pause Angel; disable ints\n",
                          "syntax: pause\n"
    },
    { "restart", log_restart,"Reboot Angel, optionally specifying restart addr.\n",
                            "syntax: reboot [address]\n"
    },
    { "stat",  log_stat,  "Display internal statistics info\n",
                          "syntax: stat [packagename]\n"
                            "  packages: serpkt\n"
    },
    { "task",  log_task,    "Display internal task details\n",
                            "syntax: task          - general scheduler details\n"
                            "        task list     - list all known task contexts\n"
                            "        task log <n>  - display <n> entries in task event log\n"
                            "        task rb <name> - print global regblock <name> from:\n"
                            "                        Intr,Desrd,SWI,Abort,Undef,Yield,Fatal,\n"
                            "        task tq <n>   - display task queue entry <n>\n"
    },
    { "trace", log_trace, "Enable/disable tracing or display trace buffer\n",
                          "syntax: trace           -- display current settings\n"
                          "        trace <n>       -- show <n> lines of buffer\n"
                          "        trace <n>       -- show <n> lines of buffer\n"
                          "        trace on | off  -- enable/disable run-time trace\n"
    },
    { "ver",   log_ver,   "Display Angel version info\n",
                          "syntax: ver\n"
    },
};
static const int log_ncmds = sizeof(log_cmds)/sizeof(log_cmds[0]);

/*
 * NT's HyperTerminal needs CRLF, not just LF!
 */
#define LOGTERM_ADD_CR_TO_LF 1

#pragma no_check_stack

#include "banner.h"
#define LOGONMESSAGE "\n\n" ANGEL_BANNER "Type 'help' for more info.\n\n"
#define PROMPT       "% "

/*
 * macros to control Interrupt Enable Register in various sensible ways
 */
#ifdef DEBUG_WITHINTERRUPTS

#define st16c552_EnableTxInterrupt(u)  (IER_set((u), TxReadyInt))
#define st16c552_DisableTxInterrupt(u) (IER_clr((u), TxReadyInt))
#define st16c552_EnableRxInterrupt(u)  (IER_set((u), RxReadyInt))
#define st16c552_DisableRxInterrupt(u) (IER_clr((u), RxReadyInt))

#endif

/*
 * prefix for lines not at start of message -- see stuff in putchar too
 */
#define PREFIXLEN   6

/*
 * prototypes
 */

/*
 *  Function: stoi
 *   Purpose: string to integer conversion; like atoi but doesn't pull in
 *            the rest of the c library!
 *
 *  Pre-conditions: none.
 *
 *    Params:
 *       Input: s - pointer to (base-10) number in ASCII
 *
 *       Output: e - pointer to last character converted
 *              
 *   Returns: number converted
 */

static int stoi(char *s, char **e)
{
    int i, sign = 0, base = 10;
    while(*s == ' ')
        s++;

    switch(*s)
    {
        case '-':
            sign = -1;
            s++;
            break;

        case '0':
            if (s[1] == 'x')
            {
                base = 16;
                s+=2;
            }
            break;

        case '+':
            s++;
            sign = 1;
            break;

        default:
            sign = 1;
            break;
    }
    i = 0;
    if (base == 10)
    {
        while(*s >= '0' && *s <= '9')
        {
            i = (i * 10) + (*s - '0');
            s++;
        }
    }
    else
    {
        while((*s >= '0' && *s <= '9') || (*s >= 'a' && *s <= 'f') ||
                (*s >= 'A' && *s <= 'F'))
        {
            if (*s >= 'a')
                i = (i * 16) + (*s - 'a' + 10);
            else if (*s >= 'A')
                i = (i * 16) + (*s - 'A' + 10);
            else
                i = (i * 16) + (*s - '0');
            s++;
        }
    }

    if (sign < 0)
        i = -i;
    *e = s;
    
    return i;
}

void logterm_flushbuf(void);

/*
 *  Function: logterm_Initialise
 *   Purpose: 
 *
 *  Pre-conditions: none.
 *
 *    Params:
 *       Input: 
 *
 *   Returns: 0.
 */

/* REMEMBER: This routine gets called VERY EARLY!! */
bool logterm_Initialise(void)
{
    logserial_Reset(LOGTERM_PORT, BAUDVALUE) ;

    /*
     * output is polled, but input is interrupt-driven.
     */
#ifdef DEBUG_WITHINTERRUPTS
    st16c552_DisableTxInterrupt(serchip);
    st16c552_EnableRxInterrupt(serchip);
#endif
    
    /*
     * print a logon banner to say we're here!
     */
    log_emitstr(LOGONMESSAGE);
    log_emitstr(PROMPT);
    
    log_deferredprompt = FALSE;
    log_tracing = TRUE;
    
    log_setupsave(&savebuf, msgsavebuf, SAVEBUFSIZE);    
    log_set_logging_options(WL_SAVEMSG|WL_PRINTMSG);
    log_set_log_id(LOG_ALWAYS, 1);

    /* this works because it's polled input... interrupts
     * are disabled in this code
     */
    log_pause(0,0);
    
    return 0;
}

struct LogSaveBuffer *log_getlogtermbuf(void)
{
    return &savebuf;
}

bool logterm_PreWarn(WarnLevel level)
{
    /*
     * set up the buffer pointers... reset in flushbuf
     */
    logterm_pos = logterm_buf;
    logterm_end = (logterm_buf + sizeof(logterm_buf) - 1);
    logterm_level = level;
    return TRUE;
}

void logterm_flushbuf(void)
{
    char *p;
    
    p = logterm_buf;
    while(p < logterm_pos)
    {
        log_emitch(*p++);
    }

    logterm_pos = logterm_buf;
    logterm_end = logterm_buf + sizeof(logterm_buf) - 1;
}


int logterm_PutChar(char c)
{
    if (logterm_pos >= logterm_end)
        logterm_flushbuf();

    *logterm_pos++ = c;

    return 0;
}

void logterm_PutString(char *str)
{
    while(*str)
    {
        logterm_PutChar(*str++);
    }
}


void logterm_PostWarn(unsigned int len)
{
    IGNORE(len);
    if (logterm_pos > logterm_buf)
        logterm_flushbuf();
}


/*
 * Show a number nlines of messages from the trace buffer, working
 * back from the current insert position.
 *
 * This is done by starting at the insert position and using the
 * count value (which is at 'insert' - 1) to skip backwards through
 * the buffer until either the oldest data is reached or the required
 * number of lines is found.
 *
 * Then work forward, calling log_logprint() to print the text.
 *
 */
static void log_showitems(struct LogSaveBuffer *sb, int nlines)
{
    int count, message_start = sb->message;
    unsigned long *ptr = sb->current;

    if (ptr == 0 || *ptr == 0) /* nothing to do */
        return;

    ptr--;      /* normally, savebufinsert points at next free slot */

    /*
     * while we haven't got to the start point  -- either the start of
     * data, or the item we want to start with, or the oldest item in
     * the buffer is reached, skip back from the insert point.
     *
     * Note: 'start of data' and 'oldest' are differently encoded -- start
     * of data is indicated by a zero count, written when the buffer is
     * set up. 'oldest' is reached when the current pointer is larger
     * than the insert pointer, and (ptr - count) is less.
     */
    count = nlines;
    while(count > 0 && *ptr != 0)
    {
        /*
         * got to oldest; no more (complete) messages available.
         */
        if (ptr >= sb->current && (ptr - (*ptr & 0xff)) < sb->current)
            break;

        /*
         * if this message marks the end of a line (flagged by the top
         * bits of the word being set), decrement the message number too.
         */
        if (*ptr & ~0xff)
        {
            message_start--;
            count--;
        }

        /* skip one more back, wrapping around the beginning. */
        ptr -= (*ptr & 0xff);
        if (ptr < sb->start)
                ptr += sb->size;
    }

    /*
     * now run forward, printing each line from the data in the buffer. Of
     * course, pointer values are printed from memory, and so may be incorrect
     * if the program changed that memory in the meantime....
     */
    count = nlines;
    while(count > 0)
    {
        log_logprint(message_start, ptr);

        if (*ptr & ~0xff)
        {
            message_start++;
            count--;
        }

        /* skip one more forward, wrapping around the beginning. */
        ptr += *ptr;
        if (ptr >= sb->end)
                ptr -= sb->size;
    }
}


void log_emitch(char ch)
{
    int status ;

#ifdef LOGTERM_ADD_CR_TO_LF
    if (ch == '\n')
    {
        do {
            status = LOG_GET_STATUS(LOGTERM_PORT) ;
        } while (!LOG_TX_READY(status)) ;

        LOG_PUT_CHAR(LOGTERM_PORT, '\r') ;
    }
#endif

    do {
        status = LOG_GET_STATUS(LOGTERM_PORT) ;
    } while (!LOG_TX_READY(status)) ;

    LOG_PUT_CHAR(LOGTERM_PORT, ch) ;

}

void log_emitstr(char *str)
{
    while(*str)
    {
        log_emitch(*str++);
    }
}

/**************************************************************************/
/*                Command Interpreter                                     */
/**************************************************************************/

/*
 *  Function: log_stat
 *   Purpose: Print status report; eventually, this should be able to, for
 *            example, print the number of packets sent, or CRC errors under ADP,
 *            or whatever else. Needs more work!
 *
 *  Pre-conditions: none.
 *
 *    Params:
 *       Input: argc, argv - "main" argc/argv pair containing args. argv[0]
 *                           is name of command (i.e. "stat").
 *
 *   Returns: 0.
 */
extern struct StatInfo spk_stat_info[];
extern struct StatInfo intr_stat_info[];
extern struct StatInfo task_stat_info[];

static int log_stat(int argc, char **argv)
{
    struct StatInfo *p = NULL;
    char *h = "known modules: serpkt, intr, task\n";
    
    if (argc < 2)
    {
        log_emitstr("stat: no module name given\n");
        log_emitstr(h);
        return 0;
    }
    
    if (__rt_strcmp(argv[1], "serpkt") == 0)
    {
        p = spk_stat_info;
    }
    else if (__rt_strcmp(argv[1], "intr") == 0)
    {
        p = intr_stat_info;
    }
    else if (__rt_strcmp(argv[1], "task") == 0)
    {
        p = task_stat_info;
    }
    if (p)
    {
        while(p->format != NULL)
        {
            int l;
            
            logterm_PreWarn(WL_INFO);
            l = log_printf(p->format, *p->param);
            logterm_PostWarn(l);
            p++;
        }
    }
    else
     {
        log_emitstr("stat: unknown module name\n");
        log_emitstr(h);
    }
   
    return 0;
}

/*
 *  Function: log_level
 *   Purpose: 
 *
 *  Pre-conditions: none.
 *
 *    Params:
 *       Input: argc, argv - "main" argc/argv pair containing args. argv[0]
 *                           is name of command (i.e. "level").
 *
 *   Returns: 0.
 */
static int log_level(int argc, char **argv)
{
    WarnLevel lvl;
    
    if (argc == 1)
    {
        lvl = log_get_log_minlevel();
        
        switch(lvl)
        {
            case WL_INFO:
                log_emitstr("level: info\n");
                break;
                
            case WL_WARN:
                log_emitstr("level: warn\n");
                break;
                
            case WL_ERROR:
                log_emitstr("level: error\n");
                break;
        }
    }
    else if (argc == 2)
    {
        
        if (argv[1][0] == 'i')
        {
            log_set_log_minlevel(WL_INFO);
        }
        else if (argv[1][0] == 'w')
        {
            log_set_log_minlevel(WL_WARN);
        }
        else if (argv[1][0] == 'e')
        {
            log_set_log_minlevel(WL_ERROR);
        }
        else
            log_emitstr("level: unknown level name\n");
    }
    else
    {
        log_emitstr("level: bad syntax\n");
    }
    
    return 0;
}

/*
 *  Function: log_go
 *   Purpose: UnPause Angel; allow it to progress after a pause.
 *
 *  Pre-conditions: none.
 *
 *    Params:
 *       Input: argc, argv - "main" argc/argv pair containing args. argv[0]
 *                           is name of command (i.e. "go").
 *
 *   Returns: 0.
 */
volatile static int log_paused = FALSE;

static int log_go(int argc, char **argv)
{
    UNUSED(argc);
    UNUSED(argv);

    if (log_paused)
        log_paused = FALSE;
    else
        log_emitstr("go: not paused\n");
    log_deferredprompt = TRUE;    
    return 0;
}

/*
 *  Function: log_restart
 *   Purpose: Reboot Angel.
 *
 *  Pre-conditions: none.
 *
 *    Params:
 *       Input: argc, argv - "main" argc/argv pair containing args. argv[0]
 *                           is name of command (i.e. "go").
 *
 *   Returns: 0.
 */
static int log_restart(int argc, char **argv)
{
    void (*addr)() = 0;
    int ok = 0;
    extern void (*__rom)();
    
    if (log_paused)
        log_paused = FALSE;

    if (argc == 1)
    {
        addr = __rom;
        ok = 1;
    }
    else
    {
        char *ep;
        addr = (void (*)())stoi(argv[1], &ep);
        
        if (ep == argv[1])
            log_emitstr("Invalid restart address\n");
        else
            ok = 1;
    }

    if (ok)
    {
        log_emitstr("\nRebooting...\n");
        addr();
    }
    
    return 0;
}

/*
 *  Function: log_pause
 *   Purpose: Pause Angel; don't allow it to progress. Terminated by 'go'
 *
 *  Pre-conditions: none.
 *
 *    Params:
 *       Input: argc, argv - "main" argc/argv pair containing args. argv[0]
 *                           is name of command (i.e. "pause").
 *
 *   Returns: 0.
 */

int log_pause(int argc, char **argv)
{
    int state, intStatus;

    UNUSED(argc);
    UNUSED(argv);
    
    if (log_paused)
    {
        log_emitstr("pause: Already paused\n");
        return 1;
    }

    state = Angel_DisableInterruptsFromSVC();
    log_paused = TRUE;
    while(log_paused)
    {
        unsigned char ch;

        intStatus = LOG_GET_STATUS(LOGTERM_PORT) ;
        if ( LOG_RX_DATA(intStatus) ) {
            ch = LOG_GET_CHAR(LOGTERM_PORT);
            log_processchar(ch, 0);        
        }
    }
    Angel_RestoreInterruptsFromSVC(state);
    
    return 0;
}

/*
 *  Function: log_echo
 *   Purpose: Debugging aid.
 *
 *  Pre-conditions: none.
 *
 *    Params:
 *       Input: argc, argv - "main" argc/argv pair containing args. argv[0]
 *                           is name of command (i.e. "ver").
 *
 *   Returns: 0.
 */

static int log_echo(int argc, char **argv)
{
    int i;
    for(i = 0; i < argc; i++)
    {
        log_emitch('\"');
        log_emitstr(argv[i]);
        log_emitch('\"');
        log_emitch(' ');
    }
    log_emitch('\n');
    return 0;
}

int buf_itoh(char *buf, unsigned long uval, int width, int padzero);

static void dump_buffer(unsigned int width, char *buffer, unsigned int length)
{
    unsigned int i, j ;
    char b[128] ;
    char *p ;

    if (width == 4)
    {
        for (i = 0; i < length; i += 16)
        {
            p = b;
            p += buf_itoh(p, (unsigned int)&buffer[i], 8, 1);
            *p++ = ':';
            *p++ = ' ';

            for (j = 0; j < 16 && (i + j) < length; j += 4)
            {
                unsigned int *ptr ;
                ptr = (unsigned int *)&buffer[i + j] ;
                p += buf_itoh(p, *ptr, 8, 1);
                *p++ = ' ';
            }

            for (; j < 16; j += 4)
            {
                *p++ = ' ';
                *p++ = ' ';
                *p++ = ' ';
                *p++ = ' ';
                *p++ = ' ';
                *p++ = ' ';
                *p++ = ' ';
                *p++ = ' ';
                *p++ = ' ';
            }

            for (j = 0; j < 16 && (i + j) < length; j++)
            {
                unsigned char c = buffer[i + j];

                *p++ = (c >= 0x20 && c < 0x7F) ? c : '.';
            }
            *p++ = '\0';
            log_printf("%s\n", b);
        }
    }
    else
    {
        /* Default width to 1 */
        for (i = 0; i < length; i +=16)
        {
            p = b;
            p += buf_itoh(p, i, 8, 1);
            *p++ = ':';
            *p++ = ' ';

            for (j = 0; j < 16 && (i + j) < length; j++)
            {
                p += buf_itoh(p, buffer[i + j] & 0xff, 2, 1);
                *p++ = ' ';
            }
            for (; j <= 16; j++)
            {
                *p++ = ' ';
                *p++ = ' ';
                *p++ = ' ';
            }
            for (j = 0; j < 16 && (i + j) < length; j++)
            {
            unsigned char c = buffer[i + j];

                *p++ = (c >= 0x20 && c < 0x7F) ? c : '.';
            }
            *p++ = '\0';
            log_printf("%s\n", b);
        }
    }
}

/*
 *  Function: log_dump
 *   Purpose: Print the contents of memory.
 *
 *  Pre-conditions: none.
 *
 *    Params:
 *       Input: argc, argv - "main" argc/argv pair containing args. argv[0]
 *                           is name of command (i.e. "ver").
 *
 *   Returns: 0.
 */

static int log_dump(int argc, char **argv)
{
    int addr1 = 0, addr2 = 0, len = 0;
    char *e;
    int width = 4;
    int i = 1;

    if (argc == 1 || argc > 4)
    {
        log_emitstr("dump: [b|w] addr1 [addr2 | +nbytes]\n");
        log_emitstr(" [b|w]  - byte or word wide (default word)\n");
        return 0;
    }

    if (argv[1][0] == 'b')
    {
        width = 1;
        i++;
    }
    else if (argv[1][0] == 'w')
    {
        width = 4;
        i++;
    }

    if (i < argc)
    {
        addr1 = stoi(argv[i++], &e);
        addr2 = addr1 + 64;
        len = 64;
    }

    if (i < argc)
    {
        if (argv[i][0] == '+')
        {
            len = stoi(argv[i]+1, &e);
            addr2 = addr1 + len;
        }
        else
        {
            addr2 = stoi(argv[i], &e);
            len = (addr2 - addr1) + 1;
        }
    }

    logterm_PreWarn(WL_INFO);

    log_printf("Memory from %lx to %lx\n", addr1, addr2);

    dump_buffer(width, (char*)addr1, len);

    logterm_PostWarn(0);

    return 0;
}
/*
 *  Function: log_list
 *   Purpose: Disassemble the contents of memory.
 *
 *  Pre-conditions: none.
 *
 *    Params:
 *       Input: argc, argv - "main" argc/argv pair containing args. argv[0]
 *                           is name of command (i.e. "ver").
 *
 *   Returns: 0.
 */

#ifdef HAVE_DISASS
static char* list_discb(dis_cb_type type, int32 offset, unsigned32 address,
                     int width, void *cb_arg, char *buffer)
{
    return buffer;
}

static int log_list(int argc, char **argv)
{
    unsigned int addr1 = 0, addr2 = 0;
    char *e;
    unsigned int *addr, instr;
    static char b[64];

    if (argc == 1 || argc > 3)
    {
        log_emitstr("list: addr1 [addr2 | +nbytes]\n");
        return 0;
    }

    if (argc == 2)
    {
        addr1 = stoi(argv[1], &e);
        addr2 = addr1 + 64;
    }
    else if (argc == 3)
    {
        addr1 = stoi(argv[1], &e);
        if (argv[2][0] == '+')
        {
            unsigned int len = 0;
            
            len = stoi(argv[2]+1, &e);
            addr2 = addr1 + len;
        }
        else
        {
            addr2 = stoi(argv[2], &e);
        }
    }
    
    logterm_PreWarn(WL_INFO);
    log_printf("Disassembly from %08lx to %08lx:\n", addr1, addr2);
    
    for(addr = (unsigned int *)addr1; addr < (unsigned int *)addr2; addr++)
    {
        instr = *addr;
        disass(instr, (unsigned int)addr, b, NULL, list_discb);
        log_printf("%08x: %08x  %s\n", addr, instr, b);
    }
    logterm_PostWarn(0);
    
    return 0;
}
#endif


/*
 *  Function: log_ver
 *   Purpose: Version command routine. Prints the Angel banner again.
 *
 *  Pre-conditions: none.
 *
 *    Params:
 *       Input: argc, argv - "main" argc/argv pair containing args. argv[0]
 *                           is name of command (i.e. "ver").
 *
 *   Returns: 0.
 */

static int log_ver(int argc, char **argv)
{
    UNUSED(argc);
    UNUSED(argv);
    
    log_emitstr(ANGEL_BANNER);
    return 0;
}

/*
 *  Function: log_format
 *   Purpose: Set the per-message info
 *
 *  Pre-conditions: none.
 *
 *    Params:
 *       Input: argc, argv - "main" argc/argv pair containing args. argv[0]
 *                           is name of command (i.e. "ver").
 *
 *   Returns: 0.
 */

static int log_format(int argc, char **argv)
{
    if (argc == 1)
    {
        log_emitstr("format: \"");
        log_emitstr(log_get_format_string());
        log_emitstr("\"\n");
    }
    else
    {
        if (argc == 2)
        {
            log_set_format_string(argv[1]);
        }
        else
        {
            log_emitstr("format: too many args\n");
        }
    }
    return 0;
}

/*
 *  Function: log_help
 *   Purpose: Help command routine. Given a single arg, prints the help
 *            string for that command, if it exists.
 *
 *            With no args, prints all help available.
 *
 *  Pre-conditions: none.
 *
 *    Params:
 *       Input: argc, argv - "main" argc/argv pair containing args. argv[0]
 *                           is name of command (i.e. "help").
 *
 *   Returns: 0 if help found, 1 otherwise.
 */

static int log_help(int argc, char **argv)
{
    int i;

    if (argc > 1)
    {
        for (i = 0; i < log_ncmds; i++)
        {
            switch(__rt_strcmp(argv[1], log_cmds[i].str))
            {
                case 0: /* match ! */
                    log_emitstr(log_cmds[i].desc);
                    log_emitstr(log_cmds[i].helpstr);
                    return 0;

                case -1: /* no match found */
                    log_emitstr("No help found?\n");
                    return 1;
            }
        }
    }
    else
    {
        log_emitstr("Help Available:\n");
        for (i = 0; i < log_ncmds; i++)
        {
            log_emitstr(log_cmds[i].str);
            log_emitstr(" - ");
            log_emitstr(log_cmds[i].desc);
        }
    }
    return 0;
}


/*
 *  Function: log_trace
 *   Purpose: The trace action. trace syntax is:
 *       trace <on>|<off>    switch real-time tracing on or off
 *       trace <lines>       show <lines> worth of previous trace history
 *       trace               show currently traced modules
 *       trace <names>       change traced modules; <names> is space separated
 *                           list, with '-' before items to be removed. 'all'
 *                           can be used to specify all modules (so '-all'
 *                           means none).
 * 
 *  Pre-conditions: none.
 *
 *    Params:
 *       Input: argc, argv - "main" argc/argv pair containing args. argv[0]
 *                           is name of command (i.e. "trace").
 *
 *   Returns: 0
 */

static int log_trace(int argc, char **argv)
{
    int i, rem, n;
    char *p;

    if (argc == 1) /* no args */
    {
        n = log_get_num_ids();
        log_emitstr("trace: ");
        for(i = 0; i < n; i++)
        {
            if (log_get_log_id((log_id)i))
            {
                log_emitstr(log_tracename((log_id)i));
                log_emitch(' ');
            }
        }
        log_emitch('\n');
    }
    else if (argc >= 2) /* at least one arg */
    {
        if (argv[1][0] >= '0' && argv[1][0] <= '9')
        {
            int lines;
            char *ep;
            lines = stoi(argv[1], &ep);
            
            if (ep > argv[1] && lines > 0 && lines < 1000)
            {
                /*
                 * don't want current and old output mixed together, do we?
                 */
                log_output(FALSE);
                log_showitems(&savebuf, lines); 
                log_output(TRUE);
            } 
            else 
                log_emitstr("trace: Bad number.\n"); 
        } 
        else if (__rt_strcmp(argv[1], "on") == 0)
        {
            log_emitstr("trace: on (was: "); 
            log_emitstr(log_tracing ? "on)\n" : "off)\n"); 
            log_tracing = TRUE;
        }
        else if (__rt_strcmp(argv[1], "off") == 0)
        {
            log_emitstr("trace: off (was: "); 
            log_emitstr(log_tracing ? "on)\n" : "off)\n"); 
            log_tracing = FALSE;
        }
        else
        {
            for(i = 1; i < argc; i++)
            {
                log_id id;

                if (argv[i][0] == '-')
                    rem = TRUE, p = argv[i]+1;
                else if (argv[i][0] == '+')
                    rem = FALSE, p = argv[i]+1;
                else
                    rem = FALSE, p = argv[i];

                id = log_tracebit(p);
                if (id != 0)
                {
                    log_set_log_id(id, !rem);
                }
                else
                {
                    log_emitstr("trace: unknown module ");
                    log_emitstr(p);
                    log_emitch('\n');
                }
            }
        }
    }
    return 0;
}


/*
 *  Function: log_RegblockName
 *   Purpose: The trace action. trace syntax is:
 *       task             show current task stats
 *       task <number>    show <number> prior task switches
 * 
 *  Pre-conditions: none.
 *
 *    Params:
 *       Input: argc, argv - "main" argc/argv pair containing args. argv[0]
 *                           is name of command (i.e. "trace").
 *
 *   Returns: 0
 */
struct angel_TaskLog
{
    int count;
    int id;
    angel_TaskQueueItem* taskblock;
    angel_RegBlock *regblock;
    int cur_CPSR;
    int cur_SP;
    int rb_PC;
    int rb_CPSR;
    int rb_SP;
    int rb_r0;
};
extern angel_TaskQueueItem *angel_TaskQueueHead;
extern struct angel_TaskLog* angel_DebugTaskBase;
extern struct angel_TaskLog* angel_DebugTaskArea;
extern int angel_DebugStartTaskCount;
extern int angel_DebugQueueTaskCount;
extern angel_TaskQueueItem angel_TQ_Pool[POOLSIZE];
static char *rbnames[7] =
{
    "Intr", "Desrd", "SWI", "Undef", "Abort", "Yield", "Fatal"
};

static char *typenames[11] =
{
    "",
    "StartTsk", "QueueTsk", "IntHand", "SaveTsk",
    "Yield", "SWIHand", "BRKHand", "Wait",
    "Abort", "DelTask"
};

static char *tasknames[6] =
{
    "IdleLoop",
    "AngelInit",
    "Application",
    "ApplCallBack",
    "AngelCallBack",
    "AngelWantLock"
};

static char *taskstates[5] =
{
    "Undefined",
    "Defined",
    "Runnable",
    "Running",
    "Blocked"
};

static void log_RegblockName(void *rb)
{
    if (rb == 0)
        logterm_PutString("        ");
    else if (rb >= (void *)&Angel_GlobalRegBlock[0] &&
        rb < (void *)&Angel_GlobalRegBlock[RB_NumRegblocks])
    {
        int index = (angel_RegBlock*)rb - &Angel_GlobalRegBlock[0];
        
        log_printf("RB %5s", rbnames[index]);
    }   
    else if (rb >= (void *)&angel_TQ_Pool[0] &&
             rb < (void *)&angel_TQ_Pool[POOLSIZE])
    {
        int index = (angel_TaskQueueItem*)rb - &angel_TQ_Pool[0];
        
        log_printf("TQ %5d", index);
    }
    else
        log_printf("rb%6lx", rb);
}


static void log_ShowRegblock(angel_RegBlock *rb)
{
    log_printf("      pc = %08lx cpsr = %08lx\n",
               rb->pc, rb->cpsr);
    
    log_printf("      r0 = %08lx   r1 = %08lx   r2 = %08lx   r3 = %08lx\n",
               rb->r0, rb->r1, rb->r2, rb->r3);
    log_printf("      r4 = %08lx   r5 = %08lx   r6 = %08lx   r7 = %08lx\n",
               rb->r4, rb->r5, rb->r6, rb->r7);
    
    log_printf("USR:  r8 = %08lx   r9 = %08lx  r10 = %08lx  r11 = %08lx\n",
               rb->r8usr, rb->r9usr, rb->r10usr, rb->r11usr);
    log_printf("     r12 = %08lx  r13 = %08lx  r14 = %08lx\n",
               rb->r12usr, rb->r13usr, rb->r14usr);
    
    log_printf("FIQ:  r8 = %08lx   r9 = %08lx  r10 = %08lx  r11 = %08lx\n",
               rb->r8fiq, rb->r9fiq, rb->r10fiq, rb->r11fiq);
    log_printf("     r12 = %08lx  r13 = %08lx  r14 = %08lx\n",
               rb->r12fiq, rb->r13fiq, rb->r14fiq);
    
    log_printf("IRQ: r13 = %08lx  r14 = %08lx spsr = %08lx\n",
               rb->r13irq, rb->r14irq, rb->spsrirq);
    
    log_printf("SVC: r13 = %08lx  r14 = %08lx spsr = %08lx\n",
               rb->r13svc, rb->r14svc, rb->spsrsvc);
    
    log_printf("UND: r13 = %08lx  r14 = %08lx spsr = %08lx\n",
               rb->r13und, rb->r14und, rb->spsrund);
    
    log_printf("ABT: r13 = %08lx  r14 = %08lx spsr = %08lx\n",
               rb->r13abt, rb->r14abt, rb->spsrabt);
}

static void log_ShowTaskItem(int tqi, angel_TaskQueueItem *tq)
{
    log_printf("Task Queue %d: (%08lx) %s\n"
               "Next = %08lx, Index = %d, Type = %s, State = %s, Priority = %u\n",
               tqi, tq, (tq->name) ? tq->name : "", tq->next,
               tq->index, tasknames[tq->type], taskstates[tq->state], tq->priority);
    
    log_printf("Stack Base = %08lx, Limit = %08lx, Entrypoint = %08lx\n",
               tq->stackBase, tq->stackLimit, tq->entryPoint);
    
    log_printf("Signal Wait = %08lx, Signal Bits = %08lx\n",
               tq->signalWaiting, tq->signalReceived);
    
    log_ShowRegblock(&angel_TQ_Pool[tqi].rb);
}


/*
 *  Function: log_task
 *   Purpose: The trace action. trace syntax is:
 *       task             show current task stats
 *       task <number>    show <number> prior task switches
 * 
 *  Pre-conditions: none.
 *
 *    Params:
 *       Input: argc, argv - "main" argc/argv pair containing args. argv[0]
 *                           is name of command (i.e. "trace").
 *
 *   Returns: 0
 */
static int log_task(int argc, char **argv)
{
    
    if (argc == 1) /* no args */
    {
        logterm_PreWarn(WL_INFO);
        
        log_printf("Log Base:        %8lx\n", angel_DebugTaskBase);
        log_printf("Next Entry @:    %8lx\n", angel_DebugTaskArea);
        log_printf("Started Tasks:   %8ld\n", angel_DebugStartTaskCount);
        log_printf("Queued Tasks:    %8ld\n", angel_DebugQueueTaskCount);
        log_printf("Deferred Block   %8ld\n", angel_SWIDeferredBlock);
        log_printf("Processing SWI   %8ld\n", angel_SWIReturnToApp);
        log_printf("Stack Alloc Mask %8lx\n", angel_stacksUsed);
        
        logterm_PostWarn(0);
    }
    else if (argc > 1 && __rt_strcmp(argv[1], "list") == 0) /* at least one arg */
    {
        angel_TaskQueueItem *tq = angel_TaskQueueHead;
        
        logterm_PreWarn(WL_INFO);
        while(tq != NULL)
        {
            log_ShowTaskItem(tq->index, tq);
            
            tq = tq->next;
            if (tq)
                logterm_PutString("\n");
        }
        
        logterm_PostWarn(0);
    }
    else if (argc > 1 && __rt_strcmp(argv[1], "log") == 0) /* at least one arg */
    {
        int ntasks;
        char *ep;
        struct angel_TaskLog *tl = angel_DebugTaskArea;
        struct angel_TaskLog *tlbase = angel_DebugTaskBase;

        logterm_PreWarn(WL_INFO);

        ntasks = stoi(argv[2], &ep);
        if (ep > argv[1] && ntasks > 0 && ntasks < 100000)
        {
            tl--; /* tl starts off pointing at next free block */
            while(ntasks > 0 && tl >= tlbase)
            {
                log_printf("%9d: %8s ", tl->count, typenames[tl->id]);
                log_RegblockName(tl->taskblock);
                logterm_PutString(", ");
                log_RegblockName(tl->regblock);
                log_printf(", from: [cpsr %08lx, sp %08lx] ",
                           tl->cur_CPSR, tl->cur_SP);
                log_printf("to: [pc %08lx, cpsr %08lx, sp %08lx, r0 %08lx]\n",
                           tl->rb_PC, tl->rb_CPSR, tl->rb_SP, tl->rb_r0);
                logterm_flushbuf();

                tl--;
                ntasks--;
            }
        }
        else
            logterm_PutString("task: not a number\n");
        
        logterm_PostWarn(0);
    }
    else if (argc > 2 && __rt_strcmp(argv[1], "rb") == 0)
    {
        int i;
        
        logterm_PreWarn(WL_INFO);
        for (i = 0; i < RB_NumRegblocks; i++)
        {
            if (__rt_strcmp(argv[2], rbnames[i]) == 0)
            {
                log_printf("Global Regblock %s:\n", rbnames[i]);
                log_ShowRegblock(&Angel_GlobalRegBlock[i]);
                break;
            }
        }
        logterm_PostWarn(0);
    }
    else if (argc > 2 && __rt_strcmp(argv[1], "tq") == 0)
    {
        char *ep;
        int tqi = stoi(argv[2], &ep);
        
        logterm_PreWarn(WL_INFO);
        if (ep > argv[2] && tqi >= 0 && tqi < POOLSIZE)
        {
            angel_TaskQueueItem *tq = &angel_TQ_Pool[tqi];
            
            log_ShowTaskItem(tq->index, tq);
        }
        else
            logterm_PutString("Invalid queue index\n");
        
        logterm_PostWarn(0);
    }
    return 0;
}

/*
 *  Function: log_execute
 *   Purpose: Accept a command string which will be split up by into
 *            overwriting spaces into individual strings indexed by
 *            an array of string pointers, to create a C library like
 *            argc/argv pair. Look up the command (the first such string,
 *            if any) in the list of known commands, and if found call
 *            the execution function. If the command is not found, print
 *            an error message ("Eh?"!!) and return.
 *
 *            Unless the command set log_deferredprompt, print a prompt
 *            for the user once command processing complete.
 *
 *  Pre-conditions: log_Initialise called
 *
 *    Params:
 *       Input: cmd - pointer to command string.
 *              
 *   Returns: nothing.
 */

static void log_execute(char *cmd)
{
    char *s = cmd;
    char *argv[MAXARGS];
    int argc, i;

    for(argc = 0; argc < MAXARGS && *s != '\0';)
    {
        while(*s == ' ' || *s == '\t')
            s++;

        /*
         * if quoted arg, search for end of quote;
         * else search for next whitespace.
         */
        if (*s == '\"')
        {
            argv[argc++] = ++s;

            while(*s != '\"' && *s != '\0')
                s++;
        }
        else
        {
            argv[argc++] = s;
            while(*s != ' ' && *s != '\t' && *s != '\0')
                s++;
        }

        /*
         * replace character which ended search (if not EOS)
         * with EOS (to terminate this arg) and increment s
         * so we can search for more...
         */
        if (*s == ' ' || *s == '\t' || *s == '\"')
        {
            *s = '\0';
            s++;
        }
    }

    argv[argc] = NULL;

    if (argv[0] == NULL)
        goto finish;

    for (i = 0; i < log_ncmds; i++)
    {
        switch(__rt_strcmp(argv[0], log_cmds[i].str))
        {
            case 0: /* match ! */
                log_cmds[i].pfn(argc, argv);
                goto finish;

            case -1: /* no match found */
                log_emitstr("Eh? Didn't understand: ");
                log_emitstr(argv[0]);
                log_emitch('\n');
                goto finish;
            
            case 1: /* still looking; */
                break;
        }
    }
finish:
    if (log_deferredprompt == FALSE)
        log_emitstr(PROMPT);
    
    return;
}


/*
 main_table
   "", do_nothing, 0,
   "help", do_execute, helptable,
   NULL,
   
 helptable
   "", do_general_help, helptable,
   "stat", do_specific_help, "Print Statistics\0syntax: ..."

 */

/*
 *  Function: log_serialexecute
 *   Purpose: Wrapper for log_execute, when that function is being called
 *            from the serializer. It basically maintains various locks,
 *            and resets the command buffer...
 *
 *  Pre-conditions: none.
 *
 *    Params:
 *       Input: cmd - the command to execute
 *
 *       Output: none
 *              
 *   Returns: none.
 */

static void log_serialexecute(char *cmd)
{
    static char buf[CMDBUFSIZE];

    __rt_strcpy(buf, cmd);
    
    log_commandbuf[0] = '\0';
    log_cursor = 0;
    log_buflock = FALSE;
    
    log_emitstr("Executing: ");
    log_emitstr(buf);
    log_output(TRUE);
    
    log_commandbuf[0] = '\0';
    log_cursor = 0;

    log_execute(buf);
    
    log_cmdlock = FALSE;
}

/*
 *  Function: log_processchar
 *   Purpose: CALLED AS AN INTERRUPT ROUTINE!
 *
 *            Process a character received from the controlling
 *            terminal. Form a command string for execution by
 *            log_execute, using the serialiser to enable the
 *            commands to overcome the 'fast' limitation of int
 *            handlers.
 *            
 *            Handle some simplistic echo/deletion codes too.
 *            
 *            If log_deferredprompt is set, we must print a prompt
 *            before eching the character. We must also bracket
 *            the operation with a request to kill run time trace
 *            ouptut temporarily while a command is typed.
 *
 *  Pre-conditions: log_Initialise, interrupt handler set up
 *
 *    Params:
 *       Input: ch - the character read from the terminal
 *              empty-stack - passed in by the int handler for the
 *                            serialiser.
 *              
 *   Returns: nothing.
 */

static void log_processchar(char ch, unsigned int empty_stack)
{
    /* can't process -- previous command hasn't copied buffer yet */
    if (log_buflock)
        return;
    
    if (log_deferredprompt)
    {
        log_emitstr(PROMPT);
        log_output(FALSE);
        log_deferredprompt = FALSE;
    }
    
    switch(ch)
    {
        case '\b':    /* backspace */
        case '\177':  /* delete */
            if (log_cursor > 0)
            {
                log_emitstr("\b \b"); /* delete */
                log_commandbuf[log_cursor--] = '\0';
                log_commandbuf[log_cursor] = '\0';
            }
            if (log_cursor == 0)
            {
                log_deferredprompt = TRUE;
                log_emitstr("\b\b  \b\b"); /* length of prompt */
                log_output(TRUE);
            }
            break;
        
        case '\n':
        case '\r':
            log_emitch('\n');
            if ( 1 /* empty_stack == 0*/ ) /* if called from 'pause' */
            {
                log_output(TRUE);
                log_execute(log_commandbuf);
                
                log_commandbuf[0] = '\0';
                log_cursor = 0;
            }
            else
            {
                /* can't execute; previous command hasn't finished yet */
                if (log_cmdlock)
                    break;
                
                /* lock buffer while we're copying it from log_commandbuf;
                   lock command interp. until command completed */
                log_cmdlock = TRUE;
                log_buflock = TRUE;
            
                Angel_SerialiseTask(0,
                                    (angel_SerialisedFn)log_serialexecute,
                                    (void *)log_commandbuf,
                                    empty_stack,
                                    &Angel_GlobalRegBlock[RB_Interrupted]);
            }
            break;
                
        default:
            if (ch >= 32) /* note: ch signed, so only 32-126 get through (127 above)! */
            {
                log_commandbuf[log_cursor++] = ch;
                log_commandbuf[log_cursor] = '\0';
                log_emitch(ch);
                log_output(FALSE);
            }
            break;
    }
}

/*
 *  Function: log_output
 *   Purpose: To enable/disable real-time trace output while commands are
 *            being typed. If log_tracing is TRUE, then overall we want tracing,
 *            while if false, we don't. log_ouptut is intended to bracket
 *            regions of code when trace would be a bad idea.
 *
 *  Pre-conditions: none.
 *
 *    Params:
 *       Input: enable          true if we're leaving a sensitive area,
 *                              false if entering one.
 *
 *   Returns: Nothing
 */

static void log_output(int enable)
{
    if (enable && log_tracing)
    {
        log_set_logging_options(log_get_logging_options() | WL_PRINTMSG);
    }
    else
    {
        log_set_logging_options(log_get_logging_options() & ~WL_PRINTMSG);        
    }
}

void logterm_fatalhandler(void)
{
    log_printf("\nFatal error. Type 'help' for command list.\n");
    log_printf("CPU Registers:\n");
    logterm_flushbuf();
    log_ShowRegblock(&Angel_GlobalRegBlock[RB_Fatal]);
    log_printf("\n");
    log_emitstr(PROMPT);
    
    log_deferredprompt = FALSE;
    log_paused = FALSE;
    log_pause(0, 0);
}



/*
 *  Function: int_*
 *   Purpose: Set of handlers for the various prioritised interrupts.
 *              These routines do all the urgent processing required
 *              for the interrupt condition
 *
 *  Pre-conditions: Processor is in IRQ / FIQ mode, with a minimal stack,
 *                      AND NO STACK LIMIT SET UP.
 *
 *    Params:
 *       Input: devid           device ID of the driver
 *              port            serial port identifier
 *              empty_stack     top of the stack
 *
 *   Returns: Nothing
 */

static void int_txrdy(unsigned int empty_stack)
{
    unsigned int lsr;
    
    IGNORE(empty_stack);

    /*
     * we really couldn't care less about these signals (in
     * fact, we shouldn't really ever get this interrupt
     * because it is never enabled); read the status to clear
     * the interrupt and go away again
     */
    lsr = LOG_GET_STATUS(LOGTERM_PORT) ;
}

static void int_rxrdy(unsigned int empty_stack)
{
    unsigned int intStatus ;

    /*
     * keep looping for as long as there are characters
     * in the Rx FIFO
     */
    intStatus = LOG_GET_STATUS(LOGTERM_PORT) ;
    if ( LOG_RX_DATA(intStatus) ) {
        do {
            unsigned char ch = LOG_GET_CHAR(LOGTERM_PORT);
            log_processchar(ch, empty_stack);        
            intStatus = LOG_GET_STATUS(LOGTERM_PORT) ;
        } while ( LOG_RX_DATA(intStatus) ) ;
    }
}

/*
 *  Function: angel_LogtermIntHandler
 *   Purpose: Entry point for interrupts from the ST16C552 UART
 *            See documentation for angel_IntHandlerFn in devdriv.h
 */
void angel_LogtermIntHandler(unsigned int ident, unsigned int devid,
                             unsigned int empty_stack)
{
    unsigned int IntSource, IntStatus ;
    
    IGNORE(ident);
    IGNORE(devid);

    IntSource = LOG_READ_INTERRUPT(LOGTERM_PORT) ;

    if (IntSource & LOG_RX_INTERRUPT) {
        int_rxrdy(empty_stack) ;   
    }
    else if (IntSource & LOG_TX_INTERRUPT) {
        int_txrdy(empty_stack) ;   
    }
    else {
        /* Unknown interrupt source */
        /* clear out any errors by reading the line status register */
        IntStatus = LOG_GET_STATUS(LOGTERM_PORT) ;
    }
}



#endif /* DEBUG && !MINIMAL ANGEL */

/* EOF logterm.c */
