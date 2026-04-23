#include <iostream>
#include <vector>
#include "Models/TaiKhoan.h"

using namespace std;
using namespace BankManagementSystem::Models;

int main() {
    vector<TaiKhoan*> danhSach;

    // ===== Tạo các tài khoản khác nhau =====
    danhSach.push_back(new TaiKhoanThanhToan("001", "Nguyen Van A", 200000));
    danhSach.push_back(new TaiKhoanTietKiem("002", "Tran Thi B", 500000, 0.05, 6));
    danhSach.push_back(new TaiKhoanTinDung("003", "Le Van C", 0, 1000000));

    cout << "\n===== TEST DA HINH =====\n";

    for (auto tk : danhSach) {
        cout << " Truoc giao dich: \n" << tk->HienThiThongTin() << endl;
        //cout << tk->HienThiThongTin() << endl;
        cout << "\n Thong tin tai khoan: \n";     
        // Test nạp tiền
        tk->NapTien(100000);

        // Test rút tiền
        bool rut = tk->RutTien(50000);
        cout << "Rut 50000 VND: " << (rut ? "Thanh cong" : "That bai") << endl;

        // Test lãi
        cout << "Lai: " << (long long)tk->TinhLai() << " VND" << endl;

        // Test phí
        cout << "Phi duy tri the: " << (long long)tk->PhiDuyTri() << " VND" << endl;

        cout << "\n Sau giao dich: " << tk->HienThiThongTin() << endl;
        cout << "------------------------\n";
    }

    // ===== Giải phóng bộ nhớ =====
    for (auto tk : danhSach) {
        delete tk;
    }

    return 0;
}