@echo off
setlocal enabledelayedexpansion

echo ============================================
echo   Truck Loading Simulator - Build Script
echo ============================================
echo.

REM Проверяем наличие CMake
cmake --version >nul 2>&1
if !errorlevel! neq 0 (
    echo ОШИБКА: CMake не найден в PATH!
    echo Убедитесь, что CMake установлен и добавлен в PATH.
    pause
    exit /b 1
)

REM Проверяем наличие vcpkg
if not exist "C:\Users\Pasha\vcpkg\scripts\buildsystems\vcpkg.cmake" (
    echo ОШИБКА: vcpkg не найден!
    echo Убедитесь, что vcpkg установлен в C:\Users\Pasha\vcpkg\
    pause
    exit /b 1
)

REM Создаем или очищаем папку сборки
echo [1/4] Подготовка папки сборки...
if exist build (
    echo Очищаем старую сборку...
    rmdir /s /q build
)
mkdir build
cd build

echo.
echo [2/4] Конфигурация проекта с CMake...
echo Используем vcpkg toolchain...

cmake .. ^
    -DCMAKE_TOOLCHAIN_FILE="C:/Users/Pasha/vcpkg/scripts/buildsystems/vcpkg.cmake" ^
    -DCMAKE_BUILD_TYPE=Release

if !errorlevel! neq 0 (
    echo.
    echo ОШИБКА: Конфигурация не удалась!
    echo Проверьте сообщения об ошибках выше.
    pause
    exit /b 1
)

echo.
echo [3/4] Сборка проекта...
echo Компилируем в Release режиме для максимальной производительности...

cmake --build . --config Release --parallel

if !errorlevel! neq 0 (
    echo.
    echo ОШИБКА: Сборка не удалась!
    echo Проверьте сообщения об ошибках выше.
    pause
    exit /b 1
)

echo.
echo [4/4] Поиск исполняемого файла...

REM Ищем исполняемый файл в разных возможных местах
set "EXE_PATH="
if exist "Release\TruckLoadingSimulator.exe" (
    set "EXE_PATH=Release\TruckLoadingSimulator.exe"
) else if exist "TruckLoadingSimulator.exe" (
    set "EXE_PATH=TruckLoadingSimulator.exe"
) else if exist "Debug\TruckLoadingSimulator.exe" (
    set "EXE_PATH=Debug\TruckLoadingSimulator.exe"
)

if "!EXE_PATH!"=="" (
    echo ПРЕДУПРЕЖДЕНИЕ: Исполняемый файл не найден!
    echo Поищите TruckLoadingSimulator.exe в папке build\
    dir *.exe /s
    pause
    exit /b 1
)

echo.
echo ============================================
echo          СБОРКА ЗАВЕРШЕНА УСПЕШНО!
echo ============================================
echo Исполняемый файл: !EXE_PATH!
echo.

REM Показываем размер файла и дату создания
for %%F in ("!EXE_PATH!") do (
    echo Размер: %%~zF bytes
    echo Дата: %%~tF
)

echo.
echo Хотите запустить программу сейчас? (y/n)
set /p "choice=Ваш выбор: "

if /i "!choice!"=="y" (
    echo.
    echo Запускаем TruckLoadingSimulator...
    echo ============================================
    echo.
    start "" "!EXE_PATH!"
) else (
    echo.
    echo Для запуска позже используйте:
    echo cd build
    echo !EXE_PATH!
)

echo.
pause