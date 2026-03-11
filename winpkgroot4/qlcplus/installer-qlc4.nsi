;--------------------------------
;Include Modern UI
  !include "MUI2.nsh"
  !include "nsDialogs.nsh"
  !include "LogicLib.nsh"

;--------------------------------
;Defines
!define QLCPLUS_HOME "/var/www/html/QVrc/winpkgroot4/qlcplus"
!define MUI_ICON "/var/www/html/QVrc/ico/imedia_icon.ico"
!define MUI_UNICON "/var/www/html/QVrc/ico/imedia_icon.ico"
!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_BITMAP "${NSISDIR}\Contrib\Graphics\Header\nsis3-vintage.bmp"
!define MUI_HEADERIMAGE_LEFT
!define MUI_PAGE_HEADER_TEXT "Imedia Group"

;--------------------------------
;General
Name "Imedia Group"
OutFile "Imedia_Group_Installer.exe"
InstallDir C:\Imedia
InstallDirRegKey HKCU "Software\ImediaGroup" "Install_Dir"
RequestExecutionLevel user
Var DesktopShortcut
Var Dialog
Var CheckboxDesktop

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
Page custom DesktopShortcutPageCreate DesktopShortcutPageLeave ": Desktop Shortcut"
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

Function DesktopShortcutPageCreate
    nsDialogs::Create 1018
    Pop $Dialog
    ${If} $Dialog == error
        Abort
    ${EndIf}

    ${NSD_CreateLabel} 0u 0u 100% 18u "Choose whether to create a desktop shortcut."
    Pop $0
    ${NSD_CreateCheckbox} 0u 24u 100% 12u "Create a desktop shortcut for Imedia Group"
    Pop $CheckboxDesktop
    ${NSD_Check} $CheckboxDesktop

    nsDialogs::Show
FunctionEnd

Function DesktopShortcutPageLeave
    ${NSD_GetState} $CheckboxDesktop $DesktopShortcut
FunctionEnd

Section
	SetOutPath $INSTDIR

	# this part is only necessary if you used /checknoshortcuts
	StrCpy $R1 $R0 1
	StrCmp $R1 ">" skip
		CreateDirectory $SMPROGRAMS\$R0
		CreateShortCut '$SMPROGRAMS\$R0\Imedia Group.lnk' $INSTDIR\ImediaViever.exe

		CreateDirectory $SMPROGRAMS\$R0
		CreateShortCut '$SMPROGRAMS\$R0\Uninstall.lnk' $INSTDIR\uninstall.exe

	skip:

	StrCmp $DesktopShortcut ${BST_CHECKED} 0 noDesktopShortcut
		CreateShortCut '$DESKTOP\Imedia Group.lnk' $INSTDIR\ImediaViever.exe
	noDesktopShortcut:
SectionEnd

Section
	File ImediaViever.exe
	File *.dll
	Delete $INSTDIR\Plugins\usbdmx.dll
	Delete $INSTDIR\Plugins\udmx.dll
	File /r platforms
	File /r iconengines
	File /r imageformats
	File /r qml
	File /r Meshes
	File /r geometryloaders
	File /r multimedia
	File /r renderers
	File /r sceneparsers
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

	WriteRegStr HKCR ".qxw" "" "ImediaGroup.Document"
	WriteRegStr HKCR "ImediaGroup.Document" "" "Imedia Group Workspace"
	WriteRegStr HKCR "ImediaGroup.Document\DefaultIcon" "" "$INSTDIR\ImediaViever.exe,0"
	WriteRegStr HKCR "ImediaGroup.Document\shell\open\command" "" '"$INSTDIR\ImediaViever.exe" --open "%1"'

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
	Delete $INSTDIR\*.dll
	Delete $INSTDIR\Plugins\usbdmx.dll
	Delete $INSTDIR\Plugins\udmx.dll
	RMDir /r $INSTDIR\platforms
	RMDir /r $INSTDIR\iconengines
	RMDir /r $INSTDIR\imageformats
	RMDir /r $INSTDIR\qml
	RMDir /r $INSTDIR\Meshes
	RMDir /r $INSTDIR\geometryloaders
	RMDir /r $INSTDIR\multimedia
	RMDir /r $INSTDIR\renderers
	RMDir /r $INSTDIR\sceneparsers
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
	Delete "$DESKTOP\Imedia Group.lnk"

	DeleteRegKey HKCR ".qxw"
	DeleteRegKey HKCR "ImediaGroup.Document"

	; This will delete all settings
	DeleteRegKey HKCU "Software\ImediaGroup"
SectionEnd
