/* Copyright (C) Acorn Computers Ltd., 1988           */
/* Copyright (C) Advanced Risc Machines Ltd., 1991    */

/*
 * RCS $Revision: 1.16 $
 * Checkin $Date: 1998/05/19 14:43:56 $
 * Revising $Author: wdijkstr $
 */

#include <stddef.h>

#include "hostsys.h"
#include "interns.h"

#if defined _sysdie_c || defined SHARED_C_LIBRARY

void _sysdie(const char *s1, const char *s2)
{   _sys_msg("*** fatal error in run time system: ", NL_PRE);
    _sys_msg(s1, 0);
    _sys_msg(s2, NL_POST);
    _exit(1);
}

#endif

#if defined _sys_alloc_c || defined SHARED_C_LIBRARY

void *_sys_alloc(size_t n)
{ void *a = __rt_malloc(n);
  if (a == NULL)
    _sysdie("No store left for I/O buffer or the like", "");
  return a;
}

#endif

#if defined argerr_c || defined SHARED_C_LIBRARY

void __arg_error(const char *s1, const char *s2, const char *s3) 
{
    _sys_msg(s1, NL_PRE);
    _sys_msg(s2, 0);
    _sys_msg(s3, NL_POST);
    _exit(1);
}

#endif


#if defined lib_init_c || defined SHARED_C_LIBRARY

struct _init_flags _init_flags;

static PROC *cpp_finalise;

void __rt_lib_init(void *topofstack, void *codebase, void *codelimit,
        PROC *cpp_init_final[2])
{   char *stdinfile  = _sys_ttyfilename(),
         *stdoutfile = _sys_ttyfilename(),
         *stderrfile = _sys_ttyfilename();
    PROC *cpp_initialise = cpp_init_final[0];
    cpp_finalise = cpp_init_final[1];
    /* Provide names for stdfile opens in _initio.  (Questionable host            */
    /* independence - wouldn't be the right thing for a UNIX-style os, where      */
    /* open stdfiles are inherited).                                              */

    _init_flags.error_recursion = 0;
    _init_flags.alloc_finalised = 0;
    _init_flags.io_finalised = 0;
#ifndef SHARED_C_LIBRARY
    /* IJR: these are RISCOS relics */
    CallIfPresent(_backtrace_init, (topofstack, codebase, codelimit));
    CallIfPresent(_getenv_init, ());
#endif
    CallIfPresent(_locale_init, ()); /* init locale data structures */
    CallIfPresent(_ctype_init, ());  /* init to C locale      */
    CallIfPresent(__rt_exit_init, ());/* must happen before exit() can be called   */
    CallIfPresent(_signal_init, ()); /* had better be done pretty early           */
    CallIfPresent(_clock_init, ());  /* set Cpu time zero point  */
    CallIfPresent(_init_alloc, ());  /* as had the allocator     */
    CallIfPresent(_init_user_alloc, ());
/* SIGINT events are not safe until about now.                           */
    _raise_stacked_interrupts();     /* enable SIGINT                  */
    CallIfPresent(_initio, (stdinfile, stdoutfile, stderrfile));

    if (cpp_initialise) cpp_initialise();
}

void __rt_lib_shutdown(int callexitfns) 
{
    /* ensure no recursion if finalisation fails */
    if (callexitfns)
        CallIfPresent(__rt_call_exit_fns, ());

    if (cpp_finalise) {cpp_finalise();  cpp_finalise = 0;}

    if (!_init_flags.io_finalised) {
        _init_flags.io_finalised = 1;
        CallIfPresent(_terminateio, ());
    }
    if (!_init_flags.alloc_finalised) {
        _init_flags.alloc_finalised = 1;
        CallIfPresent(_terminate_user_alloc, ());
    }
}

#endif


#include <stdio.h>
#include <stdlib.h>
#include "ioguts.h"                         /* private flag bits, _iob   */
#include "interns.h"
#include "externs.h"

typedef int bool;

#ifndef CMD_STR_LEN
#   define CMD_STR_LEN 256
#endif

typedef enum
{
    NO_REDIRECTION, FILE_REDIRECTION, FILE_REDIRECTED, AFTER_REDIRECTION  
} Redirection_Status;

typedef struct
{
    Redirection_Status status;
    int pre_digit;
    int dup_arg1;
    char *file;
    char mode[2];

} Redirection;


#ifdef _main_redirect_c

const char _main_redirection = 0;

