@echo off
rem ============================================================
rem  Хост-сборка и запуск теста FIRST WAVE / PassThrough (MinGW)
rem  Windows. Требуется gcc (MinGW) в PATH или переменная MINGW.
rem ============================================================
setlocal

if not defined MINGW set MINGW=c:\dev\mingw64
set GCC=%MINGW%\bin\gcc.exe

if not exist "%GCC%" (
    echo ERROR: gcc not found at "%GCC%". Укажите MINGW=путь_к_mingw64.
    exit /b 1
)

set ROOT=..
set SRC=%ROOT%\Core\Device\device.c %ROOT%\Core\Device\tiristor.c
set SRC=%SRC% sim\hal_mock.c sim\sync_stub.c test_main.c

set INC=-I sim -I %ROOT%\Core\Device -I %ROOT%\Core\BSP -I %ROOT%\Core\Inc

echo === Сборка ===
"%GCC%" -Wall -Wextra -O0 -g -DSTM32F103xB -DUSE_HAL_DRIVER %INC% %SRC% -o firstwave_test.exe
if errorlevel 1 (
    echo BUILD FAILED
    exit /b 1
)
echo OK - firstwave_test.exe
echo.
echo === Запуск ===
"%~dp0firstwave_test.exe"
set RC=%ERRORLEVEL%
echo.
exit /b %RC%