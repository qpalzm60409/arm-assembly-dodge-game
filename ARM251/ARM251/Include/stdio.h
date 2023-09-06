#pragma force_top_level
#pragma include_only_once

/* stdio.h: ANSI 'C' (X3J11 Oct 88) library header, section 4.9 */
/* Copyright (C) Codemist Ltd., 1988-1993                       */
/* Copyright (C) Advanced Risc Machines Ltd., 1991-1998 -       */
/* All rights reserved */

/*
 * RCS $Revision: 1.6 $
 * Checkin $Date: 1998/05/19 09:54:37 $
 * Revising $Author: wdijkstr $
 */

/*
 * stdio.h declares two types, several macros, and many functions for
 * performing input and output. For a discussion on Streams and Files
 * refer to sections 4.9.2 and 4.9.3 in the above ANSI draft, or to a
 * modern textbook on C.
 */

#ifndef __stdio_h
#define __stdio_h

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __size_t
#define __size_t 1
typedef unsigned int size_t;   /* from <stddef.h> */
#endif

/* ANSI forbids va_list to be defined here */
typedef char *__va_list[1];       /* keep in step with <stdarg.h> */

#ifndef NULL
#  define NULL 0                /* see <stddef.h> */
#endif

typedef struct __fpos_t_struct
{ unsigned long __lo;             /* add hi one day */
} fpos_t;
   /*
    * fpos_t is an object capable of recording all information needed to
    * specify uniquely every position within a file.
    */

typedef struct __FILE FILE;
   /*
    * FILE is an object capable of recording all information needed to control
    * a stream, such as its file position indicator, a pointer to its
    * associated buffer, an error indicator that records whether a read/write
    * error has occurred and an end-of-file indicator that records whether the
    * end-of-file has been reached.
    * Its structure is not made known to library clients.
    */

#define _IOFBF           0x100 /* fully buffered IO */
#define _IOLBF           0x200 /* line buffered IO */
#define _IONBF           0x400 /* unbuffered IO */

    /* Various default file IO buffer sizes */
#define BUFSIZ       (512)  /* system buffer size (as used by setbuf) */
#define STDIN_BUFSIZ  (64)  /* default stdin buffer size */
#define STDOUT_BUFSIZ (64)  /* default stdout buffer size */
#define STDERR_BUFSIZ (16)  /* default stderr buffer size */

#define EOF      (-1)
   /*
    * negative integral constant, indicates end-of-file, that is, no more input
    * from a stream.
    */
/* It is not clear to me what value FOPEN_MAX should have, so I will
   err in the cautious direction - ANSI requires it to be at least 8 */
#define FOPEN_MAX 8           /* check re arthur/unix/mvs */
   /*
    * an integral constant expression that is the minimum number of files that
    * this implementation guarantees can be open simultaneously.
    */
/* _SYS_OPEN defines a limit on the number of open files that is imposed
   by this C library */
#define _SYS_OPEN 16
#define FILENAME_MAX 80
   /*
    * an integral constant expression that is the size of an array of char
    * large enough to hold the longest filename string
    */
#define L_tmpnam FILENAME_MAX
   /*
    * an integral constant expression that is the size of an array of char
    * large enough to hold a temporary file name string generated by the
    * tmpnam function.
    */

#define SEEK_SET 0 /* start of stream (see fseek) */
#define SEEK_CUR 1 /* current position in stream (see fseek) */
#define SEEK_END 2 /* end of stream (see fseek) */

#define TMP_MAX 256
   /*
    * an integral constant expression that is the minimum number of unique
    * file names that shall be generated by the tmpnam function.
    */

extern FILE __stdin, __stdout, __stderr;

#define stdin  (&__stdin)
   /* pointer to a FILE object associated with standard input stream */
#define stdout (&__stdout)
   /* pointer to a FILE object associated with standard output stream */
#define stderr (&__stderr)
   /* pointer to a FILE object associated with standard error stream */

