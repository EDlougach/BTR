# Microsoft Developer Studio Project File - Name="BTR" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=BTR - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "BTR.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "BTR.mak" CFG="BTR - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "BTR - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "BTR - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "BTR - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
F90=df.exe
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MT /W2 /GX /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 /nologo /subsystem:windows /machine:I386

!ELSEIF  "$(CFG)" == "BTR - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
F90=df.exe
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# SUBTRACT LINK32 /incremental:no

!ENDIF 

# Begin Target

# Name "BTR - Win32 Release"
# Name "BTR - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\AskDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\BeamDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\BeamletDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\BTR.cpp
# End Source File
# Begin Source File

SOURCE=.\BTR.rc
# End Source File
# Begin Source File

SOURCE=.\BTRDoc.cpp
# End Source File
# Begin Source File

SOURCE=.\BTRView.cpp
# End Source File
# Begin Source File

SOURCE=.\CommentsDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\config.cpp
# End Source File
# Begin Source File

SOURCE=.\DataView.cpp
# End Source File
# Begin Source File

SOURCE=.\FieldsDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\GasDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\InjectDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\IonSourceDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\LoadStepDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\LoadView.cpp
# End Source File
# Begin Source File

SOURCE=.\MainFrm.cpp
# End Source File
# Begin Source File

SOURCE=.\MainView.cpp
# End Source File
# Begin Source File

SOURCE=.\MenuPop.cpp
# End Source File
# Begin Source File

SOURCE=.\MFDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\NBIconfDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\NeutrDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\partcfg.cpp
# End Source File
# Begin Source File

SOURCE=.\ParticlesDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\PlotDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\PreviewDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ProjectBar.cpp
# End Source File
# Begin Source File

SOURCE=.\ReionDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\RemnDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ResSmooth.cpp
# End Source File
# Begin Source File

SOURCE=.\SetView.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\SurfDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\TaskDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ThickDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ThinDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ViewSizeDlg.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\AskDlg.h
# End Source File
# Begin Source File

SOURCE=.\BeamDlg.h
# End Source File
# Begin Source File

SOURCE=.\BeamletDlg.h
# End Source File
# Begin Source File

SOURCE=.\BTR.h
# End Source File
# Begin Source File

SOURCE=.\BTRDoc.h
# End Source File
# Begin Source File

SOURCE=.\BTRView.h
# End Source File
# Begin Source File

SOURCE=.\CommentsDlg.h
# End Source File
# Begin Source File

SOURCE=.\config.h
# End Source File
# Begin Source File

SOURCE=.\DataView.h
# End Source File
# Begin Source File

SOURCE=.\FieldsDlg.h
# End Source File
# Begin Source File

SOURCE=.\GasDlg.h
# End Source File
# Begin Source File

SOURCE=.\InjectDlg.h
# End Source File
# Begin Source File

SOURCE=.\IonSourceDlg.h
# End Source File
# Begin Source File

SOURCE=.\LoadStepDlg.h
# End Source File
# Begin Source File

SOURCE=.\LoadView.h
# End Source File
# Begin Source File

SOURCE=.\MainFrm.h
# End Source File
# Begin Source File

SOURCE=.\MainView.h
# End Source File
# Begin Source File

SOURCE=.\MenuPop.h
# End Source File
# Begin Source File

SOURCE=.\MFDlg.h
# End Source File
# Begin Source File

SOURCE=.\NBIconfDlg.h
# End Source File
# Begin Source File

SOURCE=.\NeutrDlg.h
# End Source File
# Begin Source File

SOURCE=.\partfld.h
# End Source File
# Begin Source File

SOURCE=.\ParticlesDlg.h
# End Source File
# Begin Source File

SOURCE=.\PlotDlg.h
# End Source File
# Begin Source File

SOURCE=.\PreviewDlg.h
# End Source File
# Begin Source File

SOURCE=.\ProjectBar.h
# End Source File
# Begin Source File

SOURCE=.\ReionDlg.h
# End Source File
# Begin Source File

SOURCE=.\RemnDlg.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\ResSmooth.h
# End Source File
# Begin Source File

SOURCE=.\SetView.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\SurfDlg.h
# End Source File
# Begin Source File

SOURCE=.\TaskDlg.h
# End Source File
# Begin Source File

SOURCE=.\ThickDlg.h
# End Source File
# Begin Source File

SOURCE=.\ThinDlg.h
# End Source File
# Begin Source File

SOURCE=.\ViewSizeDlg.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\BTR.ico
# End Source File
# Begin Source File

SOURCE=.\res\BTR.rc2
# End Source File
# Begin Source File

SOURCE=.\res\BTRDoc.ico
# End Source File
# Begin Source File

SOURCE=.\res\icon.ico
# End Source File
# Begin Source File

SOURCE=.\res\icon1.ico
# End Source File
# Begin Source File

SOURCE=.\res\Toolbar.bmp
# End Source File
# End Group
# Begin Source File

SOURCE=.\GasKstar.txt
# End Source File
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
