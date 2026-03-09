;--------------------------------
;Include Modern UI
  !include "MUI2.nsh"

;--------------------------------
;Defines
!ifndef QLCPLUS_HOME
!define QLCPLUS_HOME "c:\projects\qlcplus"
!endif
!define MUI_ICON "${QLCPLUS_HOME}\resources\icons\qlcplus.ico"
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\nsis3-uninstall.ico"
!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_BITMAP "${NSISDIR}\Contrib\Graphics\Header\nsis3-vintage.bmp"
!define MUI_HEADERIMAGE_LEFT
!define MUI_PAGE_HEADER_TEXT "Imedia Group"

;--------------------------------
;General
Name "Imedia Group"
OutFile "Imedia_Group_Installer.exe"
InstallDir C:\immedia
InstallDirRegKey HKCU "Software\ImediaGroup" "Install_Dir"
RequestExecutionLevel user

!define MUI_LICENSEPAGE_TEXT_TOP "Do you accept the following statement of the Apache 2.0 license?"

!insertmacro MUI_PAGE_LICENSE "${QLCPLUS_HOME}\platforms\windows\apache_2.0.txt"

;--------------------------------
;Languages
!insertmacro MUI_LANGUAGE "English"
!insertmacro MUI_LANGUAGE "Italian"
!insertmacro MUI_LANGUAGE "German"
!insertmacro MUI_LANGUAGE "Spanish"
!insertmacro MUI_LANGUAGE "SpanishInternational"
!insertmacro MUI_LANGUAGE "Czech"
!insertmacro MUI_LANGUAGE "French"
!insertmacro MUI_LANGUAGE "Finnish"
!insertmacro MUI_LANGUAGE "Japanese"
!insertmacro MUI_LANGUAGE "Catalan"

;--------------------------------
; Pages
Page directory
Page custom StartMenuGroupSelect "" ": Start Menu Folder"
Page instfiles

Function StartMenuGroupSelect
	Push $R1

	StartMenu::Select /checknoshortcuts "Don't create a start menu folder" /autoadd /lastused $R0 "Imedia Group"
	Pop $R1

	StrCmp $R1 "success" success
	StrCmp $R1 "cancel" done
		; error
		MessageBox MB_OK $R1
		StrCpy $R0 "Imedia Group" # use default
		Return
	success:
	Pop $R0

	done:
	Pop $R1
FunctionEnd

Section
	SetOutPath $INSTDIR

	# this part is only necessary if you used /checknoshortcuts
	StrCpy $R1 $R0 1
	StrCmp $R1 ">" skip
		CreateDirectory $SMPROGRAMS\$R0
		CreateShortCut '$SMPROGRAMS\$R0\Imedia Group.lnk' $INSTDIR\ImediaViever.exe

		CreateDirectory $SMPROGRAMS\$R0
		CreateShortCut '$SMPROGRAMS\$R0\Fixture Definition Editor.lnk' $INSTDIR\qlcplus-fixtureeditor.exe

		CreateDirectory $SMPROGRAMS\$R0
		CreateShortCut '$SMPROGRAMS\$R0\Uninstall.lnk' $INSTDIR\uninstall.exe

	skip:
SectionEnd

Section
	File ImediaViever.exe
	File qlcplus-fixtureeditor.exe
	File *.dll
	File /r platforms
	File /r imageformats
	File /r mediaservice
	File /r audio
	File /r styles
	File Sample.qxw
	File *.qm
	File /r Fixtures
	File /r Gobos
	File /r InputProfiles
	File /r MidiTemplates
	File /r ModifiersTemplates
	File /r Plugins
	File /r RGBScripts
	File /r Web

	WriteRegStr HKCR ".qxf" "" "ImediaGroupFixture.Document"
	WriteRegStr HKCR "ImediaGroupFixture.Document" "" "Imedia Group Fixture"
	WriteRegStr HKCR "ImediaGroupFixture.Document\DefaultIcon" "" "$INSTDIR\qlcplus-fixtureeditor.exe,0"
	WriteRegStr HKCR "ImediaGroupFixture.Document\shell\open\command" "" '"$INSTDIR\qlcplus-fixtureeditor.exe" --open "%1"'

	WriteRegStr HKCU "SOFTWARE\ImediaGroup" "Install_Dir" "$INSTDIR"

	WriteUninstaller $INSTDIR\uninstall.exe
SectionEnd

;--------------------------------
; Uninstallation

UninstPage uninstConfirm
UninstPage instfiles
Section "Uninstall"
	Delete $INSTDIR\uninstall.exe
	Delete $INSTDIR\ImediaViever.exe
	Delete $INSTDIR\qlcplus-fixtureeditor.exe
	Delete $INSTDIR\*.dll
	RMDir /r $INSTDIR\platforms
        RMDir /r $INSTDIR\imageformats
	RMDir /r $INSTDIR\mediaservice
	RMDir /r $INSTDIR\audio
	RMDir /r $INSTDIR\styles
	Delete $INSTDIR\Sample.qxw
	Delete $INSTDIR\*.qm
	RMDir /r $INSTDIR\Fixtures
	RMDir /r $INSTDIR\Gobos
	RMDir /r $INSTDIR\InputProfiles
	RMDir /r $INSTDIR\MidiTemplates
	RMDir /r $INSTDIR\ModifiersTemplates
	RMDir /r $INSTDIR\Plugins
	RMDir /r $INSTDIR\RGBScripts
	RMDir /r $INSTDIR\Web

	RMDir $INSTDIR

	DeleteRegKey HKCR ".qxf"
	DeleteRegKey HKCR "ImediaGroupFixture.Document"

	; This will delete all settings
	DeleteRegKey HKCU "Software\ImediaGroup"
SectionEnd
