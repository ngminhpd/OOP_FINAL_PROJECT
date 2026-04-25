#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <algorithm>
#include <ctime>
#include <iomanip>
#include <exception>

#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#else
#include <sys/stat.h>
#include <unistd.h>
#endif

#include "httplib.h"

using namespace std;

// ============================================================
// MODELS
// ============================================================
struct GiaoDich {
    string thoiGian, loai, noiDung;
    double soTien;
    GiaoDich(string l, double st, string nd) : loai(l), soTien(st), noiDung(nd) {
        time_t t = time(nullptr); tm* now = localtime(&t); char buf[80];
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", now); thoiGian = string(buf);
    }
    GiaoDich(string tg, string l, double st, string nd) : thoiGian(tg), loai(l), soTien(st), noiDung(nd) {}
};

struct YeuCau {
    string type, stk, ten, val, hang, pin, thoiGian;
    YeuCau(string tp, string s, string t, string v, string h, string p="") 
        : type(tp), stk(s), ten(t), val(v), hang(h), pin(p) {
        time_t now = time(0); tm *ltm = localtime(&now); char buf[80];
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", ltm); thoiGian = string(buf);
    }
    YeuCau(string tp, string s, string t, string v, string h, string p, string tg) 
        : type(tp), stk(s), ten(t), val(v), hang(h), pin(p), thoiGian(tg) {}
};

struct KhoanVay {
    string stk; double soTien; long long hanTra; // Unix timestamp
    KhoanVay(string s, double st, long long ht) : stk(s), soTien(st), hanTra(ht) {}
};

class TaiKhoan {
protected:
    string SoTaiKhoan, TenKhachHang, MaPIN, Hang;
    double SoDu; bool DaKhoa;
public:
    TaiKhoan(string stk, string ten, double du, string pin = "1234", bool khoa = false, string hang = "Thành viên")
        : SoTaiKhoan(stk), TenKhachHang(ten), SoDu(du), MaPIN(pin), DaKhoa(khoa), Hang(hang) {}
    virtual ~TaiKhoan() {}
    virtual void NapTien(double amt) { if (amt > 0) SoDu += amt; }
    virtual bool RutTien(double amt) = 0;
    virtual double TinhLai() = 0;
    string GetSoTaiKhoan() const { return SoTaiKhoan; }
    string GetTenKhachHang() const { return TenKhachHang; }
    string GetMaPIN() const { return MaPIN; }
    bool IsLocked() const { return DaKhoa; }
    void SetLocked(bool s) { DaKhoa = s; }
    double GetSoDu() const { return SoDu; }
    void SetTenKhachHang(string t) { TenKhachHang = t; }
    void SetMaPIN(string p) { MaPIN = p; }
    string GetHang() const { return Hang; }
    void SetHang(string h) { Hang = h; }
};

class TaiKhoanThanhToan : public TaiKhoan {
public:
    TaiKhoanThanhToan(string a, string b, double c, string p="1234", bool k=false, string h="Thành viên") : TaiKhoan(a,b,c,p,k,h) {}
    bool RutTien(double s) override { if(!DaKhoa && s>0 && SoDu-s>=50000){ SoDu-=s; return true;} return false; }
    double TinhLai() override { return SoDu * 0.001; }
};

class TaiKhoanTietKiem : public TaiKhoan {
    double LaiSuat; int KyHan;
public:
    TaiKhoanTietKiem(string a, string b, double c, double ls, int kh, string p="1234", bool k=false, string h="Thành viên") : TaiKhoan(a,b,c,p,k,h), LaiSuat(ls), KyHan(kh) {}
    bool RutTien(double s) override { if(!DaKhoa && s==SoDu){ SoDu=0; return true;} return false; }
    double TinhLai() override { return SoDu * LaiSuat; }
    double GetLaiSuat() const { return LaiSuat; }
    int GetKyHan() const { return KyHan; }
};

