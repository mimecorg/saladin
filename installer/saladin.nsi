/**************************************************************************
* This file is part of the Saladin program
* Copyright (C) 2011-2017 Michał Męciński
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
**************************************************************************/

!define SRCDIR ".."
!define BUILDDIR "..\release"

!define UNINST_KEY "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\Saladin"

Unicode true

!include "MUI2.nsh"

!ifdef INNER
    SetCompress off

    OutFile "$%TEMP%\innerinst.exe"
!else
    !verbose 4

    SetCompressor /SOLID lzma
    SetCompressorDictSize 32

    OutFile "saladin-${VERSION}-${ARCHITECTURE}.exe"
!endif

!define MULTIUSER_EXECUTIONLEVEL "Highest"
!define MULTIUSER_MUI
!define MULTIUSER_INSTALLMODE_COMMANDLINE
!define MULTIUSER_INSTALLMODE_DEFAULT_REGISTRY_KEY "${UNINST_KEY}"
!define MULTIUSER_INSTALLMODE_DEFAULT_REGISTRY_VALUENAME "UninstallString"
!define MULTIUSER_INSTALLMODE_INSTDIR "Saladin"
!define MULTIUSER_INSTALLMODE_INSTDIR_REGISTRY_KEY "${UNINST_KEY}"
!define MULTIUSER_INSTALLMODE_INSTDIR_REGISTRY_VALUENAME "InstallLocation"
!if ${ARCHITECTURE} == "win_x64"
  !define MULTIUSER_USE_PROGRAMFILES64
!endif
!ifndef INNER
    !define MULTIUSER_NOUNINSTALL
!endif
!include "include\multiuser64.nsh"

!include "WinVer.nsh"

Name "$(NAME)"

!define MUI_ICON "${SRCDIR}\src\saladin.ico"
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\modern-uninstall-blue.ico"

!define MUI_WELCOMEFINISHPAGE_BITMAP "images\wizard.bmp"
!define MUI_UNWELCOMEFINISHPAGE_BITMAP "images\wizard.bmp"

!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_BITMAP "images\header.bmp"
!define MUI_HEADERIMAGE_RIGHT

!define MUI_WELCOMEPAGE_TITLE "$(TITLE)"
!define MUI_WELCOMEPAGE_TEXT "$(WELCOME_TEXT)"
!insertmacro MUI_PAGE_WELCOME

!define MUI_LICENSEPAGE_CHECKBOX
!insertmacro MUI_PAGE_LICENSE "${SRCDIR}\COPYING"

!insertmacro MULTIUSER_PAGE_INSTALLMODE

!insertmacro MUI_PAGE_DIRECTORY

ShowInstDetails nevershow
!insertmacro MUI_PAGE_INSTFILES

!define MUI_FINISHPAGE_TITLE "$(TITLE)"
!insertmacro MUI_PAGE_FINISH
  
!ifdef INNER
    !define MUI_WELCOMEPAGE_TITLE "$(TITLE)"
    !insertmacro MUI_UNPAGE_WELCOME

    !insertmacro MUI_UNPAGE_CONFIRM

    ShowUninstDetails nevershow
    !insertmacro MUI_UNPAGE_INSTFILES

    !define MUI_FINISHPAGE_TITLE "$(TITLE)"
    !insertmacro MUI_UNPAGE_FINISH
!endif

!insertmacro MUI_LANGUAGE "English"

!include "languages\saladin_en.nsh"

VIProductVersion "${BUILDVERSION}"
VIAddVersionKey /LANG=${LANG_ENGLISH} "CompanyName" "Michał Męciński"
VIAddVersionKey /LANG=${LANG_ENGLISH} "FileDescription" "Saladin Setup"
VIAddVersionKey /LANG=${LANG_ENGLISH} "FileVersion" "${VERSION}"
VIAddVersionKey /LANG=${LANG_ENGLISH} "LegalCopyright" "Copyright (C) 2011-2017 Michał Męciński"
VIAddVersionKey /LANG=${LANG_ENGLISH} "OriginalFilename" "saladin-${VERSION}-${ARCHITECTURE}.exe"
VIAddVersionKey /LANG=${LANG_ENGLISH} "ProductName" "Saladin"
VIAddVersionKey /LANG=${LANG_ENGLISH} "ProductVersion" "${VERSION}"

Function .onInit

!ifndef INNER
    ${Unless} ${AtLeastWinVista}
        MessageBox MB_ICONEXCLAMATION|MB_OK "$(WINVER_TEXT)"
        Abort
    ${EndIf}
!endif

!if ${ARCHITECTURE} == "win_x64"
    SetRegView 64
!endif

!ifdef INNER
    WriteUninstaller "$%TEMP%\uninstall.exe"
    Quit
!endif

    !insertmacro MULTIUSER_INIT

FunctionEnd

