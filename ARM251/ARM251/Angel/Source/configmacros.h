/* -*-C-*-
 *
 * $Revision: 1.1.2.3 $
 *   $Author: rivimey $
 *     $Date: 1998/10/19 12:17:20 $
 *
 * Copyright (c) 1996 Advanced RISC Machines Limited.
 * All Rights Reserved.
 *
 *   Project: ANGEL
 *
 *     Title: The message to send up when we boot
 */

#ifndef configmacros_h
#define configmacros_h

#define ANGEL_NAME      "Angel Debug Monitor"

#define SER_STR "Serial"

#if defined(PARALLEL_SUPPORTED) && PARALLEL_SUPPORTED > 0
# define PAR_STR ", Parallel"
#else
# define PAR_STR
#endif

#if defined(ETHERNET_SUPPORTED) && ETHERNET_SUPPORTED > 0
# define ETH_STR ", Ethernet"
#else
# define ETH_STR
#endif

#if defined(DCC_SUPPORTED) && DCC_SUPPORTED > 0
# define DCC_STR ", DCC"
#else
# define DCC_STR
#endif

#if defined(PROFILE_SUPPORTED) && PROFILE_SUPPORTED > 0
# define PRF_STR ", Profile"
#else
# define PRF_STR
#endif

#if HANDLE_INTERRUPTS_ON_IRQ == 1
#define IRQ_STR    ", IRQ"
#else
#define IRQ_STR
#endif

#if HANDLE_INTERRUPTS_ON_FIQ == 1
#define FIQ_STR    ", FIQ"
#else
#define FIQ_STR
#endif

#if defined(SYSTEM_COPROCESSOR_SUPPORTED) && SYSTEM_COPROCESSOR_SUPPORTED > 0
# define SYS_STR ", Sys Copro"
#else
# define SYS_STR
#endif

#if defined( __TARGET_CPU_generic )
# if defined( __TARGET_ARCH_3 )
#  define CPU_STR    "Arch 3"
# elif defined( __TARGET_ARCH_4 )
#   define CPU_STR    "Arch 4"
# elif defined( __TARGET_ARCH_4T )
#   define CPU_STR    "Arch 4T"
# else
#   define CPU_STR    ""
# endif
#else
# if defined( __TARGET_CPU_ARM6 )
#  define CPU_STR    "ARM6"
# elif defined( __TARGET_CPU_ARM7 )
#  define CPU_STR    "ARM7"
# elif defined( __TARGET_CPU_ARM7T )
#  define CPU_STR    "ARM7T"
# elif defined( __TARGET_CPU_ARM7M )
#  define CPU_STR    "ARM7M"
# elif defined( __TARGET_CPU_ARM7TDI )
#  define CPU_STR    "ARM7TDI"
# elif defined( __TARGET_CPU_ARM7TDMI )
#  define CPU_STR    "ARM7TDMI"
# elif defined( __TARGET_CPU_ARM8 )
#  define CPU_STR    "ARM8"
# elif defined( __TARGET_CPU_StrongARM1 )
#  define CPU_STR    "StrongARM1"
# elif defined( __TARGET_CPU_ARM9 )
#  define CPU_STR    "ARM9"
# elif defined( __TARGET_CPU_ARM9TM )
#  define CPU_STR    "ARM9TM"
# endif
#endif

#if DEBUG == 1
# define DBG_STR ", debug: " ## STRINGIFY(DEBUG_METHOD)
#else
# define DBG_STR
#endif

#define ANGEL_CONFIG  "Built for " CPU_STR " " \
                        SER_STR PAR_STR ETH_STR DCC_STR \
                        SYS_STR PRF_STR IRQ_STR FIQ_STR DBG_STR

#endif

 /* EOF */
