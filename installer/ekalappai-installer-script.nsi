#eKalappai installer creation script

Name "eKalappai"
OutFile "eKalappai-3.0-beta2-dev-installer.exe"

RequestExecutionLevel admin

LicenseText "You must agree to this license before installing."
LicenseData "license.txt"

InstallDir "$PROGRAMFILES\ekalappai"
InstallDirRegKey HKEY_LOCAL_MACHINE "SOFTWARE\eKalappai" ""

DirText "Select the directory to install eKalappai in:"

Section "" ; (default section)
SetShellVarContext all
SetOutPath "$INSTDIR"

#Installed files list
File  "app\ekalappai.exe"
File  "app\ekhook.dll"
File  "app\libgcc_s_dw2-1.dll"
File  "app\mingwm10.dll"
File  "app\QtCore4.dll"
File  "app\QtGui4.dll"
File  "app\settings.ini"

#Registry settings
WriteRegStr HKEY_LOCAL_MACHINE "SOFTWARE\eKalappai" "" "$INSTDIR"
WriteRegStr HKEY_LOCAL_MACHINE "Software\Microsoft\Windows\CurrentVersion\Uninstall\eKalappai" "DisplayName" "eKalappai (remove only)"
WriteRegStr HKEY_LOCAL_MACHINE "Software\Microsoft\Windows\CurrentVersion\Uninstall\eKalappai" "UninstallString" '"$INSTDIR\ekuninstall.exe"'

#write out uninstaller
WriteUninstaller "$INSTDIR\ekuninstall.exe"

#Create a shortcuts in the start menu programs directory
CreateDirectory "$SMPROGRAMS\eKalappai"
createShortCut "$SMPROGRAMS\eKalappai\eKalappai.lnk" "$INSTDIR\ekalappai.exe"
createShortCut "$SMPROGRAMS\eKalappai\Uninstall eKalappai.lnk" "$INSTDIR\ekuninstall.exe"

#Desktop shortcut
createShortCut "$DESKTOP\eKalappai.lnk" "$INSTDIR\ekalappai.exe"

#Open an output file called "install.log" on the install directory and write installation info on it when the installation happens.
fileOpen $0 "$INSTDIR\install.log" w
 
fileWrite $0 "Installed version: 3.0-beta2-dev$\r$\n"
fileWrite $0 "Installed Date: $\r$\n"
 
#Close the file
fileClose $0

SectionEnd ; end of default section


#Begin uninstall settings/section
UninstallText "This will uninstall eKalappai from your system"

Section Uninstall
SetShellVarContext all

#Uninstall files & folders
Delete "$INSTDIR\ekuninstall.exe"
Delete "$INSTDIR\ekalappai.exe"
Delete "$INSTDIR\ekhook.dll"
Delete "$INSTDIR\libgcc_s_dw2-1.dll"
Delete "$INSTDIR\mingwm10.dll"
Delete "$INSTDIR\QtCore4.dll"
Delete "$INSTDIR\QtGui4.dll"
Delete "$INSTDIR\install.log"
Delete "$INSTDIR\settings.ini"
RMDir $INSTDIR

#Uninstall shortcuts
Delete "$SMPROGRAMS\eKalappai\eKalappai.lnk"
Delete "$SMPROGRAMS\eKalappai\Uninstall eKalappai.lnk"
RMDir "$SMPROGRAMS\eKalappai"
Delete "$DESKTOP\eKalappai.lnk"

#Delete registry keys
DeleteRegKey HKEY_LOCAL_MACHINE "SOFTWARE\eKalappai"
DeleteRegKey HKEY_LOCAL_MACHINE "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\eKalappai"
RMDir "$INSTDIR"
SectionEnd ; end of uninstall section


/* Replace the values of the two defines below to your application's window class and window title, respectivelly. */
!define WNDCLASS "QWidget"
!define WNDTITLE "eKalappai 3.0b1"
 
Function un.onInit
  FindWindow $0 "${WNDCLASS}" "${WNDTITLE}"
  StrCmp $0 0 continueInstall
    MessageBox MB_ICONSTOP|MB_OK "The application you are trying to remove is running. Close it and try again."
    Abort
  continueInstall:
FunctionEnd
