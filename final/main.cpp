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
#include <cstdio>

#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#include <process.h>
#else
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>
#endif

#include "httplib.h"

using namespace std;

const string GS_URL = "https://script.google.com/macros/s/AKfycbygfKZMkf1MAztbT2vYArSq3yLeo74qZVpnFOUIWNttjxlkuUgAEm84DOmu0RmYCfjq/exec";

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
    string stk; double soTien; long long hanTra; double laiSuat;
    KhoanVay(string s, double st, long long ht, double ls = 0.05) : stk(s), soTien(st), hanTra(ht), laiSuat(ls) {}
};

struct LichSuVay {
    string stk, ten, thoiGian, hanVay, trangThai;
    double soTien, laiSuat;
    LichSuVay(string s, string t, string tg, string hv, double st, double ls, string tt)
        : stk(s), ten(t), thoiGian(tg), hanVay(hv), soTien(st), laiSuat(ls), trangThai(tt) {}
};

class TaiKhoan {
protected:
    string SoTaiKhoan, TenKhachHang, MaPIN, Hang;
    double SoDu, SoDuVay; bool DaKhoa;
    double HanMucTinDung, NoTinDung; // Thuộc tính mới cho tín dụng liên kết
public:
    TaiKhoan(string stk, string ten, double du, string pin = "1234", bool khoa = false, string hang = "Thành viên", double duVay = 0, double hmTD = 0, double noTD = 0)
        : SoTaiKhoan(stk), TenKhachHang(ten), SoDu(du), SoDuVay(duVay), MaPIN(pin), DaKhoa(khoa), Hang(hang), HanMucTinDung(hmTD), NoTinDung(noTD) {}
    virtual ~TaiKhoan() {}
    virtual void NapTien(double amt) { if (amt > 0) SoDu += amt; }
    void NapTienVay(double amt) { if (amt > 0) SoDuVay += amt; }
    virtual bool RutTien(double amt) = 0;
    bool ThucHienRut(double amt, double minBalance = 0) {
        if(DaKhoa || amt <= 0) return false;
        if(SoDu + SoDuVay - amt < minBalance) return false;
        if(SoDu >= amt) { SoDu -= amt; } else { double rem = amt - SoDu; SoDu = 0; SoDuVay -= rem; }
        return true;
    }
    // Giao dịch tín dụng liên kết
    virtual bool RutTienTinDung(double amt) {
        if(DaKhoa || HanMucTinDung <= 0 || amt <= 0) return false;
        if(NoTinDung + amt > HanMucTinDung) return false;
        NoTinDung += amt; return true;
    }
    virtual void NapTienTinDung(double amt) {
        if(amt > 0) { if(NoTinDung >= amt) NoTinDung -= amt; else NoTinDung = 0; }
    }

    virtual double TinhLai() = 0;
    string GetSoTaiKhoan() const { return SoTaiKhoan; }
    string GetTenKhachHang() const { return TenKhachHang; }
    string GetMaPIN() const { return MaPIN; }
    bool IsLocked() const { return DaKhoa; }
    void SetLocked(bool s) { DaKhoa = s; }
    
    double GetSoDu() const { return SoDu; }
    virtual double GetBalance() const { return SoDu; }
    
    double GetSoDuVay() const { return SoDuVay; }
    void SetSoDu(double d) { SoDu = d; }
    void SetSoDuVay(double d) { SoDuVay = d; }
    void SetTenKhachHang(string t) { TenKhachHang = t; }
    void SetMaPIN(string p) { MaPIN = p; }
    string GetHang() const { return Hang; }
    void SetHang(string h) { Hang = h; }

    double GetHanMucTinDung() const { return HanMucTinDung; }
    void SetHanMucTinDung(double h) { HanMucTinDung = h; }
    double GetNoTinDung() const { return NoTinDung; }
    void SetNoTinDung(double n) { NoTinDung = n; }
};

class TaiKhoanThanhToan : public TaiKhoan {
public:
    TaiKhoanThanhToan(string a, string b, double c, string p="1234", bool k=false, string h="Thành viên", double dv=0, double hmTD=0, double noTD=0) 
        : TaiKhoan(a,b,c,p,k,h,dv,hmTD,noTD) {}
    bool RutTien(double s) override { return ThucHienRut(s, 50000); }
    double TinhLai() override { return SoDu * 0.001; }
};

class TaiKhoanTietKiem : public TaiKhoan {
    double LaiSuat; int KyHan;
public:
    TaiKhoanTietKiem(string a, string b, double c, double ls, int kh, string p="1234", bool k=false, string h="Thành viên", double dv=0, double hmTD=0, double noTD=0) 
        : TaiKhoan(a,b,c,p,k,h,dv,hmTD,noTD), LaiSuat(ls), KyHan(kh) {}
    bool RutTien(double s) override { return ThucHienRut(s, 0); }
    double TinhLai() override { return SoDu * LaiSuat; }
    double GetLaiSuat() const { return LaiSuat; }
    int GetKyHan() const { return KyHan; }
};

