/****************************************************
 * debug.h - ARM Windows debugging                  *
 *==================================================*
 *                                                  *
 ****************************************************/

/*
 * RCS $Revision: 1.2 $
 * Checkin $Date: 1998/05/12 16:58:50 $
 * Revising $Author: dcleland $
 */

#ifdef __cplusplus
    extern "C" {
#endif

#define DllExport __declspec(dllexport)

typedef enum
{
    none    = 0,
    console,
    file
    /* network etc. */
} debug_output_type;

#ifdef __cplusplus
    }
#endif
