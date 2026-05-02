#pragma once
#include "Components.h"
#include "../final/BankManager.h"
#include <memory>
#include <vector>
#include <string>
#include <algorithm>
#include <fstream>
#include <map>

namespace BankManagementSystem {
namespace UI {
using namespace std;
using namespace BankSystem;

static string CURRENT_USER_STK = "";

class BaseForm {
protected:
    BankManager& manager;
public:
    BaseForm(BankManager& m) : manager(m) {}
    virtual ~BaseForm() {}
    virtual int Show() = 0;

    double SafeStod(string s) {
        try { return stod(s); }
        catch (...) { return 0; }
    }
};

// --- 1. START MENU ---
class StartMenuForm : public BaseForm {
public:
    using BaseForm::BaseForm;
    int Show() override {
        Components::Header("HE THONG NGAN HANG OOP (CONSOLE VERSION)");
        int c = Components::Menu({"Dang nhap", "Mo tai khoan moi"});
        if (c == 1) return 9;
        if (c == 2) return 8;
        return 0;
    }
};

// --- 9. LOGIN FORM ---
class LoginForm : public BaseForm {
public:
    using BaseForm::BaseForm;
    int Show() override {
        Components::Header("DANG NHAP");
        string user = Components::TextBox("STK / Username");
        string pass = Components::TextBox("Ma PIN");
        
        if (user == "admin" && pass == "123") return 2;
        
        auto tk = manager.Tim(user);
        if (tk && tk->GetMaPIN() == pass) {
            if (tk->IsLocked()) {
                Components::Label("Tai khoan dang bi khoa!", true);
                system("pause");
                return 1;
            }
            CURRENT_USER_STK = user;
            return 7;
        }
        
        Components::Label("Sai thong tin dang nhap!", true);
        system("pause");
        return 1;
    }
};

// --- 8. REGISTER FORM ---
class RegisterForm : public BaseForm {
public:
    using BaseForm::BaseForm;
    int Show() override {
        Components::Header("MO TAI KHOAN MOI");
        string stk = Components::TextBox("So TK muon tao");
        if (manager.Tim(stk)) {
            Components::Label("Loi: STK da ton tai!", true);
            system("pause");
            return 1;
        }

        string ten = Components::TextBox("Ho va Ten");
        string pin = Components::TextBox("Ma PIN");
        int type = Components::ComboBox("Loai tai khoan", {"Thanh Toan (Tang 50k)", "Tiet Kiem"});
        if (type <= 0) return 1;

        shared_ptr<TaiKhoan> n;
        if (type == 1) n = make_shared<TaiKhoanThanhToan>(stk, ten, 50000, pin);
        else n = make_shared<TaiKhoanTietKiem>(stk, ten, 50000, 0.05, 12, pin);

        manager.Add(n);
        manager.Save();

        Components::Label("Chuc mung! Tai khoan da duoc tao thanh cong.");
        system("pause");
        return 1;
    }
};

// --- 2. ADMIN MENU ---
class AdminMenuForm : public BaseForm {
public:
    using BaseForm::BaseForm;
    int Show() override {
        Components::Header("ADMIN CONTROL PANEL");
        cout << " [ He thong ]: Lan cuoi tinh lai: " << manager.GetThoiGianTinhLai() << "\n";
        int c = Components::Menu({
            "Quan ly Tai khoan", 
            "Phe duyet Yeu cau", 
            "Quan ly Khoan vay", 
            "Giao dich tai quay", 
            "Tinh lai toan he thong"
        });
        if (c == 1) return 4;
        if (c == 2) return 10;
        if (c == 3) return 11;
        if (c == 4) return 5;
        if (c == 5) {
            manager.TinhLaiHeThong();
            manager.Save();
            Components::Label("Da tinh lai va cap nhat cho toan he thong!");
            system("pause");
            return 2;
        }
        return 1;
    }
};

// --- 4. ACCOUNT MANAGER ---
class AccountForm : public BaseForm {
public:
    using BaseForm::BaseForm;
    int Show() override {
        Components::Header("QUAN LY TAI KHOAN");
        auto ds = manager.GetDS();
        vector<string> h = {"STK", "Chu TK", "Loai", "So Du", "Hang", "Trang Thai"};
        vector<vector<string>> d;
        for (auto& tk : ds) {
            d.push_back({tk->GetSoTaiKhoan(), tk->GetTenKhachHang(), tk->GetLoai(), to_string((long long)tk->GetSoDu()), tk->GetHang(), tk->IsLocked() ? "KHOA" : "MO"});
        }
        Components::Table(h, d);
        
        int c = Components::Menu({"Mo tai khoan", "Xoa tai khoan", "Khoa/Mo khoa", "Sua hang thanh vien"});
        if (c == 1) {
            string stk = Components::TextBox("So TK");
            string ten = Components::TextBox("Chu TK");
            string pin = Components::TextBox("PIN");
            double du = SafeStod(Components::TextBox("So du dau"));
            int type = Components::ComboBox("Loai", {"Thanh Toan", "Tiet Kiem", "Tin Dung"});
            if (type > 0) {
                shared_ptr<TaiKhoan> n;
                if (type == 1) n = make_shared<TaiKhoanThanhToan>(stk, ten, du, pin);
                else if (type == 2) n = make_shared<TaiKhoanTietKiem>(stk, ten, du, 0.05, 12, pin);
                else n = make_shared<TaiKhoanTinDung>(stk, ten, 0, du, pin);
                manager.Add(n); manager.Save();
                Components::Label("Thanh cong!");
            }
        } else if (c == 2) {
            manager.XoaTK(Components::TextBox("STK can xoa"));
            manager.Save();
            Components::Label("Da xoa!");
        } else if (c == 3) {
            string stk = Components::TextBox("Nhap STK");
            auto tk = manager.Tim(stk);
            if (tk) {
                int tt = Components::ComboBox("Trang thai", {"Mo khoa", "Khoa"});
                if (tt > 0) { tk->SetLocked(tt == 2); manager.Save(); Components::Label("Da cap nhat!"); }
            }
        } else if (c == 4) {
            string stk = Components::TextBox("STK");
            auto tk = manager.Tim(stk);
            if (tk) {
                vector<string> tiers = {"Thanh vien", "VIP", "Signature", "Private"};
                int t = Components::ComboBox("Chon hang moi", tiers);
                if (t > 0) { tk->SetHang(tiers[t-1]); manager.Save(); Components::Label("Da cap nhat!"); }
            }
        }
        
        if (c == 0) return 2;
        system("pause"); return 4;
    }
};

// --- 10. REQUEST APPROVER ---
class RequestApproverForm : public BaseForm {
public:
    using BaseForm::BaseForm;
    int Show() override {
        Components::Header("PHE DUYET YEU CAU");
        auto ds = manager.GetYeuCau();
        if (ds.empty()) {
            cout << " Khong co yeu cau nao cho xu ly.\n";
            system("pause"); return 2;
        }

        vector<string> h = {"STK", "Loai", "Noi dung/Gia tri", "Thoi gian"};
        vector<vector<string>> d;
        vector<string> options;
        for (auto& r : ds) {
            d.push_back({r.stk, r.type, r.val + " (" + r.hang + ")", r.thoiGian});
            options.push_back("[" + r.type + "] " + r.stk + " - " + r.ten);
        }
        Components::Table(h, d);

        int c = Components::ComboBox("Chon yeu cau de xu ly", options);
        if (c <= 0 || c > (int)ds.size()) return 2;

        auto selected = ds[c-1];
        int act = Components::Menu({"Duyet (Approve)", "Tu choi (Reject)"});
        
        if (act == 1) {
            auto tk = manager.Tim(selected.stk);
            if (tk) {
                if (selected.type == "LOAN") manager.AddVay(selected.stk, stod(selected.val), stoi(selected.hang));
                else if (selected.type == "UPGRADE") tk->SetHang(selected.val);
                else if (selected.type == "CREDIT") tk->SetHanMucTinDung(stod(selected.val));
                else if (selected.type == "RESET_PIN") tk->SetMaPIN("1234");
                
                manager.KiemTraVaXoaYeuCau(selected.stk, selected.type);
                manager.Save();
                Components::Label("Da duyet yeu cau!");
            } else Components::Label("Loi: Tai khoan khong ton tai!", true);
        } else if (act == 2) {
            manager.XoaYeuCau(selected.stk, selected.type);
            manager.Save();
            Components::Label("Da tu choi yeu cau!");
        }

        system("pause"); return 10;
    }
};

// --- 11. LOAN MANAGER ---
class LoanManagerForm : public BaseForm {
public:
    using BaseForm::BaseForm;
    int Show() override {
        Components::Header("QUAN LY KHOAN VAY");
        int c = Components::Menu({"Danh sach vay hien tai", "Lich su vay"});
        if (c == 0) return 2;

        if (c == 1) {
            auto ds = manager.GetDSVay();
            vector<string> h = {"STK", "So Tien", "Lai Suat", "Con lai (s)"};
            vector<vector<string>> d;
            long long now = time(0);
            for (auto& v : ds) d.push_back({v.stk, to_string((long long)v.soTien), to_string(v.laiSuat), to_string(v.hanTra - now)});
            Components::Table(h, d);
        } else {
            auto ds = manager.GetLichSuVay();
            vector<string> h = {"STK", "Ten", "So Tien", "Han", "Trang Thai"};
            vector<vector<string>> d;
            for (auto& l : ds) d.push_back({l.stk, l.ten, to_string((long long)l.soTien), l.hanVay, l.trangThai});
            Components::Table(h, d);
        }
        system("pause"); return 11;
    }
};

// --- 5. TRANSACTION FORM ---
class TransactionForm : public BaseForm {
public:
    using BaseForm::BaseForm;
    int Show() override {
        Components::Header("GIAO DICH TAI QUAY");
        string stk = Components::TextBox("So TK");
        auto tk = manager.Tim(stk);
        if (!tk) { Components::Label("Khong tim thay TK!", true); system("pause"); return 2; }

        int c = Components::Menu({"Nap tien", "Rut tien"});
        if (c <= 0) return 2;

        double amt = SafeStod(Components::TextBox("So tien"));
        if (amt <= 0) { Components::Label("So tien khong hop le!", true); system("pause"); return 5; }

        if (c == 1) {
            tk->NapTien(amt);
            manager.GhiLog(stk, "NAP", amt, "Nap tai quay");
            Components::Label("Thanh cong!");
        } else {
            if (tk->RutTien(amt)) {
                manager.GhiLog(stk, "RUT", -amt, "Rut tai quay");
                Components::Label("Thanh cong!");
            } else Components::Label("Khong du so du!", true);
        }
        manager.Save();
        system("pause"); return 2;
    }
};

// --- 7. USER MENU ---
class UserMenuForm : public BaseForm {
public:
    using BaseForm::BaseForm;
    int Show() override {
        auto tk = manager.Tim(CURRENT_USER_STK);
        if (!tk) return 1;

        Components::Header("DICH VU KHACH HANG - " + tk->GetTenKhachHang());
        cout << " [ TK ]: " << tk->GetSoTaiKhoan() << " | Hang: " << tk->GetHang() << "\n";
        cout << " [ So du ]: " << (long long)tk->GetSoDu() << " VND | Lai du kien: " << (long long)tk->TinhLai() << " VND\n";
        if (tk->GetLoai() == "TinDung") cout << " [ Tin dung ]: Han muc: " << (long long)tk->GetHanMucTinDung() << " | No: " << (long long)tk->GetNoTinDung() << "\n";
        if (tk->GetSoDuVay() > 0) cout << " [ No vay ]: " << (long long)tk->GetSoDuVay() << " VND\n";

        int c = Components::Menu({
            "Nap/Rut/Chuyen tien", 
            "Lich su giao dich", 
            "Dich vu Vay von", 
            "The Tin dung & Nang hang", 
            "Doi ma PIN"
        });

        if (c == 1) return 12;
        if (c == 2) {
            auto lsMap = manager.GetLS();
            if (lsMap.count(CURRENT_USER_STK)) {
                Components::Header("LICH SU GIAO DICH");
                auto v = lsMap.at(CURRENT_USER_STK);
                vector<string> h = {"Thoi gian", "Loai", "So tien", "Noi dung"};
                vector<vector<string>> d;
                for (int i = v.size() - 1; i >= 0 && i >= (int)v.size() - 15; --i) d.push_back({v[i].thoiGian, v[i].loai, to_string((long long)v[i].soTien), v[i].noiDung});
                Components::Table(h, d);
            } else Components::Label("Chua co giao dich nao.");
            system("pause"); return 7;
        }
        if (c == 3) return 13;
        if (c == 4) return 14;
        if (c == 5) {
            string oldP = Components::TextBox("PIN cu");
            if (tk->GetMaPIN() == oldP) {
                tk->SetMaPIN(Components::TextBox("PIN moi"));
                manager.Save();
                Components::Label("Doi PIN thanh cong!");
            } else Components::Label("Sai PIN!", true);
            system("pause"); return 7;
        }
        
        if (c == 0) return 1;
        return 7;
    }
};

// --- 12. USER TRANSACTIONS ---
class UserTransactForm : public BaseForm {
public:
    using BaseForm::BaseForm;
    int Show() override {
        auto tk = manager.Tim(CURRENT_USER_STK);
        Components::Header("GIAO DICH");
        int c = Components::Menu({"Nap tien", "Rut tien", "Chuyen tien"});
        if (c <= 0) return 7;

        string source = "main";
        if (tk->GetLoai() == "TinDung") {
            int src = Components::ComboBox("Nguon tien", {"Tai khoan chinh", "The tin dung"});
            if (src == 2) source = "credit";
            else if (src <= 0) return 12;
        }

        double amt = SafeStod(Components::TextBox("So tien"));
        if (amt <= 0) { Components::Label("So tien khong hop le!", true); system("pause"); return 12; }
        
        string pin = Components::TextBox("Xac nhan PIN");
        if (tk->GetMaPIN() != pin) { Components::Label("Sai PIN!", true); system("pause"); return 12; }

        if (c == 1) {
            if (source == "credit") { tk->NapTienTinDung(amt); manager.GhiLog(CURRENT_USER_STK, "TRA_NO_TD", amt, "Thanh toan no TD"); }
            else { tk->NapTien(amt); manager.GhiLog(CURRENT_USER_STK, "NAP", amt, "Nap tien"); }
            Components::Label("Thanh cong!");
        } else if (c == 2) {
            bool ok = (source == "credit") ? tk->RutTienTinDung(amt) : tk->RutTien(amt);
            if (ok) { manager.GhiLog(CURRENT_USER_STK, source == "credit" ? "RUT_TD" : "RUT", -amt, "Rut tien"); Components::Label("Thanh cong!"); }
            else Components::Label("Khong du so du/han muc!", true);
        } else if (c == 3) {
            string destS = Components::TextBox("STK thu huong");
            auto dt = manager.Tim(destS);
            if (!dt) Components::Label("TK thu huong khong ton tai!", true);
            else if (destS == CURRENT_USER_STK) Components::Label("Khong the chuyen cho chinh minh!", true);
            else {
                bool ok = (source == "credit") ? tk->RutTienTinDung(amt) : tk->RutTien(amt);
                if (ok) {
                    dt->NapTien(amt);
                    manager.GhiLog(CURRENT_USER_STK, "CHUYEN", -amt, "Chuyen den " + destS);
                    manager.GhiLog(destS, "NHAN", amt, "Nhan tu " + CURRENT_USER_STK);
                    Components::Label("Thanh cong!");
                } else Components::Label("Khong du so du/han muc!", true);
            }
        }
        manager.Save();
        system("pause"); return 7;
    }
};

// --- 13. USER LOAN SERVICES ---
class UserLoanForm : public BaseForm {
public:
    using BaseForm::BaseForm;
    int Show() override {
        Components::Header("DICH VU VAY VON");
        int c = Components::Menu({"Gui yeu cau vay", "Xem khoan vay hien tai", "Tat toan khoan vay"});
        if (c <= 0) return 7;

        if (c == 1) {
            double amt = SafeStod(Components::TextBox("So tien muon vay"));
            int phut = SafeStod(Components::TextBox("Thoi han vay (phut)"));
            if (amt <= 0 || phut <= 0) { Components::Label("Thong tin khong hop le!", true); }
            else {
                auto tk = manager.Tim(CURRENT_USER_STK);
                manager.AddYeuCau(YeuCau("LOAN", CURRENT_USER_STK, tk->GetTenKhachHang(), to_string((long long)amt), to_string(phut)));
                manager.Save();
                Components::Label("Da gui yeu cau vay! Vui long cho Admin duyet.");
            }
        } else if (c == 2) {
            auto ds = manager.GetDSVay();
            bool found = false;
            for (auto& v : ds) if (v.stk == CURRENT_USER_STK) {
                cout << " > Khoan vay: " << (long long)v.soTien << " VND | Lai suat: " << v.laiSuat * 100 << "%\n";
                cout << " > Tong no: " << (long long)(v.soTien * (1 + v.laiSuat)) << " VND | Con lai: " << (v.hanTra - time(0)) << "s\n";
                found = true; break;
            }
            if (!found) Components::Label("Ban khong co khoan vay nao.");
        } else if (c == 3) {
            string pin = Components::TextBox("Xac nhan PIN");
            auto tk = manager.Tim(CURRENT_USER_STK);
            if (tk->GetMaPIN() == pin) {
                if (manager.TatToanVay(CURRENT_USER_STK)) Components::Label("Da tat toan khoan vay!");
                else Components::Label("Khong du tien hoac khong co khoan vay!", true);
            } else Components::Label("Sai PIN!", true);
        }
        system("pause"); return 13;
    }
};

// --- 14. CREDIT & UPGRADE ---
class UserCreditForm : public BaseForm {
public:
    using BaseForm::BaseForm;
    int Show() override {
        Components::Header("DICH VU CA NHAN NANG CAO");
        int c = Components::Menu({
            "Yeu cau mo the Tin dung", 
            "Yeu cau nang hang thanh vien",
            "Yeu cau cap lai ma PIN (Reset PIN)"
        });
        if (c <= 0) return 7;

        auto tk = manager.Tim(CURRENT_USER_STK);
        if (c == 1) {
            long long limit = 20000000;
            if (tk->GetHang() == "VIP") limit = 40000000;
            else if (tk->GetHang() == "Signature") limit = 60000000;
            manager.AddYeuCau(YeuCau("CREDIT", CURRENT_USER_STK, tk->GetTenKhachHang(), to_string(limit), tk->GetHang()));
            Components::Label("Da gui yeu cau mo the!");
        } else if (c == 2) {
            vector<string> tiers = {"VIP", "Signature", "Private"};
            int t = Components::ComboBox("Chon hang muon nang", tiers);
            if (t > 0) {
                manager.AddYeuCau(YeuCau("UPGRADE", CURRENT_USER_STK, tk->GetTenKhachHang(), tiers[t-1], ""));
                Components::Label("Da gui yeu cau nang hang!");
            }
        } else if (c == 3) {
            manager.AddYeuCau(YeuCau("RESET_PIN", CURRENT_USER_STK, tk->GetTenKhachHang(), "Yeu cau cap lai PIN", ""));
            Components::Label("Da gui yeu cau Reset PIN!");
        }
        manager.Save();
        system("pause"); return 14;
    }
};


class BankApp {
private:
    BankManager manager;
    int currentID;
    unique_ptr<BaseForm> currentForm;
public:
    BankApp() : currentID(1) { manager.Load(); }
    void Run() {
        while (currentID != 0) {
            switch (currentID) {
                case 1: currentForm = make_unique<StartMenuForm>(manager); break;
                case 9: currentForm = make_unique<LoginForm>(manager); break;
                case 8: currentForm = make_unique<RegisterForm>(manager); break;
                case 2: currentForm = make_unique<AdminMenuForm>(manager); break;
                case 4: currentForm = make_unique<AccountForm>(manager); break;
                case 10: currentForm = make_unique<RequestApproverForm>(manager); break;
                case 11: currentForm = make_unique<LoanManagerForm>(manager); break;
                case 5: currentForm = make_unique<TransactionForm>(manager); break;
                case 7: currentForm = make_unique<UserMenuForm>(manager); break;
                case 12: currentForm = make_unique<UserTransactForm>(manager); break;
                case 13: currentForm = make_unique<UserLoanForm>(manager); break;
                case 14: currentForm = make_unique<UserCreditForm>(manager); break;
                default: currentID = 1; continue;
            }
            currentID = currentForm->Show();
        }
    }
};

}
}