class TaiKhoanTinDung : public TaiKhoan {
    double HanMuc;
public:
    TaiKhoanTinDung(string a, string b, double c, double hm, string p="1234", bool k=false, string h="Thành viên") : TaiKhoan(a,b,c,p,k,h), HanMuc(hm) {}
    bool RutTien(double s) override { if(!DaKhoa && s>0 && SoDu+s<=HanMuc){ SoDu+=s; return true;} return false; }
    double TinhLai() override { return SoDu > 0 ? SoDu * 0.2 : 0; }
    double GetHanMuc() const { return HanMuc; }
};

// ============================================================
// BUSINESS
// ============================================================
class NganHang {
    vector<shared_ptr<TaiKhoan>> dsTK;
    map<string, vector<GiaoDich>> dsLS;
    vector<YeuCau> dsYeuCau;
    vector<KhoanVay> dsVay;
    string thoiGianTinhLai = "Chưa có dữ liệu";
public:
    shared_ptr<TaiKhoan> Tim(string stk) {
        for(auto& tk : dsTK) if(tk->GetSoTaiKhoan()==stk) return tk;
        return nullptr;
    }
    void GhiLog(string stk, string loai, double tien, string nd) { dsLS[stk].emplace_back(loai, tien, nd); }
    
    void SetThoiGianTinhLai(string t) { thoiGianTinhLai = t; Save(); }
    string GetThoiGianTinhLai() { return thoiGianTinhLai; }

    void KiemTraTuDongNangHang() {
        for(auto& tk : dsTK) {
            double value = 0;
            if(auto* td = dynamic_cast<TaiKhoanTinDung*>(tk.get())) value = td->GetHanMuc();
            else if(auto* tt = dynamic_cast<TaiKhoanThanhToan*>(tk.get())) value = tt->GetSoDu();
            
            if(value >= 1000000000.0) { if(tk->GetHang() != "Private") tk->SetHang("Private"); }
            else if(value >= 500000000.0) { if(tk->GetHang() != "Signature") tk->SetHang("Signature"); }
        }
    }

    void Load() {
        dsTK.clear(); dsLS.clear(); dsYeuCau.clear(); dsVay.clear();
        ifstream f("data/accounts.csv"); 
        if(f) {
            string l;
            while(getline(f, l)) {
                if(l.empty()) continue;
                stringstream ss(l); string type, stk, ten, pin, du_s, khoa_s, extra1, extra2, hang;
                getline(ss,type,';'); getline(ss,stk,';'); getline(ss,ten,';'); getline(ss,pin,';'); getline(ss,du_s,';'); getline(ss,khoa_s,';');
                try {
                    double du = stod(du_s); bool khoa = (khoa_s=="1");
                    if(type=="ThanhToan") { getline(ss, hang, ';'); if(hang.empty()) hang="Thành viên"; dsTK.push_back(make_shared<TaiKhoanThanhToan>(stk, ten, du, pin, khoa, hang)); }
                    else if(type=="TietKiem") {
                        getline(ss,extra1,';'); getline(ss,extra2,';'); getline(ss, hang, ';'); if(hang.empty()) hang="Thành viên";
                        dsTK.push_back(make_shared<TaiKhoanTietKiem>(stk, ten, du, stod(extra1), stoi(extra2), pin, khoa, hang));
                    } else if(type=="TinDung") {
                        getline(ss,extra1,';'); getline(ss, hang, ';'); if(hang.empty()) hang="Thành viên";
                        dsTK.push_back(make_shared<TaiKhoanTinDung>(stk, ten, du, stod(extra1), pin, khoa, hang));
                    }
                } catch(...) {}
            }
            f.close();
        }
        ifstream f2("data/history.csv");
        if(f2) {
            string l;
            while(getline(f2, l)) {
                if(l.empty()) continue;
                stringstream ss(l); string stk, loai, tien, nd, t;
                getline(ss,stk,';'); getline(ss,loai,';'); getline(ss,tien,';'); getline(ss,nd,';'); getline(ss,t,';');
                try { dsLS[stk].emplace_back(t, loai, stod(tien), nd); } catch(...) {}
            }
            f2.close();
        }
        ifstream f3("data/requests.csv");
        if(f3) {
            string l;
            while(getline(f3, l)) {
                if(l.empty()) continue;
                stringstream ss(l); string type, stk, ten, val, hang, pin, tg;
                getline(ss,type,';'); getline(ss,stk,';'); getline(ss,ten,';'); getline(ss,val,';'); getline(ss,hang,';'); getline(ss,pin,';'); getline(ss,tg,';');
                dsYeuCau.emplace_back(type, stk, ten, val, hang, pin, tg);
            }
            f3.close();
        }
        ifstream f4("data/loans.csv");
        if(f4) {
            string l;
            while(getline(f4, l)) {
                if(l.empty()) continue;
                stringstream ss(l); string stk, amt, han;
                getline(ss,stk,';'); getline(ss,amt,';'); getline(ss,han,';');
                try { dsVay.emplace_back(stk, stod(amt), stoll(han)); } catch(...) {}
            }
            f4.close();
        }
        ifstream fs("data/system.csv");
        if(fs) { getline(fs, thoiGianTinhLai); fs.close(); }
        
        KiemTraTuDongNangHang();
    }

