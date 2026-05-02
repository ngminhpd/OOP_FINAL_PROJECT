#pragma once
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
#include <atomic>
#include "Models.h"

#ifdef _WIN32
#include <windows.h>
#include <process.h>
#else
#include <pthread.h>
#endif

namespace BankSystem {
    using namespace std;

    class BankManager {
    private:
        vector<shared_ptr<TaiKhoan>> dsTK;
        map<string, vector<GiaoDich>> dsLS;
        vector<YeuCau> dsYeuCau;
        vector<KhoanVay> dsVay;
        vector<LichSuVay> dsLichSuVay;
        
        string thoiGianTinhLai = "Chưa có dữ liệu";
        string dsCustomers = ""; 
        
#ifdef _WIN32
        CRITICAL_SECTION cs;
#else
        pthread_mutex_t mtx;
#endif
        atomic<bool> isSaving{false};
        atomic<bool> dirty{false};
        time_t lastSave = 0;

        const string GS_URL = "https://script.google.com/macros/s/AKfycbygfKZMkf1MAztbT2vYArSq3yLeo74qZVpnFOUIWNttjxlkuUgAEm84DOmu0RmYCfjq/exec";

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

        string JSON_Escape(string data) {
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

        string GetJSONValue(const string& json, string key) {
            string search = "\"" + key + "\":\"";
            size_t start = json.find(search);
            if (start == string::npos) return "";
            start += search.length();
            size_t end = start;
            while (end < json.length()) {
                if (json[end] == '\"' && (end == 0 || json[end - 1] != '\\')) break;
                end++;
            }
            string val = json.substr(start, end - start);
            string clean = ""; clean.reserve(val.length());
            for (size_t i = 0; i < val.length(); i++) {
                if (val[i] == '\\' && i + 1 < val.length()) {
                    if (val[i + 1] == 'n') clean += '\n';
                    else if (val[i + 1] == '\"') clean += '\"';
                    else if (val[i + 1] == '\\') clean += '\\';
                    else clean += val[i + 1];
                    i++;
                }
                else clean += val[i];
            }
            return clean;
        }

        void CleanString(string& s) {
            if (!s.empty() && s[0] == '\'') s = s.substr(1);
            size_t first = s.find_first_not_of(" \t\r\n");
            if (first == string::npos) { s = ""; return; }
            size_t last = s.find_last_not_of(" \t\r\n");
            s = s.substr(first, (last - first + 1));
        }

        struct SaveContext { string json; BankManager* manager; };

        static unsigned __stdcall SaveThreadFunc(void* p) {
            SaveContext* ctx = (SaveContext*)p;
            string tmpFile = "post_" + to_string(time(0)) + "_" + to_string(rand()) + ".json";
            ofstream f(tmpFile); f << ctx->json; f.close();
            string cmd = "curl.exe -k -L -s -X POST -H \"Content-Type: application/json\" -d @" + tmpFile + " \"" + ctx->manager->GS_URL + "\" > nul 2>&1 && del " + tmpFile;
            system(cmd.c_str());
            ctx->manager->FinishSaving();
            delete ctx;
            return 0;
        }

    public:
        void FinishSaving() { isSaving = false; lastSave = time(0); }

        BankManager() {
#ifdef _WIN32
            InitializeCriticalSection(&cs);
#else
            pthread_mutex_init(&mtx, NULL);
#endif
        }
        ~BankManager() {
#ifdef _WIN32
            DeleteCriticalSection(&cs);
#else
            pthread_mutex_destroy(&mtx);
#endif
        }

        shared_ptr<TaiKhoan> Tim(string stk) {
            Lock();
            shared_ptr<TaiKhoan> res = nullptr;
            for (auto& tk : dsTK) if (tk->GetSoTaiKhoan() == stk) { res = tk; break; }
            Unlock();
            return res;
        }

        void GhiLog(string stk, string loai, double st, string nd) {
            Lock();
            for (char& c : nd) if (c == ';' || c == '\n' || c == '\r') c = ' ';
            dsLS[stk].emplace_back(loai, st, nd);
            Unlock();
        }

        void SetDirty() { dirty = true; }

        void Load() {
            // Khong load neu dang luu hoac co du lieu moi trong RAM chua kip luu
            if (isSaving || dirty || time(0) - lastSave < 5) return;

            string tmp = "res_load_" + to_string(time(0)) + ".json";
            string cmd = "curl.exe -k -L -s --connect-timeout 3 \"" + GS_URL + "\" > " + tmp;
            system(cmd.c_str());

            ifstream f(tmp);
            if (!f) return;
            string all((istreambuf_iterator<char>(f)), istreambuf_iterator<char>());
            f.close(); remove(tmp.c_str());
            if (all.find("accounts") == string::npos) return;

            vector<shared_ptr<TaiKhoan>> nextTK;
            map<string, vector<GiaoDich>> nextLS;
            vector<YeuCau> nextYeuCau;
            vector<KhoanVay> nextVay;
            vector<LichSuVay> nextLichSuVay;

            auto ParseTab = [&](string data, string mode) {
                stringstream ss(data); string l;
                while (getline(ss, l)) {
                    if (l.empty()) continue;
                    stringstream row(l);
                    if (mode == "acc") {
                        string tp, stk, ten, pin, du_s, khoa_s, ls_s, kh_s, hm_s, hang, dv_s, hmTD_s, noTD_s;
                        getline(row, tp, ';'); getline(row, stk, ';'); getline(row, ten, ';'); getline(row, pin, ';');
                        getline(row, du_s, ';'); getline(row, khoa_s, ';'); getline(row, ls_s, ';'); getline(row, kh_s, ';');
                        getline(row, hm_s, ';'); getline(row, hang, ';'); getline(row, dv_s, ';');
                        getline(row, hmTD_s, ';'); getline(row, noTD_s, ';');
                        CleanString(stk); CleanString(pin);
                        if (stk.empty()) continue;
                        try {
                            double du = stod(du_s); bool k = (khoa_s == "1"); double dv = dv_s.empty() ? 0 : stod(dv_s);
                            double hmTD = hmTD_s.empty() ? 0 : stod(hmTD_s); double noTD = noTD_s.empty() ? 0 : stod(noTD_s);
                            if (tp == "ThanhToan") nextTK.push_back(make_shared<TaiKhoanThanhToan>(stk, ten, du, pin, k, hang, dv));
                            else if (tp == "TietKiem") nextTK.push_back(make_shared<TaiKhoanTietKiem>(stk, ten, du, stod(ls_s), stoi(kh_s), pin, k, hang, dv));
                            else if (tp == "TinDung") nextTK.push_back(make_shared<TaiKhoanTinDung>(stk, ten, du, stod(hm_s), pin, k, hang, dv, noTD));
                        } catch (...) {}
                    }
                    else if (mode == "req") {
                        string t, s, n, v, h, p, tg;
                        getline(row, t, ';'); getline(row, s, ';'); getline(row, n, ';'); getline(row, v, ';'); getline(row, h, ';'); getline(row, p, ';'); getline(row, tg, ';');
                        CleanString(s); CleanString(p);
                        if (!t.empty()) nextYeuCau.emplace_back(t, s, n, v, h, p, tg);
                    }
                    else if (mode == "his") {
                        string s, lo, ti, nd, t;
                        if (!getline(row, s, ';')) continue;
                        getline(row, lo, ';'); getline(row, ti, ';'); getline(row, nd, ';'); getline(row, t, ';');
                        CleanString(s);
                        if (!s.empty()) try { nextLS[s].emplace_back(t, lo, stod(ti), nd); } catch (...) {}
                    }
                    else if (mode == "loan") {
                        string s, a, h, ls;
                        getline(row, s, ';'); getline(row, a, ';'); getline(row, h, ';'); getline(row, ls, ';');
                        CleanString(s);
                        if (!s.empty()) try { 
                            double amt = stod(a);
                            double rate = stod(ls);
                            if (rate > 1.0 || rate < 0) {
                                if (amt < 1000000) rate = 0.04;
                                else if (amt < 10000000) rate = 0.045;
                                else if (amt < 100000000) rate = 0.05;
                                else rate = 0.06;
                            }
                            nextVay.emplace_back(s, amt, stoll(h), rate); 
                        } catch (...) {}
                    }
                    else if (mode == "l_his") {
                        string s, n, tg, hv, st, ls, tt;
                        getline(row, s, ';'); getline(row, n, ';'); getline(row, tg, ';'); getline(row, hv, ';'); getline(row, st, ';'); getline(row, ls, ';'); getline(row, tt, ';');
                        CleanString(s);
                        if (!s.empty()) try { nextLichSuVay.emplace_back(s, n, tg, hv, stod(st), stod(ls), tt); } catch (...) {}
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
            if (!isSaving && !dirty) {
                dsTK = nextTK; dsLS = nextLS; dsYeuCau = nextYeuCau; dsVay = nextVay; dsLichSuVay = nextLichSuVay;
                thoiGianTinhLai = GetJSONValue(all, "system"); dsCustomers = GetJSONValue(all, "customers");
            }
            Unlock();
        }

        void Save() {
            if (isSaving) return;
            Lock();
            isSaving = true;
            dirty = false;

            for (auto& tk : dsTK) {
                double val = tk->GetLoai() == "TinDung" ? tk->GetHanMucTinDung() : tk->GetSoDu();
                if (val >= 1000000000.0) tk->SetHang("Private");
                else if (val >= 500000000.0) tk->SetHang("Signature");
            }

            stringstream accSS, hisSS, reqSS, loanSS, lhisSS;
            for (auto& tk : dsTK) {
                string tp = tk->GetLoai(), ls = "0", kh = "0", hm = "0";
                if (auto* s = dynamic_cast<TaiKhoanTietKiem*>(tk.get())) { ls = to_string(s->GetLaiSuat()); kh = to_string(s->GetKyHan()); }
                else if (tk->GetLoai() == "TinDung") { hm = to_string((long long)tk->GetHanMucTinDung()); }
                
                accSS << tp << ";'" << tk->GetSoTaiKhoan() << ";" << tk->GetTenKhachHang() << ";'" << tk->GetMaPIN() << ";" 
                      << fixed << setprecision(0) << tk->GetSoDu() << ";" << (tk->IsLocked() ? "1" : "0") << ";" 
                      << ls << ";" << kh << ";" << hm << ";" << tk->GetHang() << ";" 
                      << (long long)tk->GetSoDuVay() << ";" << (long long)tk->GetHanMucTinDung() << ";" << (long long)tk->GetNoTinDung() << "\n";
            }

            for (auto& it : dsLS) {
                for (auto& g : it.second) {
                    hisSS << "'" << it.first << ";" << g.loai << ";" << fixed << setprecision(0) << g.soTien << ";" << g.noiDung << ";" << g.thoiGian << "\n";
                }
            }

            for (auto& r : dsYeuCau) reqSS << r.type << ";'" << r.stk << ";" << r.ten << ";" << r.val << ";" << r.hang << ";'" << r.pin << ";" << r.thoiGian << "\n";
            for (auto& v : dsVay) loanSS << "'" << v.stk << ";" << fixed << setprecision(0) << v.soTien << ";" << v.hanTra << ";" << fixed << setprecision(4) << v.laiSuat << "\n";
            for (auto& l : dsLichSuVay) lhisSS << "'" << l.stk << ";" << l.ten << ";" << l.thoiGian << ";" << l.hanVay << ";" << fixed << setprecision(0) << l.soTien << ";" << fixed << setprecision(4) << l.laiSuat << ";" << l.trangThai << "\n";

            string json = "{\"bulk\":{\"accounts\":\"" + JSON_Escape(accSS.str()) + "\",\"history\":\"" + JSON_Escape(hisSS.str()) + "\",\"requests\":\"" + JSON_Escape(reqSS.str()) + "\",\"loans\":\"" + JSON_Escape(loanSS.str()) + "\",\"loan_history\":\"" + JSON_Escape(lhisSS.str()) + "\",\"system\":\"" + JSON_Escape(thoiGianTinhLai) + "\",\"customers\":\"" + JSON_Escape(dsCustomers) + "\"}}";
            Unlock();

            SaveContext* ctx = new SaveContext{json, this};
#ifdef _WIN32
            _beginthreadex(NULL, 0, SaveThreadFunc, ctx, 0, NULL);
#else
            // Simple logic for non-windows
            string tmpFile = "post_" + to_string(time(0)) + ".json";
            ofstream f(tmpFile); f << json; f.close();
            string cmd = "curl -k -L -s -X POST -H \"Content-Type: application/json\" -d @" + tmpFile + " \"" + GS_URL + "\" > /dev/null 2>&1 && rm " + tmpFile + " &";
            system(cmd.c_str());
            FinishSaving();
#endif
        }

        void Add(shared_ptr<TaiKhoan> tk) {
            Lock(); dsTK.push_back(tk); Unlock();
            GhiLog(tk->GetSoTaiKhoan(), "KHOI_TAO", tk->GetSoDu(), "Mo tai khoan");
            SetDirty();
        }

        void AddYeuCau(YeuCau y) { Lock(); dsYeuCau.push_back(y); Unlock(); SetDirty(); }

        bool KiemTraVaXoaYeuCau(string stk, string type) {
            Lock();
            auto it = find_if(dsYeuCau.begin(), dsYeuCau.end(), [&](const YeuCau& r) { return r.stk == stk && r.type == type; });
            if (it != dsYeuCau.end()) { dsYeuCau.erase(it); Unlock(); SetDirty(); return true; }
            Unlock(); return false;
        }

        void XoaYeuCau(string stk, string type) {
            Lock();
            dsYeuCau.erase(remove_if(dsYeuCau.begin(), dsYeuCau.end(), [&](const YeuCau& r) { return r.stk == stk && r.type == type; }), dsYeuCau.end());
            Unlock(); SetDirty();
        }

        void AddVay(string stk, double amt, int phut) {
            double ls = 0.06;
            if (amt < 1000000) ls = 0.04;
            else if (amt < 10000000) ls = 0.045;
            else if (amt < 100000000) ls = 0.05;

            string timeStr = GetCurrentTimeStr();
            Lock();
            dsVay.emplace_back(stk, amt, (long long)time(0) + phut * 60, ls);
            for (auto& tk : dsTK) if (tk->GetSoTaiKhoan() == stk) {
                tk->NapTienVay(amt);
                dsLichSuVay.emplace_back(stk, tk->GetTenKhachHang(), timeStr, to_string(phut) + " phút", amt, ls, "Chưa trả");
                break;
            }
            Unlock();
            GhiLog(stk, "VAY_TIEN", amt, "Duyet khoan vay (" + to_string(phut) + " phut)");
            SetDirty();
        }

        void XoaTK(string stk) {
            Lock();
            dsTK.erase(remove_if(dsTK.begin(), dsTK.end(), [&](const shared_ptr<TaiKhoan>& t) { return t->GetSoTaiKhoan() == stk; }), dsTK.end());
            dsLS.erase(stk);
            Unlock(); SetDirty();
        }

        bool TatToanVay(string stk) {
            Lock();
            auto itV = find_if(dsVay.begin(), dsVay.end(), [&](const KhoanVay& v) { return v.stk == stk; });
            if (itV == dsVay.end()) { Unlock(); return false; }

            for (auto& tk : dsTK) if (tk->GetSoTaiKhoan() == stk) {
                double tongNo = itV->soTien * (1 + itV->laiSuat);
                if (tk->GetSoDu() + tk->GetSoDuVay() < tongNo) { Unlock(); return false; }

                if (tk->GetSoDu() >= tongNo) tk->SetSoDu(tk->GetSoDu() - tongNo);
                else { double rem = tongNo - tk->GetSoDu(); tk->SetSoDu(0); tk->SetSoDuVay(tk->GetSoDuVay() - rem); }

                dsVay.erase(itV);
                for (auto& l : dsLichSuVay) if (l.stk == stk && l.trangThai == "Chưa trả") { l.trangThai = "Đã trả"; break; }
                Unlock();
                GhiLog(stk, "TRA_NO", -tongNo, "Tat toan khoan vay");
                SetDirty();
                return true;
            }
            Unlock(); return false;
        }

        void TinhLaiHeThong() {
            Lock();
            for (auto& tk : dsTK) {
                double lai = tk->TinhLai();
                if (lai > 0) { tk->NapTien(lai); dsLS[tk->GetSoTaiKhoan()].emplace_back("LAI", lai, "Lai dinh ky"); }
            }
            thoiGianTinhLai = GetCurrentTimeStr();
            Unlock(); SetDirty();
        }

        void Sync() {
            if (isSaving) return;
            if (dirty) Save();
            else Load();
        }

        vector<shared_ptr<TaiKhoan>> GetDS() { Lock(); auto res = dsTK; Unlock(); return res; }
        map<string, vector<GiaoDich>> GetLS() { Lock(); auto res = dsLS; Unlock(); return res; }
        vector<YeuCau> GetYeuCau() { Lock(); auto res = dsYeuCau; Unlock(); return res; }
        vector<KhoanVay> GetDSVay() { Lock(); auto res = dsVay; Unlock(); return res; }
        vector<LichSuVay> GetLichSuVay() { Lock(); auto res = dsLichSuVay; Unlock(); return res; }
        string GetThoiGianTinhLai() { Lock(); string res = thoiGianTinhLai; Unlock(); return res; }
        void SetThoiGianTinhLai(string t) { Lock(); thoiGianTinhLai = t; Unlock(); Save(); }
    };
}
