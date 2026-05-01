#pragma once
#include "httplib.h"
#include "BankManager.h"
#include <fstream>
#include <sstream>

namespace BankSystem {
    using namespace std;

    class BankServer {
    private:
        BankManager& nh;
        httplib::Server svr;

        string url_decode(string str) {
            string res;
            for (size_t i = 0; i < str.length(); i++) {
                if (str[i] == '%' && i + 2 < str.length()) {
                    int ii; sscanf(str.substr(i + 1, 2).c_str(), "%x", &ii);
                    res += static_cast<char>(ii); i += 2;
                }
                else if (str[i] == '+') res += ' ';
                else res += str[i];
            }
            return res;
        }

        string param(const httplib::Request& req, string key) {
            return url_decode(req.get_param_value(key));
        }

    public:
        BankServer(BankManager& manager) : nh(manager) {
            SetupRoutes();
        }

        void SetupRoutes() {
            auto handle = [&](string path, httplib::Server::Handler h) {
                svr.Get(path, h); svr.Post(path, h);
            };

            // HTML Root
            svr.Get("/", [](const httplib::Request&, httplib::Response& res) {
                ifstream f("index.html", ios::binary);
                if (!f) { res.set_content("index.html not found", "text/html"); return; }
                res.set_content(string((istreambuf_iterator<char>(f)), istreambuf_iterator<char>()), "text/html");
            });

            // Core APIs
            handle("/api/sync", [](const httplib::Request&, httplib::Response& res) {
                res.set_content("{\"status\":\"success\"}", "application/json");
            });

            handle("/api/login", [this](const httplib::Request& req, httplib::Response& res) {
                string u = param(req, "user"), p = param(req, "pin");
                if (u == "admin" && p == "123") { res.set_content("{\"status\":\"success\",\"role\":\"admin\"}", "application/json"); return; }
                auto tk = nh.Tim(u);
                if (tk && tk->GetMaPIN() == p) {
                    if (tk->IsLocked()) res.set_content("{\"status\":\"error\",\"msg\":\"Tài khoản bị khóa\"}", "application/json");
                    else res.set_content("{\"status\":\"success\",\"role\":\"user\",\"stk\":\"" + u + "\",\"name\":\"" + tk->GetTenKhachHang() + "\"}", "application/json");
                }
                else res.set_content("{\"status\":\"error\",\"msg\":\"Sai thông tin\"}", "application/json");
            });

            handle("/api/register", [this](const httplib::Request& req, httplib::Response& res) {
                string s = param(req, "stk"), t = param(req, "ten"), p = param(req, "pin"), y = param(req, "type");
                if (nh.Tim(s)) { res.set_content("{\"status\":\"error\",\"msg\":\"STK đã tồn tại\"}", "application/json"); return; }
                if (y == "TietKiem") nh.Add(make_shared<TaiKhoanTietKiem>(s, t, 50000, 0.05, 12, p));
                else nh.Add(make_shared<TaiKhoanThanhToan>(s, t, 50000, p));
                res.set_content("{\"status\":\"success\"}", "application/json");
            });

            // User Info & History
            handle("/api/user/info", [this](const httplib::Request& req, httplib::Response& res) {
                auto tk = nh.Tim(param(req, "stk"));
                if (!tk) { res.set_content("{\"error\":\"404\"}", "application/json"); return; }
                string type = "Thanh Toán";
                if (tk->GetLoai() == "TietKiem") type = "Tiết Kiệm";
                else if (tk->GetLoai() == "TinDung") type = "Tín Dụng";

                stringstream ss;
                ss << "{\"balance\":" << (long long)tk->GetSoDu()
                   << ",\"loanBalance\":" << (long long)tk->GetSoDuVay()
                   << ",\"name\":\"" << tk->GetTenKhachHang()
                   << "\",\"type\":\"" << type
                   << "\",\"interest\":" << (long long)tk->TinhLai()
                   << ",\"locked\":" << (tk->IsLocked() ? "true" : "false")
                   << ",\"hang\":\"" << tk->GetHang()
                   << "\",\"hanMuc\":" << (long long)tk->GetHanMucTinDung()
                   << ",\"hmTD\":" << (long long)tk->GetHanMucTinDung()
                   << ",\"noTD\":" << (long long)tk->GetNoTinDung() << "}";
                res.set_content(ss.str(), "application/json");
            });

            handle("/api/user/history", [this](const httplib::Request& req, httplib::Response& res) {
                string stk = param(req, "stk"); string j = "["; bool f = true;
                auto lsMap = nh.GetLS();
                if (lsMap.count(stk)) {
                    auto v = lsMap.at(stk);
                    for (int i = v.size() - 1; i >= 0 && i >= (int)v.size() - 20; --i) {
                        if (!f) j += ","; f = false;
                        j += "{\"time\":\"" + v[i].thoiGian + "\",\"type\":\"" + v[i].loai + "\",\"amount\":" + to_string((long long)v[i].soTien) + ",\"note\":\"" + v[i].noiDung + "\"}";
                    }
                }
                j += "]"; res.set_content(j, "application/json");
            });

            // Transactions
            handle("/api/user/transact", [this](const httplib::Request& req, httplib::Response& res) {
                string stk = param(req, "stk"), type = param(req, "type"), pin = param(req, "pin"), dest = param(req, "dest"), note = param(req, "note"), source = param(req, "source");
                double amt = 0;
                try { amt = stod(param(req, "amount")); } catch (...) { res.set_content("{\"status\":\"error\",\"msg\":\"Số tiền không hợp lệ\"}", "application/json"); return; }

                auto tk = nh.Tim(stk);
                try {
                    if (!tk) throw runtime_error("Tài khoản không tồn tại");
                    if (tk->IsLocked()) throw runtime_error("Tài khoản đang bị khóa");
                    if (amt <= 0) throw runtime_error("Số tiền phải lớn hơn 0");
                    if (tk->GetMaPIN() != pin) throw runtime_error("Sai mã PIN");

                    if (type == "nap") {
                        if (source == "credit") { tk->NapTienTinDung(amt); nh.GhiLog(stk, "TRA_NO_TD", amt, "Thanh toán nợ tín dụng"); }
                        else { tk->NapTien(amt); nh.GhiLog(stk, "NAP", amt, note.empty() ? "Nạp tiền" : note); }
                    }
                    else if (type == "rut") {
                        bool ok = (source == "credit") ? tk->RutTienTinDung(amt) : tk->RutTien(amt);
                        if (!ok) throw runtime_error("Số dư hoặc hạn mức không đủ");
                        nh.GhiLog(stk, (source == "credit" ? "RUT_TD" : "RUT"), -amt, (note.empty() ? "Rút tiền" : note) + (source == "credit" ? " (Từ tín dụng)" : ""));
                    }
                    else if (type == "chuyen") {
                        if (stk == dest) throw runtime_error("Không thể chuyển cho chính mình");
                        auto dt = nh.Tim(dest);
                        if (!dt) throw runtime_error("Tài khoản thụ hưởng không tồn tại");
                        if (dt->IsLocked()) throw runtime_error("Tài khoản thụ hưởng đang bị khóa");
                        bool ok = (source == "credit") ? tk->RutTienTinDung(amt) : tk->RutTien(amt);
                        if (ok) {
                            dt->NapTien(amt);
                            nh.GhiLog(stk, "CHUYEN", -amt, "Chuyển đến " + dest + ": " + note + (source == "credit" ? " (Từ tín dụng)" : ""));
                            nh.GhiLog(dest, "NHAN_TIEN", amt, "Nhận từ " + stk + ": " + note);
                        }
                        else throw runtime_error("Số dư hoặc hạn mức không đủ");
                    }
                    nh.Save();
                    res.set_content("{\"status\":\"success\"}", "application/json");
                }
                catch (exception& e) { res.set_content("{\"status\":\"error\",\"msg\":\"" + string(e.what()) + "\"}", "application/json"); }
            });

            // Loans
            handle("/api/user/request_loan", [this](const httplib::Request& req, httplib::Response& res) {
                nh.AddYeuCau(YeuCau("LOAN", param(req, "stk"), "", param(req, "amount"), param(req, "phut")));
                res.set_content("{\"status\":\"success\"}", "application/json");
            });

            handle("/api/user/loan_info", [this](const httplib::Request& req, httplib::Response& res) {
                string stk = param(req, "stk"); long long now = (long long)time(0);
                for (auto& v : nh.GetDSVay()) if (v.stk == stk) {
                    res.set_content("{\"status\":\"active\",\"amount\":" + to_string((long long)v.soTien) + ",\"interestRate\":" + to_string(v.laiSuat) + ",\"totalDue\":" + to_string((long long)(v.soTien * (1 + v.laiSuat))) + ",\"remaining\":" + to_string(v.hanTra - now) + "}", "application/json");
                    return;
                }
                res.set_content("{\"status\":\"none\"}", "application/json");
            });

            handle("/api/user/repay_loan", [this](const httplib::Request& req, httplib::Response& res) {
                string stk = param(req, "stk"), pin = param(req, "pin");
                auto tk = nh.Tim(stk);
                if (!tk || tk->GetMaPIN() != pin) { res.set_content("{\"status\":\"error\",\"msg\":\"Sai PIN\"}", "application/json"); return; }
                if (nh.TatToanVay(stk)) res.set_content("{\"status\":\"success\"}", "application/json");
                else res.set_content("{\"status\":\"error\",\"msg\":\"Không đủ tiền hoặc không có nợ\"}", "application/json");
            });

            // Requests & Settings
            handle("/api/user/request_credit", [this](const httplib::Request& req, httplib::Response& res) {
                auto tk = nh.Tim(param(req, "stk"));
                if (!tk) { res.set_content("{\"status\":\"error\",\"msg\":\"Không tìm thấy TK\"}", "application/json"); return; }
                string h = tk->GetHang();
                long long limit = 20000000;
                if (h == "VIP") limit = 40000000;
                else if (h == "Signature") limit = 60000000;
                else if (h == "Private") limit = 100000000;
                nh.AddYeuCau(YeuCau("CREDIT", tk->GetSoTaiKhoan(), "", to_string(limit), "Mở thẻ tín dụng hạng " + h));
                res.set_content("{\"status\":\"success\"}", "application/json");
            });

            handle("/api/user/request_upgrade", [this](const httplib::Request& req, httplib::Response& res) {
                nh.AddYeuCau(YeuCau("UPGRADE", param(req, "stk"), "", param(req, "hang"), ""));
                res.set_content("{\"status\":\"success\"}", "application/json");
            });

            handle("/api/user/request_reset_pin", [this](const httplib::Request& req, httplib::Response& res) {
                nh.AddYeuCau(YeuCau("RESET_PIN", param(req, "stk"), "", "Yêu cầu cấp lại mã PIN", ""));
                res.set_content("{\"status\":\"success\"}", "application/json");
            });

            handle("/api/user/changepin", [this](const httplib::Request& req, httplib::Response& res) {
                auto tk = nh.Tim(param(req, "stk"));
                if (tk && tk->GetMaPIN() == param(req, "oldpin")) { tk->SetMaPIN(param(req, "newpin")); nh.Save(); res.set_content("{\"status\":\"success\"}", "application/json"); }
                else res.set_content("{\"status\":\"error\",\"msg\":\"Sai PIN cũ\"}", "application/json");
            });

            // Admin Management
            handle("/api/admin/all", [this](const httplib::Request&, httplib::Response& res) {
                string j = "["; bool f = true;
                for (auto& tk : nh.GetDS()) {
                    if (!f) j += ","; f = false;
                    j += "{\"stk\":\"" + tk->GetSoTaiKhoan() + "\",\"name\":\"" + tk->GetTenKhachHang() + "\",\"balance\":" + to_string((long long)tk->GetSoDu()) + ",\"type\":\"" + tk->GetLoai() + "\",\"locked\":" + (tk->IsLocked() ? "true" : "false") + ",\"hang\":\"" + tk->GetHang() + "\"}";
                }
                j += "]"; res.set_content(j, "application/json");
            });

            handle("/api/admin/upgrade_requests", [this](const httplib::Request&, httplib::Response& res) {
                string j = "["; bool f = true;
                for (auto& r : nh.GetYeuCau()) { if (!f) j += ","; f = false; j += "{\"type\":\"" + r.type + "\",\"stk\":\"" + r.stk + "\",\"name\":\"" + r.ten + "\",\"val\":\"" + r.val + "\",\"hang\":\"" + r.hang + "\",\"time\":\"" + r.thoiGian + "\"}"; }
                j += "]"; res.set_content(j, "application/json");
            });

            handle("/api/admin/approve_request", [this](const httplib::Request& req, httplib::Response& res) {
                string stk = param(req, "stk"), type = param(req, "type"), val = param(req, "val"), hang = param(req, "hang");
                if (nh.KiemTraVaXoaYeuCau(stk, type)) {
                    auto tk = nh.Tim(stk);
                    if (tk) {
                        if (type == "LOAN") nh.AddVay(stk, stod(val), stoi(hang));
                        else if (type == "UPGRADE") tk->SetHang(val);
                        else if (type == "CREDIT") tk->SetHanMucTinDung(stod(val));
                        else if (type == "RESET_PIN") tk->SetMaPIN("1234");
                        nh.Save();
                        res.set_content("{\"status\":\"success\"}", "application/json");
                    }
                    else res.set_content("{\"status\":\"error\",\"msg\":\"Không tìm thấy TK\"}", "application/json");
                }
                else res.set_content("{\"status\":\"error\",\"msg\":\"Yêu cầu không tồn tại\"}", "application/json");
            });

            handle("/api/admin/reject_request", [this](const httplib::Request& req, httplib::Response& res) {
                nh.XoaYeuCau(param(req, "stk"), param(req, "type"));
                res.set_content("{\"status\":\"success\"}", "application/json");
            });

            handle("/api/admin/loans", [this](const httplib::Request&, httplib::Response& res) {
                string j = "["; bool f = true; long long now = (long long)time(0);
                for (auto& v : nh.GetDSVay()) {
                    auto tk = nh.Tim(v.stk); if (!f) j += ","; f = false;
                    j += "{\"stk\":\"" + v.stk + "\",\"name\":\"" + (tk ? tk->GetTenKhachHang() : "---") + "\",\"amount\":" + to_string((long long)v.soTien) + ",\"interestRate\":" + to_string(v.laiSuat) + ",\"totalDue\":" + to_string((long long)(v.soTien * (1 + v.laiSuat))) + ",\"remaining\":" + to_string(v.hanTra - now) + "}";
                }
                j += "]"; res.set_content(j, "application/json");
            });

            handle("/api/admin/loan_history", [this](const httplib::Request&, httplib::Response& res) {
                string j = "["; bool f = true;
                for (auto& l : nh.GetLichSuVay()) { if (!f) j += ","; f = false; j += "{\"stk\":\"" + l.stk + "\",\"name\":\"" + l.ten + "\",\"amount\":" + to_string((long long)l.soTien) + ",\"rate\":" + to_string(l.laiSuat) + ",\"han\":\"" + l.hanVay + "\",\"time\":\"" + l.thoiGian + "\",\"status\":\"" + l.trangThai + "\"}"; }
                j += "]"; res.set_content(j, "application/json");
            });

            handle("/api/admin/lock", [this](const httplib::Request& req, httplib::Response& res) {
                auto tk = nh.Tim(param(req, "stk")); if (tk) { tk->SetLocked(param(req, "lock") == "1"); nh.Save(); }
                res.set_content("{\"status\":\"success\"}", "application/json");
            });

            handle("/api/admin/deposit", [this](const httplib::Request& req, httplib::Response& res) {
                string s = param(req, "stk"); double a = stod(param(req, "amount"));
                auto tk = nh.Tim(s);
                if (tk) { tk->NapTien(a); nh.GhiLog(s, "NAP", a, "Nạp tại quầy"); nh.Save(); res.set_content("{\"status\":\"success\"}", "application/json"); }
                else res.set_content("{\"status\":\"error\",\"msg\":\"Không tìm thấy TK\"}", "application/json");
            });

            handle("/api/admin/add", [this](const httplib::Request& req, httplib::Response& res) {
                string s = param(req, "stk"), t = param(req, "ten"), p = param(req, "pin"), y = param(req, "type");
                double d = stod(param(req, "sodu"));
                if (nh.Tim(s)) { res.set_content("{\"status\":\"error\",\"msg\":\"STK đã tồn tại\"}", "application/json"); return; }
                if (y == "TietKiem") nh.Add(make_shared<TaiKhoanTietKiem>(s, t, d, 0.05, 12, p));
                else if (y == "TinDung") nh.Add(make_shared<TaiKhoanTinDung>(s, t, 0, d, p));
                else nh.Add(make_shared<TaiKhoanThanhToan>(s, t, d, p));
                res.set_content("{\"status\":\"success\"}", "application/json");
            });

            handle("/api/admin/delete", [this](const httplib::Request& req, httplib::Response& res) {
                nh.XoaTK(param(req, "stk"));
                res.set_content("{\"status\":\"success\"}", "application/json");
            });

            handle("/api/admin/upgrade", [this](const httplib::Request& req, httplib::Response& res) {
                auto tk = nh.Tim(param(req, "stk"));
                if (tk) { tk->SetHang(param(req, "hang")); nh.Save(); res.set_content("{\"status\":\"success\"}", "application/json"); }
                else res.set_content("{\"status\":\"error\"}", "application/json");
            });

            handle("/api/admin/interest", [this](const httplib::Request&, httplib::Response& res) {
                nh.TinhLaiHeThong();
                res.set_content("{\"status\":\"success\"}", "application/json");
            });

            handle("/api/admin/system_info", [this](const httplib::Request&, httplib::Response& res) {
                res.set_content("{\"lastInterest\":\"" + nh.GetThoiGianTinhLai() + "\"}", "application/json");
            });
        }

        void Listen(const string& host, int port) {
            cout << "OOP BANK ONLINE: http://" << host << ":" << port << endl;
            svr.listen(host.c_str(), port);
        }
    };
}
