[Setup]
AppId={{8DC4614B-EFAC-40FA-BB6F-225890D62E28}
AppName=KSesh IME
AppVersion=1.0.0
AppPublisher=kbinani
AppPublisherURL=https://github.com/kbinani/KSeshKeyboardWindows
AppSupportURL=https://github.com/kbinani/KSeshKeyboardWindows
AppUpdatesURL=https://github.com/kbinani/KSeshKeyboardWindows
CreateAppDir=no
PrivilegesRequired=admin
OutputBaseFilename=KSeshIME
SolidCompression=yes
WizardStyle=modern dynamic
ArchitecturesInstallIn64BitMode=x64os

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Files]
Source: "..\x64\Release\KSeshIME.dll"; DestDir: "{sys}"; Flags: regserver restartreplace