Section

    SetOutPath "$INSTDIR"

    File "${SRCDIR}\ChangeLog"
    File "${SRCDIR}\COPYING"
    File "${SRCDIR}\README"

    SetOutPath "$INSTDIR\bin"

    Delete "$INSTDIR\bin\*.*"

    File "${BUILDDIR}\saladin.exe"

    File "qt.conf"

    File "${QTDIR}\bin\Qt5Core.dll"
    File "${QTDIR}\bin\Qt5Gui.dll"
    File "${QTDIR}\bin\Qt5Network.dll"
    File "${QTDIR}\bin\Qt5Svg.dll"
    File "${QTDIR}\bin\Qt5Widgets.dll"
    File "${QTDIR}\bin\Qt5WinExtras.dll"
    File "${QTDIR}\bin\Qt5Xml.dll"

    File "${OPENSSLDIR}\bin\libeay32.dll"
    File "${OPENSSLDIR}\bin\ssleay32.dll"

    File "${VCRTDIR}\msvcp100.dll"
    File "${VCRTDIR}\msvcr100.dll"

    SetOutPath "$INSTDIR\plugins\platforms"

    File "${QTDIR}\plugins\platforms\qwindows.dll"

    SetOutPath "$INSTDIR\plugins\imageformats"

    File "${QTDIR}\plugins\imageformats\qdds.dll"
    File "${QTDIR}\plugins\imageformats\qgif.dll"
    File "${QTDIR}\plugins\imageformats\qicns.dll"
    File "${QTDIR}\plugins\imageformats\qico.dll"
    File "${QTDIR}\plugins\imageformats\qjp2.dll"
    File "${QTDIR}\plugins\imageformats\qjpeg.dll"
    File "${QTDIR}\plugins\imageformats\qmng.dll"
    File "${QTDIR}\plugins\imageformats\qsvg.dll"
    File "${QTDIR}\plugins\imageformats\qtga.dll"
    File "${QTDIR}\plugins\imageformats\qtiff.dll"
    File "${QTDIR}\plugins\imageformats\qwbmp.dll"
    File "${QTDIR}\plugins\imageformats\qwebp.dll"

    Delete "$INSTDIR\doc\*.*"

    SetOutPath "$INSTDIR\translations"

    Delete "$INSTDIR\translations\*.*"

    File "${SRCDIR}\translations\locale.ini"

    File "${SRCDIR}\translations\saladin_de.qm"
    File "${SRCDIR}\translations\saladin_es.qm"
    File "${SRCDIR}\translations\saladin_pl.qm"
    File "${SRCDIR}\translations\saladin_pt_BR.qm"
    File "${SRCDIR}\translations\saladin_ru.qm"

    File "${QTDIR}\translations\qt_de.qm"
    File "${QTDIR}\translations\qt_es.qm"
    File "${QTDIR}\translations\qt_pl.qm"
    File "${QTDIR}\translations\qt_pt.qm"
    File "${QTDIR}\translations\qt_ru.qm"

    File "${QTDIR}\translations\qtbase_de.qm"
    ;File "${QTDIR}\translations\qtbase_es.qm"
    ;File "${QTDIR}\translations\qtbase_pl.qm"
    ;File "${QTDIR}\translations\qtbase_pt.qm"
    File "${QTDIR}\translations\qtbase_ru.qm"

    SetOutPath "$INSTDIR\bin"

    CreateShortCut "$SMPROGRAMS\Saladin.lnk" "$INSTDIR\bin\saladin.exe"
    CreateShortCut "$DESKTOP\Saladin.lnk" "$INSTDIR\bin\saladin.exe"

    WriteRegStr SHCTX "${UNINST_KEY}" "DisplayIcon" '"$INSTDIR\bin\saladin.exe"'
    WriteRegStr SHCTX "${UNINST_KEY}" "DisplayName" "Saladin ${VERSION}${SUFFIX}"
    WriteRegStr SHCTX "${UNINST_KEY}" "DisplayVersion" "${VERSION}"
    WriteRegStr SHCTX "${UNINST_KEY}" "UninstallString" '"$INSTDIR\uninstall.exe" /$MultiUser.InstallMode'
    WriteRegStr SHCTX "${UNINST_KEY}" "InstallLocation" "$INSTDIR"
    WriteRegStr SHCTX "${UNINST_KEY}" "Publisher" "Michał Męciński"
    WriteRegStr SHCTX "${UNINST_KEY}" "HelpLink" "http://saladin.mimec.org"
    WriteRegStr SHCTX "${UNINST_KEY}" "URLInfoAbout" "http://saladin.mimec.org"
    WriteRegStr SHCTX "${UNINST_KEY}" "URLUpdateInfo" "http://saladin.mimec.org/downloads"
    WriteRegDWORD SHCTX "${UNINST_KEY}" "NoModify" 1
    WriteRegDWORD SHCTX "${UNINST_KEY}" "NoRepair" 1

!ifndef INNER
    SetOutPath "$INSTDIR"
    File "$%TEMP%\uninstall.exe"
!endif

SectionEnd

!ifdef INNER

Function un.onInit

!if ${ARCHITECTURE} == "win_x64"
    SetRegView 64
!endif

    !insertmacro MULTIUSER_UNINIT

FunctionEnd

Section "Uninstall"

    DeleteRegKey SHCTX "${UNINST_KEY}"

    Delete "$SMPROGRAMS\Saladin.lnk"
    Delete "$DESKTOP\Saladin.lnk"

    Delete "$INSTDIR\ChangeLog"
    Delete "$INSTDIR\COPYING"
    Delete "$INSTDIR\README"
    RMDir /r "$INSTDIR\bin"
    RMDir /r "$INSTDIR\plugins"
    RMDir /r "$INSTDIR\doc"
    RMDir /r "$INSTDIR\translations"
    Delete "$INSTDIR\uninstall.exe"
    RMDir "$INSTDIR"

SectionEnd

!endif
