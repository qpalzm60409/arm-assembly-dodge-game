/*
 * string.c: ANSI draft (X3J11 Oct 86) library code, section 4.11
 * Copyright (C) Codemist Ltd., 1988                              
 * Copyright (C) Advanced Risc Machines Ltd., 1997. All rights reserved.
 */

/*
 * RCS $Revision: 1.11.6.1 $
 * Checkin $Date: 1998/08/14 13:20:08 $
 * Revising $Author: wdijkstr $
 */

#include "config.h"                /* for BYTESEX_EVEN/BYTESEX_ODD         */
#include <stddef.h>                /* for size_t                           */
#include <string.h>

#define _chararg int               /* arg spec for char when ANSI says int */
#define _copywords 1               /* do fast cpy/cmp if word aligned      */

/* The following magic check was designed by A. Mycroft. It yields a     */
/* nonzero value if the argument w has a zero byte in it somewhere. The  */
/* messy constants have been unfolded a bit in this code so that they    */
/* get preloaded into registers before relevant loops.                   */

#ifdef _copywords
#  define ONES_WORD   0x01010101
#  define EIGHTS_WORD (ones_word << 7)
#  define nullbyte_prologue_() \
      int ones_word = ONES_WORD;
#  define word_has_nullbyte(w) (((w) - ones_word) & ~(w) & EIGHTS_WORD)
#endif

#if defined memcpy_c

void *memcpy(void *a, const void *b, size_t n)
/* copy memory (upwards) - it is an errof for args to overlap. */
/* Relies on sizeof(int)=sizeof(void *) and byte addressing.   */
{
#ifdef _copywords
    /* do it fast if word aligned ... */
    if ((((int)a | (int)b | (int)n) & 3) == 0)
    { int *wa,*wb;
      n >>= 2;
      for (wa = (int *)a, wb = (int *)b; n-- > 0;) *wa++ = *wb++;
    }
    else
#endif
    { char *ca,*cb;
      for (ca = (char *)a, cb = (char *)b; n-- > 0;) *ca++ = *cb++;
    }
    return a;
}

#endif

#if defined memmove_c

void *memmove(void *a, const void *b, size_t n)
/* copy memory taking care of overlap */
/* Relies on sizeof(int)=sizeof(void *) and byte addressing.
   Also that memory does not wrap round for direction test. */
{
#ifdef _copywords
    /* do it fast if word aligned ... */
    if ((((int)a | (int)b | (int)n) & 3) == 0)
    { int *wa,*wb;
      n >>= 2;
      if (a < (void *)b)
         for (wa = (int *)a, wb = (int *)b; n-- > 0;) *wa++ = *wb++;
      else for (wa = n+(int *)a, wb = n+(int *)b; n-- > 0;) *--wa = *--wb;
    }
    else
#endif
    { char *ca,*cb;
      if (a < (void *)b)
         for (ca = (char *)a, cb = (char *)b; n-- > 0;) *ca++ = *cb++;
      else for (ca = n+(char *)a, cb = n+(char *)b; n-- > 0;) *--ca = *--cb;
    }
    return a;
}

#endif

#if defined memchr_c || defined SHARED_C_LIBRARY

void *memchr(const void *s, _chararg ch, size_t n)
                                            /* first instance of ch in s */
{   const unsigned char *t = (const unsigned char *)s;
    unsigned char c1 = (unsigned char)ch, c2;
    if (n > 0)
    {   const unsigned char *end = t + n;
        for (;;)
        {
            c2 = *t++;
            if (t == end || c1 == c2) break;
        }   
        if (c1 == c2) return (void *) (t-1);
    }
    return 0;
}

#endif

#if defined memcmp_c || defined SHARED_C_LIBRARY

