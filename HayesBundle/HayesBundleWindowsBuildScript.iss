; Script generated by the Inno Setup Script Wizard.

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{82BFB9FC-E450-4741-8C5E-F0C3B0BC8FDC}}
AppName=HayesBundle
AppVersion=1.0.0
DefaultDirName={commonpf64}\Common Files\VST3\HayesBundle\VST3
DefaultGroupName=EmmetSoft
OutputDir=userdocs:HayesInstallers
OutputBaseFilename=HayesBundleInstaller
Compression=lzma
SolidCompression=yes

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "..\HayesCompressor\Builds\VisualStudio2019\x64\Release\VST3\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "..\HayesDelay\Builds\VisualStudio2019\x64\Release\VST3\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "..\HayesEQ\Builds\VisualStudio2019\x64\Release\VST3\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "..\HayesFDNReverb\Builds\VisualStudio2019\x64\Release\VST3\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "..\HayesDistortion\Builds\VisualStudio2019\x64\Release\VST3\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "..\HayesPitchShifter\Builds\VisualStudio2019\x64\Release\VST3\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "..\HayesReverb\Builds\VisualStudio2019\x64\Release\VST3\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "..\HayesTapeDelay\Builds\VisualStudio2019\x64\Release\VST3\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs


