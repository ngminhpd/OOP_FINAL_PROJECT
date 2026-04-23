#include <iostream>
#include <vector>
#include <iomanip>
#include "TaiKhoan.h"
#include "../featureData/DataManager.h"
#include "../featureui/BankSystemUI.h"

using namespace std;
using namespace BankManagementSystem::Models;
using namespace BankManagementSystem::Data;
using namespace BankManagementSystem::UI;

void InGachNgang() {
    cout << string(80, '-') << endl;
}

int main() {
    BankApp ui;
    ui.Run();

    // 1. Tải dữ liệu từ thư mục featureData
    vector<TaiKhoan*> danhSachTK = DataManager::LoadTaiKhoan("featureData/accounts.csv");
    vector<KhachHang> danhSachKH = DataManager::LoadKhachHang("featureData/customers.csv");
    vector<GiaoDich> lichSuGD = DataManager::LoadGiaoDich("featuremodel/transactions.txt");

    // 2. Khởi tạo dữ liệu mẫu nếu file trống
    if (danhSachTK.empty()) {
        danhSachTK.push_back(new TaiKhoanThanhToan("001", "Nguyen Van A", 500000));
        danhSachTK.push_back(new TaiKhoanTietKiem("002", "Tran Thi B", 1000000, 0.05, 12));
        danhSachTK.push_back(new TaiKhoanTinDung("003", "Le Van C", 0, 5000000));
    }

    cout << "\n" << string(25, '=') << " DEMO NGAN HANG " << string(25, '=') << "\n";

    // 4. Thực hiện giao dịch mô phỏng
    for (auto tk : danhSachTK) {
        double tienNap = 100000;
        tk->NapTien(tienNap);
        string maGD = "GD" + to_string(lichSuGD.size() + 1);
        lichSuGD.push_back(GiaoDich(maGD, tk->GetSoTaiKhoan(), "Nap Tien", tienNap, "2026-04-23"));
    }

    // 6. Lưu dữ liệu vào thư mục featureData
    DataManager::SaveTaiKhoan(danhSachTK, "featureData/accounts.csv");
    DataManager::SaveKhachHang(danhSachKH, "featureData/customers.csv");
    DataManager::SaveGiaoDich(lichSuGD, "featuremodel/transactions.txt");

    cout << "\n>>> HE THONG: DA CAP NHAT DU LIEU VAO THU MUC featureData/ THANH CONG.\n";
    // 7. Giải phóng bộ nhớ
    for (auto tk : danhSachTK) delete tk;

    return 0;
}
