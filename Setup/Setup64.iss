[Setup]
;-- Main Setup Information
 AppName                         = NetRouter
 AppVerName                      = NetRouter 1.0.1.2
 AppCopyright                    = Copyright © 1995-2015 NetworkDLS.
 DefaultDirName                  = {pf}\NetworkDLS\NetRouter
 DefaultGroupName                = NetworkDLS\NetRouter
 UninstallDisplayIcon            = {app}\Bin\NetRouterMgt.Exe
 WizardImageFile                 = /../../@Resources/Setup/LgSetup.bmp
 WizardSmallImageFile            = /../../@Resources/Setup/SmSetup.bmp
 PrivilegesRequired              = PowerUser
 Uninstallable                   = Yes
 AppMutex                        = Global\NetRouter_NetworkDLS
 Compression                     = zip/9
 OutputBaseFilename              = NetRouter64
 MinVersion                      = 0.0,5.0
 ArchitecturesInstallIn64BitMode = x64
 ArchitecturesAllowed            = x64

;-- Windows 2000 & XP (Support Dialog)
 AppPublisher    = NetworkDLS
 AppPublisherURL = http://www.NetworkDLS.com/
 AppUpdatesURL   = http://www.NetworkDLS.com/
 AppVersion      = 1.0.1.2

[Components]
 Name: Base;            Description: "Base Install";       Types: full compact custom;  Flags: Fixed;
 Name: Base\AutoUpdate; Description: "AutoUpdate Utility"; Types: full compact custom;
 Name: Base\Management; Description: "Management Console"; Types: full compact custom;
 Name: Service;         Description: "Router Service";     Types: full compact custom;

[Files]
  Source: "..\..\@AutoUpdate\x64\Release\AutoUpdate.Exe";     DestDir: "{app}\Bin";  Components: Base\AutoUpdate; Flags: RestartReplace;
 Source: "AutoUpdate.xml";                                    DestDir: "{app}\Bin";  Components: Base\AutoUpdate; Flags: IgnoreVersion;
 Source: "..\Source\Console\Release\x64\NetRouterMgt.exe";    DestDir: "{app}\Bin";  Components: Base\Management; Flags: IgnoreVersion;
 Source: "..\Source\Service\Release\x64\NetRouterSvc.exe";    DestDir: "{app}\Bin";  Components: Service;         Flags: IgnoreVersion;
 Source: "Resources\Start.ico";                               DestDir: "{app}";      Components: Service;
 Source: "Resources\Stop.ico";                                DestDir: "{app}";      Components: Service;
 Source: "Data\*.*";                                          DestDir: "{app}\Data"; Components: Service;         Flags: OnlyIfDoesntExist;

[Icons]
 Name: "{group}\Manage NetRouter";        Filename: "{app}\Bin\NetRouterMgt.Exe"; WorkingDir: "{app}\Bin";     Components: Base\Management;
 Name: "{group}\Update NetRouter";        Filename: "{app}\Bin\AutoUpdate.Exe";   WorkingDir: "{app}\Bin";     Components: Base\AutoUpdate;
 Name: "{group}\Service\Start NetRouter"; Filename: "{app}\Bin\NetRouterSvc.Exe"; Parameters: "/VerboseStart"; WorkingDir: "{app}\Bin"; IconFilename: "{app}\Start.ico"; Components: Service;
 Name: "{group}\Service\Stop NetRouter";  Filename: "{app}\Bin\NetRouterSvc.Exe"; Parameters: "/VerboseStop";  WorkingDir: "{app}\Bin"; IconFilename: "{app}\Stop.ico";  Components: Service;

[Registry]
 Root: HKLM; Subkey: "Software\NetworkDLS\NetRouter"; Flags: uninsdeletekey noerror;
 Root: HKLM; Subkey: "Software\NetworkDLS\NetRouter"; ValueName: "DataPath";        ValueType: String; ValueData: "{app}\Data";    Flags: CreateValueIfDoesntExist; Components: Service;
 Root: HKLM; Subkey: "Software\NetworkDLS\NetRouter"; ValueName: "Path";            ValueType: String; ValueData: "{app}";         Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\NetRouter"; ValueName: "ManagementPort";  ValueType: DWORD;  ValueData: "5565";          Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\NetRouter"; ValueName: "LastConnectHost"; ValueType: String; ValueData: "localhost";     Flags: CreateValueIfDoesntExist; Components: Base\Management;
 Root: HKLM; Subkey: "Software\NetworkDLS\NetRouter"; ValueName: "LastConnectUser"; ValueType: String; ValueData: "Administrator"; Flags: CreateValueIfDoesntExist; Components: Base\Management;
 Root: HKLM; Subkey: "Software\NetworkDLS\NetRouter"; ValueName: "Password";        ValueType: String; ValueData: "da39a3ee5e6b4b0d3255bfef95601890afd80709"; Flags: CreateValueIfDoesntExist; Components: Service;

[Run]
 Filename: "{app}\Bin\NetRouterSvc.exe"; Parameters: "/Install"; StatusMsg: "Installing service...";                                        Components: Service;
 Filename: "{app}\Bin\NetRouterSvc.exe"; Parameters: "/Start";   StatusMsg: "Starting Service...";                                          Components: Service;
 Filename: "{app}\Bin\NetRouterMgt.exe"; Description: "Open Management Console"; Flags: postinstall nowait skipifsilent runasoriginaluser;  Components: Base\Management;

[UninstallRun]
 Filename: "{app}\Bin\NetRouterSvc.exe"; Parameters: "/Stop";   RunOnceId: "StopService";   MinVersion: 0, 4.0; Components: Service;
 Filename: "{app}\Bin\NetRouterSvc.exe"; Parameters: "/Delete"; RunOnceId: "DeleteService"; MinVersion: 0, 4.0; Components: Service;

