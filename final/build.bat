@echo off
chcp 65001 >nul
cls
echo [ HE THONG ] Dang bien dich do an OOP (UTF-8)...
taskkill /F /IM BankSystem.exe >nul 2>&1
g++ -std=c++11 -finput-charset=utf-8 -fexec-charset=utf-8 main.cpp -lws2_32 -o BankSystem.exe
if %errorlevel% equ 0 (
    echo [ THANH CONG ] Khoi chay server 
    echo [ LUU Y ] Giu cua so nay de duy tri server.
    BankSystem.exe
) else (
    echo [ THAT BAI ] Kiem tra lai trinh bien dich g++.
    pause
)
