@echo off
echo Deleting all .obj and .exe files in the current directory:
echo.

rem Delete all .obj files
echo Deleting .obj files
del /q *.obj
echo.

rem Delete all .exe files
echo Deleting .exe files
del /q *.exe
echo.

echo Deletion complete.
pause
