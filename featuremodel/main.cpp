#include <iostream>
#include <vector>
#include <iomanip>
#include "TaiKhoan.h"
#include "../featureData/DataManager.h"

using namespace std;
using namespace BankManagementSystem::Models;
using namespace BankManagementSystem::Data;

void InGachNgang() {
    cout << string(80, '-') << endl;
}

int main() {
    // 1. Tải dữ liệu
    vector<TaiKhoan*> danhSachTK = DataManager::LoadTaiKhoan("accounts.txt");
    vector<KhachHang> danhSachKH = DataManager::LoadKhachHang("customers.txt");
    vector<GiaoDich> lichSuGD = DataManager::LoadGiaoDich("transactions.txt");

    // 2. Khởi tạo dữ liệu mẫu nếu file trống (Chỉ chạy lần đầu)
    if (danhSachTK.empty()) {
        danhSachTK.push_back(new TaiKhoanThanhToan("001", "Nguyen Van A", 500000));
        danhSachTK.push_back(new TaiKhoanTietKiem("002", "Tran Thi B", 1000000, 0.05, 12));
        danhSachTK.push_back(new TaiKhoanTinDung("003", "Le Van C", 0, 5000000));

        danhSachKH.push_back(KhachHang("KH01", "Nguyen Van A", "0901234567", "TP.HCM"));
        danhSachKH.push_back(KhachHang("KH02", "Tran Thi B", "0907654321", "Ha Noi"));
        danhSachKH.push_back(KhachHang("KH03", "Le Van C", "0988888888", "Da Nang"));
    }

    cout << "\n" << string(25, '=') << " DEMO NGAN HANG " << string(25, '=') << "\n";

    // 3. Hiển thị danh sách khách hàng
    cout << "\n[1] DANH SACH KHACH HANG:\n";
    for (const auto& kh : danhSachKH) {
        cout << "- " << kh.GetMaKH() << " | " << setw(15) << left << kh.GetHoTen() 
             << " | " << kh.GetSoDienThoai() << " | " << kh.GetDiaChi() << endl;
    }
    InGachNgang();

    // 4. Hiển thị danh sách tài khoản & Thực hiện giao dịch mẫu
    cout << "\n[2] DANH SACH TAI KHOAN & GIAO DICH MO PHONG:\n";
    for (auto tk : danhSachTK) {
        cout << "Truoc: " << tk->HienThiThongTin() << endl;

        // Mô phỏng: Nạp 100k cho mỗi tài khoản
        double tienNap = 100000;
        tk->NapTien(tienNap);

        // Ghi log giao dịch
        string maGD = "GD" + to_string(lichSuGD.size() + 1);
        lichSuGD.push_back(GiaoDich(maGD, tk->GetSoTaiKhoan(), "Nap Tien", tienNap, "2026-04-23"));

        cout << " -> Da nap " << (long long)tienNap << " VND. " << endl;
        cout << "Sau : " << tk->HienThiThongTin() << "\n\n";
    }
    InGachNgang();

    // 5. Hiển thị lịch sử giao dịch
    cout << "\n[3] LICH SU GIAO DICH GAN DAY:\n";
    int start = lichSuGD.size() > 5 ? lichSuGD.size() - 5 : 0;
    for (size_t i = start; i < lichSuGD.size(); ++i) {
        const auto& gd = lichSuGD[i];
        cout << gd.GetMaGD() << " | STK: " << gd.GetSoTaiKhoan() << " | " 
             << setw(10) << gd.GetLoaiGD() << " | +" << setw(10) << (long long)gd.GetSoTien() 
             << " | Ngay: " << gd.GetNgayGiaoDich() << endl;
    }
    InGachNgang();

    // 6. Lưu dữ liệu
    DataManager::SaveTaiKhoan(danhSachTK, "accounts.txt");
    DataManager::SaveKhachHang(danhSachKH, "customers.txt");
    DataManager::SaveGiaoDich(lichSuGD, "transactions.txt");

    cout << "\n>>> HE THONG: DA CAP NHAT DU LIEU VAO FILE THANH CONG.\n";

    // 7. Giải phóng bộ nhớ
    for (auto tk : danhSachTK) delete tk;

    return 0;
}