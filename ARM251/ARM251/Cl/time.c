/* time.c: ANSI draft (X3J11 Oct 86) section 4.12 code  */
/* Copyright (C) Codemist Ltd, 1988                     */
/* Copyright (C) Advanced Risc Machines Ltd., 1991      */
/* version 0.02a */

/*
 * RCS $Revision: 1.3 $
 * Checkin $Date: 1998/05/13 14:29:14 $
 * Revising $Author: lsmith $
 */

#ifndef Y2KTEST
#  include <time.h>
#endif
#include <stdio.h>
#include <string.h>

extern const unsigned char _monlen[12];

#if defined _monlen_c || defined SHARED_C_LIBRARY

const unsigned char _monlen[12] = { 31,29,31,30,31,30,31,31,30,31,30,31 };

#endif

#if defined mktime_c || defined SHARED_C_LIBRARY

static int tm_carry(int *a, int b, int q)
{   /* *a = (*a + b) % q, return (*a + b)/q.  Care with overflow.          */
    int aa = *a;
    int hi = (aa >> 16) + (b >> 16);    /* NB signed shift arithmetic here */
    int lo = (aa & 0xffff) + (b & 0xffff);
    lo += (hi % q) << 16;
    hi = hi / q;
    aa = lo % q;
    lo = lo / q;
    while (aa < 0)
    {   aa += q;
        lo -= 1;
    }
    *a = aa;        /* remainder is positive here */
    return (hi << 16) + lo;
}

time_t mktime(struct tm *timeptr)
{   /* the Oct 1986 ANSI draft spec allows ANY values for the contents     */
    /* of timeptr.  This leave the question - what is month -9 or +123?    */
    /* the code below resolves it in one way:                              */
    /* Also note that struct tm is allowed to have signed values in it for */
    /* the purposes of this function even though normalized times all have */
    /* just positive entries.                                              */
    time_t t;
    int w, v, yday;
    int sec = timeptr->tm_sec;
    int min = timeptr->tm_min;
    int hour = timeptr->tm_hour;
    int mday = timeptr->tm_mday;
    int mon = timeptr->tm_mon;
    int year = timeptr->tm_year;
    int quadyear = 0;
/* The next line applies a simple test that detects some gross overflows */
    if (year > 0x40000000 || year < -0x40000000) return (time_t)-1;

    /* we really do have to propagate carries up it seems                  */
    /* careful about overflow for divide, but not carry add.               */

    w = tm_carry(&sec,0,60);    /* leaves 0 <= sec < 60  */
    w = tm_carry(&min,w,60);    /* leaves 0 <= min < 60  */
    w = tm_carry(&hour,w,24);   /* leaves 0 <= hour < 24 */
    quadyear = tm_carry(&mday,w - 1,(4*365+1));  /* 0 <= mday < 4 years    */
/* The next line can not possibly result in year overflowing since the     */
/* initial values was checked earlier and the month can only cause a       */
/* carry of size up to MAXINT/12 with quadyear limited to MAXINT/365.      */
    year += quadyear*4 + tm_carry(&mon,0,12);
    /* at last the mday is in 0..4*365 and the mon in 0..11                */

#define notleapyear(year) (((year) & 3)!=0)
/* Note that 1900 is not in the range of valid dates and so I will fudge   */
/* the issue about it not being a leap year.                               */

    while (mday >= _monlen[mon])
    {   mday -= _monlen[mon++];
        if (mon==2 && notleapyear(year)) mday++;
        else if (mon == 12) mon = 0, year++;
    }
    if (mon==1 && mday==28 && notleapyear(year)) mon++, mday=0;

#define YEARS (0x7fffffff/60/60/24/365 + 1)
    if (year < 70 || year > 70+2*YEARS) return (time_t)-1;

    yday = mday;
    {   int i;
        for (i = 0; i<mon; i++) yday += _monlen[i];
    }
    if (mon > 1 && notleapyear(year)) yday--;

    v = (365*4+1)*(year/4) + 365*(year & 3) + yday;
    if (!notleapyear(year)) v--;
/* v is now the number of days since 1 Jan 1900, and I have subtracted a   */
/* sly 1 to adjust for 1900 not being a leap year.                         */

/* Adjust for a base at 1 Jan 1970                                       */

#define DAYS (17*(365*4+1)+2*365)
    t = min + 60*(hour + 24*(v - DAYS));

    {   int thi = ((int)t >> 16)*60;
        int tlo = ((int)t & 0xffff)*60 + sec;
        thi += (tlo >> 16) & 0xffff;
        t = (time_t)((thi << 16) | (tlo & 0xffff));
        if ((thi & 0xffff0000) != 0) return (time_t)-1;
    }

    timeptr->tm_sec = sec;
    timeptr->tm_min = min;
    timeptr->tm_hour = hour;
    timeptr->tm_mday = mday + 1;
    timeptr->tm_mon = mon;
    timeptr->tm_year = year;
    timeptr->tm_wday = (v + 1) % 7;
    timeptr->tm_yday = yday;
    timeptr->tm_isdst = -1;                  /* unavailable */

    return t;
    /* Now I know why Unix didn't have this                              */
}