class TaiKhoanTinDung : public TaiKhoan {
    double HanMuc;
public:
    TaiKhoanTinDung(string a, string b, double c, double hm, string p="1234", bool k=false, string h="Thành viên", double dv=0) 
        : TaiKhoan(a,b,c,p,k,h,dv), HanMuc(hm) {}
    void NapTien(double amt) override { if (amt > 0) { if (SoDu >= amt) SoDu -= amt; else SoDu = 0; } }
    bool RutTien(double s) override { if(!DaKhoa && s > 0 && SoDu + s <= HanMuc) { SoDu += s; return true; } return false; }
    
    double GetBalance() const override { return HanMuc - SoDu; }
    
    double TinhLai() override { return SoDu > 0 ? SoDu * 0.2 : 0; }
    double GetHanMuc() const { return HanMuc; }
};

string GetJSONValue(const string& json, string key) {
    string search = "\"" + key + "\":\"";
    size_t start = json.find(search);
    if (start == string::npos) return "";
    start += search.length();
    size_t end = start;
    while (end < json.length()) {
        if (json[end] == '\"' && (end == 0 || json[end-1] != '\\')) break;
        end++;
    }
    string val = json.substr(start, end - start);
    string clean = ""; clean.reserve(val.length());
    for(size_t i=0; i<val.length(); i++) {
        if (val[i] == '\\' && i+1 < val.length()) {
            if (val[i+1] == 'n') clean += '\n';
            else if (val[i+1] == '\"') clean += '\"';
            else if (val[i+1] == '\\') clean += '\\';
            else clean += val[i+1];
            i++;
        } else clean += val[i];
    }
    return clean;
}

string Escape(string data) {
    string res = "";
    for (unsigned char c : data) {
        if (c == '\"') res += "\\\"";
        else if (c == '\\') res += "\\\\";
        else if (c == '\n') res += "\\n";
        else if (c == '\r') continue;
        else if (c < 32) continue;
        else res += c;
    }
    return res;
}

class NganHang {
    vector<shared_ptr<TaiKhoan>> dsTK;
    map<string, vector<GiaoDich>> dsLS;
    vector<YeuCau> dsYeuCau;
    vector<KhoanVay> dsVay;
    vector<LichSuVay> dsLichSuVay;
    string thoiGianTinhLai = "Chưa có dữ liệu";
    string dsCustomers = "";
    time_t lastSave = 0;
    bool isSaving = false; 
#ifdef _WIN32
    CRITICAL_SECTION cs;
#else
    pthread_mutex_t mtx;
#endif

    void KiemTraTuDongNangHang() {
        for(auto& tk : dsTK) {
            double val = 0;
            if(auto* td = dynamic_cast<TaiKhoanTinDung*>(tk.get())) val = td->GetHanMuc();
            else val = tk->GetSoDu();
            if(val >= 1000000000.0) tk->SetHang("Private");
            else if(val >= 500000000.0) tk->SetHang("Signature");
        }
    }

    void Lock() {
#ifdef _WIN32
        EnterCriticalSection(&cs);
#else
        pthread_mutex_lock(&mtx);
#endif
    }
    void Unlock() {
#ifdef _WIN32
        LeaveCriticalSection(&cs);
#else
        pthread_mutex_unlock(&mtx);
#endif
    }

public:
    NganHang() {
#ifdef _WIN32
        InitializeCriticalSection(&cs);
#else
        pthread_mutex_init(&mtx, NULL);
#endif
    }
    ~NganHang() {
#ifdef _WIN32
        DeleteCriticalSection(&cs);
#else
        pthread_mutex_destroy(&mtx);
#endif
    }

    shared_ptr<TaiKhoan> Tim(string stk) {
        Lock();
        shared_ptr<TaiKhoan> res = nullptr;
        for(auto& tk : dsTK) if(tk->GetSoTaiKhoan()==stk) { res = tk; break; }
        Unlock();
        return res;
    }

    void GhiLog(string stk, string loai, double tan_suat, string nd) { 
        Lock();
        for(char &c : nd) if(c == ';' || c == '\n' || c == '\r') c = ' ';
        dsLS[stk].emplace_back(loai, tan_suat, nd); 
        Unlock();
    }

    void SetThoiGianTinhLai(string t) { 
        Lock(); thoiGianTinhLai = t; Unlock();
        Save(); 
    }
    string GetThoiGianTinhLai() { Lock(); string res = thoiGianTinhLai; Unlock(); return res; }