int memcmp(const void *a, const void *b, size_t n)
{   const unsigned char *ac = (const unsigned char *)a,
                        *bc = (const unsigned char *)b;
    unsigned int c1, c2;
#ifdef _copywords
    if (n >= 4 && (((int)ac | (int)bc) & 3) == 0)
    {   
        do
        {
            c1 = *(int *)ac, ac += 4;
            c2 = *(int *)bc, bc += 4;
            if (c1 != c2) break;
            n -= 4;
        } while (n >= 4);
        if (c1 != c2)           /* redo last word using byte compares */
        {
            ac -= 4;
            bc -= 4;
        }
    }
#endif
    if (n == 0) return 0;
    if (n & 1)
    {   n++;
        goto memcmp_odd;
    }
    do
    {   
        c1 = *ac++;
        c2 = *bc++;
        if (c1 != c2) break;
memcmp_odd:
        c1 = *ac++;
        c2 = *bc++;
        if (c1 != c2) break;
    } while ( (n -= 2) > 0);
    return c1 - c2;
}

#endif

#if defined memset_c

void *memset(void *s, _chararg c, size_t n)
{
    unsigned char *p = (unsigned char *)s;
    while (n > 0)
    {
#ifdef _copywords
        if (n >= 4 && ((int)p & 3) == 0)
        {   int w = 0x01010101 * (unsigned char)c;     /* duplicate 4 times */
            do *(int *)p = w, p += 4, n -= 4; while (n >= 4);
        }
        else
#endif
            *p++ = (unsigned char)c, n--;
    }
    return s;
}

#endif

#if defined strcat_c || defined SHARED_C_LIBRARY

char *strcat(char *a, const char *b)    /* concatenate b on the end of a */
{   char *p = a;
    while (*p != 0) p++;
    while ((*p++ = *b++) != 0);
    return a;
}

#endif

#if defined strchr_c || defined SHARED_C_LIBRARY

char *strchr(const char *s, _chararg ch)
                                        /* find first instance of ch in s */
{   char c1 = (char)ch, c2;
    for (--s;;)
    {   c2 = *++s;
        if (c1 == c2 || c2 == 0) break;
    }
    if (c1 == c2) return (char *)s;
    return 0;
}

#endif

#if defined strcmp_c || defined SHARED_C_LIBRARY

__inline unsigned int submask(unsigned int w1, unsigned int w2, int shift)
{
    if (shift == 24)
        return (w1 >> 24) - (w2 >> 24);
#ifdef __thumb
    return ((w1 << (24-shift)) >> 24) - 
           ((w2 << (24-shift)) >> 24);
#else
    return (w1 & (255 << shift)) - (w2 & (255 << shift));
#endif
}

int strcmp(const char *a, const char *b) /* lexical comparison on strings */
{
#ifdef _copywords

    if ((((int)a | (int)b) & 3) == 0)
    {   
        unsigned int w1, w2, tmp, res;
        nullbyte_prologue_();

        do
        {
            w1 = *(int *)a, a += 4;
            w2 = *(int *)b, b += 4;
            tmp = word_has_nullbyte(w1);
        } while (tmp == 0 && w1 == w2);

#ifdef BYTESEX_EVEN
        res = submask(w1, w2, 0);
        if (res != 0 || (tmp & 0x80))
            return res;
        res = submask(w1, w2, 8);
        if (res != 0 || (tmp & 0x8000))
            return res;
        res = submask(w1, w2, 16);
        if (res != 0 || (tmp & 0x800000))
            return res;
        res = submask(w1, w2, 24);
#else
        res = submask(w1, w2, 24);
        if (res != 0 || (tmp & 0x80000000))
            return res;
        res = submask(w1, w2, 16);
        if (res != 0 || (tmp & 0x800000))
            return res;
        res = submask(w1, w2, 8);
        if (res != 0 || (tmp & 0x8000))
            return res;
        res = submask(w1, w2, 0);
#endif
        return res;
    }
    else
#endif
    {   /* byte-wise string copy, twice unrolled */
        unsigned int c1, c2;        
        for (;;)
        {   c1 = *a++;
            c2 = *b++;
            if (c1 < 1 || c1 != c2) break;      /* 2 cycle test */
            c1 = *a++;
            c2 = *b++;
            if (c1 < 1 || c1 != c2) break;      /* 2 cycle test */
        }
        return c1 - c2;
    }
}

#endif

#if defined strcoll_c || defined SHARED_C_LIBRARY

#include "interns.h"

static unsigned const char *strcoll_table = 0;

