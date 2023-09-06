/*
 * tool version numbers file - in one place for easier updating.
 * Copyright (C) 1996 Advanced RISC Machines Ltd. All rights reserved.
 */

/*
 * RCS $Revision: 1.50.2.3 $
 * Checkin $Date: 2000/01/17 12:52:56 $
 * Revising $Author: wdijkstr $
 */

#ifndef toolver_h
#define toolver_h

#ifdef TOOLVER_RELEASE

/* for use when all tools are labelled with the release version number */
/* may need to think about patching - what if not all tools are updated? */
#define TOOLVER_TCC       TOOLVER_RELEASE
#define TOOLVER_ARMCC     TOOLVER_RELEASE
#define TOOLVER_ARMCPP    TOOLVER_RELEASE
#define TOOLVER_TCPP      TOOLVER_RELEASE
#define TOOLVER_ARMSD     TOOLVER_RELEASE
#define TOOLVER_ARMASM    TOOLVER_RELEASE
#define TOOLVER_ARMLIB    TOOLVER_RELEASE
#define TOOLVER_ARMLINK   TOOLVER_RELEASE
#define TOOLVER_ARMUL     TOOLVER_RELEASE
#define TOOLVER_ARM9UL    TOOLVER_RELEASE
#define TOOLVER_SARMUL    TOOLVER_RELEASE
#define TOOLVER_PICUL     TOOLVER_RELEASE
#define TOOLVER_DECAOF    TOOLVER_RELEASE
#define TOOLVER_DECAXF    TOOLVER_RELEASE
#define TOOLVER_RECONFIG  TOOLVER_RELEASE
#define TOOLVER_TOPCC     TOOLVER_RELEASE
#define TOOLVER_ARMPROF   TOOLVER_RELEASE
#define TOOLVER_AIF2HEX   TOOLVER_RELEASE
#define TOOLVER_ARMMAKE   TOOLVER_RELEASE
#define TOOLVER_ANGEL     TOOLVER_RELEASE
#define TOOLVER_ICEMAN2   TOOLVER_RELEASE
#define TOOLVER_FROMELF   TOOLVER_RELEASE  

#else /* TOOLVER_RELEASE */

/* #define ARM_RELEASE in the Makefile (or at Makemega time). */
#ifndef ARM_RELEASE
# define ARM_RELEASE "unreleased"
#endif

#ifdef SUPPLIER_RELEASE
# define SUPPLIER_INFO "(ARM Ltd " ARM_RELEASE "/" SUPPLIER_RELEASE ")"
#else
# define SUPPLIER_INFO "(ARM Ltd " ARM_RELEASE ")"
#endif

/*
 * This was changed so we can redefine version numbers for specific versions of specifc
 * tools easily. -- RIC 11/1997.
 */
#define TOOLNUM_AIF2HEX   "1.10"
#define TOOLNUM_ANGEL     "1.20"
#define TOOLNUM_ARMASM    "2.50"
#define TOOLNUM_ARMCC     "4.91"
#define TOOLNUM_ARMCPP    "0.61/C" TOOLNUM_ARMCC
#define TOOLNUM_TCPP      "0.61/C" TOOLNUM_TCC
#define TOOLNUM_ARMLIB    "4.50"
#define TOOLNUM_ARMLINK   "5.20"
#define TOOLNUM_ARMMAKE   "4.10"
#define TOOLNUM_ARMPROF   "1.10"
#define TOOLNUM_ARMSD     "4.60"
#define TOOLNUM_ARMUL     "2.10"
#define TOOLNUM_ARM9UL    "1.10"
#define TOOLNUM_DECAOF    "4.20"
#define TOOLNUM_DECAXF    "1.10"
#define TOOLNUM_ICEMAN2   "2.07"
#define TOOLNUM_RECONFIG  "2.07"
#define TOOLNUM_SARMUL    "2.10"
#define TOOLNUM_TCC       "1.21"
#define TOOLNUM_TOPCC     "3.40"
#define TOOLNUM_FROMELF   "1.00"

/*
 * Insert any changes for special versions here, with a description of the product in
 * which they appear, if not readily apparent:
 */

#ifdef HACK_FOR_SLOW_TARGETS
/*
 * Special IceAgent which copes with slow (kilohertz-speed) target processors
 */
# undef TOOLNUM_ICEMAN2 
# define TOOLNUM_ICEMAN2  "2.07a"
#endif


#define TOOLVER_AIF2HEX   TOOLNUM_AIF2HEX " " SUPPLIER_INFO
#define TOOLVER_ANGEL     TOOLNUM_ANGEL " " SUPPLIER_INFO
#define TOOLVER_ARMASM    TOOLNUM_ARMASM " " SUPPLIER_INFO /* also used for tasm */
#define TOOLVER_ARMCC     TOOLNUM_ARMCC " " SUPPLIER_INFO
#define TOOLVER_ARMCPP    TOOLNUM_ARMCPP " " SUPPLIER_INFO
#define TOOLVER_TCPP      TOOLNUM_TCPP " " SUPPLIER_INFO
#define TOOLVER_ARMLIB    TOOLNUM_ARMLIB " " SUPPLIER_INFO
#define TOOLVER_ARMLINK   TOOLNUM_ARMLINK " " SUPPLIER_INFO
#define TOOLVER_ARMMAKE   TOOLNUM_ARMMAKE " " SUPPLIER_INFO
#define TOOLVER_ARMPROF   TOOLNUM_ARMPROF " " SUPPLIER_INFO
#define TOOLVER_ARMSD     TOOLNUM_ARMSD " " SUPPLIER_INFO
#define TOOLVER_ARMUL     TOOLNUM_ARMUL                /* no ARM needed */
#define TOOLVER_ARM9UL    TOOLNUM_ARM9UL               /* no ARM needed */
#define TOOLVER_DECAOF    TOOLNUM_DECAOF " " SUPPLIER_INFO
#define TOOLVER_DECAXF    TOOLNUM_DECAXF " " SUPPLIER_INFO
#define TOOLVER_ICEMAN2   TOOLNUM_ICEMAN2 " " SUPPLIER_INFO
#define TOOLVER_RECONFIG  TOOLNUM_RECONFIG " " SUPPLIER_INFO
#define TOOLVER_SARMUL    TOOLNUM_SARMUL                /* no ARM needed */
#define TOOLVER_TCC       TOOLNUM_TCC " " SUPPLIER_INFO
#define TOOLVER_TOPCC     TOOLNUM_TOPCC " " SUPPLIER_INFO
#define TOOLVER_FROMELF   TOOLNUM_FROMELF " " SUPPLIER_INFO
#endif /* TOOLVER_RELEASE */

#define YY_INT_TO_STRING(int) #int
#define YY_CALL_INT_TO_STRING(int) YY_INT_TO_STRING(int)

#ifdef BUILD_NUMBER
  #ifdef BUILD_DATE
    #define BUILD_STRING "Minor build " YY_CALL_INT_TO_STRING( BUILD_NUMBER ) "-" YY_CALL_INT_TO_STRING( BUILD_DATE )
  #endif
#endif

#ifdef BUILD_NUMBER
  #ifndef BUILD_DATE
    #define BUILD_STRING "Build number " YY_CALL_INT_TO_STRING( BUILD_NUMBER )
  #endif
#endif

#ifndef BUILD_NUMBER
  #ifndef BUILD_DATE
    #define BUILD_STRING "Unreleased build " __DATE__
  #endif
#endif

#endif /* toolver_h */