    void Load() {
        if (isSaving || time(0) - lastSave < 8) return; 
        
        string tmp = "res_load.json";
        system(("curl.exe -k -L -s --connect-timeout 3 \"" + GS_URL + "\" > " + tmp).c_str());
        
        ifstream f(tmp);
        if(!f) return;
        string all((istreambuf_iterator<char>(f)), istreambuf_iterator<char>());
        f.close(); ::remove(tmp.c_str());
        if(all.find("accounts") == string::npos) return;

        vector<shared_ptr<TaiKhoan>> nextTK;
        map<string, vector<GiaoDich>> nextLS;
        vector<YeuCau> nextYeuCau;
        vector<KhoanVay> nextVay;
        vector<LichSuVay> nextLichSuVay;

        auto Clean = [](string& s) { 
            if(!s.empty() && s[0]=='\'') s = s.substr(1); 
            size_t first = s.find_first_not_of(" \t\r\n");
            if (first == string::npos) { s = ""; return; }
            size_t last = s.find_last_not_of(" \t\r\n");
            s = s.substr(first, (last - first + 1));
        };

        auto ParseTab = [&](string data, string mode) {
            stringstream ss(data); string l;
            while(getline(ss, l)) {
                if(l.empty()) continue;
                stringstream row(l);
                if(mode=="acc") {
                    string tp, stk, ten, pin, du_s, khoa_s, ls_s, kh_s, hm_s, hang, dv_s, hmTD_s, noTD_s;
                    getline(row, tp, ';'); getline(row, stk, ';'); getline(row, ten, ';'); getline(row, pin, ';'); 
                    getline(row, du_s, ';'); getline(row, khoa_s, ';'); getline(row, ls_s, ';'); getline(row, kh_s, ';'); 
                    getline(row, hm_s, ';'); getline(row, hang, ';'); getline(row, dv_s, ';');
                    getline(row, hmTD_s, ';'); getline(row, noTD_s, ';');
                    Clean(stk); Clean(pin);
                    if(stk.empty()) continue;
                    try {
                        double du = stod(du_s); bool k = (khoa_s=="1"); double dv = dv_s.empty()?0:stod(dv_s);
                        double hmTD = hmTD_s.empty()?0:stod(hmTD_s); double noTD = noTD_s.empty()?0:stod(noTD_s);
                        if(tp=="ThanhToan") nextTK.push_back(make_shared<TaiKhoanThanhToan>(stk, ten, du, pin, k, hang, dv, hmTD, noTD));
                        else if(tp=="TietKiem") nextTK.push_back(make_shared<TaiKhoanTietKiem>(stk, ten, du, stod(ls_s), stoi(kh_s), pin, k, hang, dv, hmTD, noTD));
                        else if(tp=="TinDung") nextTK.push_back(make_shared<TaiKhoanTinDung>(stk, ten, du, stod(hm_s), pin, k, hang, dv));
                    } catch(...) {}
                } else if(mode=="req") {
                    string t, s, n, v, h, p, tg;
                    getline(row,t,';'); getline(row,s,';'); getline(row,n,';'); getline(row,v,';'); getline(row,h,';'); getline(row,p,';'); getline(row,tg,';');
                    Clean(s); Clean(p);
                    if(!t.empty()) nextYeuCau.emplace_back(t, s, n, v, h, p, tg);
                } else if(mode=="his") {
                    string s, lo, ti, nd, t;
                    if(!getline(row,s,';')) continue; 
                    getline(row,lo,';'); getline(row,ti,';'); getline(row,nd,';'); getline(row,t,';');
                    Clean(s);
                    if(s.empty()) continue;
                    try { nextLS[s].emplace_back(t, lo, stod(ti), nd); } catch(...) {}
                } else if(mode=="loan") {
                    string s, a, h, ls;
                    getline(row,s,';'); getline(row,a,';'); getline(row,h,';'); getline(row,ls,';');
                    Clean(s);
                    if(!s.empty()) try { nextVay.emplace_back(s, stod(a), stoll(h), stod(ls)); } catch(...) {}
                } else if(mode=="l_his") {
                    string s, n, tg, hv, st, ls, tt;
                    getline(row,s,';'); getline(row,n,';'); getline(row,tg,';'); getline(row,hv,';'); getline(row,st,';'); getline(row,ls,';'); getline(row,tt,';');
                    Clean(s);
                    if(!s.empty()) try { nextLichSuVay.emplace_back(s, n, tg, hv, stod(st), stod(ls), tt); } catch(...) {}
                }
            }
        };
        ParseTab(GetJSONValue(all, "accounts"), "acc");
        if (nextTK.empty()) return; 

        ParseTab(GetJSONValue(all, "requests"), "req");
        ParseTab(GetJSONValue(all, "history"), "his");
        ParseTab(GetJSONValue(all, "loans"), "loan");
        ParseTab(GetJSONValue(all, "loan_history"), "l_his");
        
        Lock();
        if (!isSaving) {
            dsTK = nextTK; 
            if(!nextLS.empty()) dsLS = nextLS; 
            dsYeuCau = nextYeuCau; 
            dsVay = nextVay; 
            dsLichSuVay = nextLichSuVay;
            thoiGianTinhLai = GetJSONValue(all, "system");
            dsCustomers = GetJSONValue(all, "customers");
        }
        Unlock();
    }

