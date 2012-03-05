# Microsoft Developer Studio Project File - Name="SMOnline" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=SMOnline - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "SMOnline.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "SMOnline.mak" CFG="SMOnline - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "SMOnline - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "SMOnline - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "SMOnline - Win32 Release"

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
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /D "_WINDOWS" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:yes /map /machine:I386
# Begin Special Build Tool
TargetDir=.\Release
TargetName=SMOnline
SOURCE="$(InputPath)"
PostBuild_Cmds=Crash\windows\mapconv Release\$(TargetName).map $(TargetDir)\$(TargetName).vdi
# End Special Build Tool

!ELSEIF  "$(CFG)" == "SMOnline - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /D "_WINDOWS" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /map /machine:I386 /pdbtype:sept
# SUBTRACT LINK32 /debug
# Begin Special Build Tool
TargetDir=.\Debug
TargetName=SMOnline
SOURCE="$(InputPath)"
PostBuild_Cmds=Crash\windows\mapconv Debug\$(TargetName).map $(TargetDir)\$(TargetName).vdi
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "SMOnline - Win32 Release"
# Name "SMOnline - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\arch_setup.cpp
# End Source File
# Begin Source File

SOURCE=.\concursor.cpp
# End Source File
# Begin Source File

SOURCE=.\ConstructCount.cpp
# End Source File
# Begin Source File

SOURCE=.\ezsockets.cpp
# End Source File
# Begin Source File

SOURCE=.\ezThreads.cpp
# End Source File
# Begin Source File

SOURCE=.\JudgeWindow.cpp
# End Source File
# Begin Source File

SOURCE=.\MercuryString.cpp
# End Source File
# Begin Source File

SOURCE=.\PacketFunctions.cpp
# End Source File
# Begin Source File

SOURCE=.\RoundInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\SimpleLog.cpp
# End Source File
# Begin Source File

SOURCE=.\SMLanPlayers.cpp
# End Source File
# Begin Source File

SOURCE=.\smonline.cpp
# End Source File
# Begin Source File

SOURCE=.\SMOnlineClient.cpp
# End Source File
# Begin Source File

SOURCE=.\SMOnlineCommands.cpp
# End Source File
# Begin Source File

SOURCE=.\SMOnlineGameRoom.cpp
# End Source File
# Begin Source File

SOURCE=.\SMOnlineIni.cpp
# End Source File
# Begin Source File

SOURCE=.\SMOnlineRoom.cpp
# End Source File
# Begin Source File

SOURCE=.\SMOnlineServer.cpp
# End Source File
# Begin Source File

SOURCE=.\SMOnlineSQL.cpp
# End Source File
# Begin Source File

SOURCE=.\SMOnlineStats.cpp
# End Source File
# Begin Source File

SOURCE=.\SQLfunctions.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\arch_setup.h
# End Source File
# Begin Source File

SOURCE=.\concursor.h
# End Source File
# Begin Source File

SOURCE=.\ConstructCount.h
# End Source File
# Begin Source File

SOURCE=.\ezsockets.h
# End Source File
# Begin Source File

SOURCE=.\ezThreads.h
# End Source File
# Begin Source File

SOURCE=.\global.h
# End Source File
# Begin Source File

SOURCE=.\JudgeWindow.h
# End Source File
# Begin Source File

SOURCE=.\MercuryString.h
# End Source File
# Begin Source File

SOURCE=.\PacketFunctions.h
# End Source File
# Begin Source File

SOURCE=.\RoundInfo.h
# End Source File
# Begin Source File

SOURCE=.\SimpleLog.h
# End Source File
# Begin Source File

SOURCE=.\SMLanPlayers.h
# End Source File
# Begin Source File

SOURCE=.\SMOnlineClient.h
# End Source File
# Begin Source File

SOURCE=.\SMOnlineCommands.h
# End Source File
# Begin Source File

SOURCE=.\SMOnlineGameRoom.h
# End Source File
# Begin Source File

SOURCE=.\SMOnlineIni.h
# End Source File
# Begin Source File

SOURCE=.\SMOnlineRoom.h
# End Source File
# Begin Source File

SOURCE=.\SMOnlineServer.h
# End Source File
# Begin Source File

SOURCE=.\SMOnlineSQL.h
# End Source File
# Begin Source File

SOURCE=.\SMOnlineStats.h
# End Source File
# Begin Source File

SOURCE=.\SQLfunctions.h
# End Source File
# Begin Source File

SOURCE=.\StdString.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Group "Crash handler"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Crash\archCrash.h
# End Source File
# Begin Source File

SOURCE=.\Crash\Windows\BACKUPCrash.h
# End Source File
# Begin Source File

SOURCE=.\Crash\Windows\Crash.cpp
# End Source File
# Begin Source File

SOURCE=.\Crash\Windows\Crash.h
# End Source File
# Begin Source File

SOURCE=.\Crash\crashDefines.cpp
# End Source File
# Begin Source File

SOURCE=.\Crash\crashDefines.h
# End Source File
# Begin Source File

SOURCE=.\Crash\Windows\RestartProgram.cpp
# End Source File
# Begin Source File

SOURCE=.\Crash\Windows\RestartProgram.h
# End Source File
# Begin Source File

SOURCE=.\Crash\Windows\verstub.cpp
# End Source File
# Begin Source File

SOURCE=.\Crash\Windows\WindowsResources.h
# End Source File
# End Group
# Begin Group "ezSQL"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ezMySQL.cpp
# End Source File
# Begin Source File

SOURCE=.\ezMySQL.h
# End Source File
# Begin Source File

SOURCE=.\ezMySQLConstants.cpp
# End Source File
# Begin Source File

SOURCE=.\ezMySQLConstants.h
# End Source File
# Begin Source File

SOURCE=.\ezSQL.cpp
# End Source File
# Begin Source File

SOURCE=.\ezSQL.h
# End Source File
# Begin Source File

SOURCE=.\ezSQLConstants.h
# End Source File
# End Group
# End Target
# End Project
