# Microsoft Developer Studio Project File - Name="ImageNotifyDll" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) External Target" 0x0106

CFG=ImageNotifyDll - Win32 XP Checked
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ImageNotifyDll.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ImageNotifyDll.mak" CFG="ImageNotifyDll - Win32 XP Checked"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ImageNotifyDll - Win32 XP Free" (based on "Win32 (x86) External Target")
!MESSAGE "ImageNotifyDll - Win32 XP Checked" (based on "Win32 (x86) External Target")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""

!IF  "$(CFG)" == "ImageNotifyDll - Win32 XP Free"

# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ""
# PROP Intermediate_Dir ""
# PROP Cmd_Line "ddkbuild -WNETXP free ."
# PROP Rebuild_Opt "-cZ"
# PROP Target_File "ImageNotifyDll.sys"
# PROP Bsc_Name ""
# PROP Target_Dir ""

!ELSEIF  "$(CFG)" == "ImageNotifyDll - Win32 XP Checked"

# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ""
# PROP Intermediate_Dir ""
# PROP Cmd_Line "ddkbuild -WNETXP checked ."
# PROP Rebuild_Opt "-cZ"
# PROP Target_File "ImageNotifyDll.sys"
# PROP Bsc_Name ""
# PROP Target_Dir ""

!ENDIF 

# Begin Target

# Name "ImageNotifyDll - Win32 XP Free"
# Name "ImageNotifyDll - Win32 XP Checked"

!IF  "$(CFG)" == "ImageNotifyDll - Win32 XP Free"

!ELSEIF  "$(CFG)" == "ImageNotifyDll - Win32 XP Checked"

!ENDIF 

# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\ImageNotifyDll.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\ImageNotifyDll.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Group "DoWork"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\DoWork.c
# End Source File
# Begin Source File

SOURCE=.\DoWork.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\sources
# End Source File
# End Target
# End Project