int strcoll(const char *a, const char *b)
{
    unsigned const char *s_t = strcoll_table;
    unsigned int c1, c2, t1, t2;
    if (s_t == 0) return strcmp(a, b);  /* C locale */
    do
    {   c1 = *a++;
        c2 = *b++;
        t1 = s_t[c1];
        t2 = s_t[c2];
    }
    while (c1 >= 1 && t1 == t2);
    return t1 - t2;
}

void __set_strcoll_table(const unsigned char *table)
{
    strcoll_table = table;
}

#endif

#if defined strcpy_c || defined SHARED_C_LIBRARY

char *strcpy(char *a, const char *b)                 /* copy from b to a */
{   char *p = a;
#ifdef _copywords
    if ((((int)p | (int)b) & 3) == 0)
    {   unsigned int w;
        nullbyte_prologue_();
        while (w = *(int *)b, b += 4, !word_has_nullbyte(w))
            *(int *)p = w, p += 4;
/* This next piece of code relies on knowledge of the order of bytes     */
/* within a word.                                                        */
#ifdef BYTESEX_EVEN
        for (;;)
        {   if ((*p++ = w) == 0) return a;
            w >>= 8;
        }
#else
        for (;;)
/* I rather assume that shifts are fast operations here.                 */
        {   if ((*p++ = (w >> 24)) == 0) return a;
            w <<= 8;
        }
#endif
    }
#endif
    do
        /* do nothing */ ;
    while ((*p++ = *b++) != 0 && (*p++ = *b++) != 0);
    return a;
}

#endif

#if defined strcspn_c || defined SHARED_C_LIBRARY

size_t strcspn(const char *s, const char *p)
                                     /* find first char in s that is in p */
{   const char *ss, *pp;
    char ch;
    for (ss=s;;ss++)
    {   char c1;
        if ((ch = *ss) == 0) return ss-s;
        for (pp=p; (c1 = *pp++) != 0; ) if (c1 == ch) return ss-s;
    }
}

#endif

#if defined strerror_c || defined SHARED_C_LIBRARY

#include "interns.h"

char *strerror(int n)
{   static char v[80];
    return _strerror(n, v);
}

#endif

#if defined strlen_c || defined SHARED_C_LIBRARY

size_t strlen(const char *a)            /* find number of chars in a string */
{   const char *x = a + 1;
#ifdef _copywords
    unsigned int w, len;
    while ((int)a & 3)
    {   if (*a++ == 0) return a - x;
    }
    {
        nullbyte_prologue_();
        while (w = *(int *)a, a += 4, !word_has_nullbyte(w)) /* do nothing */;
        /* a now points one word after the one containing
           the terminating null */
    }
    len = a - x;
#ifdef BYTESEX_EVEN
    if ((w & 0xff) == 0)
        len -= 3;
    else if ((w & 0xff00) == 0)
        len -= 2;
    else if ((w & 0xff0000) == 0)
        len -= 1;
#else
    if ((w & 0xff000000) == 0)
        len -= 3;
    else if ((w & 0xff0000) == 0)
        len -= 2;
    else if ((w & 0xff00) == 0)
        len -= 1;
#endif
    return len;
#else
    while (*a++ != 0);
    return a - x;
#endif
}

#endif

#if defined strncat_c || defined SHARED_C_LIBRARY

char *strncat(char *a, const char *b, size_t n)
                                       /* as strcat, but at most n chars */
{   char *p = a;
    while (*p != 0) p++;
    while (n > 0)
    {
        if ((*p++ = *b++) == 0) return a;
        n--;
    }
    *p = 0;
    return a;
}

#endif

#if defined strncmp_c || defined SHARED_C_LIBRARY

int strncmp(const char *a, const char * b, size_t n)
                                        /* as strcmp, but at most n chars */
{
    unsigned int c1, c2;
#ifdef _copywords
    if ((((int)a | (int)b) & 3) == 0)
    {   int w;
        nullbyte_prologue_();
        while (n >= 4 && (w = *(int *)a) == *(int *)b && !word_has_nullbyte(w))
            a += 4, b += 4, n -= 4;
    }
#endif
    if (n == 0) return 0;
    do
    {   c1 = *a++;
        c2 = *b++;
        if (c1 < 1 || c1 != c2)
            break;
    } while (--n > 0);
    return c1 - c2;
}

#endif

#if defined strncpy_c || defined SHARED_C_LIBRARY

