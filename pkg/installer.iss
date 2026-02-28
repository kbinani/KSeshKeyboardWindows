#define app_version "1.0.0"
#define app_url "https://github.com/kbinani/KSeshKeyboardWindows"

[Setup]
AppId={{8DC4614B-EFAC-40FA-BB6F-225890D62E28}}
AppName=KSesh IME
AppVersion={#app_version}
AppPublisher=Buntaro Okada
AppPublisherURL={#app_url}
AppSupportURL={#app_url}
AppUpdatesURL={#app_url}
CreateAppDir=no
PrivilegesRequired=admin
OutputBaseFilename=KSeshIME-{#app_version}
SolidCompression=yes
WizardStyle=modern dynamic
ArchitecturesInstallIn64BitMode=x64os
LicenseFile=../LICENSE.txt
OutputDir=output
; signtool="C:\Program Files (x86)\Windows Kits\10\bin\10.0.26100.0\x64\signtool.exe" $p
SignTool=signtool sign /n $qOpen Source Developer, Buntaro Okada$q /t http://time.certum.pl/ /fd sha1 /v $f
SignTool=signtool sign /n $qOpen Source Developer, Buntaro Okada$q /tr http://time.certum.pl/ /td sha256 /fd sha256 /as /v $f

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Files]
Source: "..\build\x64\Release\KSeshIME.dll"; DestDir: "{sys}"; Flags: regserver restartreplace replacesameversion signonce
