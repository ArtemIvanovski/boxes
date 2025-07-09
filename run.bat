@echo off
echo Запуск Truck Loading Simulator...

cd /d "%~dp0"

REM Поиск исполняемого файла
set "EXE_PATH="
if exist "build\Release\TruckLoadingSimulator.exe" (
    set "EXE_PATH=build\Release\TruckLoadingSimulator.exe"
) else if exist "build\TruckLoadingSimulator.exe" (
    set "EXE_PATH=build\TruckLoadingSimulator.exe"
) else if exist "build\Debug\TruckLoadingSimulator.exe" (
    set "EXE_PATH=build\Debug\TruckLoadingSimulator.exe"
) else if exist "TruckLoadingSimulator.exe" (
    set "EXE_PATH=TruckLoadingSimulator.exe"
)

if "%EXE_PATH%"=="" (
    echo ОШИБКА: Исполняемый файл не найден!
    echo.
    echo Возможные причины:
    echo 1. Проект еще не собран - запустите build.bat
    echo 2. Сборка завершилась с ошибками
    echo.
    echo Попробуйте запустить build.bat для сборки проекта.
    pause
    exit /b 1
)

echo Найден: %EXE_PATH%
echo Запуск...
echo.

start "" "%EXE_PATH%"

REM Ждем немного, чтобы увидеть, запустилась ли программа
timeout /t 2 /nobreak >nul

echo Программа должна была запуститься.
echo Если окно не появилось, проверьте консоль на ошибки.
pause