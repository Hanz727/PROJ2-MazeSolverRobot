@echo off
setlocal

:: Set paths
set "ARDUINO_LIB=%LOCALAPPDATA%\Arduino15\libraries"
set "SOURCE_DIR=%CD%"

:: List of libraries to update
set "LIBRARIES=MazeSolver RangeFinder ShieldMotor"

echo Updating Arduino libraries...

:: Loop through each library and replace it
for %%L in (%LIBRARIES%) do (
    echo Removing %%L...
    rmdir /s /q "%ARDUINO_LIB%\%%L"
    
    echo Copying %%L...
    xcopy /e /i /y "%SOURCE_DIR%\%%L" "%ARDUINO_LIB%\%%L"
)

echo Done!
pause
