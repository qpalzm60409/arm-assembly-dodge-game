/*
 * ARM RDI - RDP numbers : rdi_rdp.h
 * Copyright (C) 1998 Advanced RISC Machines Ltd. All rights reserved.
 */

/*
 * RCS $Revision: 1.1 $
 * Checkin $Date: 1998/03/26 13:59:45 $
 * Revising $Author: mwilliam $
 */

#ifndef rdi_rdp_h
#define rdi_rdp_h

/*********                controller -> agent                         *********/

#define RDP_Start               (unsigned char)0x0
/* byte type, word memorysize {, byte speed (if (type & 2)) }           */
/* returns byte status                                                  */

#define RDP_End                 (unsigned char)0x1
/* no argument                                                          */
/* returns byte status                                                  */

#define RDP_Read                (unsigned char)0x2
/* word address, word nbytes                                            */
/* returns bytes data, byte status {, word count }                      */
/* The count value is returned only if status != 0, and is the number   */
/* of bytes successfully read                                           */

#define RDP_Write               (unsigned char)0x3
/* word address, word nbytes, bytes data                                */
/* returns byte status {, word count }                                  */
/* The count value is returned only if status != 0, and is the number   */
/* of bytes successfully written                                        */

#define RDP_CPUread             (unsigned char)0x4
/* byte mode, word mask                                                 */
/* returns words data, byte status                                      */

#define RDP_CPUwrite            (unsigned char)0x5
/* byte mode, word mask, words data                                     */
/* returns byte status                                                  */

#define RDP_CPread              (unsigned char)0x6
/* byte cpnum, word mask                                                */
/* returns words data, byte status                                      */

#define RDP_CPwrite             (unsigned char)0x7
/* byte cpnum, word mask, words data                                    */
/* returns byte status                                                  */

#define RDP_SetBreak            (unsigned char)0xa
/* word address, byte type {, word bound }                              */
/* if !(type & RDIPoint_Inquiry)                                        */
/*    returns {word pointhandle, } byte status                          */
/* if (type & RDIPoint_Inquiry)                                         */
/*    returns word address {, word bound }, byte status                 */
/* pointhandle is present if type & RDIPoint_Handle                     */
/* bound arguments and replies are present if                           */
/*   (type & 7) == RDIPoint_IN, RDIPoint_OUT or RDIPoint_MASK           */

#define RDP_ClearBreak          (unsigned char)0xb
/* word pointhandle                                                     */
/* returns byte status                                                  */

#define RDP_SetWatch            (unsigned char)0xc
/* word address, byte type, byte datatype {, word bound }               */
/* if !(type & RDIPoint_Inquiry)                                        */
/*    returns {word pointhandle, } byte status                          */
/* if (type & RDIPoint_Inquiry)                                         */
/*    returns word address {, word bound }, byte status                 */
/* pointhandle is present if type & RDIPoint_Handle                     */
/* bound arguments and replies are present if                           */
/*   (type & 7) == RDIPoint_IN or RDIPoint_OUT                          */

#define RDP_ClearWatch          (unsigned char)0xd
/* word pointhandle                                                     */
/* returns byte status                                                  */

#define RDP_Execute             (unsigned char)0x10
/* byte type                                                            */
/* returns {word pointhandle, } byte status                             */
/* pointhandle is returned if (type & RDIPoint_Handle); type also has:  */
#  define RDIExecute_Async 1

#define RDP_Step                (unsigned char)0x11
/* byte type, word stepcount                                            */
/* returns {word pointhandle, } byte status                             */
/* (type as for RDP_Execute)                                            */

#define RDP_Info                (unsigned char)0x12
/* argument and return type different for each operation: see below     */

#define RDP_OSOpReply           (unsigned char)0x13

#define RDP_AddConfig           (unsigned char)0x14
/* word nbytes                                                          */
/* returns byte status                                                  */

#define RDP_LoadConfigData      (unsigned char)0x15
/* word nbytes, nbytes * bytes data                                     */
/* returns byte status                                                  */

#define RDP_SelectConfig        (unsigned char)0x16
/* byte aspect, byte namelen, byte matchtype, word version,             */
/*      namelen * bytes name                                            */
/* returns word version selected, byte status                           */

#define RDP_LoadAgent           (unsigned char)0x17
/* word loadaddress, word size                                          */
/* followed by a number of messages:                                    */
/*   byte = RDP_LoadConfigData, word size, size * bytes data            */
/*   returns byte status                                                */

#define RDP_Interrupt           (unsigned char)0x18

#define RDP_CCToHostReply       (unsigned char)0x19
#define RDP_CCFromHostReply     (unsigned char)0x1a

/*********                agent -> controller                         *********/

#define RDP_Stopped             (unsigned char)0x20
/* reply to step or execute with RDIExecute_Async                       */

#define RDP_OSOp                (unsigned char)0x21
#define RDP_CCToHost            (unsigned char)0x22
#define RDP_CCFromHost          (unsigned char)0x23

#define RDP_Fatal               (unsigned char)0x5e
#define RDP_Return              (unsigned char)0x5f
#define RDP_Reset               (unsigned char)0x7f

#endif /* rdi_rdp_h */