extern int remove(const char * /*filename*/);
   /*
    * causes the file whose name is the string pointed to by filename to be
    * removed. Subsequent attempts to open the file will fail, unless it is
    * created anew. If the file is open, the behaviour of the remove function
    * is implementation-defined (under RISCOS/Arthur/Brazil the operation
    * fails).
    * Returns: zero if the operation succeeds, nonzero if it fails.
    */
extern int rename(const char * /*old*/, const char * /*new*/);
   /*
    * causes the file whose name is the string pointed to by old to be
    * henceforth known by the name given by the string pointed to by new. The
    * file named old is effectively removed. If a file named by the string
    * pointed to by new exists prior to the call of the rename function, the
    * behaviour is implementation-defined (under RISCOS/Arthur/Brazil, the
    * operation fails).
    * Returns: zero if the operation succeeds, nonzero if it fails, in which
    *          case if the file existed previously it is still known by its
    *          original name.
    */
extern FILE *tmpfile(void);
   /*
    * creates a temporary binary file that will be automatically removed when
    * it is closed or at program termination. The file is opened for update.
    * Returns: a pointer to the stream of the file that it created. If the file
    *          cannot be created, a null pointer is returned.
    */
extern char *tmpnam(char * /*s*/);
   /*
    * generates a string that is not the same as the name of an existing file.
    * The tmpnam function generates a different string each time it is called,
    * up to TMP_MAX times. If it is called more than TMP_MAX times, the
    * behaviour is implementation-defined (under RISCOS/Arthur/Brazil the
    * algorithm for the name generation works just as well after tmpnam has
    * been called more than TMP_MAX times as before; a name clash is impossible
    * in any single half year period).
    * Returns: If the argument is a null pointer, the tmpnam function leaves
    *          its result in an internal static object and returns a pointer to
    *          that object. Subsequent calls to the tmpnam function may modify
    *          the same object. if the argument is not a null pointer, it is
    *          assumed to point to an array of at least L_tmpnam characters;
    *          the tmpnam function writes its result in that array and returns
    *          the argument as its value.
    */

extern int fclose(FILE * /*stream*/);
   /*
    * causes the stream pointed to by stream to be flushed and the associated
    * file to be closed. Any unwritten buffered data for the stream are
    * delivered to the host environment to be written to the file; any unread
    * buffered data are discarded. The stream is disassociated from the file.
    * If the associated buffer was automatically allocated, it is deallocated.
    * Returns: zero if the stream was succesfully closed, or nonzero if any
    *          errors were detected or if the stream was already closed.
    */
extern int fflush(FILE * /*stream*/);
   /*
    * If the stream points to an output or update stream in which the most
    * recent operation was output, the fflush function causes any unwritten
    * data for that stream to be delivered to the host environment to be
    * written to the file. If the stream points to an input or update stream,
    * the fflush function undoes the effect of any preceding ungetc operation
    * on the stream.
    * Returns: nonzero if a write error occurs.
    */
