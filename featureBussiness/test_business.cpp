#include <iostream>
#include "../featuremodel/TaiKhoan.h"
#include "NganHang.h"
#include "Exceptions.h"

using namespace BankManagementSystem::Models;
using namespace BankManagementSystem::Business;

int main() {
    NganHang vcb;

    // 1. Thêm tài khoản với mã PIN
    std::cout << "--- Dang mo tai khoan ---\n";
    vcb.ThemTaiKhoan(new TaiKhoanThanhToan("V001", "Cau Chu Quan", 1000000), "1234");
    vcb.ThemTaiKhoan(new TaiKhoanTietKiem("S001", "Nguyen Van A", 5000000, 0.05, 12), "5678");

    vcb.LietKeDanhSach();

    // 2. Test Nap Tien
    std::cout << "\n--- Nap tien ---\n";
    try {
        vcb.NapTien("V001", 500000);
        std::cout << "Nap 500k thanh cong.\n";
    } catch (const BankException& e) {
        std::cerr << e.what() << "\n";
    }

    // 3. Test Rut Tien voi PIN
    std::cout << "\n--- Rut tien voi PIN ---\n";
    // Kich ban: Sai PIN
    try {
        std::cout << "Thu rut 100k voi PIN sai (0000)...\n";
        vcb.RutTien("V001", 100000, "0000");
    } catch (const BankException& e) {
        std::cerr << "Bat duoc loi: " << e.what() << "\n";
    }

    // Kich ban: Dung PIN
    try {
        std::cout << "Thu rut 100k voi PIN dung (1234)...\n";
        vcb.RutTien("V001", 100000, "1234");
        std::cout << "Rut tien thanh cong.\n";
    } catch (const BankException& e) {
        std::cerr << e.what() << "\n";
    }

    // 4. Test Chuyen Tien & Khoa the
    std::cout << "\n--- Chuyen tien & Khoa the ---\n";
    vcb.KhoaTaiKhoan("S001", true);
    try {
        std::cout << "Thu chuyen 200k den tai khoan S001 dang bi khoa...\n";
        vcb.ChuyenTien("V001", "S001", 200000, "1234");
    } catch (const BankException& e) {
        std::cerr << "Bat duoc loi: " << e.what() << "\n";
    }
    vcb.KhoaTaiKhoan("S001", false);

    // 5. In sao ke lich su
    std::cout << "\n";
    vcb.InSaoKe("V001");

    // 6. Test Luu & Doc File
    std::cout << "\n--- Luu & Doc file ---\n";
    vcb.LuuDuLieu("bank_data.txt");
    
    NganHang bankMoi;
    bankMoi.DocDuLieu("bank_data.txt");

    return 0;
}
