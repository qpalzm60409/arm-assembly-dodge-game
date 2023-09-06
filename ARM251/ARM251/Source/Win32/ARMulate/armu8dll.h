/* RCS $Revision: 1.7 $
 * Checkin $Date: 1998/04/07 13:46:40 $
 * Revising $Author: mwilliam $
 */

#define BUFSIZE 80
#include <setjmp.h>
#include "rdi.h"

#ifdef __WATCOMC__
#define DLL_EXPORT __export
#elif _MSC_VER
#define DLL_EXPORT _declspec(dllexport)
#endif

#ifdef __cplusplus
extern "C"
{
#endif
#define NOT_IMPLEMENTED "__NotImplemented"
/******************************************************************************\
*                               GLOBAL VARIABLES
\******************************************************************************/
HANDLE ghArmulateMod;

/******************************************************************************\
*                              FUNCTION PROTOTYPES
\******************************************************************************/

#ifdef __cplusplus
}
#endif
