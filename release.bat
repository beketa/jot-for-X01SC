set DEVENV="D:\Program Files (x86)\Microsoft Visual Studio 8\Common7\IDE\devenv"

%DEVENV% jot.sln /clean Release
%DEVENV% jot.sln /build "Release|Windows Mobile 5.0 Pocket PC SDK (ARMV4I)" /project jot
%DEVENV% jot.sln /build "Release|Windows Mobile 5.0 Pocket PC SDK (ARMV4I)" /project jotppc

mkdir old
copy jotppc\release\jot.cab old\jotXXX.cab

pause
