@echo off
echo Dang khoi tao BankConsole.exe...
g++ -o BankConsole.exe main_console.cpp -I../final -lws2_32 -lwininet
if %errorlevel% equ 0 (
    echo Khoi tao thanh cong! Chay BankConsole.exe de bat dau.
) else (
    echo Khoi tao that bai! Vui long kiem tra loi va thu lai.
)
pause