extern FILE *fopen(const char * /*filename*/, const char * /*mode*/);
   /*
    * opens the file whose name is the string pointed to by filename, and
    * associates a stream with it.
    * The argument mode points to a string beginning with one of the following
    * sequences:
    * "r"         open text file for reading
    * "w"         create text file for writing, or truncate to zero length
    * "a"         append; open text file or create for writing at eof
    * "rb"        open binary file for reading
    * "wb"        create binary file for writing, or truncate to zero length
    * "ab"        append; open binary file or create for writing at eof
    * "r+"        open text file for update (reading and writing)
    * "w+"        create text file for update, or truncate to zero length
    * "a+"        append; open text file or create for update, writing at eof
    * "r+b"/"rb+" open binary file for update (reading and writing)
    * "w+b"/"wb+" create binary file for update, or truncate to zero length
    * "a+b"/"ab+" append; open binary file or create for update, writing at eof
    *
    * Opening a file with read mode ('r' as the first character in the mode
    * argument) fails if the file does not exist or cannot be read.
    * Opening a file with append mode ('a' as the first character in the mode
    * argument) causes all subsequent writes to be forced to the current end of
    * file, regardless of intervening calls to the fseek function. In some
    * implementations, opening a binary file with append mode ('b' as the
    * second or third character in the mode argument) may initially position
    * the file position indicator beyond the last data written, because of the
    * NUL padding (but not under RISCOS/Arthur/Brazil).
    * When a file is opened with update mode ('+' as the second or third
    * character in the mode argument), both input and output may be performed
    * on the associated stream. However, output may not be directly followed by
    * input without an intervening call to the fflush fuction or to a file
    * positioning function (fseek, fsetpos, or rewind), and input be not be
    * directly followed by output without an intervening call to the fflush
    * fuction or to a file positioning function, unless the input operation
    * encounters end-of-file. Opening a file with update mode may open or
    * create a binary stream in some implementations (but not under RISCOS/
    * Arthur/Brazil). When opened, a stream is fully buffered if and only if
    * it does not refer to an interactive device. The error and end-of-file
    * indicators for the stream are cleared.
    * Returns: a pointer to the object controlling the stream. If the open
    *          operation fails, fopen returns a null pointer.
    */
extern FILE *freopen(const char * /*filename*/, const char * /*mode*/,
                     FILE * /*stream*/);
   /*
    * opens the file whose name is the string pointed to by filename and
    * associates the stream pointed to by stream with it. The mode argument is
    * used just as in the fopen function.
    * The freopen function first attempts to close any file that is associated
    * with the specified stream. Failure to close the file successfully is
    * ignored. The error and end-of-file indicators for the stream are cleared.
    * Returns: a null pointer if the operation fails. Otherwise, freopen
    *          returns the value of the stream.
    */
extern void setbuf(FILE * /*stream*/, char * /*buf*/);
   /*
    * Except that it returns no value, the setbuf function is equivalent to the
    * setvbuf function invoked with the values _IOFBF for mode and BUFSIZ for
    * size, or (if buf is a null pointer), with the value _IONBF for mode.
    * Returns: no value.
    */
extern int setvbuf(FILE * /*stream*/, char * /*buf*/,
                   int /*mode*/, size_t /*size*/);
   /*
    * may be used after the stream pointed to by stream has been associated
    * with an open file but before it is read or written. The argument mode
    * determines how stream will be buffered, as follows: _IOFBF causes
    * input/output to be fully buffered; _IOLBF causes output to be line
    * buffered (the buffer will be flushed when a new-line character is
    * written, when the buffer is full, or when input is requested); _IONBF
    * causes input/output to be completely unbuffered. If buf is not the null
    * pointer, the array it points to may be used instead of an automatically
    * allocated buffer (the buffer must have a lifetime at least as great as
    * the open stream, so the stream should be closed before a buffer that has
    * automatic storage duration is deallocated upon block exit). The argument
    * size specifies the size of the array. The contents of the array at any
    * time are indeterminate.
    * Returns: zero on success, or nonzero if an invalid value is given for
    *          mode or size, or if the request cannot be honoured.
    */

#pragma -v1   /* hint to the compiler to check f/s/printf format */
extern int fprintf(FILE * /*stream*/, const char * /*format*/, ...);
   /*
    * writes output to the stream pointed to by stream, under control of the
    * string pointed to by format that specifies how subsequent arguments are
    * converted for output. If there are insufficient arguments for the format,
    * the behaviour is undefined. If the format is exhausted while arguments
    * remain, the excess arguments are evaluated but otherwise ignored. The
    * fprintf function returns when the end of the format string is reached.
    * The format shall be a multibyte character sequence, beginning and ending
    * in its initial shift state. The format is composed of zero or more
    * directives: ordinary multibyte characters (not %), which are copied
    * unchanged to the output stream; and conversion specifiers, each of which
    * results in fetching zero or more subsequent arguments. Each conversion
    * specification is introduced by the character %. For a description of the
    * available conversion specifiers refer to section 4.9.6.1 in the ANSI
    * draft mentioned at the start of this file or to any modern textbook on C.
    * The minimum value for the maximum number of characters producable by any
    * single conversion is at least 509.
    * Returns: the number of characters transmitted, or a negative value if an
    *          output error occurred.
    */