    void Save() {
        Lock();
        isSaving = true; 
        KiemTraTuDongNangHang();
        
        stringstream accSS, hisSS, reqSS, loanSS, lhisSS;
        for(auto& tk : dsTK) {
            string tp = "ThanhToan", ls = "", kh = "", hm = "";
            if(auto* s = dynamic_cast<TaiKhoanTietKiem*>(tk.get())) { tp="TietKiem"; ls=to_string(s->GetLaiSuat()); kh=to_string(s->GetKyHan()); }
            else if(auto* d = dynamic_cast<TaiKhoanTinDung*>(tk.get())) { tp="TinDung"; hm=to_string((long long)d->GetHanMuc()); }
            accSS << tp << ";'" << tk->GetSoTaiKhoan() << ";" << tk->GetTenKhachHang() << ";'" << tk->GetMaPIN() << ";" << fixed << setprecision(0) << tk->GetSoDu() << ";" << (tk->IsLocked()?"1":"0") << ";" << ls << ";" << kh << ";" << hm << ";" << tk->GetHang() << ";" << (long long)tk->GetSoDuVay() << ";" << (long long)tk->GetHanMucTinDung() << ";" << (long long)tk->GetNoTinDung() << "\n";
        }
        
        for(auto& it : dsLS) {
            for(auto& g : it.second) {
                hisSS << "'" << it.first << ";" << g.loai << ";" << fixed << setprecision(0) << g.soTien << ";" << g.noiDung << ";" << g.thoiGian << "\n";
            }
        }

        for(auto& r : dsYeuCau) reqSS << r.type << ";'" << r.stk << ";" << r.ten << ";" << r.val << ";" << r.hang << ";'" << r.pin << ";" << r.thoiGian << "\n";
        for(auto& v : dsVay) loanSS << "'" << v.stk << ";" << fixed << setprecision(0) << v.soTien << ";" << v.hanTra << ";" << v.laiSuat << "\n";
        for(auto& l : dsLichSuVay) lhisSS << "'" << l.stk << ";" << l.ten << ";" << l.thoiGian << ";" << l.hanVay << ";" << fixed << setprecision(0) << l.soTien << ";" << l.laiSuat << ";" << l.trangThai << "\n";
        
        string accS = accSS.str(), hisS = hisSS.str(), reqS = reqSS.str(), loanS = loanSS.str(), l_hisS = lhisSS.str();
        string sysS = thoiGianTinhLai, custS = dsCustomers;
        Unlock();

        string json = "{\"bulk\":{\"accounts\":\"" + Escape(accS) + "\",\"history\":\"" + Escape(hisS) + "\",\"requests\":\"" + Escape(reqS) + "\",\"loans\":\"" + Escape(loanS) + "\",\"loan_history\":\"" + Escape(l_hisS) + "\",\"system\":\"" + Escape(sysS) + "\",\"customers\":\"" + Escape(custS) + "\"}}";
        
        string tmpFile = "post_" + to_string(time(0)) + "_" + string(1, 'A' + rand()%26) + ".json";
        ofstream f(tmpFile); f << json; f.close();
        
        cout << " [Cloud] Dang day du lieu len Google Sheets (Chay ngam)..." << endl;
        
#ifdef _WIN32
        string cmd = "start /B cmd /c \"curl.exe -k -L -s -X POST -H \"Content-Type: application/json\" -d @" + tmpFile + " \"" + GS_URL + "\" > nul 2>&1 && del " + tmpFile + "\"";
#else
        string cmd = "curl -k -L -s -X POST -H \"Content-Type: application/json\" -d @" + tmpFile + " \"" + GS_URL + "\" > /dev/null 2>&1 && rm " + tmpFile + " &";
#endif
        system(cmd.c_str());
        
        Lock();
        lastSave = time(0); 
        isSaving = false;   
        Unlock();
    }

