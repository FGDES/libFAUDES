REM ================================== build libFAUDES installer with Inno Setup

REM === (need to build libFAUDES with MS VC before, e.g. via  msvcvuild.bat))

REM === sanity check
if not exist VERSION.bat {
  echo need to cd to libFAUDES base path
  exit
}

REM ==== have Inno Setup
set INNOSCC="%ProgramFiles(x86)%\Inno Setup 6\iscc"

REM ==== set FAUDES version numbers
call VERSION.bat

REM ==== set paths for MSVC redistributables
set FAUDES_MSVCREDIST="%ProgramFiles%\Microsoft Visual Studio\2022\Community\VC\Redist\MSVC\14.38.33130\x64\Microsoft.VC143.CRT"

REM ==== run Inno to build setup.exe
%INNOSCC% package.iss /DVMAJOR=%FAUDES_VERSION_MAJOR% /DVMINOR=%FAUDES_VERSION_MINOR% /DMSVCREDIST=%FAUDES_MSVCREDIST%