extern int printf(const char * /*format*/, ...);
   /*
    * is equivalent to fprintf with the argument stdout interposed before the
    * arguments to printf.
    * Returns: the number of characters transmitted, or a negative value if an
    *          output error occurred.
    */
extern int sprintf(char * /*s*/, const char * /*format*/, ...);
   /*
    * is equivalent to fprintf, except that the argument s specifies an array
    * into which the generated output is to be written, rather than to a
    * stream. A null character is written at the end of the characters written;
    * it is not counted as part of the returned sum.
    * Returns: the number of characters written to the array, not counting the
    *          terminating null character.
    */
#pragma -v2   /* hint to the compiler to check f/s/scanf format */
extern int fscanf(FILE * /*stream*/, const char * /*format*/, ...);
   /*
    * reads input from the stream pointed to by stream, under control of the
    * string pointed to by format that specifies the admissible input sequences
    * and how thay are to be converted for assignment, using subsequent
    * arguments as pointers to the objects to receive the converted input. If
    * there are insufficient arguments for the format, the behaviour is
    * undefined. If the format is exhausted while arguments remain, the excess
    * arguments are evaluated but otherwise ignored.
    * The format is composed of zero or more directives: one or more
    * white-space characters; an ordinary character (not %); or a conversion
    * specification. Each conversion specification is introduced by the
    * character %. For a description of the available conversion specifiers
    * refer to section 4.9.6.2 in the ANSI draft mentioned at the start of this
    * file, or to any modern textbook on C.
    * If end-of-file is encountered during input, conversion is terminated. If
    * end-of-file occurs before any characters matching the current directive
    * have been read (other than leading white space, where permitted),
    * execution of the current directive terminates with an input failure;
    * otherwise, unless execution of the current directive is terminated with a
    * matching failure, execution of the following directive (if any) is
    * terminated with an input failure.
    * If conversions terminates on a conflicting input character, the offending
    * input character is left unread in the input strem. Trailing white space
    * (including new-line characters) is left unread unless matched by a
    * directive. The success of literal matches and suppressed asignments is
    * not directly determinable other than via the %n directive.
    * Returns: the value of the macro EOF if an input failure occurs before any
    *          conversion. Otherwise, the fscanf function returns the number of
    *          input items assigned, which can be fewer than provided for, or
    *          even zero, in the event of an early conflict between an input
    *          character and the format.
    */
extern int scanf(const char * /*format*/, ...);
   /*
    * is equivalent to fscanf with the argument stdin interposed before the
    * arguments to scanf.
    * Returns: the value of the macro EOF if an input failure occurs before any
    *          conversion. Otherwise, the scanf function returns the number of
    *          input items assigned, which can be fewer than provided for, or
    *          even zero, in the event of an early matching failure.
    */
extern int sscanf(const char * /*s*/, const char * /*format*/, ...);
   /*
    * is equivalent to fscanf except that the argument s specifies a string
    * from which the input is to be obtained, rather than from a stream.
    * Reaching the end of the string is equivalent to encountering end-of-file
    * for the fscanf function.
    * Returns: the value of the macro EOF if an input failure occurs before any
    *          conversion. Otherwise, the scanf function returns the number of
    *          input items assigned, which can be fewer than provided for, or
    *          even zero, in the event of an early matching failure.
    */
#pragma -v0   /* back to default */
extern int vprintf(const char * /*format*/, __va_list /*arg*/);
   /*
    * is equivalent to printf, with the variable argument list replaced by arg,
    * which has been initialised by the va_start macro (and possibly subsequent
    * va_arg calls). The vprintf function does not invoke the va_end function.
    * Returns: the number of characters transmitted, or a negative value if an
    *          output error occurred.
    */
