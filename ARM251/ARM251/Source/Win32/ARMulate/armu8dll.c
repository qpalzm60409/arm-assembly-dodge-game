  /* RCS $Revision: 1.22.2.2 $
 * Checkin $Date: 1999/03/10 12:06:15 $
 * Revising $Author: dcleland $
 */

#include <windows.h>
#include <stdio.h>
#include <limits.h>

#include "armdefs.h"
#include "armu8dll.h"
#include "armu8dll.h"
#include "resource.h"

#include "winrdi.h"
#include "windebug.h"
#include "multirdi.h"

static RDI_ConfigPointer armul_config;

typedef struct { char name[16]; unsigned val; } Processor;
static BOOL AssignConfig();

extern struct RDI_ProcVec armul_rdi;

LRESULT CALLBACK ConfigDlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

/*
 * Function to register callback function to be called back
 *      in YieldControl()
 *
 */

static WinRDI_YieldProc *pfnYield = NULL;
static WinRDI_YieldArg *hYield = NULL;

void WINAPI WinRDI_Register_Yield_Callback(
    WinRDI_YieldProc *yieldcallback, WinRDI_YieldArg *hyieldcallback)
{
        pfnYield = yieldcallback;
        hYield   = hyieldcallback;
}


/*
 * In order for a DLL to be recognised as an RDI connection by ADW the
 *      following function is needed.
 */
BOOL WINAPI WinRDI_Valid_RDI_DLL(void)
{
    return TRUE;
}

/*
 * The following function returns a description of what this DLL does, to ADW.
 *      This will be displayed on the 'Debugger Configuration' Target property page.
 */
char* WINAPI WinRDI_Get_DLL_Description(void)
{
    /* Limited to around 200 characters */
    static char *msg = "Use the ARM Debugger with the 'ARMulator' Instruction "
                       "Set Simulator. This allows you to execute ARM programs "
                       "without physical ARM hardware, by simulating the ARM "
                       "instructions in software.";
    return msg ;
}

int WINAPI WinRDI_Config(RDI_ConfigPointer config, HWND hParent) 
{
    int ret;
    armul_config = config;

    ret = DialogBox(ghArmulateMod, MAKEINTRESOURCE(IDD_CONFIG), hParent, ConfigDlgProc);
    return ret;
}

RDI_ProcVec *WINAPI WinRDI_GetRDIProcVec(void)
{
   return (RDI_ProcVec *)&armul_rdi;

}

int WINAPI WinRDI_GetVersion(void)
{
   return WinRDI_Version;
}

/******************************************************************************\
*
*  FUNCTION:    DllMain
*
*  INPUTS:      hDLL       - handle of DLL
*               dwReason   - indicates why DLL called
*               lpReserved - reserved
*
*  RETURNS:     TRUE (always, in this example.)
*
*               Note that the retuRn value is used only when
*               dwReason = DLL_PROCESS_ATTACH.
*
*               Normally the function would return TRUE if DLL initial-
*               ization succeeded, or FALSE it it failed.
*
*  GLOBAL VARS: ghArmulateMod - handle of DLL (initialized when PROCESS_ATTACHes)
*
*  COMMENTS:    The function will display a dialog box informing user of
*               each notification message & the name of the attaching/
*               detaching process/thread. For more information see
*               "DllMain" in the Win32 API reference.
*
\******************************************************************************/

BOOL WINAPI DllMain(HANDLE hDLL, DWORD dwReason, LPVOID lpReserved)
{
    switch (dwReason)
    {
        case DLL_PROCESS_ATTACH:
        {
            //
            // DLL is attaching to the address space of the current process.
            //
            ghArmulateMod = hDLL;

#ifdef _DEBUG
            {
                char buf[BUFSIZE+1];
                GetModuleFileName (NULL, (LPTSTR) buf, BUFSIZE);
                ADBG("ARMul810", "ARMulator DLL: Process attaching - %s\n", buf);
            }
#endif
            break;
        }

        case DLL_THREAD_ATTACH:
        {
            //
            // A new thread is being created in the current process.
            //
            ADBG("ARMul810", "ARMulator DLL: Thread attaching\n");
            break;
        }
        case DLL_THREAD_DETACH:
        {
            //
            // A thread is exiting cleanly.
            //
            ADBG("ARMul810", "ARMulator DLL: Thread detaching\n");
            break;
        }
        case DLL_PROCESS_DETACH:
        {
            //
            // The calling process is detaching the DLL from its address space.
            //
            ADBG("ARMul810", "ARMulator DLL: Process detaching\n");
            break;
        }
    }
    return TRUE;
}

