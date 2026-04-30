#pragma once
#include "Components.h"
#include "../featureBussiness/NganHang.h"
#include "../featureData/DataManager.h"
#include "../featuremodel/TaiKhoan.h"
#include <memory>
#include <vector>
#include <string>
#include <algorithm>
#include <fstream>
#include <map>

namespace BankManagementSystem {
namespace UI {
using namespace std;

static string CURRENT_USER_STK = "";

class BaseForm {
protected:
    Business::NganHang& nganHang;
public:
    BaseForm(Business::NganHang& nh) : nganHang(nh) {}
    virtual ~BaseForm() {}
    virtual int Show() = 0;

    void SyncData() {
        const auto& dsShared = nganHang.GetDanhSach();
        vector<Models::TaiKhoan*> dsRaw;
        for (auto& ptr : dsShared) dsRaw.push_back(ptr.get());
        Data::DataManager::SaveTaiKhoan(dsRaw, "featureData/accounts.csv");

        ofstream f("featureData/history.csv");
        const auto& dsLS = nganHang.GetLichSu();
        for (auto const& it : dsLS) {
            for (auto const& gd : it.second) {
                f << it.first << ";" << gd.loai << ";" << gd.soTien << ";" << gd.noiDung << ";" << gd.thoiGian << "\n";
            }
        }
        f.close();
    }
};

// --- 1. START MENU ---
class StartMenuForm : public BaseForm {
public:
    StartMenuForm(Business::NganHang& nh) : BaseForm(nh) {}
    int Show() override {
        Components::Header("HE THONG NGAN HANG OOP");
        int c = Components::Menu({"Dang nhap", "Mo tai khoan moi"});
        if (c == 1) return 9;
        if (c == 2) return 8;
        return 0;
    }
};

// --- 9. LOGIN FORM ---
class LoginForm : public BaseForm {
public:
    LoginForm(Business::NganHang& nh) : BaseForm(nh) {}
    int Show() override {
        Components::Header("DANG NHAP");
        string user = Components::TextBox("STK / Username");
        string pass = Components::TextBox("Ma PIN");
        if (user == "admin" && pass == "123") return 2;
        auto ds = Data::DataManager::LoadTaiKhoan("featureData/accounts.csv");
        bool ok = false;
        for (auto tk : ds) {
            if (tk->GetSoTaiKhoan() == user && tk->GetMaPIN() == pass) { CURRENT_USER_STK = user; ok = true; }
            delete tk;
        }
        if (ok) return 7;
        Components::Label("Sai thong tin!", true); system("pause"); return 1;
    }
};

// --- 8. REGISTER FORM ---
class RegisterForm : public BaseForm {
public:
    RegisterForm(Business::NganHang& nh) : BaseForm(nh) {}
    int Show() override {
        Components::Header("DANG KY KHACH HANG & MO TAI KHOAN");
        string stk = Components::TextBox("So TK muon tao");
        auto dsTK = Data::DataManager::LoadTaiKhoan("featureData/accounts.csv");
        for (auto t : dsTK) { if(t->GetSoTaiKhoan() == stk) { Components::Label("Loi: STK da ton tai!", true); system("pause"); for(auto x:dsTK) delete x; return 1; } }
        for(auto x:dsTK) delete x;

        string ten = Components::TextBox("Ho va Ten");
        string sdt = Components::TextBox("So dien thoai");
        string dc = Components::TextBox("Dia chi");
        string pin = Components::TextBox("Ma PIN bao mat");
        int type = Components::ComboBox("Loai tai khoan", {"Thanh Toan (Co san 50k)", "Tiet Kiem"});

        Models::TaiKhoan* n = (type == 1) ? (Models::TaiKhoan*)new Models::TaiKhoanThanhToan(stk, ten, 50000, pin) : (Models::TaiKhoan*)new Models::TaiKhoanTietKiem(stk, ten, 0, 0.05, 12, pin);
        nganHang.ThemTaiKhoan(n, pin);
        SyncData();

        auto dsKH = Data::DataManager::LoadKhachHang("featureData/customers.csv");
        dsKH.push_back(Models::KhachHang("KH" + to_string(dsKH.size() + 1), ten, sdt, dc));
        Data::DataManager::SaveKhachHang(dsKH, "featureData/customers.csv");

        Components::Label("Chuc mung! Tai khoan va Ho so khach hang da duoc tao.");
        system("pause"); return 1;
    }
};

// --- 2. ADMIN MENU ---
class AdminMenuForm : public BaseForm {
public:
    AdminMenuForm(Business::NganHang& nh) : BaseForm(nh) {}
    int Show() override {
        Components::Header("ADMIN MENU");
        int c = Components::Menu({"QL Khach hang", "QL Tai khoan", "Giao dich tai quay", "Sao ke he thong", "Liet ke danh sach (Console Style)", "Tinh lai cho tat ca"});
        if (c == 1) return 3;
        if (c == 2) return 4;
        if (c == 3) return 5;
        if (c == 4) return 6;
        if (c == 5) {
            string ds = nganHang.LietKeDanhSach();
            Components::Header("KET QUA LIET KE");
            cout << ds << endl;
            system("pause");
            return 2;
        }
        if (c == 6) {
            nganHang.TinhLaiVaCapNhat();
            SyncData();
            Components::Label("Da tinh lai va cap nhat cho tat ca tai khoan!");
            system("pause");
            return 2;
        }
        return 1;
    }
};

// --- 3. CUSTOMER MANAGER ---
class CustomerForm : public BaseForm {
public:
    CustomerForm(Business::NganHang& nh) : BaseForm(nh) {}
    int Show() override {
        Components::Header("DANH SACH KHACH HANG");
        auto dsKH = Data::DataManager::LoadKhachHang("featureData/customers.csv");
        vector<string> h = {"Ma KH", "Ho Ten", "SDT", "Dia Chi"};
        vector<vector<string>> d;
        for (const auto& kh : dsKH) { vector<string> r; r.push_back(kh.GetMaKH()); r.push_back(kh.GetHoTen()); r.push_back(kh.GetSoDienThoai()); r.push_back(kh.GetDiaChi()); d.push_back(r); }
        Components::Table(h, d);
        
        int c = Components::Menu({"Xoa Khach hang"});
        if (c == 1) {
            string ma = Components::TextBox("Nhap Ma KH can xoa");
            auto it = remove_if(dsKH.begin(), dsKH.end(), [&](const Models::KhachHang& k) { return k.GetMaKH() == ma; });
            if (it != dsKH.end()) { dsKH.erase(it, dsKH.end()); Data::DataManager::SaveKhachHang(dsKH, "featureData/customers.csv"); Components::Label("Da xoa!"); }
        }
        if (c == 0) return 2;
        system("pause"); return 3;
    }
};

// --- 4. ACCOUNT MANAGER (FULL ADMIN RIGHTS) ---
class AccountForm : public BaseForm {
public:
    AccountForm(Business::NganHang& nh) : BaseForm(nh) {}
    int Show() override {
        Components::Header("QUAN LY TAI KHOAN (ADMIN)");
        auto dsTK = Data::DataManager::LoadTaiKhoan("featureData/accounts.csv");
        vector<string> h = {"Loai", "So TK", "Chu TK", "PIN", "So Du", "T.Thai"};
        vector<vector<string>> d;
        for (auto tk : dsTK) {
            string bal = to_string((long long)tk->GetSoDu());
            vector<string> r; r.push_back("TK"); r.push_back(tk->GetSoTaiKhoan()); r.push_back(tk->GetTenKhachHang()); r.push_back(tk->GetMaPIN()); r.push_back(bal); r.push_back(tk->IsLocked() ? "KHOA" : "MO");
            d.push_back(r); delete tk;
        }
        Components::Table(h, d);
        
        int c = Components::Menu({"Mo tai khoan moi", "Xoa tai khoan", "Khoa/Mo khoa tai khoan", "Sua ten chu tai khoan"});
        if (c == 1) {
            string stk = Components::TextBox("So TK");
            string ten = Components::TextBox("Chu TK");
            string pin = Components::TextBox("Ma PIN");
            double du = stod(Components::TextBox("So du") + "0");
            int type = Components::ComboBox("Loai", {"Thanh Toan", "Tiet Kiem"});
            Models::TaiKhoan* n = (type == 1) ? (Models::TaiKhoan*)new Models::TaiKhoanThanhToan(stk, ten, du, pin) : (Models::TaiKhoan*)new Models::TaiKhoanTietKiem(stk, ten, du, 0.05, 12, pin);
            if (n) { nganHang.ThemTaiKhoan(n, pin); SyncData(); Components::Label("Thanh cong!"); }
        } else if (c == 2) {
            string stk = Components::TextBox("STK can xoa");
            if (nganHang.XoaTaiKhoan(stk)) { SyncData(); Components::Label("Da xoa!"); }
        } else if (c == 3) {
            string stk = Components::TextBox("Nhap STK");
            int tt = Components::ComboBox("Trang thai", {"Mo khoa", "Khoa"});
            nganHang.KhoaTaiKhoan(stk, (tt == 2)); SyncData();
            Components::Label("Da cap nhat!");
        } else if (c == 4) {
            string stk = Components::TextBox("Nhap STK can sua");
            string ten = Components::TextBox("Ten moi");
            if (nganHang.SuaThongTin(stk, ten)) { SyncData(); Components::Label("Da cap nhat!"); }
            else { Components::Label("Loi: Khong tim thay STK!", true); }
        }
        
        if (c == 0) return 2;
        system("pause"); return 4;
    }
};

// --- 5. TRANSACTION FORM ---
class TransactionForm : public BaseForm {
public:
    TransactionForm(Business::NganHang& nh) : BaseForm(nh) {}
    int Show() override {
        Components::Header("GIAO DICH TAI QUAY");
        int c = Components::Menu({"Nap tien", "Rut tien", "Chuyen tien"});
        if (c == 0) return 2;
        string stk = Components::TextBox("So TK");
        double m = stod(Components::TextBox("So tien") + "0");
        try {
            if (c == 1) nganHang.NapTien(stk, m);
            else {
                string pin = Components::TextBox("PIN xac thuc");
                if (c == 2) nganHang.RutTien(stk, m, pin);
                else nganHang.ChuyenTien(stk, Components::TextBox("TK nhan"), m, pin);
            }
            SyncData(); Components::Label("Thanh cong!");
        } catch (const exception& e) { Components::Label(e.what(), true); }
        system("pause"); return 5;
    }
};

// --- 6. SEARCH FORM ---
class SearchForm : public BaseForm {
public:
    SearchForm(Business::NganHang& nh) : BaseForm(nh) {}
    int Show() override {
        Components::Header("SAO KE HE THONG");
        nganHang.InSaoKe(Components::TextBox("Nhap STK"));
        system("pause"); return 2;
    }
};

// --- 7. CUSTOMER MENU ---
class CustomerMenuForm : public BaseForm {
public:
    CustomerMenuForm(Business::NganHang& nh) : BaseForm(nh) {}
    int Show() override {
        Components::Header("DICH VU KHACH HANG");
        cout << " [ TK ]: " << CURRENT_USER_STK << "\n";
        int c = Components::Menu({"Xem so du", "Nap tien", "Rut tien", "Chuyen tien", "Sao ke", "KHOA/MO KHOA TK", "Xem lai du kien"});
        try {
            if (c == 1) {
                auto ds = Data::DataManager::LoadTaiKhoan("featureData/accounts.csv");
                for (auto tk : ds) {
                    if (tk->GetSoTaiKhoan() == CURRENT_USER_STK) {
                        vector<string> h = {"STK", "Chu TK", "So Du", "T.Thai"};
                        vector<vector<string>> d;
                        vector<string> r; r.push_back(tk->GetSoTaiKhoan()); r.push_back(tk->GetTenKhachHang()); r.push_back(to_string((long long)tk->GetSoDu())); r.push_back(tk->IsLocked() ? "KHOA" : "MO");
                        d.push_back(r); Components::Table(h, d);
                    }
                    delete tk;
                }
            } else if (c == 2) {
                nganHang.NapTien(CURRENT_USER_STK, stod(Components::TextBox("So tien nap"))); SyncData();
            } else if (c == 3) {
                nganHang.RutTien(CURRENT_USER_STK, stod(Components::TextBox("So tien rut")), Components::TextBox("PIN")); SyncData();
            } else if (c == 4) {
                string stkD = Components::TextBox("TK nhan");
                double m = stod(Components::TextBox("So tien chuyen"));
                nganHang.ChuyenTien(CURRENT_USER_STK, stkD, m, Components::TextBox("PIN")); SyncData();
            } else if (c == 5) {
                nganHang.InSaoKe(CURRENT_USER_STK);
            } else if (c == 6) {
                int tt = Components::ComboBox("Trang thai moi", {"Mo khoa", "Khoa"});
                nganHang.KhoaTaiKhoan(CURRENT_USER_STK, (tt == 2)); SyncData();
                Components::Label("Da cap nhat!");
            } else if (c == 7) {
                const auto& ds = nganHang.GetDanhSach();
                for (auto& tk : ds) {
                    if (tk->GetSoTaiKhoan() == CURRENT_USER_STK) {
                        double lai = tk->TinhLai();
                        Components::Label("Lai du kien: " + to_string((long long)lai) + " VND");
                    }
                }
            } else if (c == 0) return 1;
        } catch (const exception& e) { Components::Label(e.what(), true); }
        system("pause"); return 7;
    }
};

class BankApp {
private:
    Business::NganHang nganHang;
    int currentID;
    unique_ptr<BaseForm> currentForm;
public:
    BankApp() : currentID(1) {
        auto ds = Data::DataManager::LoadTaiKhoan("featureData/accounts.csv");
        for (auto tk : ds) nganHang.ThemTaiKhoan(tk, tk->GetMaPIN());
        ifstream f("featureData/history.csv");
        string l;
        while(getline(f, l)) {
            if(l.empty()) continue;
            stringstream ss(l);
            string stk, loai, tien, nd, time;
            getline(ss, stk, ';'); getline(ss, loai, ';');
            getline(ss, tien, ';'); getline(ss, nd, ';');
            if(getline(ss, time, ';')) nganHang.NapLichSu(stk, loai, stod(tien), nd);
        }
        f.close();
    }
    void Run() {
        while (currentID != 0) {
            switch (currentID) {
                case 1: currentForm = make_unique<StartMenuForm>(nganHang); break;
                case 9: currentForm = make_unique<LoginForm>(nganHang); break;
                case 8: currentForm = make_unique<RegisterForm>(nganHang); break;
                case 2: currentForm = make_unique<AdminMenuForm>(nganHang); break;
                case 3: currentForm = make_unique<CustomerForm>(nganHang); break;
                case 4: currentForm = make_unique<AccountForm>(nganHang); break;
                case 5: currentForm = make_unique<TransactionForm>(nganHang); break;
                case 6: currentForm = make_unique<SearchForm>(nganHang); break;
                case 7: currentForm = make_unique<CustomerMenuForm>(nganHang); break;
                default: currentID = 1; continue;
            }
            currentID = currentForm->Show();
        }
    }
};

}
}