extern int vfprintf(FILE * /*stream*/,
                   const char * /*format*/, __va_list /*arg*/);
   /*
    * is equivalent to fprintf, with the variable argument list replaced by
    * arg, which has been initialised by the va_start macro (and possibly
    * subsequent va_arg calls). The vfprintf function does not invoke the
    * va_end function.
    * Returns: the number of characters transmitted, or a negative value if an
    *          output error occurred.
    */
extern int vsprintf(char * /*s*/, const char * /*format*/, __va_list /*arg*/);
   /*
    * is equivalent to sprintf, with the variable argument list replaced by
    * arg, which has been initialised by the va_start macro (and possibly
    * subsequent va_arg calls). The vsprintf function does not invoke the
    * va_end function.
    * Returns: the number of characters written in the array, not counting the
    *          terminating null character.
    */

extern int fgetc(FILE * /*stream*/);
   /*
    * obtains the next character (if present) as an unsigned char converted to
    * an int, from the input stream pointed to by stream, and advances the
    * associated file position indicator (if defined).
    * Returns: the next character from the input stream pointed to by stream.
    *          If the stream is at end-of-file, the end-of-file indicator is
    *          set and fgetc returns EOF. If a read error occurs, the error
    *          indicator is set and fgetc returns EOF.
    */
extern char *fgets(char * /*s*/, int /*n*/, FILE * /*stream*/);
   /*
    * reads at most one less than the number of characters specified by n from
    * the stream pointed to by stream into the array pointed to by s. No
    * additional characters are read after a new-line character (which is
    * retained) or after end-of-file. A null character is written immediately
    * after the last character read into the array.
    * Returns: s if successful. If end-of-file is encountered and no characters
    *          have been read into the array, the contents of the array remain
    *          unchanged and a null pointer is returned. If a read error occurs
    *          during the operation, the array contents are indeterminate and a
    *          null pointer is returned.
    */
extern int fputc(int /*c*/, FILE * /*stream*/);
   /*
    * writes the character specified by c (converted to an unsigned char) to
    * the output stream pointed to by stream, at the position indicated by the
    * asociated file position indicator (if defined), and advances the
    * indicator appropriately. If the file position indicator is not defined,
    * the character is appended to the output stream.
    * Returns: the character written. If a write error occurs, the error
    *          indicator is set and fputc returns EOF.
    */
extern int fputs(const char * /*s*/, FILE * /*stream*/);
   /*
    * writes the string pointed to by s to the stream pointed to by stream.
    * The terminating null character is not written.
    * Returns: EOF if a write error occurs; otherwise it returns a nonnegative
    *          value.
    */
extern int getc(FILE * /*stream*/);
   /*
    * is equivalent to fgetc except that it may be implemented as an unsafe
    * macro (stream may be evaluated more than once, so the argument should
    * never be an expression with side-effects).
    * Returns: the next character from the input stream pointed to by stream.
    *          If the stream is at end-of-file, the end-of-file indicator is
    *          set and getc returns EOF. If a read error occurs, the error
    *          indicator is set and getc returns EOF.
    */
#define getchar() getc(stdin)
extern int (getchar)(void);
   /*
    * is equivalent to getc with the argument stdin.
    * Returns: the next character from the input stream pointed to by stdin.
    *          If the stream is at end-of-file, the end-of-file indicator is
    *          set and getchar returns EOF. If a read error occurs, the error
    *          indicator is set and getchar returns EOF.
    */
extern char *gets(char * /*s*/);
   /*
    * reads characters from the input stream pointed to by stdin into the array
    * pointed to by s, until end-of-file is encountered or a new-line character
    * is read. Any new-line character is discarded, and a null character is
    * written immediately after the last character read into the array.
    * Returns: s if successful. If end-of-file is encountered and no characters
    *          have been read into the array, the contents of the array remain
    *          unchanged and a null pointer is returned. If a read error occurs
    *          during the operation, the array contents are indeterminate and a
    *          null pointer is returned.
    */
