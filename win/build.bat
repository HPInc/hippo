@echo off

REM This batch file can take up to two arguments. The first determines the
REM build config and the second is the platform. If arguments aren't provided,
REM the following defaults are used.
set default_config=Release
set default_platform=x64

set config=%1%
set platform=%2%
IF [%config%]==[] set config=%default_config%
IF [%platform%]==[] set platform=%default_platform%

pushd %~dp0

"C:\\Program Files (x86)\\MSBuild\\14.0\Bin\\MSBuild.exe" hippo.sln^
 -t:Build -p:Configuration=%config% -p:Platform=%platform%^
 -p:VisualStudioVersion=14.0 || goto :error

popd
echo.
echo Build Done! :)
goto :EOF

:error
popd
echo Build Failed :(
exit /b %errorlevel%
