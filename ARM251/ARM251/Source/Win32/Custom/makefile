# RCS $Revision: 1.5.2.2 $
# Checkin $Date: 1998/10/14 15:04:11 $
# Revising $Author: swoodall $
#
# Makefile for NMAKE from VC++ 4.0

#dearlam 3.3.1998
# get the names of products from the install program directory (prodver.h)
# so that the programs can create registry entry for the correct version of the correct
# product, and the uninstaller, uninstalls the correct one.


# More sensible to get prodver.h locally to avoid delivering all of the windows/setup installer
# dearlam/swoodall 23.8.98
#INSTALL_INCLUDE= ..\windows\setup
INSTALL_INCLUDE= .


!IF "$(CFG)" == ""
CFG=Release
!MESSAGE No configuration specified.  Defaulting to $(CFG)
!ENDIF 

!if [if not exist $(CFG) md $(CFG)]
!endif


!IF "$(CFG)" == "ireleval"
DEFEVAL=-DEVAL
!ENDIF 


all : $(CFG)\custom.dll


$(CFG)\custom.obj : custom.c $(INSTALL_INCLUDE)\prodver.h
               cl -c -I$(INSTALL_INCLUDE) custom.c $(DEFEVAL) -DWIN32 /Fo$(CFG)\custom.obj



$(CFG)\custom.res : custom.rc resource.h $(INSTALL_INCLUDE)\prodver.h
               rc /r -I$(INSTALL_INCLUDE)  $(DEFEVAL) /fo$(CFG)\custom.res custom.rc 

custom.h: resource.h
               copy resource.h custom.h

$(CFG)\custom.dll : $(CFG)\custom.obj $(CFG)\custom.res custom.def custom.h
        link $(CFG)\custom.obj $(CFG)\custom.res /def:custom.def /out:$(CFG)\custom.dll /DLL
