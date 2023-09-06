/*
 * ARM RDI - error codes : rdi_err.h
 * Copyright (C) 1998 Advanced RISC Machines Ltd. All rights reserved.
 */

/*
 * RCS $Revision: 1.2 $
 * Checkin $Date: 1998/05/06 09:17:39 $
 * Revising $Author: hbullman $
 */

#ifndef rdi_err_h
#define rdi_err_h

/*
 * Error codes
 */

/* THESE ARE DUPLICATED IN adp.h */

#define RDIError_NoError                0

#define RDIError_Reset                  1
#define RDIError_UndefinedInstruction   2
#define RDIError_SoftwareInterrupt      3
#define RDIError_PrefetchAbort          4
#define RDIError_DataAbort              5
#define RDIError_AddressException       6
#define RDIError_IRQ                    7
#define RDIError_FIQ                    8
#define RDIError_Error                  9
#define RDIError_BranchThrough0         10

#define RDIError_NotInitialised         128
#define RDIError_UnableToInitialise     129
#define RDIError_WrongByteSex           130
#define RDIError_UnableToTerminate      131
#define RDIError_BadInstruction         132
#define RDIError_IllegalInstruction     133
#define RDIError_BadCPUStateSetting     134
#define RDIError_UnknownCoPro           135
#define RDIError_UnknownCoProState      136
#define RDIError_BadCoProState          137
#define RDIError_BadPointType           138
#define RDIError_UnimplementedType      139
#define RDIError_BadPointSize           140
#define RDIError_UnimplementedSize      141
#define RDIError_NoMorePoints           142
#define RDIError_BreakpointReached      143
#define RDIError_WatchpointAccessed     144
#define RDIError_NoSuchPoint            145
#define RDIError_ProgramFinishedInStep  146
#define RDIError_UserInterrupt          147
#define RDIError_CantSetPoint           148
#define RDIError_IncompatibleRDILevels  149

#define RDIError_CantLoadConfig         150
#define RDIError_BadConfigData          151
#define RDIError_NoSuchConfig           152
#define RDIError_BufferFull             153
#define RDIError_OutOfStore             154
#define RDIError_NotInDownload          155
#define RDIError_PointInUse             156
#define RDIError_BadImageFormat         157
#define RDIError_TargetRunning          158
#define RDIError_DeviceWouldNotOpen     159
#define RDIError_NoSuchHandle           160
#define RDIError_ConflictingPoint       161

#define RDIError_TargetBroken           162
#define RDIError_TargetStopped          163

#define RDIError_LinkTimeout            200  /* data link timeout error */
#define RDIError_OpenTimeout            201  /* open timeout (c.f link timeout, on an open */
#define RDIError_LinkDataError          202  /* data error (read/write) on link */
#define RDIError_Interrupted            203  /* (e.g. boot) interrupted */

#define RDIError_LittleEndian           240
#define RDIError_BigEndian              241
#define RDIError_SoftInitialiseError    242

/* New error: ReadOnly, when state can't be written */
#define RDIError_ReadOnly               252
#define RDIError_InsufficientPrivilege  253
#define RDIError_UnimplementedMessage   254
#define RDIError_UndefinedMessage       255

/* Range of numbers that are reserved for RDI implementations */
#define RDIError_TargetErrorBase        256
#define RDIError_TargetErrorTop         (0x1000 - 1)

#endif /* rdi_err_h */
