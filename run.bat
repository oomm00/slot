@echo off
start "" "D:\fu ck\slot\slot-server.exe"
echo Slot server started on http://localhost:8080
echo Open http://localhost:8080 in your browser.
echo.
echo Press any key to stop the server...
pause >nul
taskkill /f /im slot-server.exe >nul 2>&1
