; Instalador do Sonic 3 & Knuckles Co-op (Inno Setup 6)
; Gerar: ISCC.exe installer\sonic3coop.iss   (a partir da pasta raiz do projeto)
; Requer a pasta dist\Sonic3Coop montada (exe, dll, data\*.bin, config.json, doc).

#define AppName "Sonic 3 & Knuckles Co-op"
#define AppVersion "1.0"
#define DistDir "..\dist\Sonic3Coop"

[Setup]
AppId={{6C3B7E2A-5F0D-4E4B-9C1E-3A7D2B9F8E41}
AppName={#AppName}
AppVersion={#AppVersion}
AppPublisher=Fan project (based on Sonic 3 A.I.R. by Eukaryot)
DefaultDirName={autopf}\Sonic 3 Coop
DefaultGroupName={#AppName}
; Lets the user choose between installing for everyone (admin) or only for the current user (no admin)
PrivilegesRequired=lowest
PrivilegesRequiredOverridesAllowed=dialog
OutputDir=output
OutputBaseFilename=Instalar-Sonic3-Coop
SetupIconFile=..\Oxygen\sonic3air\___internal\icon\app-no-border.ico
UninstallDisplayIcon={app}\Sonic3AIR.exe
LicenseFile={#DistDir}\doc\license.txt
InfoBeforeFile=LEIA-ME.txt
Compression=lzma2/max
SolidCompression=yes
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
WizardStyle=modern

[Languages]
Name: "brazilianportuguese"; MessagesFile: "compiler:Languages\BrazilianPortuguese.isl"
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"

[Files]
Source: "{#DistDir}\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "LEIA-ME.txt"; DestDir: "{app}"; Flags: ignoreversion isreadme

[Icons]
Name: "{group}\{#AppName}"; Filename: "{app}\Sonic3AIR.exe"; WorkingDir: "{app}"
Name: "{group}\Leia-me"; Filename: "{app}\LEIA-ME.txt"
Name: "{group}\{cm:UninstallProgram,{#AppName}}"; Filename: "{uninstallexe}"
Name: "{autodesktop}\{#AppName}"; Filename: "{app}\Sonic3AIR.exe"; WorkingDir: "{app}"; Tasks: desktopicon

[Run]
Filename: "{app}\Sonic3AIR.exe"; WorkingDir: "{app}"; Description: "{cm:LaunchProgram,{#AppName}}"; Flags: nowait postinstall skipifsilent
