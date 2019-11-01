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
call build.bat %config% %platform% || goto :error

for %%f in (..\include\*.h) do (
    cpplint.exe --counting=detailed %%f || goto :error
)

for %%f in (..\src\*.cc) do (
    cpplint.exe --counting=detailed %%f || goto :error
)

for %%f in (..\py\*.py) do (
    pylint --rcfile="..\.pylintrc" %%f || goto :error
)

popd
echo.
echo Build and Lint Done! Nice Job! :)
goto :EOF

:error
popd
echo build_and_lint failed :(
exit /b %errorlevel%