#endif

#if defined asctime_c || defined SHARED_C_LIBRARY

char *asctime(const struct tm *timeptr)
{   static char _timebuf[26+(8+3*9+7)];  /* slop in case illegal args */
    sprintf(_timebuf, "%.3s %.3s%3d %.2d:%.2d:%.2d %d\n",
       "SunMonTueWedThuFriSat" + (timeptr -> tm_wday)*3,
       "JanFebMarAprMayJunJulAugSepOctNovDecBad" + (timeptr -> tm_mon)*3,
       timeptr -> tm_mday,
       timeptr -> tm_hour, timeptr -> tm_min, timeptr -> tm_sec,
       timeptr -> tm_year + 1900);
    return _timebuf;
}

#endif

#if defined ctime_c || defined SHARED_C_LIBRARY

char *ctime(const time_t *timer)
{   return asctime(localtime(timer));
}

#endif

#if defined difftime_c || defined SHARED_C_LIBRARY

double difftime(time_t time1, time_t time0)
{   return (double)time1 - (double)time0;
}

#endif

#if defined gmtime_c || defined SHARED_C_LIBRARY

struct tm *gmtime(const time_t *timer)
/*                           ========== GMT not available ==========     */
{
    timer = timer;
    return 0;
}

#endif

#if defined localtime_c || defined SHARED_C_LIBRARY

struct tm *localtime(const time_t *timer)
{   time_t t = *timer;
    static struct tm _tms;
    int i = 0, yr;
/* treat unset dates as 1-Jan-1900 - any better ideas? */
    if (t == (time_t)-1) memset(&_tms, 0, sizeof(_tms)), _tms.tm_mday = 1;
    else
    {   /* unix time already in seconds (since 1-Jan-1970) ... */
        _tms.tm_sec = t % 60; t /= 60;
        _tms.tm_min = t % 60; t /= 60;
        _tms.tm_hour = t % 24; t /= 24;
/* The next line converts *timer arg into days since 1-Jan-1900 from t which
   now holds days since 1-Jan-1970.  Now there are really only 17 leap years
   in this range 04,08,...,68 but we use 18 so that we do not have to do
   special case code for 1900 which was not a leap year.  Of course this
   cannot give problems as pre-1970 times are not representable in *timer. */
        t += 70*365 + 18;
        _tms.tm_wday = t % 7;               /* it just happens to be so */
        yr = 4 * (t / (365*4+1)); t %= (365*4+1);
        if (t >= 366) yr += (t-1) / 365, t = (t-1) % 365;
        _tms.tm_year = yr;
        _tms.tm_yday = t;
        if ((yr & 3) != 0 && t >= 31+28) t++;
        while (t >= _monlen[i]) t -= _monlen[i++];
        _tms.tm_mday = t+1;
        _tms.tm_mon = i;
        _tms.tm_isdst = -1;                  /* unavailable */
    }
    return &_tms;
}

#endif

#if defined strftime_c || defined SHARED_C_LIBRARY

#define abbrweekday "Sun\0Mon\0Tue\0Wed\0Thu\0Fri\0Sat"

#define fullweekday "\
Sunday\0xxx\
Monday\0xxx\
Tuesday\0xx\
Wednesday\0\
Thursday\0x\
Friday\0xxx\
Saturday"