extern int putc(int /*c*/, FILE * /*stream*/);
   /*
    * is equivalent to fputc except that it may be implemented as aan unsafe
    * macro (stream may be evaluated more than once, so the argument should
    * never be an expression with side-effects).
    * Returns: the character written. If a write error occurs, the error
    *          indicator is set and putc returns EOF.
    */
#define putchar(ch) putc(ch, stdout)
extern int (putchar)(int /*c*/);
   /*
    * is equivalent to putc with the second argument stdout.
    * Returns: the character written. If a write error occurs, the error
    *          indicator is set and putc returns EOF.
    */
extern int puts(const char * /*s*/);
   /*
    * writes the string pointed to by s to the stream pointed to by stdout, and
    * appends a new-line character to the output. The terminating null
    * character is not written.
    * Returns: EOF if a write error occurs; otherwise it returns a nonnegative
    *          value.
    */
extern int ungetc(int /*c*/, FILE * /*stream*/);
   /*
    * pushes the character specified by c (converted to an unsigned char) back
    * onto the input stream pointed to by stream. The character will be
    * returned by the next read on that stream. An intervening call to the
    * fflush function or to a file positioning function (fseek, fsetpos,
    * rewind) discards any pushed-back characters. The external storage
    * corresponding to the stream is unchanged.
    * One character pushback is guaranteed. If the unget function is called too
    * many times on the same stream without an intervening read or file
    * positioning operation on that stream, the operation may fail.
    * If the value of c equals that of the macro EOF, the operation fails and
    * the input stream is unchanged.
    * A successful call to the ungetc function clears the end-of-file
    * indicator. The value of the file position indicator after reading or
    * discarding all pushed-back characters shall be the same as it was before
    * the characters were pushed back. For a text stream, the value of the file
    * position indicator after a successful call to the ungetc function is
    * unspecified until all pushed-back characters are read or discarded. For a
    * binary stream, the file position indicator is decremented by each
    * successful call to the ungetc function; if its value was zero before a
    * call, it is indeterminate after the call.
    * Returns: the character pushed back after conversion, or EOF if the
    *          operation fails.
    */

extern size_t fread(void * /*ptr*/,
                    size_t /*size*/, size_t /*nmemb*/, FILE * /*stream*/);
   /*
    * reads into the array pointed to by ptr, up to nmemb members whose size is
    * specified by size, from the stream pointed to by stream. The file
    * position indicator (if defined) is advanced by the number of characters
    * successfully read. If an error occurs, the resulting value of the file
    * position indicator is indeterminate. If a partial member is read, its
    * value is indeterminate. The ferror or feof function shall be used to
    * distinguish between a read error and end-of-file.
    * Returns: the number of members successfully read, which may be less than
    *          nmemb if a read error or end-of-file is encountered. If size or
    *          nmemb is zero, fread returns zero and the contents of the array
    *          and the state of the stream remain unchanged.
    */
extern size_t fwrite(const void * /*ptr*/,
                    size_t /*size*/, size_t /*nmemb*/, FILE * /*stream*/);
   /*
    * writes, from the array pointed to by ptr up to nmemb members whose size
    * is specified by size, to the stream pointed to by stream. The file
    * position indicator (if defined) is advanced by the number of characters
    * successfully written. If an error occurs, the resulting value of the file
    * position indicator is indeterminate.
    * Returns: the number of members successfully written, which will be less
    *          than nmemb only if a write error is encountered.
    */

extern int fgetpos(FILE * /*stream*/, fpos_t * /*pos*/);
   /*
    * stores the current value of the file position indicator for the stream
    * pointed to by stream in the object pointed to by pos. The value stored
    * contains unspecified information usable by the fsetpos function for
    * repositioning the stream to its position at the time  of the call to the
    * fgetpos function.
    * Returns: zero, if successful. Otherwise nonzero is returned and the
    *          integer expression errno is set to an implementation-defined
    *          nonzero value (under RISCOS/Arthur/Brazil fgetpos cannot fail).
    */
