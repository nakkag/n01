# Microsoft Developer Studio Project File - Name="n01" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** 編集しないでください **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=n01 - Win32 Debug
!MESSAGE これは有効なﾒｲｸﾌｧｲﾙではありません。 このﾌﾟﾛｼﾞｪｸﾄをﾋﾞﾙﾄﾞするためには NMAKE を使用してください。
!MESSAGE [ﾒｲｸﾌｧｲﾙのｴｸｽﾎﾟｰﾄ] ｺﾏﾝﾄﾞを使用して実行してください
!MESSAGE 
!MESSAGE NMAKE /f "n01.mak".
!MESSAGE 
!MESSAGE NMAKE の実行時に構成を指定できます
!MESSAGE ｺﾏﾝﾄﾞ ﾗｲﾝ上でﾏｸﾛの設定を定義します。例:
!MESSAGE 
!MESSAGE NMAKE /f "n01.mak" CFG="n01 - Win32 Debug"
!MESSAGE 
!MESSAGE 選択可能なﾋﾞﾙﾄﾞ ﾓｰﾄﾞ:
!MESSAGE 
!MESSAGE "n01 - Win32 Release" ("Win32 (x86) Application" 用)
!MESSAGE "n01 - Win32 Debug" ("Win32 (x86) Application" 用)
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "n01 - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /Ox /Og /Os /Op /Ob0 /Gf /Gy /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x411 /d "NDEBUG"
# ADD RSC /l 0x411 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 Comctl32.lib imm32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386 /opt:nowin98
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "n01 - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "UNICODE" /D "_UNICODE" /D "_DEBUG_NO_MSG" /U "UNICODE" /U "_UNICODE" /U "_DEBUG_NO_MSG" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x411 /d "_DEBUG"
# ADD RSC /l 0x411 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 Comctl32.lib imm32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "n01 - Win32 Release"
# Name "n01 - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\arrange.c
# End Source File
# Begin Source File

SOURCE=.\check_out.c
# End Source File
# Begin Source File

SOURCE=.\file.c
# End Source File
# Begin Source File

SOURCE=.\finish.c
# End Source File
# Begin Source File

SOURCE=.\Font.c
# End Source File
# Begin Source File

SOURCE=.\game_history.c
# End Source File
# Begin Source File

SOURCE=.\game_option.c
# End Source File
# Begin Source File

SOURCE=.\game_schedule.c
# End Source File
# Begin Source File

SOURCE=.\ini.c
# End Source File
# Begin Source File

SOURCE=.\main.c
# End Source File
# Begin Source File

SOURCE=.\Memory.c
# End Source File
# Begin Source File

SOURCE=.\Message.c
# End Source File
# Begin Source File

SOURCE=.\middle.c
# End Source File
# Begin Source File

SOURCE=.\n01.rc
# End Source File
# Begin Source File

SOURCE=.\nEdit.c
# End Source File
# Begin Source File

SOURCE=.\option.c
# End Source File
# Begin Source File

SOURCE=.\option_key.c
# End Source File
# Begin Source File

SOURCE=.\option_player.c
# End Source File
# Begin Source File

SOURCE=.\option_plugin.c
# End Source File
# Begin Source File

SOURCE=.\option_view.c
# End Source File
# Begin Source File

SOURCE=.\plugin.c
# End Source File
# Begin Source File

SOURCE=.\Profile.c
# End Source File
# Begin Source File

SOURCE=.\recovery.c
# End Source File
# Begin Source File

SOURCE=.\score_com.c
# End Source File
# Begin Source File

SOURCE=.\score_guide.c
# End Source File
# Begin Source File

SOURCE=.\score_info.c
# End Source File
# Begin Source File

SOURCE=.\score_left.c
# End Source File
# Begin Source File

SOURCE=.\score_list.c
# End Source File
# Begin Source File

SOURCE=.\score_name.c
# End Source File
# Begin Source File

SOURCE=.\score_player.c
# End Source File
# Begin Source File

SOURCE=.\score_save.c
# End Source File
# Begin Source File

SOURCE=.\shortcut.cpp
# End Source File
# Begin Source File

SOURCE=.\String.c
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\arrange.txt
# End Source File
# Begin Source File

SOURCE=.\res\arrange_com.txt
# End Source File
# Begin Source File

SOURCE=.\res\icon1.ico
# End Source File
# Begin Source File

SOURCE=.\res\manifest.xml
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\arrange.h
# End Source File
# Begin Source File

SOURCE=.\check_out.h
# End Source File
# Begin Source File

SOURCE=.\file.h
# End Source File
# Begin Source File

SOURCE=.\finish.h
# End Source File
# Begin Source File

SOURCE=.\Font.h
# End Source File
# Begin Source File

SOURCE=.\game_history.h
# End Source File
# Begin Source File

SOURCE=.\game_option.h
# End Source File
# Begin Source File

SOURCE=.\game_schedule.h
# End Source File
# Begin Source File

SOURCE=.\general.h
# End Source File
# Begin Source File

SOURCE=.\ini.h
# End Source File
# Begin Source File

SOURCE=.\Memory.h
# End Source File
# Begin Source File

SOURCE=.\Message.h
# End Source File
# Begin Source File

SOURCE=.\middle.h
# End Source File
# Begin Source File

SOURCE=.\nEdit.h
# End Source File
# Begin Source File

SOURCE=.\option.h
# End Source File
# Begin Source File

SOURCE=.\option_key.h
# End Source File
# Begin Source File

SOURCE=.\option_player.h
# End Source File
# Begin Source File

SOURCE=.\option_plugin.h
# End Source File
# Begin Source File

SOURCE=.\option_view.h
# End Source File
# Begin Source File

SOURCE=.\plugin.h
# End Source File
# Begin Source File

SOURCE=.\Profile.h
# End Source File
# Begin Source File

SOURCE=.\recovery.h
# End Source File
# Begin Source File

SOURCE=.\score_com.h
# End Source File
# Begin Source File

SOURCE=.\score_guide.h
# End Source File
# Begin Source File

SOURCE=.\score_info.h
# End Source File
# Begin Source File

SOURCE=.\score_left.h
# End Source File
# Begin Source File

SOURCE=.\score_list.h
# End Source File
# Begin Source File

SOURCE=.\score_name.h
# End Source File
# Begin Source File

SOURCE=.\score_player.h
# End Source File
# Begin Source File

SOURCE=.\score_save.h
# End Source File
# Begin Source File

SOURCE=.\shortcut.h
# End Source File
# Begin Source File

SOURCE=.\String.h
# End Source File
# End Group
# End Target
# End Project