    void Add(TaiKhoan* tk) { 
        Lock(); dsTK.push_back(shared_ptr<TaiKhoan>(tk)); Unlock();
        GhiLog(tk->GetSoTaiKhoan(), "KHOI_TAO", tk->GetSoDu(), "Mo tai khoan"); 
        Save(); 
    }
    void AddYeuCau(YeuCau y) { Lock(); dsYeuCau.push_back(y); Unlock(); Save(); }
    void XoaYeuCau(string stk, string type) {
        Lock();
        dsYeuCau.erase(remove_if(dsYeuCau.begin(), dsYeuCau.end(), [&](const YeuCau& r){ return r.stk == stk && r.type == type; }), dsYeuCau.end());
        Unlock();
        Save();
    }
    void AddVay(string stk, double amt, int phut) {
        double ls = 0.05; // Mặc định
        if (amt < 1000000) ls = 0.04;
        else if (amt < 10000000) ls = 0.045;
        else if (amt < 100000000) ls = 0.05;
        else ls = 0.06;

        time_t now = time(0);
        Lock();
        dsVay.emplace_back(stk, amt, (long long)now + phut * 60, ls);
        for(auto& tk : dsTK) if(tk->GetSoTaiKhoan()==stk) { 
            tk->NapTienVay(amt); 
            dsLichSuVay.emplace_back(stk, tk->GetTenKhachHang(), "Vừa xong", to_string(phut) + " phút", amt, ls, "Chưa trả"); 
            break; 
        }
        Unlock();
        GhiLog(stk, "VAY_TIEN", amt, "Duyet khoan vay (" + to_string(phut) + " phut)");
        Save();
    }
    bool TatToanVay(string stk) {
        Lock();
        TaiKhoan* tk_ptr = nullptr;
        for(auto& t : dsTK) if(t->GetSoTaiKhoan() == stk) { tk_ptr = t.get(); break; }
        
        if(!tk_ptr) { Unlock(); return false; }
        
        auto it = find_if(dsVay.begin(), dsVay.end(), [&](const KhoanVay& v){ return v.stk == stk; });
        if(it == dsVay.end()) { Unlock(); return false; }
        
        double tongNo = it->soTien * (1 + it->laiSuat);
        if(tk_ptr->GetSoDu() + tk_ptr->GetSoDuVay() < tongNo) { Unlock(); return false; }
        
        if(tk_ptr->GetSoDu() >= tongNo) {
            tk_ptr->SetSoDu(tk_ptr->GetSoDu() - tongNo);
        } else {
            double conLai = tongNo - tk_ptr->GetSoDu();
            tk_ptr->SetSoDu(0);
            tk_ptr->SetSoDuVay(tk_ptr->GetSoDuVay() - conLai);
        }
        
        dsVay.erase(it);
        for(auto& l : dsLichSuVay) if(l.stk == stk && l.trangThai == "Chưa trả") { l.trangThai = "Đã trả"; break; }
        
        Unlock();
        GhiLog(stk, "TRA_NO", -tongNo, "Tat toan khoan vay bao gom lai");
        Save();
        return true;
    }

    void XoaTK(string stk) {
        Lock();
        dsTK.erase(remove_if(dsTK.begin(), dsTK.end(), [&](const shared_ptr<TaiKhoan>& t){ return t->GetSoTaiKhoan() == stk; }), dsTK.end());
        dsLS.erase(stk);
        Unlock();
        Save();
    }

    vector<shared_ptr<TaiKhoan>> GetDS() { Lock(); auto res = dsTK; Unlock(); return res; }
    map<string, vector<GiaoDich>> GetLS() { Lock(); auto res = dsLS; Unlock(); return res; }
    vector<YeuCau> GetYeuCau() { Lock(); auto res = dsYeuCau; Unlock(); return res; }
    vector<KhoanVay> GetDSVay() { Lock(); auto res = dsVay; Unlock(); return res; }
    vector<LichSuVay> GetLichSuVay() { Lock(); auto res = dsLichSuVay; Unlock(); return res; }
};

#ifdef _WIN32
unsigned __stdcall SyncThreadFunc(void* p) {
    NganHang* nh = (NganHang*)p;
    while(true) { Sleep(3000); nh->Load(); }
    return 0;
}
#else
void* SyncThreadFunc(void* p) {
    NganHang* nh = (NganHang*)p;
    while(true) { sleep(3); nh->Load(); }
    return NULL;
}
#endif

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(65001); SetConsoleCP(65001);
#endif
    NganHang nh; 
    cout << " > Dang khoi tao he thong... " << endl;
    nh.Load(); 
    
#ifdef _WIN32
    _beginthreadex(NULL, 0, SyncThreadFunc, &nh, 0, NULL);
#else
    pthread_t t; pthread_create(&t, NULL, SyncThreadFunc, &nh);