extern int fseek(FILE * /*stream*/, long int /*offset*/, int /*whence*/);
   /*
    * sets the file position indicator for the stream pointed to by stream.
    * For a binary stream, the new position is at the signed number of
    * characters specified by offset away from the point specified by whence.
    * The specified point is the beginning of the file for SEEK_SET, the
    * current position in the file for SEEK_CUR, or end-of-file for SEEK_END.
    * A binary stream need not meaningfully support fseek calls with a whence
    * value of SEEK_END.
    * For a text stream, either offset shall be zero, or offset shall be a
    * value returned by an earlier call to the ftell function on the same
    * stream and whence shall be SEEK_SET.
    * The fseek function clears the end-of-file indicator and undoes any
    * effects of the ungetc function on the same stream. After an fseek call,
    * the next operation on an update stream may be either input or output.
    * Returns: nonzero only for a request that cannot be satisfied.
    */
extern int fsetpos(FILE * /*stream*/, const fpos_t * /*pos*/);
   /*
    * sets  the file position indicator for the stream pointed to by stream
    * according to the value of the object pointed to by pos, which shall be a
    * value returned by an earlier call to the fgetpos function on the same
    * stream.
    * The fsetpos function clears the end-of-file indicator and undoes any
    * effects of the ungetc function on the same stream. After an fsetpos call,
    * the next operation on an update stream may be either input or output.
    * Returns: zero, if successful. Otherwise nonzero is returned and the
    *          integer expression errno is set to an implementation-defined
    *          nonzero value (under RISCOS/Arthur/Brazil the value that of EDOM
    *          in math.h).
    */
extern long int ftell(FILE * /*stream*/);
   /*
    * obtains the current value of the file position indicator for the stream
    * pointed to by stream. For a binary stream, the value is the number of
    * characters from the beginning of the file. For a text stream, the file
    * position indicator contains unspecified information, usable by the fseek
    * function for returning the file position indicator to its position at the
    * time of the ftell call; the difference between two such return values is
    * not necessarily a meaningful measure of the number of characters written
    * or read.
    * Returns: if successful, the current value of the file position indicator.
    *          On failure, the ftell function returns -1L and sets the integer
    *          expression errno to an implementation-defined nonzero value
    *          (under RISCOS/Arthur/Brazil ftell cannot fail).
    */
extern void rewind(FILE * /*stream*/);
   /*
    * sets the file position indicator for the stream pointed to by stream to
    * the beginning of the file. It is equivalent to
    *          (void)fseek(stream, 0L, SEEK_SET)
    * except that the error indicator for the stream is also cleared.
    * Returns: no value.
    */

extern void clearerr(FILE * /*stream*/);
   /*
    * clears the end-of-file and error indicators for the stream pointed to by
    * stream. These indicators are cleared only when the file is opened or by
    * an explicit call to the clearerr function or to the rewind function.
    * Returns: no value.
    */

extern int feof(FILE * /*stream*/);
   /*
    * tests the end-of-file indicator for the stream pointed to by stream.
    * Returns: nonzero iff the end-of-file indicator is set for stream.
    */
extern int ferror(FILE * /*stream*/);
   /*
    * tests the error indicator for the stream pointed to by stream.
    * Returns: nonzero iff the error indicator is set for stream.
    */
extern void perror(const char * /*s*/);
   /*
    * maps the error number  in the integer expression errno to an error
    * message. It writes a sequence of characters to the standard error stream
    * thus: first (if s is not a null pointer and the character pointed to by
    * s is not the null character), the string pointed to by s followed by a
    * colon and a space; then an appropriate error message string followed by
    * a new-line character. The contents of the error message strings are the
    * same as those returned by the strerror function with argument errno,
    * which are implementation-defined.
    * Returns: no value.
    */

#ifdef __cplusplus
}
#endif

#endif

/* end of stdio.h */
