# Microsoft Developer Studio Project File - Name="StirMark" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=StirMark - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "StirMark.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "StirMark.mak" CFG="StirMark - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "StirMark - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "StirMark - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/StirMark", EFAAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "StirMark - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /Ob2 /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "__STDC__" /FR /YX /FD /c
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy Release\stirmark.exe Test\stirmark.exe	copy Release\stirmark.exe "E:\Shared\Research\Image marking\" 	copy Release\stirmark.exe "E:\Shared\Research\Experiments\"
# End Special Build Tool

!ELSEIF  "$(CFG)" == "StirMark - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W4 /Gm /GX /ZI /Od /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "__STDC__" /FR /YX /FD /c
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "StirMark - Win32 Release"
# Name "StirMark - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "*.c"
# Begin Source File

SOURCE=.\bench.c
# End Source File
# Begin Source File

SOURCE=.\error.c
# End Source File
# Begin Source File

SOURCE=.\image.c
# End Source File
# Begin Source File

SOURCE=.\lrattack.c
# End Source File
# Begin Source File

SOURCE=.\quality.c
# End Source File
# Begin Source File

SOURCE=.\quantise.c
# End Source File
# Begin Source File

SOURCE=.\reconstructers.c
# End Source File
# Begin Source File

SOURCE=.\resamplers.c
# End Source File
# Begin Source File

SOURCE=.\stirmark.c
# End Source File
# Begin Source File

SOURCE=.\transformations.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "*.h"
# Begin Source File

SOURCE=.\bench.h
# End Source File
# Begin Source File

SOURCE=.\error.h
# End Source File
# Begin Source File

SOURCE=.\image.h
# End Source File
# Begin Source File

SOURCE=.\lrattack.h
# End Source File
# Begin Source File

SOURCE=.\quality.h
# End Source File
# Begin Source File

SOURCE=.\quantise.h
# End Source File
# Begin Source File

SOURCE=.\reconstructers.h
# End Source File
# Begin Source File

SOURCE=.\resamplers.h
# End Source File
# Begin Source File

SOURCE=.\stirmark.h
# End Source File
# Begin Source File

SOURCE=.\transformations.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "*.rc"
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\Version.rc
# End Source File
# End Group
# Begin Group "Other files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Test\Attack.bat
# End Source File
# Begin Source File

SOURCE=.\Test\Benchmark.txt
# End Source File
# Begin Source File

SOURCE=.\Test\Detect.bat
# End Source File
# Begin Source File

SOURCE=.\Makefile
# End Source File
# Begin Source File

SOURCE=.\Test\PSNR.bat
# End Source File
# Begin Source File

SOURCE=.\Test\Sample.jpg
# End Source File
# Begin Source File

SOURCE=.\Test\Sample.pgm
# End Source File
# Begin Source File

SOURCE=.\Test\Sample.ppm
# End Source File
# Begin Source File

SOURCE=.\stirmark.1
# End Source File
# Begin Source File

SOURCE=.\Test\test.bat
# End Source File
# Begin Source File

SOURCE=.\Test\Watermark.bat
# End Source File
# End Group
# Begin Source File

SOURCE=.\Copyright.txt
# End Source File
# Begin Source File

SOURCE=.\README.txt
# End Source File
# End Target
# End Project