#endif

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

    handle("/api/sync", [&](const httplib::Request&, httplib::Response& res) { 
        res.set_content("{\"status\":\"success\"}", "application/json"); 
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
        auto tk = nh.Tim(param(req, "stk"));
        if(!tk) { res.set_content("{\"error\":\"404\"}", "application/json"); return; }
        string type = "Thanh Toán"; double hm = 0;
        if(dynamic_cast<TaiKhoanTietKiem*>(tk.get())) type="Tiết Kiệm";
        else if(auto* d = dynamic_cast<TaiKhoanTinDung*>(tk.get())) { type="Tín Dụng"; hm=d->GetHanMuc(); }
        // Trả thêm thông tin tín dụng liên kết
        res.set_content("{\"balance\":"+to_string((long long)tk->GetBalance())+",\"loanBalance\":"+to_string((long long)tk->GetSoDuVay())+",\"name\":\""+tk->GetTenKhachHang()+"\",\"type\":\""+type+"\",\"interest\":"+to_string((long long)tk->TinhLai())+",\"locked\":"+(tk->IsLocked()?"true":"false")+",\"hang\":\""+tk->GetHang()+"\",\"hanMuc\":"+to_string((long long)hm)+",\"hmTD\":"+to_string((long long)tk->GetHanMucTinDung())+",\"noTD\":"+to_string((long long)tk->GetNoTinDung())+"}", "application/json");
    });

    handle("/api/user/history", [&](const httplib::Request& req, httplib::Response& res) {
        string stk = param(req, "stk"); string j = "["; bool f = true;
        auto lsMap = nh.GetLS();
        if(lsMap.count(stk)) {
            auto v = lsMap.at(stk);
            for(int i=v.size()-1; i>=0 && i>=(int)v.size()-20; --i) {
                if(!f) j+=","; f=false;
                j += "{\"time\":\""+v[i].thoiGian+"\",\"type\":\""+v[i].loai+"\",\"amount\":"+to_string((long long)v[i].soTien)+",\"note\":\""+v[i].noiDung+"\"}";
            }
        }
        j+="]"; res.set_content(j, "application/json");
    });

    handle("/api/user/transact", [&](const httplib::Request& req, httplib::Response& res) {
        string stk=param(req,"stk"), type=param(req,"type"), pin=param(req,"pin"), dest=param(req,"dest"), note=param(req,"note"), source=param(req,"source");
        if(note.empty()) note = (type=="nap"?"Nạp tiền":type=="rut"?"Rút tiền":"Chuyển khoản");
        double amt = 0;
        try { amt = stod(param(req,"amount")); } catch(...) { res.set_content("{\"status\":\"error\",\"msg\":\"Số tiền không hợp lệ\"}", "application/json"); return; }
        
        auto tk = nh.Tim(stk);
        try {
            if(!tk) throw runtime_error("Tài khoản không tồn tại");
            if(tk->IsLocked()) throw runtime_error("Tài khoản đang bị khóa");
            if(amt <= 0) throw runtime_error("Số tiền phải lớn hơn 0");

            if(type=="nap") {
                if(source == "credit") {
                    tk->NapTienTinDung(amt);
                    nh.GhiLog(stk, "TRA_NO_TD", amt, "Thanh toán nợ tín dụng");
                } else {
                    tk->NapTien(amt);
                    nh.GhiLog(stk, "NAP", amt, note);
                }
            }
            else if(type=="rut") {
                if(tk->GetMaPIN() != pin) throw runtime_error("Sai mã PIN");
                bool ok = (source == "credit") ? tk->RutTienTinDung(amt) : tk->RutTien(amt);
                if(!ok) throw runtime_error("Số dư hoặc hạn mức không đủ");
                nh.GhiLog(stk, (source == "credit" ? "RUT_TD" : "RUT"), -amt, note + (source == "credit" ? " (Từ tín dụng)" : ""));
            }
            else if(type=="chuyen") {
                if(stk == dest) throw runtime_error("Không thể chuyển cho chính mình");
                auto dt = nh.Tim(dest);
                if(!dt) throw runtime_error("Tài khoản thụ hưởng không tồn tại");
                if(dt->IsLocked()) throw runtime_error("Tài khoản thụ hưởng đang bị khóa");
                if(tk->GetMaPIN() != pin) throw runtime_error("Sai mã PIN");
                
                bool ok = (source == "credit") ? tk->RutTienTinDung(amt) : tk->RutTien(amt);
                if(ok) {
                    dt->NapTien(amt);
                    nh.GhiLog(stk, "CHUYEN", -amt, "Chuyển đến " + dest + ": " + note + (source == "credit" ? " (Từ tín dụng)" : ""));
                    nh.GhiLog(dest, "NHAN_TIEN", amt, "Nhận từ " + stk + ": " + note);
                } else throw runtime_error("Số dư hoặc hạn mức không đủ");
            }
            else throw runtime_error("Loại giao dịch không hợp lệ");

            nh.Save(); 
            res.set_content("{\"status\":\"success\"}", "application/json");
        } catch(exception& e) {
            res.set_content("{\"status\":\"error\",\"msg\":\"" + string(e.what()) + "\"}", "application/json");
        }
    });

    handle("/api/user/request_credit", [&](const httplib::Request& req, httplib::Response& res) {
        auto tk = nh.Tim(param(req,"stk"));
        if(!tk) { res.set_content("{\"status\":\"error\",\"msg\":\"Tài khoản không tồn tại\"}", "application/json"); return; }
        
        string h = tk->GetHang();
        long long limit = 20000000;
        if(h == "VIP") limit = 40000000;
        else if(h == "Signature") limit = 60000000;
        else if(h == "Private") limit = 100000000;

        nh.AddYeuCau(YeuCau("CREDIT", tk->GetSoTaiKhoan(), "", to_string(limit), "Mở thẻ tín dụng hạng " + h));
        res.set_content("{\"status\":\"success\"}", "application/json");
    });

    handle("/api/admin/reject_request", [&](const httplib::Request& req, httplib::Response& res) {
        nh.XoaYeuCau(param(req, "stk"), param(req, "type"));
        nh.Save();
        res.set_content("{\"status\":\"success\"}", "application/json");
    });

    handle("/api/user/request_loan", [&](const httplib::Request& req, httplib::Response& res) {
        nh.AddYeuCau(YeuCau("LOAN", param(req,"stk"), "", param(req,"amount"), param(req,"phut")));
        res.set_content("{\"status\":\"success\"}", "application/json");
    });

    handle("/api/user/loan_info", [&](const httplib::Request& req, httplib::Response& res) {
        string stk = param(req, "stk"); long long now = (long long)time(0);
        for(auto& v : nh.GetDSVay()) if(v.stk == stk) {
            long long rem = v.hanTra - now;
            res.set_content("{\"status\":\"active\",\"amount\":"+to_string((long long)v.soTien)+",\"interestRate\":"+to_string(v.laiSuat)+",\"totalDue\":"+to_string((long long)(v.soTien*(1+v.laiSuat)))+",\"remaining\":"+to_string(rem)+"}", "application/json");
            return;
        }
        res.set_content("{\"status\":\"none\"}", "application/json");
    });

    handle("/api/admin/all", [&](const httplib::Request&, httplib::Response& res) {
        string j = "["; bool f = true;
        for(auto& tk : nh.GetDS()) {
            if(!f) j+=","; f=false;
            string t = "ThanhToan"; if(dynamic_cast<TaiKhoanTietKiem*>(tk.get())) t="TietKiem"; else if(dynamic_cast<TaiKhoanTinDung*>(tk.get())) t="TinDung";
            // Sử dụng GetBalance() để Admin thấy đúng số tiền người dùng có
            j += "{\"stk\":\""+tk->GetSoTaiKhoan()+"\",\"name\":\""+tk->GetTenKhachHang()+"\",\"balance\":"+to_string((long long)tk->GetBalance())+",\"type\":\""+t+"\",\"locked\":"+(tk->IsLocked()?"true":"false")+",\"hang\":\""+tk->GetHang()+"\"}";
        }
        j+="]"; res.set_content(j, "application/json");
    });

    handle("/api/admin/upgrade_requests", [&](const httplib::Request&, httplib::Response& res) {
        string j = "["; bool f = true;
        for(auto& r : nh.GetYeuCau()) { if(!f) j+=","; f=false; j += "{\"type\":\""+r.type+"\",\"stk\":\""+r.stk+"\",\"name\":\""+r.ten+"\",\"val\":\""+r.val+"\",\"hang\":\""+r.hang+"\",\"time\":\""+r.thoiGian+"\"}"; }
        j+="]"; res.set_content(j, "application/json");
    });

    handle("/api/admin/loans", [&](const httplib::Request&, httplib::Response& res) {
        string j = "["; bool f = true; long long now = (long long)time(0);
        for(auto& v : nh.GetDSVay()) {
            auto tk_ptr = nh.Tim(v.stk); if(!f) j+=","; f=false;
            j += "{\"stk\":\""+v.stk+"\",\"name\":\""+(tk_ptr?tk_ptr->GetTenKhachHang():"---")+"\",\"amount\":"+to_string((long long)v.soTien)+",\"interestRate\":"+to_string(v.laiSuat)+",\"totalDue\":"+to_string((long long)(v.soTien*(1+v.laiSuat)))+",\"remaining\":"+to_string(v.hanTra - now)+"}";
        }
        j+="]"; res.set_content(j, "application/json");
    });

    handle("/api/admin/loan_history", [&](const httplib::Request&, httplib::Response& res) {
        string j = "["; bool f = true;
        for(auto& l : nh.GetLichSuVay()) { if(!f) j+=","; f=false; j += "{\"stk\":\""+l.stk+"\",\"name\":\""+l.ten+"\",\"amount\":"+to_string((long long)l.soTien)+",\"rate\":"+to_string(l.laiSuat)+",\"han\":\""+l.hanVay+"\",\"time\":\""+l.thoiGian+"\",\"status\":\""+l.trangThai+"\"}"; }
        j+="]"; res.set_content(j, "application/json");
    });

    handle("/api/admin/system_info", [&](const httplib::Request&, httplib::Response& res) {
        res.set_content("{\"lastInterest\":\""+nh.GetThoiGianTinhLai()+"\"}", "application/json");
    });

    handle("/api/admin/approve_request", [&](const httplib::Request& req, httplib::Response& res) {
        string stk = param(req, "stk"), type = param(req, "type"), val = param(req, "val"), hang = param(req, "hang");
        auto tk = nh.Tim(stk);
        if(tk) {
            if(type=="LOAN") { double a = stod(val); int p = stoi(hang); tk->NapTien(a); nh.AddVay(stk, a, p); }
            else if(type=="UPGRADE") tk->SetHang(val);
            else if(type=="CREDIT") { tk->SetHanMucTinDung(stod(val)); tk->SetNoTinDung(0); }
            else if(type=="RESET_PIN") tk->SetMaPIN("1234");
            
            nh.XoaYeuCau(stk, type); res.set_content("{\"status\":\"success\"}", "application/json");
        } else res.set_content("{\"status\":\"error\"}", "application/json");
    });

    handle("/api/admin/lock", [&](const httplib::Request& req, httplib::Response& res) {
        auto tk = nh.Tim(param(req,"stk")); if(tk) { tk->SetLocked(param(req,"lock")=="1"); nh.Save(); }
        res.set_content("{\"status\":\"success\"}", "application/json");
    });

    handle("/api/register", [&](const httplib::Request& req, httplib::Response& res) {
        string s = param(req,"stk"), t = param(req,"ten"), p = param(req,"pin"), y = param(req,"type");
        if(nh.Tim(s)) { res.set_content("{\"status\":\"error\",\"msg\":\"STK đã tồn tại\"}", "application/json"); return; }
        if(y == "TietKiem") nh.Add(new TaiKhoanTietKiem(s, t, 50000, 0.05, 12, p));
        else nh.Add(new TaiKhoanThanhToan(s, t, 50000, p));
        res.set_content("{\"status\":\"success\"}", "application/json");
    });

    handle("/api/admin/deposit", [&](const httplib::Request& req, httplib::Response& res) {
        string s = param(req,"stk"); double a = stod(param(req,"amount"));
        auto tk = nh.Tim(s);
        if(tk) { 
            tk->NapTien(a); 
            nh.GhiLog(s, "NAP", a, "Nap tai quay"); 
            nh.Save(); 
            res.set_content("{\"status\":\"success\"}", "application/json"); 
        }
        else res.set_content("{\"status\":\"error\",\"msg\":\"Khong tim thay TK\"}", "application/json");
    });

    handle("/api/admin/add", [&](const httplib::Request& req, httplib::Response& res) {
        string s = param(req,"stk"), t = param(req,"ten"), p = param(req,"pin"), y = param(req,"type");
        double d = stod(param(req,"sodu"));
        if(nh.Tim(s)) { res.set_content("{\"status\":\"error\",\"msg\":\"STK da ton tai\"}", "application/json"); return; }
        if(y=="TietKiem") nh.Add(new TaiKhoanTietKiem(s, t, d, 0.05, 12, p));
        else if(y=="TinDung") nh.Add(new TaiKhoanTinDung(s, t, 0, d, p));
        else nh.Add(new TaiKhoanThanhToan(s, t, d, p));
        res.set_content("{\"status\":\"success\"}", "application/json");
    });

    handle("/api/admin/delete", [&](const httplib::Request& req, httplib::Response& res) {
        string s = param(req, "stk");
        nh.XoaTK(s);
        res.set_content("{\"status\":\"success\"}", "application/json");
    });

    handle("/api/admin/interest", [&](const httplib::Request&, httplib::Response& res) {
        for(auto& tk : nh.GetDS()) {
            double lai = tk->TinhLai();
            if(lai > 0) { tk->NapTien(lai); nh.GhiLog(tk->GetSoTaiKhoan(), "LAI", lai, "Lai dinh ky"); }
        }
        time_t now = time(0); tm *ltm = localtime(&now); char buf[80];
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", ltm);
        nh.SetThoiGianTinhLai(string(buf));
        res.set_content("{\"status\":\"success\"}", "application/json");
    });

    handle("/api/admin/upgrade", [&](const httplib::Request& req, httplib::Response& res) {
        auto tk = nh.Tim(param(req,"stk"));
        if(tk) { tk->SetHang(param(req,"hang")); nh.Save(); res.set_content("{\"status\":\"success\"}", "application/json"); }
        else res.set_content("{\"status\":\"error\"}", "application/json");
    });

    handle("/api/user/repay_loan", [&](const httplib::Request& req, httplib::Response& res) {
        string stk = param(req, "stk"), pin = param(req, "pin");
        auto tk = nh.Tim(stk);
        if(!tk || tk->GetMaPIN() != pin) { res.set_content("{\"status\":\"error\",\"msg\":\"Sai PIN\"}", "application/json"); return; }
        if(nh.TatToanVay(stk)) {
            res.set_content("{\"status\":\"success\"}", "application/json");
        } else {
            res.set_content("{\"status\":\"error\",\"msg\":\"Không có khoản vay hoặc không đủ tiền\"}", "application/json");
        }
    });

    handle("/api/user/changepin", [&](const httplib::Request& req, httplib::Response& res) {
        auto tk = nh.Tim(param(req,"stk"));
        if(tk && tk->GetMaPIN() == param(req,"oldpin")) { tk->SetMaPIN(param(req,"newpin")); nh.Save(); res.set_content("{\"status\":\"success\"}", "application/json"); }
        else res.set_content("{\"status\":\"error\",\"msg\":\"Sai PIN cu\"}", "application/json");
    });

    handle("/api/user/request_upgrade", [&](const httplib::Request& req, httplib::Response& res) {
        nh.AddYeuCau(YeuCau("UPGRADE", param(req,"stk"), "", param(req,"hang"), ""));
        res.set_content("{\"status\":\"success\"}", "application/json");
    });

    handle("/api/user/request_reset_pin", [&](const httplib::Request& req, httplib::Response& res) {
        nh.AddYeuCau(YeuCau("RESET_PIN", param(req,"stk"), "", "Yêu cầu cấp lại mã PIN", ""));
        res.set_content("{\"status\":\"success\"}", "application/json");
    });

    cout << "OOP BANK ONLINE: http://localhost:8080" << endl;
    svr.listen("0.0.0.0", 8080);
    return 0;
}
