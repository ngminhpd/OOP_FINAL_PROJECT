@echo off
cls
echo [ HE THONG ] Dang kiem tra va don dep...
taskkill /F /IM BankManagementSystem.exe >nul 2>&1
del BankManagementSystem.exe >nul 2>&1

echo [ HE THONG ] Dang bien dich toan bo du an (Model, Data, Business, UI)...
g++ -std=c++14 featuremodel/main.cpp ^
featuremodel/TaiKhoan.cpp ^
featureData/DataManager.cpp ^
featureData/KhachHang.cpp ^
featureData/GiaoDich.cpp ^
featureBussiness/NganHang.cpp ^
-o BankManagementSystem.exe

if %errorlevel% equ 0 (
    echo [ THANH CONG ] Bien dich hoan tat.
    echo [ THONG BAO ] Bat dau chay chuong trinh...
    BankManagementSystem.exe
) else (
    echo [ THAT BAI ] Co loi xay ra. Vui long kiem tra lai code.
)
pause