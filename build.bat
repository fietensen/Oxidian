@echo off

REM Use this to build Oxidian on a Windows device
REM Make sure gcc is installed and set in the PATH variable

SET IDIR=.\src
SET ODIR=.\bin
SET LDIR=%IDIR%\lib
SET CC=gcc
SET OxyOrigin=%CD%\OxyLibs
SET OxyLibs=%CD%\bin\OxyLibs
SET add_path=%CD%\bin
SET base=%IDIR%\main.c %IDIR%\VM.c %IDIR%\environment.c %IDIR%\helper_functions.c %IDIR%\functions.c %IDIR%\errors.c
SET modules=%LDIR%\lib_empty.c %LDIR%\lib_io.c %LDIR%\lib_mem.c %LDIR%\lib_net.c

SET FILES=%base% %modules%
SET CFLAGS=-Wall -O2 -lwsock32 -std=c99 -I.\include
REM remove comments to add additional functionality:

IF [%1] == [OxyLibs] GOTO MAKE_OXYLIBS
IF [%1] == [remove] GOTO MAKE_REMOVE
IF [%1] == [] ( GOTO MAKE ) ELSE ( GOTO FAIL )


:MAKE_OXYLIBS
    DEL /Q %OxyLibs%
    COPY %OxyOrigin% %OxyLibs%
GOTO END

:MAKE_REMOVE
    RMDIR /S /Q %ODIR%
GOTO END

:MAKE_DEBUG
    ECHO Building Oxidian(Debug)... please wait.
    MKDIR %ODIR%
    MKDIR %OxyLibs%
    %CC% -o %ODIR%\Oxidian %FILES% %CFLAGS% -g
    COPY .\olc.py %ODIR%\olc.py
GOTO MAKE_OXYLIBS

:MAKE
    ECHO Building Oxidian... please wait.
    MKDIR %ODIR%
    MKDIR %OxyLibs%
    %CC% -o %ODIR%\Oxidian %FILES% %CFLAGS%
    COPY .\olc.py %ODIR%\olc.py
GOTO MAKE_OXYLIBS

:FAIL
    ECHO Building option "%1" not defined.
    ECHO "", "OxyLibs", "remove" are available.
:END