@echo off
CLS

ECHO Cleaning Build dir..
if exist build (
	rmdir /s /q build
)

ECHO Config CMAKE..
cmake -B build -G "Visual Studio 17 2022" -A x64 -D BUILD_SHARED_LIBS=OFF

if %errorlevel% neq 0 (
	ECHO CMAKE config failed.
	PAUSE
	EXIT /b 1
)

ECHO Compiling..
cmake --build build --config Release

if %errorlevel% neq 0 (
	ECHO Compilation failiure.
	PAUSE
	EXIT /b 1
)


REM Move .PYD (module) file to the main dir.
move build\Release\gl*.pyd .


ECHO.
ECHO Compilation complete
ECHO.

PAUSE

cd build\Release
python ..\..\test.py