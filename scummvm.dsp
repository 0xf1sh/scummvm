# Microsoft Developer Studio Project File - Name="scummvm" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=scummvm - Win32 MP3 Enabled Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "scummvm.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "scummvm.mak" CFG="scummvm - Win32 MP3 Enabled Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "scummvm - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "scummvm - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE "scummvm - Win32 MP3 Enabled Debug" (based on "Win32 (x86) Console Application")
!MESSAGE "scummvm - Win32 MP3 Enabled Release" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "scummvm - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "scummvm___Release"
# PROP Intermediate_Dir "scummvm___Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /G6 /MD /W3 /O2 /Ob2 /I "." /I "sound" /I "common" /I "scumm" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /D "USE_ADLIB" /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE RSC /l 0x41d /d "NDEBUG"
# ADD RSC /l 0x41d /fo"scummvm.res" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib sdl.lib winmm.lib wsock32.lib simon___Release\simon.lib scumm___Release\scumm.lib /nologo /subsystem:console /machine:I386 /nodefaultlib:"libc"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "scummvm - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "scummvm___Debug"
# PROP Intermediate_Dir "scummvm___Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /I "." /I "sound" /I "common" /I "scumm" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /D "ALLOW_GDI" /D "BYPASS_COPY_PROT" /D "DUMP_SCRIPTS" /YX /FD /GZ /c
# SUBTRACT CPP /Fr
# ADD BASE RSC /l 0x41d /d "_DEBUG"
# ADD RSC /l 0x41d /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib sdl.lib winmm.lib wsock32.lib simon___Debug\simon.lib scumm___Debug\scumm.lib /nologo /subsystem:console /debug /machine:I386 /nodefaultlib:"libc" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "scummvm - Win32 MP3 Enabled Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "scummvm___Win32_MP3_Enabled_Debug"
# PROP BASE Intermediate_Dir "scummvm___Win32_MP3_Enabled_Debug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "scummvm___MP3_Enabled_Debug"
# PROP Intermediate_Dir "scummvm___MP3_Enabled_Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /I "./sound" /I "./" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /D "ALLOW_GDI" /D "BYPASS_COPY_PROT" /D "USE_ADLIB" /D "DUMP_SCRIPTS" /D "COMPRESSED_SOUND_FILE" /Yu"stdafx.h" /FD /GZ /c
# SUBTRACT BASE CPP /Fr
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /I "." /I "sound" /I "common" /I "scumm" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /D "ALLOW_GDI" /D "BYPASS_COPY_PROT" /D "USE_ADLIB" /D "DUMP_SCRIPTS" /D "COMPRESSED_SOUND_FILE" /YX /FD /GZ /c
# SUBTRACT CPP /Fr
# ADD BASE RSC /l 0x41d /d "_DEBUG"
# ADD RSC /l 0x41d /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib sdl.lib winmm.lib libmad.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib sdl.lib winmm.lib libmad.lib wsock32.lib scumm___MP3_Enabled_Debug\scumm.lib simon___MP3_Enabled_Debug\simon.lib /nologo /subsystem:console /debug /machine:I386 /nodefaultlib:"libc" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none /incremental:no

!ELSEIF  "$(CFG)" == "scummvm - Win32 MP3 Enabled Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "scummvm___Win32_MP3_Enabled_Release"
# PROP BASE Intermediate_Dir "scummvm___Win32_MP3_Enabled_Release"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "scummvm___MP3_Enabled_Release"
# PROP Intermediate_Dir "scummvm___MP3_Enabled_Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G6 /MD /W3 /O2 /Ob2 /I "." /I "sound" /I "common" /I "scumm" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /D "USE_ADLIB" /D "COMPRESSED_SOUND_FILE" /FD /c
# SUBTRACT BASE CPP /YX /Yc /Yu
# ADD CPP /nologo /G6 /MD /W3 /O2 /Ob2 /I "." /I "sound" /I "common" /I "scumm" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /D "USE_ADLIB" /D "COMPRESSED_SOUND_FILE" /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE RSC /l 0x41d /d "NDEBUG"
# ADD RSC /l 0x41d /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib sdl.lib winmm.lib wsock32.lib libmad.lib simon___Release\simon.lib scumm___Release\scumm.lib sdl.lib /nologo /subsystem:console /machine:I386 /nodefaultlib
# ADD LINK32 kernel32.lib user32.lib gdi32.lib sdl.lib winmm.lib wsock32.lib libmad.lib scumm___MP3_Enabled_Release\scumm.lib simon___MP3_Enabled_Release\simon.lib /nologo /subsystem:console /incremental:yes /machine:I386 /nodefaultlib:"libcd" /nodefaultlib:"libc"
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "scummvm - Win32 Release"
# Name "scummvm - Win32 Debug"
# Name "scummvm - Win32 MP3 Enabled Debug"
# Name "scummvm - Win32 MP3 Enabled Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "common"