/*
 * Functions for dealing with the CLOCK parts of the dialogue
 */
static BOOL clockIsEnabled;
static char szClockSpeed[20];

static void SetClock(HWND hwnd, bool isEnabled)
{
    clockIsEnabled = isEnabled;
    SendDlgItemMessage(hwnd, IDC_SPEED,       WM_ENABLE,   (WPARAM)isEnabled,  (LPARAM)(LPCSTR)0);
    SendDlgItemMessage(hwnd, IDC_CLOCKON,     BM_SETCHECK, (WPARAM)isEnabled,  (LPARAM)0);
    SendDlgItemMessage(hwnd, IDC_CLOCKOFF,    BM_SETCHECK, (WPARAM)!isEnabled, (LPARAM)0);
    SendDlgItemMessage(hwnd, IDC_CLOCK_SPEED, WM_ENABLE,   (WPARAM)isEnabled,  (LPARAM)0);
  if (!isEnabled) {
      SendDlgItemMessage(hwnd, IDC_CLOCK_SPEED, WM_SETTEXT, (WPARAM)0, (LPARAM)(LPCSTR)"");
  } else {
      SendDlgItemMessage (hwnd, IDC_CLOCK_SPEED, WM_SETTEXT, (WPARAM)0, (LPARAM)(LPCSTR)szClockSpeed);
  }
}

static void UpdateClock(HWND hwnd)
{
    SendDlgItemMessage(hwnd, IDC_CLOCK_SPEED, WM_GETTEXT, (WPARAM)20, (LPARAM)szClockSpeed);
}

/**************************************************************************\
*
*  function:  MainDlgProc()
*
*  input parameters:  standard window procedure parameters.
*
\**************************************************************************/

LRESULT CALLBACK ConfigDlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        /********************************************************************\
        * WM_INITDIALOG
        \********************************************************************/
        case WM_INITDIALOG:
        {
            int nProcessor, nRes, nProc = 0, nItem;
            int fpe, bytesex;
            unsigned long cpu_speed;
            RDI_NameList const *theList = armul_rdi.cpunames( RDI150(NULL) );
            int defaultProcessor = ARMul_DefaultProcessor( RDI150(NULL) );
#if RDI_VERSION==150
            char const *str_bytesex, *processor_name;

            fpe = ToolConf_DLookupBool(armul_config, Dbg_Cnf_FPE, TRUE);
            
            str_bytesex = ToolConf_Lookup(armul_config, Dbg_Cnf_ByteSex);
            if (str_bytesex != NULL && str_bytesex[0]=='L')
                bytesex = 0;
            else
                bytesex = 1;

            processor_name = ToolConf_Lookup(armul_config,Dbg_Cnf_Processor);

            nItem = ARMul_DefaultProcessor;
#else
            fpe = armul_config->fpe;
            bytesex = armul_config->bytesex;
            nItem = (armul_config->processor ?
                                         armul_config->processor :
                                         defaultProcessor);
#endif


            // Fill the drop-down list with the processor names
            for (nProcessor = 0; nProcessor < theList->itemmax; nProcessor++)
            {
                nRes = SendDlgItemMessage (hwnd, IDC_PROCESSOR,  CB_ADDSTRING, 0,
                                           (LPARAM) (LPCTSTR)theList->names[nProcessor]);
#if RDI_VERSION==150
                if( (processor_name != NULL) && 
                    !strcmp(processor_name, theList->names[nProcessor]))
                    nItem = nProcessor;
#endif
            }

            nRes = SendDlgItemMessage (hwnd, IDC_PROCESSOR,  CB_SETCURSEL,
                                       (WPARAM)nItem, (LPARAM)0);
              

            // Set the Endian radio button

            /*  NO ENDIAN Button  ###########################
            if (bytesex)
                nRes = SendDlgItemMessage (hwnd, IDC_BIG_ENDIAN, BM_SETCHECK, (WPARAM)1, (LPARAM)0);
            else
                nRes = SendDlgItemMessage (hwnd, IDC_ENDIAN, BM_SETCHECK, (WPARAM)1, (LPARAM)0);
                NO ENDIAN Button  ###########################   */

            // Set the FPE checkbox
            if (fpe)
                nRes = SendDlgItemMessage (hwnd, IDC_FPE, BM_SETCHECK, (WPARAM)1, (LPARAM)0);
            else
                nRes = SendDlgItemMessage (hwnd, IDC_FPE, BM_SETCHECK, (WPARAM)0, (LPARAM)0);


            // Set the clockspeed editbox
            {
#if RDI_VERSION==150
                char const *clock = ToolConf_Lookup(armul_config, Dbg_Cnf_ClockSpeed);
                if (clock == NULL)
                    cpu_speed == 0;
                else {
                    cpu_speed = ToolConf_Power(clock, FALSE);
                }
#else
                cpu_speed = armul_config->cpu_speed;
#endif
                if (cpu_speed != 0) {
#if RDI_VERSION==150
                    // preserve the string from the toolconf.
                    strncpy(szClockSpeed, clock, sizeof(szClockSpeed)-1);
#else
                    double clk;
                    char *mult;
                    clk = ARMul_SIRange(cpu_speed, &mult, FALSE);
                    sprintf(szClockSpeed, "%5.2f%sHz", clk, mult);
#endif
                } else {
                    strcpy(szClockSpeed, "1MHz");
                }
            }

            SetClock(hwnd, cpu_speed != 0);

            return TRUE;
        }

        /********************************************************************\
        * WM_SYSCOMMAND
        \********************************************************************/
        case WM_SYSCOMMAND:
        {
            if (wParam == SC_CLOSE)
            {
                EndDialog (hwnd, TRUE);
                return TRUE;
            }
            else
                return FALSE;
        }
        break;

        /********************************************************************\
        * WM_COMMAND
        *
        * When the different buttons are hit, clear the list box, disable
        *  updating to it, call the function which will fill it, reenable
        *  updating, and then force a repaint.
        *
        \********************************************************************/
        case WM_COMMAND:
        {
            /* if the list box sends back messages, return.  */
            if (LOWORD(wParam)==IDC_PROCESSOR)
                return TRUE;

            /* switch on the control ID of the button that is pressed. */
            switch (LOWORD(wParam))
            {
                case IDOK:
                {
                    BOOL bAssignOk = AssignConfig(hwnd);
                    if (bAssignOk)
                        EndDialog(hwnd, TRUE);
                    return bAssignOk;
                }
                case IDCANCEL:
                {
                    EndDialog (hwnd, FALSE);
                    return TRUE;
                }
                case IDC_CLOCKON:
                case IDC_CLOCKOFF:
                {
                    BOOL isEnabled = SendDlgItemMessage(hwnd, IDC_CLOCKON, BM_GETCHECK, (WPARAM)0, (LPARAM)0);
                    if (clockIsEnabled && !isEnabled)
                        UpdateClock(hwnd);      // Save clock value
                    SetClock(hwnd, isEnabled);
                    return TRUE;
                }
            }
            return TRUE;
            break; /* end WM_COMMAND */
        }
        default:
            return FALSE;

    } /* end switch(message) */

    return FALSE;
}

