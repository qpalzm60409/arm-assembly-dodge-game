
/*
 * Copyright (C) 1998 ARM Ltd. All rights reserved.
 *
 * RCS $Revision: 1.5.2.15 $
 * Checkin $Date: 1999/12/22 15:12:18 $
 * Revising $Author: swoodall $
 *
 *
 * David Earlam (5th March, 1998)
 *
 *  Attempt to create a header file which can be read by Installshield and Resource compiler
 *  so that
 * (1)  Windows tools and Uninstaller all use the same name for the product,
 * (2)  APM and ADW (MDW, ADW3 , ADW for C++ ??) create profile settings with a per version key
 *   eg
 *   HKEY_CURRENT_USER
 *              /Software
 *                      /Advanced RISC Machines
 *                              /ARM Software Development Toolkit
 *                                      /v2.11a
 *                                              /ADW text may appear here
 *                                              /APM text may appear here
 *                                      /v2.50
 *                                              /ADW text may appear here
 *                                              /APM text may appear here
 *                                              
 * 
 *  The uninstaller dll will remove the registry key 'v2.50' and all keys below it.
 *  It does not care what the programs use for their own key (normally AFX_IDS_APP_TITLE resource string),
 *  so long as they get the string for the registry key from resource strings in custom.dll,
 *  
 *    INSTALL_COMPANY/INSTALL_PACKAGE_NAME/INSTALL_PACKAGE_VERSION
 *
 *
 *  This file will be read by custom.rc and setup.rul
 *
 *  Unfortunately, neither Installshield's compiler preprocessor, nor Microsoft's Resource compiler
 *  can concatenate strings, so 'v2.50' occurs at multiple places in this text,
 *  and hopefully NOWHERE ELSE.
 *
 *  So to change the version globally replace 'v2.50' in this file and rebuild the install script
 *  and custom.dll.
 *  Ensure that the deinstallkey is correct for the version too! 
 *
 *  I am continuing the trend established with the SDT211 demo, that it uses the same defaults
 *  as the real SDT211.
 *  Installing the demo (say to c:\arm211) installing the real thing (say to c:\arm211real),
 *  uninstalling the demo, would remove the copy in c:\arm211 and the defaults for both the demo
 *  and the real thing.
 * 
 *  If we wish demo and real to coexist with different defaults then one of INSTALL_PACKAGE_NAME
 *  or INSTALL_PACKAGE_VERSION will need to be changed based on the makefile defined SDTDEMO flag
 *  and the same flag would have to be defined in the makefile for custom.dll and the build system
 *  needs to ensure both are in sync.
 *
 */

/* Firstly strings that will become resource strings in custom.dll and entries in the registry */

/* Changes with each release*/
#define INSTALL_PACKAGE_VERSION      "v2.51"
#define INSTALL_PACKAGE_EVAL_VERSION "v2.51 EVALUATION VERSION"
#define PRODUCT_VERSION_1  2
#define PRODUCT_VERSION_2  5
#define PRODUCT_VERSION_3  0
#define PRODUCT_VERSION_4  0
#define PRODUCT_VERSION_NULLTERM "2.51\0"
#define PRODUCT_NAME_AND_VERSION_NULLTERM "ARM SDT v2.51\0"

#define COPYRIGHT_THIS_YEAR "Copyright © 2000\0"

#define INSTALL_PACKAGE_NAME "ARM Software Development Toolkit" 
#define INSTALL_PACKAGE_COPYRIGHT "Copyright (c) 1991-2000 ARM Limited. All rights reserved."

#define INSTALL_PACKAGE_ACRONYM "ARM SDT"

#define INSTALL_COMPANY "ARM Limited"
#define INSTALL_COMPANY_NULLTERM "ARM Limited\0"

/* Used in the window tool about dialogs - keep it fairly short, shame you can't form it from the previous defines*/
#define INSTALL_PACKAGE_NAME_VERSION "ARM Software Development Toolkit v2.51"

/* Secondly strings from the install script, so we only have to edit this file for a new version */

#define STR_LONG_PRODUCT_NAME_STANDARD  "ARM Software Development Toolkit v2.51"
#define STR_LONG_PRODUCT_NAME_DEMO      "Evaluation version of the ARM Software Development Toolkit v2.51"
#define STR_LONG_PRODUCT_NAME_CPP       "ARM C++ Version 1.11 for ARM Software Development Toolkit v2.51"

#define STR_SHORT_PRODUCT_NAME_STANDARD "ARM SDT v2.51"
#define STR_SHORT_PRODUCT_NAME_DEMO     "ARM SDT v2.51 Eval"
#define STR_SHORT_PRODUCT_NAME_CPP      "ARM C++ 1.11"

#define STR_REG_COMP                    INSTALL_COMPANY
#define STR_REG_PRODUCT                 INSTALL_PACKAGE_NAME
#define STR_REG_VERSION                 INSTALL_PACKAGE_VERSION
#define STR_REG_VERSION_CPP             "v1.11"

#define STR_DEFAULT_PATH                "\\ARM251"   // The default installation path

/*
 * This string defines the product name as registered with C-Dilla's protection
 * software (only used for the Demo or Eval product at present). With each new
 * version that we release, a new string of this kind is needed, otherwise users
 * will not be able to try the new version on any machine on which they had
 * previously tried an earlier version.
 */

#define CDILLA_ARM_PRODUCT_NAME   "Arm Evaluation Toolkit Version 2.5"
