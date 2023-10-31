@echo OFF
cls

:: Automatically runs C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat
if NOT defined VSCMD_ARG_TGT_ARCH (
	call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
)

:: MUST be after the enviroment setup
SETLOCAL EnableDelayedExpansion

:: -O2					= Creates fast code.
:: -Ot					= Favors fast code.
:: -GF					= Enables string pooling.
:: -GR-					= Disable RTTI.
:: -Z7					= Produces object files that also contain full symbolic debugging information.
:: -Zc:preprocessor		= Use the new conforming preprocessor.
:: -Zc:strictStrings	= Disable string-literal to char*
:: -MD					= Compiles to create a multithreaded DLL, by using MSVCRT.lib.
:: -MDd					= Compiles to create a debug multithreaded DLL, by using MSVCRTD.lib.
:: -LD					= Creates a dynamic-link library.
:: -W0 -W1 -W2 -W3 -W4 	= Set output warning level.
::							:: 4100	: The formal parameter is not referenced in the body of the function. The unreferenced parameter is ignored.
::							:: 4189	: A variable is declared and initialized but not used.
::							:: 4201	: nonstandard extension used : nameless struct/union
::							:: 4324	: structure was padded due to alignment specifier
:: -wd<Number> 			= Disable a specific warning.
:: -WX 					= Treat warnings as errors.
:: -FC 					= Displays the full path of source code files passed to cl.exe in diagnostic text.

SET debugMode=1
SET platform=PLATFORM_WINDOWS
SET name=prime_factor
SET buildDir=TEMP\
SET objectDir=%buildDir%Objects\
SET warnings=-WX -W4 -wd4100 -wd4201 -wd4706 -wd4324
SET includes=-Ithird_party\ -Iassets\shaders\
SET defines=-DC_PLUS_PLUS -D_CRT_SECURE_NO_WARNINGS -DLITTLE_ENDIAN -D%platform% -DPLATFORM_ENGINE="\"%platform%\""
SET links=
SET flags=-std:c++20 -Zc:preprocessor -Zc:strictStrings -GR- -EHsc

if not exist %buildDir% ( mkdir %buildDir% )
if not exist %objectDir% ( mkdir %objectDir% )

if %debugMode% == 1 (
	SET defines=%defines% -DDEBUG
	SET flags=%flags% -Z7 -FC -MDd
) else (
	SET flags=%flags% -MD -O2 -Ot -GF
)

SET commands=-nologo %flags% %warnings% %defines% %math%

cl %commands% -Fe%buildDir%%name%.exe -Fo%objectDir% src\main.cpp %includes% %links% -INCREMENTAL:NO
if not %ERRORLEVEL% == 0 ( goto build_failed )

:build_success
echo Build success!
goto build_end

:build_failed
echo Build failed.
goto build_end

:build_end