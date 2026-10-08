@echo off
echo Compiling Smart Emergency Response System...

g++ -o emergency.exe main.cpp graph.cpp emergency.cpp resource.cpp user.cpp file_handler.cpp dispatcher.cpp sha1.cpp -std=c++17

if %errorlevel% == 0 (
    echo.
    echo SUCCESS: emergency.exe created!
    echo.
    echo Testing ping...
    emergency.exe ping
) else (
    echo.
    echo FAILED: Check the errors above.
)

pause