void _handle_redirection(Redirection *r, char *s, bool quoted)
{
#ifdef STDFILE_REDIRECTION
    char *sold = s;
    if (r->status == AFTER_REDIRECTION)
        r->status = NO_REDIRECTION;
    if (r->status == NO_REDIRECTION)
    {
        if (quoted) return;
        r->pre_digit = r->dup_arg1 = -1;

        if (is_digit(*s)) 
            r->pre_digit = *s++ - '0';
        if (*s != '<' && *s != '>')
            return;

        if (*s == '>')
        {
            s++;
            if (*s == '>')
            {
                s++;
                r->mode[0] = 'a';
            }
            else
                r->mode[0] = 'w';
            if (r->pre_digit == 0 || r->pre_digit > 2)
                goto bad_redirection;
            if (*s == '&')
            {
                s++;
                if ((r->pre_digit == 1 && *s == '2') ||
                    (r->pre_digit == 2 && *s == '1'))
                {   /* 1>&2 or 2>&1 */
                    s++;
                    r->mode[0] = 0;
                    r->dup_arg1 = r->pre_digit;
                }
                else if (r->pre_digit == -1)
                    r->dup_arg1 = 2;
                else
                    goto bad_redirection;
            }
        }
        else /* (*s == '<') */
        {   
            char *p;
            s++;
            /* If we find a matching '>' we assume it is not file indirection */
            for (p = s; *p != 0 && !is_space(*p); p++) 
                if (*p == '>')
                    return;
            if (r->pre_digit != -1)
                goto bad_redirection;
            r->mode[0] = 'r';
        }
        if (r->pre_digit == -1)
            r->pre_digit = (r->mode[0] != 'r');

        if (*s != 0)
        {   /* Filename present after redirection */
            r->status = FILE_REDIRECTED;
            if (r->mode[0] == 0)
                goto bad_redirection;
            r->file = s;
        }
        else
        {
            r->status = FILE_REDIRECTION;
            r->file = NULL;
            if (r->mode[0] != 0)
                return;
            r->status = FILE_REDIRECTED;
        }

    }
    if (r->status == FILE_REDIRECTION)
    {
        r->file = s;
        r->status = FILE_REDIRECTED;
    }
    if (r->status == FILE_REDIRECTED)
    {
        int arg1 = r->dup_arg1, arg2 = (r->dup_arg1 % 2) + 1;

        if (r->file != NULL)
        {
            FILE *f = &_iob[r->pre_digit];
            r->mode[1] = 0;
            if (freopen(r->file, r->mode, f) == 0)
                __arg_error("Can't open '", r->file, "' for I/O redirection\n");
            /* Enable fully buffered input/output - but not for stderr */
            setvbuf(f, NULL, (f == stderr) ? _IOLBF : _IOFBF, BUFSIZ);
        }
        if (r->dup_arg1 != -1 && !_sys_istty(&_iob[__dup(arg1, arg2)]))
        {   /* Combine stdout and stderr. We must revert to line buffering,
             * or otherwise the output will be not be mixed on a line by line basis.
             */
            setvbuf(&_iob[arg1], NULL, _IOLBF, STDOUT_BUFSIZ);
            setvbuf(&_iob[arg2], NULL, _IOLBF, STDOUT_BUFSIZ);
        }
        r->status = AFTER_REDIRECTION;
    }
    return;

bad_redirection:
    __arg_error("Unsupported or illegal I/O redirection '", sold, "'\n");
#endif
}

#endif

#ifdef _main_arg_c

___weak void _handle_redirection(Redirection *r, char *s, bool quoted);

void _main_arg(int (*main)(int, char **))
{   
    int ch;
    char **argv;
    char *args, *curarg, *s, *cmdstr;
    bool in_quotes = 0, was_quoted = 0;
    int argc, retval;
    Redirection redirect;
    
    redirect.status = NO_REDIRECTION;
    cmdstr = (char*) _sys_alloc(CMD_STR_LEN);
    s = __rt_command_string(cmdstr, CMD_STR_LEN);
    /* Note that s can be either cmdstr or point to a newly allocated block */

    argc = 1;
    curarg = args = cmdstr;
    do
    {   ch = *s++;
        if (!in_quotes)
        {   
            if (ch == '"' || ch == '\'')
            {   
                was_quoted = in_quotes = ch;
                continue;
            }
        }
        else
        {   /* Handle \\, \' and \" escape sequences */
            if (ch == '\\' && (*s == '"' || *s == '\\' || *s == '\''))
                ch = *s++;
            else if (ch == in_quotes)
            {
                in_quotes = 0;
                continue;
            }
        }
        if (ch != 0 && (in_quotes || !is_space(ch)))
        {
            *args++ = ch;
        }
        else if (args != curarg || was_quoted)
        {   /* End of argument word */
            *args++ = 0;
            CallIfPresent(_handle_redirection, (&redirect, curarg, was_quoted));
            if (redirect.status == NO_REDIRECTION)
                curarg = args;
            else 
                args = curarg;
            was_quoted = 0;
            argc++;
        }
    }
    while (ch != 0);

    if (in_quotes)
        __arg_error("missing closing ' or \"", "", "");

    argv = (char **)_sys_alloc(argc * sizeof(char *));

    argv[0] = cmdstr;
    for (s = cmdstr, argc = 0; s < args; 0)
        if (*s++ == 0) 
            argv[++argc] = s;
    argv[argc] = 0;      /* for ANSI spec */

    retval = main(argc, argv);
    __rt_free(argv);
    __rt_free(cmdstr);
    _exit(retval);
}

#endif

#ifdef _main_c

const char _main_redirection = 0;

void _main(int (*main)(int, char **))
{
    _exit(main(0, NULL));
}

#endif


/* end of armsys.c */