# PROP Default_Filter ""
# Begin Source File

SOURCE=".\common\config-file.cpp"
# End Source File
# Begin Source File

SOURCE=".\common\config-file.h"
# End Source File
# Begin Source File

SOURCE=.\common\engine.cpp
# End Source File
# Begin Source File

SOURCE=.\common\engine.h
# End Source File
# Begin Source File

SOURCE=.\common\file.cpp
# End Source File
# Begin Source File

SOURCE=.\common\file.h
# End Source File
# Begin Source File

SOURCE=.\common\gameDetector.cpp
# End Source File
# Begin Source File

SOURCE=.\common\gameDetector.h
# End Source File
# Begin Source File

SOURCE=.\common\main.cpp
# End Source File
# Begin Source File

SOURCE=.\common\scaler.cpp
# End Source File
# Begin Source File

SOURCE=.\common\scaler.h
# End Source File
# Begin Source File

SOURCE=.\common\scummsys.h
# End Source File
# Begin Source File

SOURCE=.\common\stdafx.cpp
# End Source File
# Begin Source File

SOURCE=.\common\stdafx.h
# End Source File
# Begin Source File

SOURCE=.\common\system.h
# End Source File
# Begin Source File

SOURCE=.\common\timer.cpp
# End Source File
# Begin Source File

SOURCE=.\common\timer.h
# End Source File
# Begin Source File

SOURCE=.\common\util.cpp
# End Source File
# Begin Source File

SOURCE=.\common\util.h
# End Source File
# End Group
# Begin Group "sound"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\sound\fmopl.cpp
# End Source File
# Begin Source File

SOURCE=.\sound\fmopl.h
# End Source File
# Begin Source File

SOURCE=.\sound\mididrv.cpp
# End Source File
# Begin Source File

SOURCE=.\sound\mididrv.h
# End Source File
# Begin Source File

SOURCE=.\sound\mixer.cpp
# End Source File
# Begin Source File

SOURCE=.\sound\mixer.h
# End Source File
# End Group
# Begin Group "gui"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\gui\dialog.cpp
# End Source File
# Begin Source File

SOURCE=.\gui\dialog.h
# End Source File
# Begin Source File

SOURCE=.\gui\gui.cpp
# End Source File
# Begin Source File

SOURCE=.\gui\gui.h
# End Source File
# Begin Source File

SOURCE=.\gui\guimaps.h
# End Source File
# Begin Source File

SOURCE=.\gui\ListWidget.cpp
# End Source File
# Begin Source File

SOURCE=.\gui\ListWidget.h
# End Source File
# Begin Source File

SOURCE=.\gui\newgui.cpp
# End Source File
# Begin Source File

SOURCE=.\gui\newgui.h
# End Source File
# Begin Source File

SOURCE=.\gui\ScrollBarWidget.cpp
# End Source File
# Begin Source File

SOURCE=.\gui\ScrollBarWidget.h
# End Source File
# Begin Source File

SOURCE=.\gui\widget.cpp
# End Source File
# Begin Source File

SOURCE=.\gui\widget.h
# End Source File
# End Group
# Begin Group "backends"

# PROP Default_Filter ""
# Begin Group "sdl"

# PROP Default_Filter ""
# Begin Source File

SOURCE=".\backends\sdl\sdl-common.cpp"
# End Source File
# Begin Source File

SOURCE=".\backends\sdl\sdl-common.h"
# End Source File
# Begin Source File

SOURCE=.\backends\sdl\sdl.cpp
# End Source File
# End Group
# End Group
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# Begin Source File

SOURCE=.\scummvm.ico
# End Source File
# Begin Source File

SOURCE=.\scummvm.rc
# End Source File
# End Target
# End Project
