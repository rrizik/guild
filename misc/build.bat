@echo off
setlocal EnableDelayedExpansion

if "%1" == "" (
     set DEBUG=1
     if !DEBUG! == 1 (
         echo Debug build.
         echo %DEBUG%
         set cl_optimization=-Od -DDEBUG=1
     ) else (
         echo Release build.
         echo %DEBUG%
         set cl_optimization=-O2 -DRELEASE=1
     )
) else if /i "%1" == "debug" (
    echo Debug build.
    set DEBUG=1
    set cl_optimization=-Od -DDEBUG=1
) else if /i "%1" == "release" (
    echo Release build.
    set DEBUG=0
    set cl_optimization=-O2 -DRELEASE=1
) else (
    echo Invalid parameter: Use "debug" or "release".
    exit /b 1
)

set cl_includes=-I ..\..\base\code
set cl_flags=-Zi -nologo -std:c++latest -Wall -WX 
set cl_ignore_warnings=-wd4201 -wd4189 -wd4101 -wd4505 -wd4820 -wd5045 -wd4996 -wd4100 -wd4668 -wd4711 -wd4062 -wd4388 -wd4018 -wd4459 -wd4626 -wd4200 -wd4710 -wd4456 -wd5246 -wd4309


IF NOT EXIST ..\build mkdir ..\build
pushd ..\build
cl %cl_flags% %cl_optimization% %cl_ignore_warnings% %cl_includes% ..\code\main.cpp 
popd


:: C5246: the initialization of a subobject should be wrapped in braces
:: C4201: nonstandard extension used: nameless struct/union
:: C4189: local variable is initialized but not referenced
:: C4101: unreferenced local variable
:: rem C4505: unreferenced local function has been removed
:: C4820: bytes padding added after data member
:: C5045: Compiler will insert Spectre mitigation for memory load if /Qspectre switch specified
:: C4996: This function or variable may be unsafe. 
:: C4100: unreferenced formal parameter
:: C4668: <term> is not defined as a preprocessor macro, replacing with '0' for '#if/#elif'
:: C4711: function selected for automatic inline expansion
:: rem C4710: function not inlined
:: C4062: enumerator in switch of enum is not handled
:: C4388: '<': signed/unsigned mismatch
:: C4018: '>': signed/unsigned mismatch
:: C4459: declaration of <variable> hides global declaration
:: C4200: empty [] initialization
:: C4710: function not inlined
:: C4456: declaration of '_i_' hides previous local declaration
:: C4309: truncation of constant value
:: 
::
:: C4626: remove and understand (something to do with defer)