    void Save() {
#ifdef _WIN32
        _mkdir("data");
#else
        mkdir("data", 0777);
#endif
        KiemTraTuDongNangHang();
        ofstream f("data/accounts.csv", ios::trunc);
        for(auto& tk : dsTK) {
            string type = "ThanhToan", extra = "";
            if(auto* s = dynamic_cast<TaiKhoanTietKiem*>(tk.get())) { type="TietKiem"; extra=";"+to_string(s->GetLaiSuat())+";"+to_string(s->GetKyHan()); }
            else if(auto* d = dynamic_cast<TaiKhoanTinDung*>(tk.get())) { type="TinDung"; extra=";"+to_string((long long)d->GetHanMuc()); }
            f << type << ";" << tk->GetSoTaiKhoan() << ";" << tk->GetTenKhachHang() << ";" << tk->GetMaPIN() << ";" << (long long)tk->GetSoDu() << ";" << (tk->IsLocked()?"1":"0") << extra << ";" << tk->GetHang() << "\n";
        }
        f.close();
        ofstream f2("data/history.csv", ios::trunc);
        for(auto& it : dsLS) for(auto& g : it.second) f2 << it.first << ";" << g.loai << ";" << (long long)g.soTien << ";" << g.noiDung << ";" << g.thoiGian << "\n";
        f2.close();
        ofstream f3("data/requests.csv", ios::trunc);
        for(auto& r : dsYeuCau) f3 << r.type << ";" << r.stk << ";" << r.ten << ";" << r.val << ";" << r.hang << ";" << r.pin << ";" << r.thoiGian << "\n";
        f3.close();
        ofstream f4("data/loans.csv", ios::trunc);
        for(auto& v : dsVay) f4 << v.stk << ";" << (long long)v.soTien << ";" << v.hanTra << "\n";
        f4.close();
        ofstream fs("data/system.csv", ios::trunc); fs << thoiGianTinhLai; fs.close();
    }

    void Add(TaiKhoan* tk) { dsTK.push_back(shared_ptr<TaiKhoan>(tk)); GhiLog(tk->GetSoTaiKhoan(), "KHOI_TAO", tk->GetSoDu(), "Mo tai khoan"); Save(); }
    void AddYeuCau(YeuCau y) { dsYeuCau.push_back(y); Save(); }
    void XoaYeuCau(string stk, string type) {
        dsYeuCau.erase(remove_if(dsYeuCau.begin(), dsYeuCau.end(), [&](const YeuCau& r){ return r.stk == stk && r.type == type; }), dsYeuCau.end());
        Save();
    }
    void AddVay(string stk, double amt, int phut) {
        time_t now = time(0);
        dsVay.emplace_back(stk, amt, (long long)now + phut * 60);
        Save();
    }
    const vector<shared_ptr<TaiKhoan>>& GetDS() { return dsTK; }
    const map<string, vector<GiaoDich>>& GetLS() { return dsLS; }
    const vector<YeuCau>& GetYeuCau() { return dsYeuCau; }
    const vector<KhoanVay>& GetDSVay() { return dsVay; }
};

