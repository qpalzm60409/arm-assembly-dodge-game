/* 
 * ARM RDI - winrdi.h
 * Definition of the interface between the Windows Debugger and a debugger DLL
 * Copyright (C) 1996-1998, Advanced RISC Machines Limited. All Rights Reserved
 */

/*
 * RCS $Revision: 1.7.6.2 $
 * Checkin $Date: 1999/03/25 14:23:18 $
 * Revising $Author: jconnell $
 */

#ifndef winrdi_h
#define winrdi_h

#if RDI_VERSION == 150
#include "toolconf.h"
#endif

enum {
  WinRDI100_Version = 1,
  WinRDI150_Version = 150
};

#if RDI_VERSION == 150
#define WinRDI_Version WinRDI150_Version
#else
#define WinRDI_Version WinRDI100_Version
#endif

/* Get the address of a WinRDI_ function, using "GetProcAddress". This
 * header defines the required strings for this to work.
 * Paramters:  handle - Windows handle onto the DLL
 *             name - name of the WinRDI function (e.g. "Valid_RDI_DLL")
 * Returns:    address of the function, cast to the appropriate type
 */
/* macros to return the addresses of standard functions using GetProcAddress */
#define WinRDI_GetProcAddress(handle,name) \
    ( (WinRDI_func##name)GetProcAddress(handle, WinRDI_str##name) )


/*
 * The following functions are those defined by an RDI DLL. Each function
 * is either REQUIRED---it must be defined by a conforming DLL---or
 * OPTIONAL---a conforming Debug Controller must be able to cope with a
 * DLL not defining that function.
 */

/*
 * MJW: @@@ would like to get rid of these names, and make style-guide
 * compliant:
 *  typedef int WinRDI_InitialiseProc(...);
 * etc.
 */


/* INITIALIZATION AND VALIDATION */

/* Identify that the DLL is an RDI Debug Target, with WinRDI extensions.
 * Returns:    TRUE if this DLL exports some version of RDI and WinRDI
 *             FALSE otherwise
 * Status:     REQUIRED
 */
typedef BOOL (WINAPI *WinRDI_funcValid_RDI_DLL)(void);

extern BOOL WINAPI WinRDI_Valid_RDI_DLL(void);

#define WinRDI_strValid_RDI_DLL "WinRDI_Valid_RDI_DLL"


/* Specify the RDI and WinRDI version which is supported by the DLL. Should
 * return WinRDI_Version, as defined by this header.
 * Returns:    WinRDI_Version
 * Status:     REQUIRED
 */
typedef int (WINAPI *WinRDI_funcGetVersion)(void);

extern int WINAPI WinRDI_GetVersion(void);

#define WinRDI_strGetVersion "WinRDI_GetVersion"        


/* Return a nul-terminated ASCII string, descibing the DLL; for display to
 * the user.
 * Returns:    A static message describing the DLL. The string should be
 *             limited to 200 characters in length. This allows the Debug
 *             Controller to estimate the amount of window space required to
 *             display the string.
 * Status:     REQUIRED
 */
typedef char *(WINAPI *WinRDI_funcGet_DLL_Description)(void);

extern char *WINAPI WinRDI_Get_DLL_Description(void);

#define WinRDI_strGet_DLL_Description "WinRDI_Get_DLL_Description"


/* Return a pointer to the RDI_ProcVec exported by this DLL. Note that the
 * Debug Controller should call WinRDI_GetVersion to assuming the form of
 * this structure.
 * Returns:    Pointer to the exported RDI interface.
 * Status:     REQUIRED
 */
typedef RDI_ProcVec *(WINAPI *WinRDI_funcGetRDIProcVec)(void);

extern RDI_ProcVec *WINAPI WinRDI_GetRDIProcVec(void);

#define WinRDI_strGetRDIProcVec "WinRDI_GetRDIProcVec"


/* Called by the Debug Controller before any RDI_ functions are called. The
 * DLL performs any initialization required. After this call, the DLL is
 * ready to accept RDI_OpenAgent. If WinRDI_Initialise is exported, the DLL
 * must be ready to accept RDI_OpenAgent at any time.
 * Parameters: hParent - Handle to the parent window. Used in the case of the
 *                 DLL creating user-interface components, such as dialog
 *                 boxes.
 *             config - Device specific configuration information
 * Returns:    TRUE if initialization was successful
 *             FALSE if initialization failed. There is no method for
 *                 returning extended error information. If initialization
 *                 fails, the Debug Target displays a dialog box or other
 *                 user-interface component, informing the user of the
 *                 reason for the failure.
 * Status:     OPTIONAL
 */
typedef int (WINAPI *WinRDI_funcInitialise)(
    HWND hParent, RDI_ConfigPointer config);

extern int WINAPI WinRDI_Initialise(HWND hParent, RDI_ConfigPointer conf);

#define WinRDI_strInitialise "WinRDI_Initialise"


/* Called when initialized from a console application, rather than a
 * windows environment.
 * Parameters: drivername - Specifies connection. (ethernet, serial or
 *                 parallel for Angel.)
 *             args - String of command-line arguments
 *             heartbeat - (Angel only) TRUE - Angel heartbeat on
 *                                      FALSE - Angel heartbeat off
 *                 may be ignored by other Targets
 * Returns:    TRUE if initialization was successful
 *             FALSE if initialization failed
 * Status:     OPTIONAL
 */
typedef int (WINAPI *WinRDI_funcConsoleInitialise)(
    char *drivername, char *args, int heartbeat);

extern int WINAPI WinRDI_ConsoleInitialise(
    char *drivername, char *args, int heartbeat);

#define WinRDI_strConsoleInitialise "WinRDI_ConsoleInitialise"



/* CONFIGURATION */

/* Called by the Debug Controller to provide the user with a dialog box to
 * configure the Target. The Target brings up a modal dialog box attached
 * to the provided window handle, and returns control to the Debug Controller
 * when the dialog is closed.
 * Parameters: hParent - Handle to the parent window. The dialog box is
 *                 created as a child of this window
 *             config - Device specific configuration information
 * Returns:    TRUE if the user clicked "Okay" in the dialog box
 *             FALSE if the user clicked "Cancel" in the dialog box
 * Status:     OPTIONAL
 */
typedef BOOL (WINAPI *WinRDI_funcConfig)(
    RDI_ConfigPointer config, HWND hParent);

extern BOOL WINAPI WinRDI_Config(RDI_ConfigPointer config, HWND hParent);

#define WinRDI_strConfig "WinRDI_Config"



/* YIELDING CONTROL */

/* Supply a callback to yield control to other processes during Target
 * execution. The callback is passed as a function/argument closure.
 */
typedef struct WinRDI_OpaqueYieldArgStr WinRDI_YieldArg;
typedef void WinRDI_YieldProc(WinRDI_YieldArg *);

/* Parameters: yieldcallback - Function to be called every-so-often during
 *                 Target execution (i.e. RDI_ExecuteProc). The calling
 *             hyieldcallback - Argument to be passed to yieldcallback
 * Status:     OPTIONAL
 */
typedef void (WINAPI *WinRDI_funcRegister_Yield_Callback)(
     WinRDI_YieldArg *yieldcallback, WinRDI_YieldArg *hyieldcallback);

extern void WINAPI WinRDI_Register_Yield_Callback(
    WinRDI_YieldProc *yieldcallback, WinRDI_YieldArg *hyieldcallback);

#define WinRDI_strRegister_Yield_Callback "WinRDI_Register_Yield_Callback"



/* PROGRESS INDICATORS */

/* Register a callback for the Debug Target to call periodically during
 * download. The callback function receives data (counters) indicating the
 * progress made so far, and typically displays this in a user-interface
 * component.
 */
/* The progress indicator callback is passed an opaque handle */
typedef struct WinRDI_OpaqueProgressArgStr WinRDI_ProgressArg;
typedef struct {
    WinRDI_ProgressArg *handle; /* handle passed to SetProgressFunc */
    int nRead;     /* number of bytes read from channel */
    int nWritten;  /* number of bytes written           */
} WinRDI_ProgressInfo;

typedef void (WINAPI *WinRDI_funcProgressFunc)(WinRDI_ProgressInfo *info);

/* Parameters: func - Function for the Debug Target to call periodically
 *                 during download
 *             handle - Argument to fill the 'handle' field of the
 *                 WinRDI_ProgressInfo structure passed to the callback
 * Status:     OPTIONAL
 */
typedef void (WINAPI *WinRDI_funcSetProgressFunc)(
    WinRDI_funcProgressFunc func, WinRDI_ProgressArg *handle);

extern void WINAPI WinRDI_SetProgressFunc(
    WinRDI_funcProgressFunc func, WinRDI_ProgressArg *handle);

#define WinRDI_strSetProgressFunc "WinRDI_SetProgressFunc"


/* Zero the counters
 * Status:     OPTIONAL
 */
typedef void (WINAPI *WinRDI_funcZeroProgressValues)(void);

extern void WINAPI WinRDI_ZeroProgressValues(void);

#define WinRDI_strZeroProgressValues "WinRDI_ZeroProgressValues"



/* EXTERNAL DOWNLOAD */

/* The DLL may need to download a debug monitor, this is called an
 * external startup download. It may also need/want to download the
 * image itself, this is called an external image download.
 * In either case, the Controller tells the Target to do what it likes.
 * It then returns saying if the Controller needs to download the image.
 */
/* Download flags */
enum {
  WinRDI_DownloadStartup  = 1,
  WinRDI_DownloadImage    = 2
};

/* Parameters: flags - a bitflag of the above two parameters, to specify
 *                 whether 'startup' or 'image' download is requested.
 *             filename - name of an image to download, if the 'image' bit
 *                 of 'flags' was set; NULL otherwise.
 * Returns:    TRUE if the download was successful
 *             FALSE otherwise
 *             On exit the 'image' bit of 'flags' is set if an image was
 *             downloaded.
 */
typedef bool (WINAPI *WinRDI_funcDownload)(int *options, char const *filename);

extern int WINAPI WinRDI_Download(int *options, char const *filename);

#define WinRDI_strDownload "WinRDI_Download"


#ifndef NO_ACI_COMMAND
/* COVERIFICATION COMMAND */

/* The DLL may specifiy an implementation of the "aci" debugger command which
 * sends a command string to a coverification kernel.
 *
 * Parameters: command - the text string following the "aci" debugger command.
 * Returns:    TRUE if the implementation exists and the command was successful
 *             FALSE otherwise
 */
typedef int (WINAPI *WinRDI_funcACICommand)(const char* command);

extern int WINAPI WinRDI_ACICommand(const char* command);

#define WinRDI_strACICommand "WinRDI_ACICommand"
#endif /* NO_ACI_COMMAND */

/* UNUSED */

/* WinRDI_Download_... download bitflags  */
/* typedef int (WINAPI *WinRDI_funcDownload)(int *options, char const *filename); */
typedef BOOL (WINAPI *WinRDI_funcIsIdleProcessing)(void);
typedef int (WINAPI *WinRDI_funcIdle)(void);
typedef BOOL (WINAPI *WinRDI_funcIsProcessingSWI)(void);
typedef void (WINAPI *WinRDI_funcSetStopping)(BOOL stopping);
typedef char *(WINAPI *WinRDI_funcFlashDLAvailable)(void);


extern void WINAPI WinRDI_ProgressFunc(WinRDI_ProgressInfo *info);
extern BOOL WINAPI WinRDI_IsIdleProcessing(void);
extern int WINAPI WinRDI_Idle(void);
extern BOOL WINAPI WinRDI_IsProcessingSWI(void);
extern void WINAPI WinRDI_SetStopping(BOOL stopping);

/* find out whether we can use Flash Download... */
extern char *WINAPI WinRDI_FlashDLAvailable(void);


/* exports from windows-based RDI DLLs */
#define WinRDI_strIsIdleProcessing    "WinRDI_IsIdleProcessing" 
#define WinRDI_strIdle                "WinRDI_Idle"              
#define WinRDI_strIsProcessingSWI     "WinRDI_IsProcessingSWI"   
#define WinRDI_strSetStopping         "WinRDI_SetStopping"
#define WinRDI_strFlashDLAvailable    "WinRDI_FlashDLAvailable"

#endif /* winrdi_h */

/* EOF winrdi.h */