#define abbrmonth "Jan\0Feb\0Mar\0Apr\0May\0Jun\0Jul\0Aug\0Sep\0Oct\0Nov\0Dec"

#define fullmonth "\
January\0xx\
February\0x\
March\0xxxx\
April\0xxxx\
May\0xxxxxx\
June\0xxxxx\
July\0xxxxx\
August\0xxx\
September\0\
October\0xx\
November\0x\
December"

static int findweek(int yday, int startday, int today)
{
    int days_into_this_week = today - startday;
    int last_weekstart;
    if (days_into_this_week < 0) days_into_this_week += 7;
    last_weekstart = yday - days_into_this_week;
    if (last_weekstart <= 0) return 1;
    return last_weekstart/7 + 1;
}

size_t strftime(char *s, size_t maxsize, const char *fmt, const struct tm *tt)
{
    size_t p = 0;
    int c;
    char *ss, buff[24];
    if (maxsize==0) return 0;
#define push(ch) { s[p++]=(ch); if (p>=maxsize) return 0; }
    for (;;)
    {   switch (c = *fmt++)
        {
    case 0: s[p] = 0;
            return p;
    default:
            push(c);
            continue;
    case '%':
            ss = buff;
            switch (c = *fmt++)
            {
        default:            /* Unknown directive - leave uninterpreted   */
                push('%');  /* NB undefined behaviour according to ANSI  */
                fmt--;
                continue;
        case 'a':
                ss = abbrweekday + 4*tt->tm_wday;
                break;
        case 'A':
                ss = fullweekday + 10*tt->tm_wday;
                break;
        case 'b':
                ss = abbrmonth + 4*tt->tm_mon;
                break;
        case 'B':
                ss = fullmonth + 10*tt->tm_mon;
                break;
        case 'c':
/* Is this the locale-specific date & time format we want?               */
                sprintf(ss, "%02d %s %d %02d:%02d:%02d",
                    tt->tm_mday, abbrmonth+4*tt->tm_mon, 1900 + tt->tm_year,
                    tt->tm_hour, tt->tm_min, tt->tm_sec);
                break;
        case 'd':
                sprintf(ss, "%.2d", tt->tm_mday);
                break;
        case 'H':
                sprintf(ss, "%.2d", tt->tm_hour);
                break;
        case 'I':
                sprintf(ss, "%.2d", (tt->tm_hour + 11)%12 + 1);
                break;
        case 'j':
                sprintf(ss, "%.3d", tt->tm_yday + 1);
                break;
        case 'm':
                sprintf(ss, "%.2d", tt->tm_mon + 1);
                break;
        case 'M':
                sprintf(ss, "%.2d", tt->tm_min);
                break;
        case 'p':
/* I am worried here re 12.00 AM/PM and times near same.                 */
                ss = (tt->tm_hour < 12)  ? "AM" : "PM";
                break;
        case 'S':
                sprintf(ss, "%.2d", tt->tm_sec);
                break;
        case 'U':
                sprintf(ss, "%.2d", findweek(tt->tm_yday, 0, tt->tm_wday));
                break;
        case 'w':
                sprintf(ss, "%.1d", tt->tm_wday);
                break;
        case 'W':
                sprintf(ss, "%.2d", findweek(tt->tm_yday, 1, tt->tm_wday));
                break;
        case 'x':
/* The next two had better agree with %c conversions                     */
                sprintf(ss, "%02d %s %d",
                    tt->tm_mday, abbrmonth + 4*tt->tm_mon, 1900 + tt->tm_year);
                break;
        case 'X':
                sprintf(ss, "%02d:%02d:%02d",
                        tt->tm_hour, tt->tm_min, tt->tm_sec);
                break;
        case 'y':
                sprintf(ss, "%.2d", tt->tm_year % 100);
                break;
        case 'Y':
                sprintf(ss, "%d", 1900 + tt->tm_year);
                break;
        case 'Z':
                /* No timezone exists here */
                continue;
        case '%':
                push('%');
                continue;
            }
            while ((c = *ss++) != 0) push(c);
            continue;
        }
#undef push
    }
}

#endif

/* end of time.c */