char *strncpy(char *a, const char *b, size_t n)
            /* as strcpy, but at most n chars */
            /* NB may not be nul-terminated   */
{   char *p = a;
#ifdef _copywords
    if ((((int)p | (int)b) & 3) == 0)
    {   int w;
        nullbyte_prologue_();
        while (n >= 4 && (w = *(int *)b, !word_has_nullbyte(w)))
            *(int *)p = w, p += 4, b += 4, n -= 4;
    }
/* Although the above code has fetched the last part-filled word I will  */
/* copy the last few bytes by steam in this case. The test on n and the  */
/* need for padding seem to make anything else seem too messy.           */
#endif
    
    if (n > 0)
        do
            if ((*p++ = *b++) == 0) break;  /* n is one too large on end of string b */
        while (--n > 0);
    if (n <= 1) return a;
    n--;                            
    do
        *p++ = 0;                   /* ANSI says pad out with nul's */
    while (--n > 0);
    return a;
}

#endif

#if defined strpbrk_c || defined SHARED_C_LIBRARY

char *strpbrk(const char *s, const char *p)
                                        /*  ditto, except ptr/NULL result */
{   const char *ss, *pp;
    char ch;
    for (ss=s;;ss++)
    {   char c1;
        if ((ch = *ss) == 0) return 0;
        for (pp=p; (c1 = *pp++) != 0; ) if (c1 == ch) return (char *)ss;
    }
}

#endif

#if defined strrchr_c || defined SHARED_C_LIBRARY

char *strrchr(const char *s, _chararg ch)  /* find last instance of ch in s */
{   const char *p = s;
    char c1 = (char)ch;
    while (*p++ != 0) 
        /* nothing */;
    do { if (*--p == c1) return (char *)p; } while (p!=s);
    return 0;
}

#endif

#if defined strspn_c || defined SHARED_C_LIBRARY

size_t strspn(const char *s, const char *p)
                                        /* find first char in s not in p */
{   const char *ss, *pp;
    char ch;
    for (ss=s;;ss++)
    {   if ((ch = *ss) == 0) return ss-s;
        for (pp=p;;)
        {   char c1 = *pp++;
            if (c1 == 0) return ss-s;
            if (c1 == ch) break;
        }
    }
}

#endif

#if defined strstr_c || defined SHARED_C_LIBRARY

char *strstr(const char *a, const char *b)
                              /* find first occurrence of b in a, or NULL */
{   int i;
    for (;;)
    {   unsigned int ch1, ch2;
        for (i=0;; i++)
        {   ch1 = b[i];
            ch2 = a[i];
            if (ch1 < 1 || ch1 != ch2) break;
        }
        if (ch1 == 0) return (char *)a;
        if (*a++ == 0) return 0;
    }
}

#endif

#if defined strtok_c || defined SHARED_C_LIBRARY

static char *saves1 = NULL;

char *strtok(char *s1, const char *s2)
{   char *s0;
    if (s1 == 0) s1 = (saves1 == NULL) ? "" : saves1;    /* use saved pointer */
    if (*(s1 += strspn(s1,s2)) == 0) s0 = 0;             /* no tokens */
    else { s0 = s1;
           if (*(s1 += strcspn(s1,s2)) != 0) *s1++ = 0;  /* insert 0 if nec */
         }
    return (saves1 = s1, s0);
}

#endif

#if defined strxfrm_c || defined SHARED_C_LIBRARY

size_t strxfrm(char *s1, const char *s2, size_t n)
{   /* In C locale this should just *resemble* a less efficient strncpy().  */
    size_t r = strlen(s2);
    for (; n != 0; n--)
        if ((*s1++ = *s2++) == 0) break;
    return r;
}

#endif

#if defined _strerror_c || defined SHARED_C_LIBRARY

#include "interns.h"

#include <errno.h>
#include <stdio.h>

char *_strerror(int n, char *v)
{   
    switch (n)
    {   case 0:      return "No error (errno = 0)";
        case EDOM:   return "EDOM - function argument out of range";
        case ERANGE: return "ERANGE - function result not representable";
        case ESIGNUM:
            return "ESIGNUM - illegal signal number to signal() or raise()";
        default:     return _hostos_error_string(n, v);
    }
}

#endif

/* end string.c */