#if RDI_VERSION==150
static BOOL AssignConfig(HWND hwnd)
{
        int nProc, nByteSex;
        BOOL isEnabled;

        RDI_NameList const* theList = armul_rdi.cpunames(NULL);

        nProc = SendDlgItemMessage (hwnd, IDC_PROCESSOR,  CB_GETCURSEL,   (WPARAM)0, (LPARAM)0);
        if (nProc == CB_ERR)
                nProc = 0;
        ToolConf_UpdateTagged(armul_config, Dbg_Cnf_Processor, (char *) theList->names[nProc]);

        // Get the Endian radio button
        /* No Endian Button #############################
        nByteSex = SendDlgItemMessage (hwnd, IDC_BIG_ENDIAN, BM_GETCHECK, (WPARAM)0, (LPARAM)0);
           No Endian Button ############################# */
        ToolConf_UpdateTagged(armul_config, Dbg_Cnf_ByteSex, (nByteSex) ? "B" : "L");

        // Get the FPE checkbox
        ToolConf_UpdateTyped(armul_config, Dbg_Cnf_FPE, tcnf_Bool,
                   SendDlgItemMessage (hwnd, IDC_FPE, BM_GETCHECK, (WPARAM)0, (LPARAM)0));

        // Get the clockspeed editbox
        isEnabled = SendDlgItemMessage(hwnd, IDC_CLOCKON, BM_GETCHECK, (WPARAM)0, (LPARAM)0);
        if (isEnabled) {
            UpdateClock(hwnd);
            UINT nClockSpeed;
            nClockSpeed = ToolConf_Power(szClockSpeed, FALSE);

            if (nClockSpeed <= INT_MAX) {  // BGC - Check that the clock speed doesn't exceed max limit
                ToolConf_UpdateTagged(armul_config, Dbg_Cnf_CPUSpeed, szClockSpeed);
            } else {
                MessageBox(GetFocus(), "Clock Speed is too high!", "ARMulator" , MB_OK | MB_ICONEXCLAMATION);
                return FALSE;  
            }
        } else {
            ToolConf_UpdateTyped(armul_config, Dbg_Cnf_CPUSpeed, tcnf_UInt, 0);
        }
        return TRUE;
}
#else
static BOOL AssignConfig(HWND hwnd)
{
        int nProc;
        BOOL isEnabled;

        nProc = SendDlgItemMessage (hwnd, IDC_PROCESSOR,  CB_GETCURSEL,   (WPARAM)0, (LPARAM)0);
        if (nProc == CB_ERR)
                nProc = 0;
        armul_config->processor = nProc;

        // Get the Endian radio button
        /* No Endian Button ##############################
        armul_config->bytesex = SendDlgItemMessage (hwnd, IDC_BIG_ENDIAN, BM_GETCHECK, (WPARAM)0, (LPARAM)0);
           No Endian Button ############################## */

        // Get the FPE checkbox
        armul_config->fpe = SendDlgItemMessage (hwnd, IDC_FPE, BM_GETCHECK, (WPARAM)0, (LPARAM)0);

        // Get the clockspeed editbox
        isEnabled = SendDlgItemMessage(hwnd, IDC_CLOCKON, BM_GETCHECK, (WPARAM)0, (LPARAM)0);
        if (isEnabled) {
            UINT nClockSpeed;

            UpdateClock(hwnd);
            nClockSpeed = ToolConf_Power(szClockSpeed, FALSE);

            if (nClockSpeed <= INT_MAX) {  // BGC - Check that the clock speed doesn't exceed max limit
                armul_config->cpu_speed = nClockSpeed;
            } else {
                MessageBox(GetFocus(), "Clock Speed is too high!", "ARMulator" , MB_OK | MB_ICONEXCLAMATION);
                return FALSE;  
            }
            armul_config->cpu_speed = ToolConf_Power(szClockSpeed, FALSE);
        } else {
            armul_config->cpu_speed = 0;
        }
        return TRUE;
}
#endif


