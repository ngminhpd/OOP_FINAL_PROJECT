#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include "../featureBussiness/NganHang.h"
#include "../featureData/DataManager.h"
#include "httplib.h"

using namespace std;
using namespace BankManagementSystem;

// --- HÀM GIẢI MÃ URL (Sửa lỗi %20 trong tên) ---
string url_decode(string str) {
    string res;
    char c;
    int i, ii;
    for (i = 0; i < str.length(); i++) {
        if (str[i] == '%') {
            sscanf(str.substr(i + 1, 2).c_str(), "%x", &ii);
            c = static_cast<char>(ii);
            res += c;
            i = i + 2;
        } else if (str[i] == '+') {
            res += ' ';
        } else {
            res += str[i];
        }
    }
    return res;
}

string format_ma_kh(int count) {
    stringstream ss;
    ss << "KH" << setfill('0') << setw(2) << count;
    return ss.str();
}

template <typename T> string num_to_str(T n) { stringstream ss; ss << n; return ss.str(); }

void sync_accounts(Business::NganHang& nh) {
    const auto& dsShared = nh.GetDanhSach();
    vector<Models::TaiKhoan*> raw;
    for (size_t i = 0; i < dsShared.size(); ++i) raw.push_back(dsShared[i].get());
    Data::DataManager::SaveTaiKhoan(raw, "featureData/accounts.csv");
}

int main() {
    Business::NganHang nh;
    auto dsCu = Data::DataManager::LoadTaiKhoan("featureData/accounts.csv");
    for (size_t i = 0; i < dsCu.size(); ++i) nh.ThemTaiKhoan(dsCu[i], dsCu[i]->GetMaPIN());

    httplib::Server svr;
    cout << "BANKING SYSTEM ONLINE. PERSISTENCE ACTIVE." << endl;

    svr.Get("/api/register", [&](const httplib::Request& req, httplib::Response& res) {
        // Giải mã URL cho tên và địa chỉ
        string s = req.get_param_value("stk");
        string t = url_decode(req.get_param_value("ten"));
        string p = req.get_param_value("pin");
        string sdt = req.get_param_value("sdt");
        string dc = url_decode(req.get_param_value("dc"));
        
        cout << "[REGISTER] Name: " << t << " STK: " << s << endl;

        // 1. Lưu Khách hàng
        auto dsKH = Data::DataManager::LoadKhachHang("featureData/customers.csv");
        string maKH = format_ma_kh(dsKH.size() + 1);
        dsKH.push_back(Models::KhachHang(maKH, t, sdt, dc));
        Data::DataManager::SaveKhachHang(dsKH, "featureData/customers.csv");

        // 2. Lưu Tài khoản
        nh.ThemTaiKhoan(new Models::TaiKhoanThanhToan(s, t, 50000, p), p);
        sync_accounts(nh);

        res.set_content("success", "text/plain");
    });

    // --- CÁC API KHÁC GIỮ NGUYÊN NHƯNG ĐÃ ĐƯỢC KIỂM TRA ---
    svr.Get("/", [&](const httplib::Request&, httplib::Response& res) { 
        ifstream f("featureUI/index.html");
        res.set_content(string((istreambuf_iterator<char>(f)), istreambuf_iterator<char>()), "text/html");
    });
    svr.Get("/style.css", [&](const httplib::Request&, httplib::Response& res) { 
        ifstream f("featureUI/style.css");
        res.set_content(string((istreambuf_iterator<char>(f)), istreambuf_iterator<char>()), "text/css");
    });
    svr.Get("/api/login", [&](const httplib::Request& req, httplib::Response& res) {
        string u = req.get_param_value("user"), p = req.get_param_value("pin");
        if(u=="admin" && p=="123") { res.set_content("{\"status\":\"success\",\"role\":\"admin\"}", "application/json"); return; }
        for(auto& tk : nh.GetDanhSach()) if(tk->GetSoTaiKhoan()==u && tk->GetMaPIN()==p) {
            res.set_content("{\"status\":\"success\",\"role\":\"user\",\"stk\":\""+u+"\",\"name\":\""+tk->GetTenKhachHang()+"\"}", "application/json"); return;
        }
        res.set_content("{\"status\":\"error\"}", "application/json");
    });
    svr.Get("/api/user/info", [&](const httplib::Request& req, httplib::Response& res) {
        string stk = req.get_param_value("stk");
        for(auto& tk : nh.GetDanhSach()) if(tk->GetSoTaiKhoan()==stk) { res.set_content("{\"balance\":"+num_to_str((long long)tk->GetSoDu())+"}", "application/json"); return; }
    });
    svr.Get("/api/admin/all", [&](const httplib::Request&, httplib::Response& res) {
        string j = "["; const auto& d = nh.GetDanhSach();
        for(size_t i=0; i<d.size(); ++i) {
            j += "{\"stk\":\""+d[i]->GetSoTaiKhoan()+"\",\"name\":\""+d[i]->GetTenKhachHang()+"\",\"balance\":"+num_to_str((long long)d[i]->GetSoDu())+",\"locked\":"+(d[i]->IsLocked()?"true":"false")+"}";
            if(i<d.size()-1) j += ",";
        }
        j += "]"; res.set_content(j, "application/json");
    });
    svr.Get("/api/user/transact", [&](const httplib::Request& req, httplib::Response& res) {
        string s = req.get_param_value("stk"), type = req.get_param_value("type");
        double a = 100000;
        try { if(type=="nap") nh.NapTien(s, a); else nh.RutTien(s, a, "1234"); sync_accounts(nh); res.set_content("success", "text/plain"); }
        catch(...) { res.set_content("error", "text/plain"); }
    });

    svr.listen("localhost", 8080);
    return 0;
}
