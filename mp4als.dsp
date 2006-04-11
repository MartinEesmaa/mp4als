# Microsoft Developer Studio Project File - Name="mp4als" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** NICHT BEARBEITEN **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=mp4als - Win32 Debug
!MESSAGE Dies ist kein gültiges Makefile. Zum Erstellen dieses Projekts mit NMAKE
!MESSAGE verwenden Sie den Befehl "Makefile exportieren" und führen Sie den Befehl
!MESSAGE 
!MESSAGE NMAKE /f "mp4als.mak".
!MESSAGE 
!MESSAGE Sie können beim Ausführen von NMAKE eine Konfiguration angeben
!MESSAGE durch Definieren des Makros CFG in der Befehlszeile. Zum Beispiel:
!MESSAGE 
!MESSAGE NMAKE /f "mp4als.mak" CFG="mp4als - Win32 Debug"
!MESSAGE 
!MESSAGE Für die Konfiguration stehen zur Auswahl:
!MESSAGE 
!MESSAGE "mp4als - Win32 Release" (basierend auf  "Win32 (x86) Console Application")
!MESSAGE "mp4als - Win32 Debug" (basierend auf  "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "mp4als - Win32 Release"

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
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /FR /YX /FD /c
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib lib/lpc_adapt.obj /nologo /subsystem:console /machine:I386 /out:"bin/win/Release/mp4alsRM17.exe"
# SUBTRACT LINK32 /profile

!ELSEIF  "$(CFG)" == "mp4als - Win32 Debug"

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
# ADD CPP /nologo /W3 /Gm /GX /ZI /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x407 /d "_DEBUG"
# ADD RSC /l 0x407 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 libcd.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib lib\lpc_adapt.obj /nologo /subsystem:console /debug /machine:I386 /nodefaultlib /out:"bin/win/Debug/mp4alsRM16.exe" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "mp4als - Win32 Release"
# Name "mp4als - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\src\audiorw.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cmdline.cpp
# End Source File
# Begin Source File

SOURCE=.\src\crc.cpp
# End Source File
# Begin Source File

SOURCE=.\src\decoder.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ec.cpp
# End Source File
# Begin Source File

SOURCE=.\src\encoder.cpp
# End Source File
# Begin Source File

SOURCE=.\src\floating.cpp
# End Source File
# Begin Source File

SOURCE=.\src\lms.cpp
# End Source File
# Begin Source File

SOURCE=.\src\lpc.cpp
# End Source File
# Begin Source File

SOURCE=.\src\mcc.cpp
# End Source File
# Begin Source File

SOURCE=.\src\mlz.cpp
# End Source File
# Begin Source File

SOURCE=.\src\mp4als.cpp
# End Source File
# Begin Source File

SOURCE=.\src\rn_bitio.cpp
# End Source File
# Begin Source File

SOURCE=.\src\wave.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\src\audiorw.h
# End Source File
# Begin Source File

SOURCE=.\src\bitio.h
# End Source File
# Begin Source File

SOURCE=.\src\cmdline.h
# End Source File
# Begin Source File

SOURCE=.\src\crc.h
# End Source File
# Begin Source File

SOURCE=.\src\decoder.h
# End Source File
# Begin Source File

SOURCE=.\src\ec.h
# End Source File
# Begin Source File

SOURCE=.\src\encoder.h
# End Source File
# Begin Source File

SOURCE=.\src\floating.h
# End Source File
# Begin Source File

SOURCE=.\src\lms.h
# End Source File
# Begin Source File

SOURCE=.\src\lpc.h
# End Source File
# Begin Source File

SOURCE=.\src\lpc_adapt.h
# End Source File
# Begin Source File

SOURCE=.\src\mcc.h
# End Source File
# Begin Source File

SOURCE=.\src\mlz.h
# End Source File
# Begin Source File

SOURCE=.\src\rn_bitio.h
# End Source File
# Begin Source File

SOURCE=.\src\wave.h
# End Source File
# End Group
# End Target
# End Project