/******************************************************************************\
*
*  FUNCTION: Utils
*
*  RETURNS:  ARMulator DLL Utility functions (not exported)
*
\******************************************************************************/

static void YieldControl(int nLoops)
{
    MSG Message;
    int loop = 0;
    static BOOL bYielding = FALSE;

    if (bYielding)
        return;
    bYielding = TRUE;

    while (PeekMessage(&Message, NULL, 0,0, PM_NOREMOVE))
    {
        if (Message.message == WM_QUIT)
            abort();
        if (Message.message == WM_LBUTTONUP) // Knowledge Base Q102552
        {
            /* We have to be careful with processing WM_LBUTTONUP or
               we will cause ourselves problems with scroll-bar processing.

               The problem is that scroll-bars enter a modal message
               processing loop when you drag them, click on them, e.t.c.
               and only terminate the loop when the WM_LBUTTONUP message is 
               seen. This loop generates scroll messages, which can cause
               ADW/ADU to call the toolbox to get data. As part of getting 
               the data, YieldControl can be called. If YieldControl is
               called it is sucking out messages from under the nose of the
               scroll-bars message loop, which normally doesn't matter.
               However if YieldControl processes the WM_LBUTTONUP message
               the scroll-bar message loop will not see it and hence 
               continue forever, or until a WM_LBUTTONUP message is 
               generated by another mouse click.

               The normal symptom of the problem is that once you click
               on a scroll bar, below the thumb, the window starts
               scrolling and keeps on scrolling.

               To fix the problem we try to avoid processing the
               WM_LBUTTONUP message. The only exception being when
               the message occurs over a window which is a toolbar.
               This is because the processing associated with a toolbar
               button only gets triggered on a WM_LBUTTONUP.  Simple
               example is that you can't interrupt an image using the
               toolbar stop button, if you don't process the message.

               dcleland.

            */
            char x64[64];
            int iChars;
            iChars = GetClassName(Message.hwnd , &x64[0], sizeof(x64));
            if(iChars != 0)
            {
                if(strcmp(x64, "ToolbarWindow32") != 0)
                {
                    break;
                }
            }
            else
            {
                break;
            }
        }
        PeekMessage(&Message, NULL, 0, 0, PM_REMOVE);
        TranslateMessage(&Message);
        DispatchMessage(&Message);

        loop++;
    }
    bYielding = FALSE;

    /*  printf("Pumped %d messages\n", loop);  */

    return;

}

void armsd_hourglass(void)
{
        YieldControl(1); // This could be Selected By Options
}

/* EOF armu8dll.c */
