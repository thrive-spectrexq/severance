; ============================================================
; Severance Installer — Inno Setup Script
; ============================================================
; Build with:  iscc severance-installer.iss
; Requires:    Inno Setup 6+ (pre-installed on GitHub Actions windows-latest)
; ============================================================

#define MyAppName      "Severance"
#define MyAppVersion   "0.1.0"
#define MyAppPublisher "Bright"
#define MyAppURL       "https://github.com/thrive-spectrexq/severance"
#define MyAppExeName   "severance.exe"

[Setup]
; Unique application ID — do NOT change after first release
AppId={{B7E3F4A1-2D5C-4F8E-9A1B-3C6D7E8F9A0B}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}/issues
AppUpdatesURL={#MyAppURL}/releases
DefaultDirName={autopf}\{#MyAppName}
DefaultGroupName={#MyAppName}
AllowNoIcons=yes
; License displayed during install
LicenseFile=..\LICENSE
; Output installer file
OutputDir=..\dist-installer
OutputBaseFilename=severance-{#MyAppVersion}-setup
; Installer icon
SetupIconFile=severance.ico
; Uninstaller icon
UninstallDisplayIcon={app}\{#MyAppExeName}
; Compression
Compression=lzma2/ultra64
SolidCompression=yes
; Require admin (installing to Program Files)
PrivilegesRequired=admin
; Modern installer look
WizardStyle=modern
WizardSizePercent=110,110
; Minimum Windows version (Windows 10)
MinVersion=10.0
; Architecture
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
; Uninstall info
UninstallDisplayName={#MyAppName}

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked
Name: "quicklaunchicon"; Description: "{cm:CreateQuickLaunchIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked; OnlyBelowVersion: 6.1; Check: not IsAdminInstallMode

[Files]
; Main application
Source: "..\dist\{#MyAppExeName}"; DestDir: "{app}"; Flags: ignoreversion

; All DLLs in the root dist folder
Source: "..\dist\*.dll"; DestDir: "{app}"; Flags: ignoreversion

; Qt platform plugin (critical — app won't start without this)
Source: "..\dist\platforms\*"; DestDir: "{app}\platforms"; Flags: ignoreversion recursesubdirs createallsubdirs

; Qt styles plugin
Source: "..\dist\styles\*"; DestDir: "{app}\styles"; Flags: ignoreversion recursesubdirs createallsubdirs skipifsourcedoesntexist

; Severance plugins
Source: "..\dist\plugins\*"; DestDir: "{app}\plugins"; Flags: ignoreversion recursesubdirs createallsubdirs skipifsourcedoesntexist

; Any other Qt plugin subdirectories that windeployqt may have deployed
Source: "..\dist\imageformats\*"; DestDir: "{app}\imageformats"; Flags: ignoreversion recursesubdirs createallsubdirs skipifsourcedoesntexist
Source: "..\dist\iconengines\*"; DestDir: "{app}\iconengines"; Flags: ignoreversion recursesubdirs createallsubdirs skipifsourcedoesntexist
Source: "..\dist\tls\*"; DestDir: "{app}\tls"; Flags: ignoreversion recursesubdirs createallsubdirs skipifsourcedoesntexist
Source: "..\dist\networkinformation\*"; DestDir: "{app}\networkinformation"; Flags: ignoreversion recursesubdirs createallsubdirs skipifsourcedoesntexist

[Icons]
; Start Menu
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; IconFilename: "{app}\{#MyAppExeName}"
Name: "{group}\{cm:UninstallProgram,{#MyAppName}}"; Filename: "{uninstallexe}"

; Desktop icon (optional)
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Run]
; "Launch Severance" checkbox on the final page
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent

[UninstallDelete]
; Clean up any runtime-generated files
Type: filesandordirs; Name: "{app}\logs"
Type: filesandordirs; Name: "{app}\cache"
Type: filesandordirs; Name: "{app}\config"
