; Installer Configuration for libFAUDES MS Windows distribution
;
; As of 2025, we are using Inno Setup version 6.5
;


[Setup]
MinVersion=10.0.22000	
AppCopyright=Moor/FGDES
AppName=libFAUDES
AppVersion={#VMAJOR}.{#VMINOR}
AppVerName=libFAUDES {#VMINOR}.{#VMAJOR}
ArchitecturesInstallIn64BitMode=x64compatible
ArchitecturesAllowed=x64compatible
PrivilegesRequired=none
ShowLanguageDialog=no
LanguageDetectionMethod=uilanguage
UsePreviousAppDir=false
DefaultDirName={sd}\FAUDES\libFAUDES
OutputDir=.\
SetupIconFile=msvcdist.ico
OutputBaseFilename=libfaudes-{#VMAJOR}_{#VMINOR}_setup

[Types]
Name: "full"; Description: "Full installation"
Name: "compact"; Description: "Compact installation"
Name: "custom"; Description: "Custom installation"; Flags: iscustom

[Components]
Name: "executables"; Description: "libFAUDES incl. executables";   Flags: fixed;  Types: full compact custom
Name: "msvcdlls"; Description: "MSVC redistributable DLLs"; Types: full
Name: "documentation"; Description: "User Reference C++ API Documentation";    Types: full
Name: "examples";      Description: "Example data and luafaudes scripts";      Types: full
Name: "develop";       Description: "C++ header files";                        Types: full

[Tasks]
Name: "syspath"; Description: "Set system PATH to include {app} [admins only, effective after reboot]"; Flags: unchecked restart; Check: CanSetSystemPath();
Name: "userpath"; Description: "Set user PATH to include {app} [effective after reboot]"; Flags: unchecked restart; Check: CanSetUserPath();
Name: "syspathok"; Description: "Set system PATH to include {app}"; Flags: unchecked; Check: SystemPathHasFaudes();
Name: "userpathok"; Description: "Set user PATH to include {app}"; Flags: unchecked; Check: UserPathHasFaudes();


[Dirs]
Name: {app}\StdFlx
Name: {app}\Doc;                      Components: documentation
Name: {app}\Include;                  Components: develop
Name: {app}\Examples\corefaudes;      Components: examples
Name: {app}\Examples\corefaudes\data; Components: examples
Name: {app}\Examples\synthesis;       Components: examples
Name: {app}\Examples\synthesis\data;  Components: examples
Name: {app}\Examples\observer;        Components: examples
Name: {app}\Examples\observer\data;   Components: examples
Name: {app}\Examples\priorities;      Components: examples
Name: {app}\Examples\priosities\data; Components: examples
Name: {app}\Examples\hiosys;          Components: examples
Name: {app}\Examples\hiosys\data;     Components: examples
Name: {app}\Examples\iosystem;        Components: examples
Name: {app}\Examples\iosysten\data;   Components: examples
Name: {app}\Examples\multitasking;    Components: examples
Name: {app}\Examples\multitasking\data; Components: examples
Name: {app}\Examples\diagnosis;       Components: examples
Name: {app}\Examples\diagnosis\data;  Components: examples
Name: {app}\Examples\simulator;       Components: examples
Name: {app}\Examples\simulator\data;  Components: examples
Name: {app}\Examples\iodevice;        Components: examples
Name: {app}\Examples\iodevice\data;   Components: examples
Name: {app}\Examples\extensions;      Components: examples

[Files]
Source: faudes.*;		DestDir: {app}
Source: faudesd.*;    		DestDir: {app}
Source: include\libfaudes.rti;	DestDir: {app}
Source: bin\*.exe;        	DestDir: {app}
Source: bin\luafaudes.flx;      DestDir: {app}
Source: include\*;        	DestDir: {app}\Include;  Flags: recursesubdirs createallsubdirs; Components: develop
Source: doc\*.* ; 		DestDir: {app}\Doc; Flags: recursesubdirs createallsubdirs; Components: documentation
Source: VERSION;                DestDir: {app}
Source: msvcreadme.md;          DestDir: {app}; DestName: Readme.md
Source: stdflx\*.flx;           DestDir: {app}\StdFlx; 
Source: {#MSVCREDIST}\msvcp140.dll;          DestDir: {app}; Components: msvcdlls
Source: {#MSVCREDIST}\vcruntime140.dll;      DestDir: {app}; Components: msvcdlls
Source: plugins\luabindings\tutorial\data\*; DestDir: {app}\Examples\corefaudes\data; Components: examples
Source: plugins\luabindings\tutorial\*.lua;  DestDir: {app}\Examples\corefaudes;      Components: examples
Source: plugins\synthesis\tutorial\*.lua;    DestDir: {app}\Examples\synthesis;       Components: examples
Source: plugins\synthesis\tutorial\data\*;   DestDir: {app}\Examples\synthesis\data;  Components: examples
Source: plugins\observer\tutorial\*.lua;     DestDir: {app}\Examples\observer;        Components: examples
Source: plugins\observer\tutorial\data\*;    DestDir: {app}\Examples\observer\data;   Components: examples; Flags: recursesubdirs createallsubdirs;
Source: plugins\hiosys\tutorial\*.lua;       DestDir: {app}\Examples\hiosys;          Components: examples
Source: plugins\hiosys\tutorial\data\*;      DestDir: {app}\Examples\hiosys\data;     Components: examples; Flags: recursesubdirs createallsubdirs;
Source: plugins\priorities\tutorial\*.lua;   DestDir: {app}\Examples\hiosys;          Components: examples
Source: plugins\priorities\tutorial\data\*;  DestDir: {app}\Examples\hiosys\data;     Components: examples
Source: plugins\iosystem\tutorial\*.lua;     DestDir: {app}\Examples\iosystem;        Components: examples
Source: plugins\iosystem\tutorial\data\*;    DestDir: {app}\Examples\iosystem\data;   Components: examples
Source: plugins\multitasking\tutorial\*.lua; DestDir: {app}\Examples\multitasking;    Components: examples
Source: plugins\multitasking\tutorial\data\*; DestDir: {app}\Examples\multitasking\data; Components: examples
Source: plugins\diagnosis\tutorial\*.lua;    DestDir: {app}\Examples\diagnosis;       Components: examples
Source: plugins\diagnosis\tutorial\data\*;   DestDir: {app}\Examples\diagnosis\data;  Components: examples
Source: plugins\simulator\tutorial\data\*;   DestDir: {app}\Examples\simulator\data;  Components: examples
Source: plugins\simulator\tutorial\*.sh;     DestDir: {app}\Examples\simulator;       Components: examples
Source: plugins\iodevice\tutorial\data\*;    DestDir: {app}\Examples\iodevice\data;   Components: examples
Source: plugins\iodevice\tutorial\*.sh;      DestDir: {app}\Examples\iodevice;        Components: examples
Source: plugins\luabindings\tutorial\*.flx;  DestDir: {app}\Examples\extensions;      Components: examples

[Registry]
Root: HKLM; Subkey: SYSTEM\CurrentControlSet\Control\Session Manager\Environment; ValueType: expandsz; ValueName: Path; ValueData: {code:GetFaudesSystemPath}; Flags: preservestringtype; Check: CanSetSystemPath(); Tasks: syspath
Root: HKCU; Subkey: Environment;                                                  ValueType: expandsz; ValueName: Path; ValueData: {code:GetFaudesUserPath};   Flags: preservestringtype; Check: CanSetUserPath(); Tasks: userpath


[Code]
var
  OldPath: String;
  NewPath: String;
  HasFaudes: Boolean;
  OkPath: Boolean;


// compose NewPath from OldPath
// sets HasFaudes if no changes required
function ComposeFaudesPath() : Boolean;
var
  FaudesPath: String;
  Location: Integer;
begin
  // expand constant
  FaudesPath:=ExpandConstant('{app}');
  HasFaudes:=false;
  if OkPath then
  begin
    // debug report
    //MsgBox('Your path [' + WhichPath + ']: ' + OldPath, mbInformation, MB_OK);
    Location := Pos(FaudesPath, OldPath);
    // path allready holds libFAUDES path
    if Location <>0 then
    begin
      // MsgBox('Your path allready contains libFAUDES. Fine.',mbInformation, MB_OK);
      HasFaudes:=true;
    end;
    // fix end with extra semicolon
    if length(OldPath)>0 then
      if OldPath[length(OldPath)] <> ';' then
        OldPath := OldPath + ';';
    // build new path
    NewPath := OldPath + FaudesPath + ';';
    // let user know
    // MsgBox('New path [' + WhichPath + ']: ' + NewPath, mbInformation, MB_OK);
  end;
  // done
  Result := OkPath;
end;


// prepare new system path
function PrepareSystemPath() : Boolean;
var
  FaudesPath: String;
begin
  // expand constant
  FaudesPath:=ExpandConstant('{app}');
  // get current reg key
  OkPath:=RegQueryStringValue(HKEY_LOCAL_MACHINE, 'SYSTEM\CurrentControlSet\Control\Session Manager\Environment', 'Path', OldPath);
  if ( length(OldPath) = 0 ) then OkPath := false;
  // compose new path
  ComposeFaudesPath();
  // done
  Result := OkPath;
end;
  
// prepare new user path
function PrepareUserPath() : Boolean;
var
  FaudesPath: String;
begin
  // expand constant
  FaudesPath:=ExpandConstant('{app}');
  // get current reg key
  OkPath:=RegQueryStringValue(HKEY_CURRENT_USER, 'Environment', 'Path', OldPath);
  if not OkPath then OldPath :='';
  OkPath:=true;
  // compose new path
  ComposeFaudesPath();
  // done
  Result := OkPath;
end;

// get the new path as string
function GetFaudesUserPath(dummy : String) : String;
begin
  // check and get
  PrepareUserPath();
  // done
  Result:=NewPath;
end;


// get the new path as string
function GetFaudesSystemPath(dummy : String) : String;
begin
  // check and get
  PrepareSystemPath();
  // done
  Result:=NewPath;
end;


// true if we can set the user path
function CanSetUserPath() : Boolean;
begin
  // check and get
  PrepareUserPath();
  // done
  Result:= OkPath AND NOT HasFaudes;
end;


// true if we can set the system path
function CanSetSystemPath() : Boolean;
begin
  // check and get
  PrepareSystemPath();
  // done
  Result:= OkPath AND NOT HasFaudes;
end;


// true if the user path already contains faudes
function UserPathHasFaudes() : Boolean;
begin
  // check and get
  PrepareUserPath();
  // done
  Result:= OkPath AND HasFaudes;
end;

// true if the system path already contains faudes
function SystemPathHasFaudes() : Boolean;
begin
  // check and get
  PrepareSystemPath();
  // done
  Result:= OkPath AND HasFaudes;
end;


