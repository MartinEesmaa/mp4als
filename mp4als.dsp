# Microsoft Developer Studio Project File - Name="mp4als" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** 編集しないでください **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=mp4als - Win32 Debug
!MESSAGE これは有効なﾒｲｸﾌｧｲﾙではありません。 このﾌﾟﾛｼﾞｪｸﾄをﾋﾞﾙﾄﾞするためには NMAKE を使用してください。
!MESSAGE [ﾒｲｸﾌｧｲﾙのｴｸｽﾎﾟｰﾄ] ｺﾏﾝﾄﾞを使用して実行してください
!MESSAGE 
!MESSAGE NMAKE /f "mp4als.mak".
!MESSAGE 
!MESSAGE NMAKE の実行時に構成を指定できます
!MESSAGE ｺﾏﾝﾄﾞ ﾗｲﾝ上でﾏｸﾛの設定を定義します。例:
!MESSAGE 
!MESSAGE NMAKE /f "mp4als.mak" CFG="mp4als - Win32 Debug"
!MESSAGE 
!MESSAGE 選択可能なﾋﾞﾙﾄﾞ ﾓｰﾄﾞ:
!MESSAGE 
!MESSAGE "mp4als - Win32 Release" ("Win32 (x86) Console Application" 用)
!MESSAGE "mp4als - Win32 Debug" ("Win32 (x86) Console Application" 用)
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
# ADD CPP /nologo /W3 /GX /O2 /I "src/AlsImf" /I "src/AlsImf/Mp4" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /D "WARN_BUFFERSIZEDB_OVER_24BIT" /D "PERMIT_SAMPLERATE_OVER_16BIT" /FR /YX /FD /c
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib lib/win/lpc_adapt.obj /nologo /subsystem:console /machine:I386 /out:"bin/win/Release/mp4alsRM18.exe"
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
# ADD CPP /nologo /W3 /Gm /GX /ZI /I "src/AlsImf" /I "src/AlsImf/Mp4" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /D "WARN_BUFFERSIZEDB_OVER_24BIT" /D "PERMIT_SAMPLERATE_OVER_16BIT" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x407 /d "_DEBUG"
# ADD RSC /l 0x407 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib lib/win/lpc_adapt.obj /nologo /subsystem:console /debug /machine:I386 /nodefaultlib:"libc.lib" /out:"bin/win/Debug/mp4alsRM18.exe" /pdbtype:sept
# SUBTRACT LINK32 /nodefaultlib

!ENDIF 

# Begin Target

# Name "mp4als - Win32 Release"
# Name "mp4als - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\src\als2mp4.cpp
# End Source File
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

SOURCE=.\src\als2mp4.h
# End Source File
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
# Begin Group "AlsImf"

# PROP Default_Filter ""
# Begin Group "Mp4"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\AlsImf\Mp4\Mp4aFile.cpp
# End Source File
# Begin Source File

SOURCE=.\src\AlsImf\Mp4\Mp4aFile.h
# End Source File
# Begin Source File

SOURCE=.\src\AlsImf\Mp4\Mp4Box.cpp
# End Source File
# Begin Source File

SOURCE=.\src\AlsImf\Mp4\Mp4Box.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\src\AlsImf\ImfBox.cpp
# End Source File
# Begin Source File

SOURCE=.\src\AlsImf\ImfBox.h
# End Source File
# Begin Source File

SOURCE=.\src\AlsImf\ImfBoxAmd1.cpp
# End Source File
# Begin Source File

SOURCE=.\src\AlsImf\ImfBoxAmd1.h
# End Source File
# Begin Source File

SOURCE=.\src\AlsImf\ImfDescriptor.cpp
# End Source File
# Begin Source File

SOURCE=.\src\AlsImf\ImfDescriptor.h
# End Source File
# Begin Source File

SOURCE=.\src\AlsImf\ImfFileStream.cpp
# End Source File
# Begin Source File

SOURCE=.\src\AlsImf\ImfFileStream.h
# End Source File
# Begin Source File

SOURCE=.\src\AlsImf\ImfSampleEntry.cpp
# End Source File
# Begin Source File

SOURCE=.\src\AlsImf\ImfSampleEntry.h
# End Source File
# Begin Source File

SOURCE=.\src\AlsImf\ImfStream.h
# End Source File
# Begin Source File

SOURCE=.\src\AlsImf\ImfType.h
# End Source File
# End Group
# End Target
# End Project