// ============================================================
// MAIN
// ============================================================
int main() {
#ifdef _WIN32
    SetConsoleOutputCP(65001); SetConsoleCP(65001);
#endif
    NganHang nh; nh.Load();
    httplib::Server svr;

    auto url_decode = [](string str) {
        string res;
        for (size_t i = 0; i < str.length(); i++) {
            if (str[i] == '%' && i + 2 < str.length()) {
                int ii; sscanf(str.substr(i + 1, 2).c_str(), "%x", &ii);
                res += static_cast<char>(ii); i += 2;
            } else if (str[i] == '+') res += ' ';
            else res += str[i];
        }
        return res;
    };

    auto param = [&](const httplib::Request& req, string key) { return url_decode(req.get_param_value(key)); };
    auto handle = [&](string path, httplib::Server::Handler h) { svr.Get(path, h); svr.Post(path, h); };

    svr.Get("/", [](const httplib::Request&, httplib::Response& res) {
        ifstream f("index.html", ios::binary);
        if(!f) { res.set_content("index.html not found", "text/html"); return; }
        res.set_content(string((istreambuf_iterator<char>(f)), istreambuf_iterator<char>()), "text/html");
    });

    handle("/api/login", [&](const httplib::Request& req, httplib::Response& res) {
        string u = param(req, "user"), p = param(req, "pin");
        if(u=="admin" && p=="123") { res.set_content("{\"status\":\"success\",\"role\":\"admin\"}", "application/json"); return; }
        auto tk = nh.Tim(u);
        if(tk && tk->GetMaPIN()==p) {
            if(tk->IsLocked()) res.set_content("{\"status\":\"error\",\"msg\":\"Tài khoản bị khóa\"}", "application/json");
            else res.set_content("{\"status\":\"success\",\"role\":\"user\",\"stk\":\""+u+"\",\"name\":\""+tk->GetTenKhachHang()+"\"}", "application/json");
        } else res.set_content("{\"status\":\"error\",\"msg\":\"Sai thông tin\"}", "application/json");
    });

    handle("/api/user/info", [&](const httplib::Request& req, httplib::Response& res) {
        nh.KiemTraTuDongNangHang();
        auto tk = nh.Tim(param(req, "stk"));
        if(!tk) { res.set_content("{\"error\":\"404\"}", "application/json"); return; }
        string type = "Thanh Toán"; double hm = 0;
        if(dynamic_cast<TaiKhoanTietKiem*>(tk.get())) type="Tiết Kiệm";
        else if(auto* d = dynamic_cast<TaiKhoanTinDung*>(tk.get())) { type="Tín Dụng"; hm=d->GetHanMuc(); }
        res.set_content("{\"balance\":"+to_string((long long)tk->GetSoDu())+",\"name\":\""+tk->GetTenKhachHang()+"\",\"type\":\""+type+"\",\"interest\":"+to_string((long long)tk->TinhLai())+",\"locked\":"+(tk->IsLocked()?"true":"false")+",\"hang\":\""+tk->GetHang()+"\",\"hanMuc\":"+to_string((long long)hm)+"}", "application/json");
    });

    handle("/api/user/history", [&](const httplib::Request& req, httplib::Response& res) {
        string stk = param(req, "stk"); string j = "["; bool f = true;
        if(nh.GetLS().count(stk)) {
            auto v = nh.GetLS().at(stk);
            for(int i=v.size()-1; i>=0 && i>=(int)v.size()-20; --i) {
                if(!f) j+=","; f=false;
                j += "{\"time\":\""+v[i].thoiGian+"\",\"type\":\""+v[i].loai+"\",\"amount\":"+to_string((long long)v[i].soTien)+",\"note\":\""+v[i].noiDung+"\"}";
            }
        }
        j+="]"; res.set_content(j, "application/json");
    });

    handle("/api/user/transact", [&](const httplib::Request& req, httplib::Response& res) {
        string stk=param(req,"stk"), type=param(req,"type"), pin=param(req,"pin"), dest=param(req,"dest");
        double amt = 0; try{ amt = stod(param(req,"amount")); }catch(...){}
        auto tk = nh.Tim(stk);
        try {
            if(!tk) throw runtime_error("Không tìm thấy TK");
            if(tk->IsLocked()) throw runtime_error("Tài khoản đang bị khóa");
            if(type=="nap") tk->NapTien(amt);
            else if(type=="rut") { if(tk->GetMaPIN()!=pin) throw runtime_error("Sai PIN"); if(!tk->RutTien(amt)) throw runtime_error("Số dư không đủ"); }
            else if(type=="chuyen") {
                auto dt = nh.Tim(dest); if(!dt) throw runtime_error("Không tìm thấy TK nhận");
                if(dt->IsLocked()) throw runtime_error("Tài khoản nhận đang bị khóa");
                if(tk->GetMaPIN()!=pin) throw runtime_error("Sai PIN");
                if(tk->RutTien(amt)) dt->NapTien(amt); else throw runtime_error("Số dư không đủ");
                nh.GhiLog(dest, "NHAN_TIEN", amt, "Nhận từ "+stk);
            }
            nh.GhiLog(stk, type=="nap"?"NAP":type=="rut"?"RUT":"CHUYEN", type=="nap"?amt:-amt, "Giao dịch thành công");
            nh.Save(); res.set_content("{\"status\":\"success\"}", "application/json");
        } catch(exception& e) { res.set_content("{\"status\":\"error\",\"msg\":\""+string(e.what())+"\"}", "application/json"); }
    });

    handle("/api/user/changepin", [&](const httplib::Request& req, httplib::Response& res) {
        auto tk = nh.Tim(param(req, "stk"));
        if(tk && tk->GetMaPIN()==param(req,"oldpin")) { tk->SetMaPIN(param(req,"newpin")); nh.Save(); res.set_content("{\"status\":\"success\"}", "application/json"); }
        else res.set_content("{\"status\":\"error\",\"msg\":\"Sai PIN cũ\"}", "application/json");
    });

    handle("/api/user/request_upgrade", [&](const httplib::Request& req, httplib::Response& res) {
        string stk = param(req, "stk"), hang = param(req, "hang");
        auto tk = nh.Tim(stk);
        if(tk) { nh.AddYeuCau(YeuCau("UPGRADE", stk, tk->GetTenKhachHang(), hang, tk->GetHang())); res.set_content("{\"status\":\"success\"}", "application/json"); }
        else res.set_content("{\"status\":\"error\"}", "application/json");
    });

    handle("/api/user/request_loan", [&](const httplib::Request& req, httplib::Response& res) {
        string stk = param(req, "stk"), amt = param(req, "amount"), phut = param(req, "phut");
        auto tk = nh.Tim(stk);
        if(tk) {
            nh.AddYeuCau(YeuCau("LOAN", stk, tk->GetTenKhachHang(), amt, phut));
            res.set_content("{\"status\":\"success\"}", "application/json");
        } else res.set_content("{\"status\":\"error\"}", "application/json");
    });

    handle("/api/user/loan_info", [&](const httplib::Request& req, httplib::Response& res) {
        string stk = param(req, "stk");
        for(auto& v : nh.GetDSVay()) {
            if(v.stk == stk) {
                long long remaining = v.hanTra - (long long)time(0);
                res.set_content("{\"status\":\"active\",\"amount\":"+to_string((long long)v.soTien)+",\"remaining\":"+to_string(remaining)+"}", "application/json");
                return;
            }
        }
        res.set_content("{\"status\":\"none\"}", "application/json");
    });

    handle("/api/user/request_reset_pin", [&](const httplib::Request& req, httplib::Response& res) {
        string stk = param(req, "stk");
        auto tk = nh.Tim(stk);
        if(tk) {
            nh.AddYeuCau(YeuCau("RESET_PIN", stk, tk->GetTenKhachHang(), "Yêu cầu cấp lại mã PIN", tk->GetHang()));
            res.set_content("{\"status\":\"success\"}", "application/json");
        } else res.set_content("{\"status\":\"error\",\"msg\":\"Không tìm thấy số tài khoản\"}", "application/json");
    });

    handle("/api/admin/all", [&](const httplib::Request&, httplib::Response& res) {
        nh.KiemTraTuDongNangHang();
        string j = "["; bool f = true;
        for(auto& tk : nh.GetDS()) {
            if(!f) j+=","; f=false;
            string t = "ThanhToan"; if(dynamic_cast<TaiKhoanTietKiem*>(tk.get())) t="TietKiem"; else if(dynamic_cast<TaiKhoanTinDung*>(tk.get())) t="TinDung";
            j += "{\"stk\":\""+tk->GetSoTaiKhoan()+"\",\"name\":\""+tk->GetTenKhachHang()+"\",\"balance\":"+to_string((long long)tk->GetSoDu())+",\"type\":\""+t+"\",\"locked\":"+(tk->IsLocked()?"true":"false")+",\"hang\":\""+tk->GetHang()+"\"}";
        }
        j+="]"; res.set_content(j, "application/json");
    });

    handle("/api/admin/system_info", [&](const httplib::Request&, httplib::Response& res) {
        res.set_content("{\"lastInterest\":\"" + nh.GetThoiGianTinhLai() + "\"}", "application/json");
    });

    handle("/api/admin/loans", [&](const httplib::Request&, httplib::Response& res) {
        string j = "["; bool f = true;
        long long now = (long long)time(0);
        for(auto& v : nh.GetDSVay()) {
            if(!f) j+=","; f=false;
            auto tk = nh.Tim(v.stk);
            string ten = tk ? tk->GetTenKhachHang() : "N/A";
            j += "{\"stk\":\""+v.stk+"\",\"name\":\""+ten+"\",\"amount\":"+to_string((long long)v.soTien)+",\"remaining\":"+to_string(v.hanTra - now)+"}";
        }
        j+="]"; res.set_content(j, "application/json");
    });

    handle("/api/admin/upgrade_requests", [&](const httplib::Request&, httplib::Response& res) {
        string j = "["; bool f = true;
        for(auto& r : nh.GetYeuCau()) {
            if(!f) j+=","; f=false;
            j += "{\"type\":\""+r.type+"\",\"stk\":\""+r.stk+"\",\"name\":\""+r.ten+"\",\"val\":\""+r.val+"\",\"hang\":\""+r.hang+"\",\"time\":\""+r.thoiGian+"\"}";
        }
        j+="]"; res.set_content(j, "application/json");
    });

    handle("/api/admin/approve_request", [&](const httplib::Request& req, httplib::Response& res) {
        string stk = param(req, "stk"), type = param(req, "type"), val = param(req, "val"), hang = param(req, "hang");
        auto tk = nh.Tim(stk);
        if(tk) {
            if(type=="UPGRADE") tk->SetHang(val);
            else if(type=="LOAN") {
                double amt = 0; int phut = 0;
                try { amt = stod(val); phut = stoi(hang); } catch(...) {}
                tk->NapTien(amt);
                nh.AddVay(stk, amt, phut);
                nh.GhiLog(stk, "VAY_TIEN", amt, "Duyệt khoản vay (" + to_string(phut) + " phút)");
            }
            else if(type=="RESET_PIN") {
                tk->SetMaPIN("1234");
                nh.GhiLog(stk, "RESET_PIN", 0, "Mã PIN đã được đặt lại về 1234");
            }
            else if(type=="CARD") {
                if(val=="TinDung") {
                    if(tk->GetSoDu() < 100000000.0) {
                        res.set_content("{\"status\":\"error\",\"msg\":\"Số dư phải trên 100 triệu để mở thẻ tín dụng\"}", "application/json");
                        return;
                    }
                    double du = tk->GetSoDu(); string ten = tk->GetTenKhachHang(), pin = tk->GetMaPIN(), hang_tk = tk->GetHang();
                    auto n = new TaiKhoanTinDung(stk, ten, du, 5000000, pin, false, hang_tk);
                    auto& ds = const_cast<vector<shared_ptr<TaiKhoan>>&>(nh.GetDS());
                    for(auto& ptr : ds) if(ptr->GetSoTaiKhoan()==stk) { ptr.reset(n); break; }
                }
            }
            nh.XoaYeuCau(stk, type); nh.Save(); res.set_content("{\"status\":\"success\"}", "application/json");
        } else res.set_content("{\"status\":\"error\"}", "application/json");
    });

    handle("/api/admin/interest", [&](const httplib::Request&, httplib::Response& res) {
        for(auto& tk : nh.GetDS()) { double lai = tk->TinhLai(); if(lai>0){ tk->NapTien(lai); nh.GhiLog(tk->GetSoTaiKhoan(),"LAI",lai,"Lãi định kỳ"); } }
        time_t t = time(nullptr); tm* now = localtime(&t); char buf[80];
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", now);
        nh.SetThoiGianTinhLai(string(buf));
        nh.Save(); res.set_content("{\"status\":\"success\"}", "application/json");
    });

    handle("/api/admin/upgrade", [&](const httplib::Request& req, httplib::Response& res) {
        auto tk = nh.Tim(param(req, "stk")); string hang = param(req, "hang");
        if(tk) { tk->SetHang(hang); nh.Save(); res.set_content("{\"status\":\"success\"}", "application/json"); }
        else res.set_content("{\"status\":\"error\"}", "application/json");
    });

    handle("/api/admin/lock", [&](const httplib::Request& req, httplib::Response& res) {
        auto tk = nh.Tim(param(req,"stk")); if(tk) tk->SetLocked(param(req,"lock")=="1");
        nh.Save(); res.set_content("{\"status\":\"success\"}", "application/json");
    });

    handle("/api/admin/add", [&](const httplib::Request& req, httplib::Response& res) {
        string stk=param(req,"stk"), ten=param(req,"ten"), pin=param(req,"pin"), type=param(req,"type");
        if(nh.Tim(stk)) { res.set_content("{\"status\":\"error\",\"msg\":\"STK tồn tại\"}", "application/json"); return; }
        double du = 0; try{ du = stod(param(req,"sodu")); }catch(...){}
        if(type=="TinDung" && du < 100000000.0) {
            res.set_content("{\"status\":\"error\",\"msg\":\"Hạn mức tín dụng phải từ 100 triệu VNĐ\"}", "application/json");
            return;
        }
        if(type=="TietKiem") nh.Add(new TaiKhoanTietKiem(stk, ten, du, 0.05, 12, pin));
        else if(type=="TinDung") nh.Add(new TaiKhoanTinDung(stk, ten, 0, du, pin));
        else nh.Add(new TaiKhoanThanhToan(stk, ten, du, pin));
        res.set_content("{\"status\":\"success\"}", "application/json");
    });

    handle("/api/admin/deposit", [&](const httplib::Request& req, httplib::Response& res) {
        auto tk = nh.Tim(param(req,"stk"));
        double amt = 0; try { amt = stod(param(req,"amount")); } catch(...) {}
        if(tk) { tk->NapTien(amt); nh.GhiLog(tk->GetSoTaiKhoan(), "NAP", amt, "Nạp tại quầy"); nh.Save(); res.set_content("{\"status\":\"success\"}", "application/json"); }
        else res.set_content("{\"status\":\"error\",\"msg\":\"Không tìm thấy TK\"}", "application/json");
    });

    handle("/api/admin/delete", [&](const httplib::Request& req, httplib::Response& res) {
        string stk = param(req,"stk");
        auto& d = const_cast<vector<shared_ptr<TaiKhoan>>&>(nh.GetDS());
        d.erase(remove_if(d.begin(), d.end(), [&](shared_ptr<TaiKhoan> t){ return t->GetSoTaiKhoan()==stk; }), d.end());
        nh.Save(); res.set_content("{\"status\":\"success\"}", "application/json");
    });

    handle("/api/register", [&](const httplib::Request& req, httplib::Response& res) {
        string stk=param(req,"stk"), ten=param(req,"ten"), pin=param(req,"pin"), type=param(req,"type");
        if(nh.Tim(stk)) { res.set_content("{\"status\":\"error\",\"msg\":\"STK tồn tại\"}", "application/json"); return; }
        if(type=="TietKiem") nh.Add(new TaiKhoanTietKiem(stk, ten, 0, 0.05, 12, pin));
        else nh.Add(new TaiKhoanThanhToan(stk, ten, 50000, pin));
        res.set_content("{\"status\":\"success\"}", "application/json");
    });

    cout << "OOP BANK ONLINE: http://localhost:8080" << endl;
    svr.listen("0.0.0.0", 8080);
    return 0;
}
