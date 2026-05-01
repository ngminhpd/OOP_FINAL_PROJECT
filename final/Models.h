#pragma once
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <memory>

namespace BankSystem {
    using namespace std;

    inline string GetCurrentTimeStr() {
        time_t t = time(nullptr);
        tm* now = localtime(&t);
        char buf[80];
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", now);
        return string(buf);
    }

    struct GiaoDich {
        string thoiGian, loai, noiDung;
        double soTien;

        GiaoDich(string l, double st, string nd) : loai(l), soTien(st), noiDung(nd) {
            thoiGian = GetCurrentTimeStr();
        }
        GiaoDich(string tg, string l, double st, string nd) : thoiGian(tg), loai(l), soTien(st), noiDung(nd) {}
    };

    struct YeuCau {
        string type, stk, ten, val, hang, pin, thoiGian;
        YeuCau(string tp, string s, string t, string v, string h, string p = "")
            : type(tp), stk(s), ten(t), val(v), hang(h), pin(p) {
            thoiGian = GetCurrentTimeStr();
        }
        YeuCau(string tp, string s, string t, string v, string h, string p, string tg)
            : type(tp), stk(s), ten(t), val(v), hang(h), pin(p), thoiGian(tg) {}
    };

    class ILoanable {
    public:
        virtual ~ILoanable() {}
        virtual double GetSoDuVay() const = 0;
        virtual void SetSoDuVay(double d) = 0;
        virtual void NapTienVay(double amt) = 0;
    };

    class TaiKhoan {
    protected:
        string SoTaiKhoan, TenKhachHang, MaPIN, Hang;
        double SoDu;
        bool DaKhoa;
        double SoDuVay = 0; 

    public:
        TaiKhoan(string stk, string ten, double du, string pin = "1234", bool khoa = false, string hang = "Thành viên", double dv = 0)
            : SoTaiKhoan(stk), TenKhachHang(ten), SoDu(du), MaPIN(pin), DaKhoa(khoa), Hang(hang), SoDuVay(dv) {}
        virtual ~TaiKhoan() {}

        virtual void NapTien(double amt) { if (amt > 0) SoDu += amt; }
        virtual bool RutTien(double amt) = 0;
        virtual double TinhLai() = 0;

        string GetSoTaiKhoan() const { return SoTaiKhoan; }
        string GetTenKhachHang() const { return TenKhachHang; }
        string GetMaPIN() const { return MaPIN; }
        void SetMaPIN(string pin) { MaPIN = pin; }
        bool IsLocked() const { return DaKhoa; }
        void SetLocked(bool s) { DaKhoa = s; }
        double GetSoDu() const { return SoDu; }
        void SetSoDu(double d) { SoDu = d; }
        string GetHang() const { return Hang; }
        void SetHang(string h) { Hang = h; }
        
        double GetSoDuVay() const { return SoDuVay; }
        void SetSoDuVay(double d) { SoDuVay = d; }
        void NapTienVay(double amt) { if (amt > 0) SoDuVay += amt; }

        virtual string GetLoai() const = 0;
        virtual double GetBalanceDisplay() const { return SoDu; }
        
        virtual double GetHanMucTinDung() const { return 0; }
        virtual void SetHanMucTinDung(double) {}
        virtual double GetNoTinDung() const { return 0; }
        virtual void SetNoTinDung(double) {}
        virtual bool RutTienTinDung(double) { return false; }
        virtual void NapTienTinDung(double) {}
    };

    class TaiKhoanThanhToan : public TaiKhoan {
    public:
        using TaiKhoan::TaiKhoan;
        bool RutTien(double amt) override {
            if (DaKhoa || amt <= 0) return false;
            if (SoDu + SoDuVay - amt < 50000) return false;
            if (SoDu >= amt) { SoDu -= amt; } else { double rem = amt - SoDu; SoDu = 0; SoDuVay -= rem; }
            return true;
        }
        double TinhLai() override { return SoDu * 0.001; }
        string GetLoai() const override { return "ThanhToan"; }
    };

    class TaiKhoanTietKiem : public TaiKhoan {
        double LaiSuat;
        int KyHan;
    public:
        TaiKhoanTietKiem(string stk, string ten, double du, double ls, int kh, string pin = "1234", bool k = false, string h = "Thành viên", double dv = 0)
            : TaiKhoan(stk, ten, du, pin, k, h, dv), LaiSuat(ls), KyHan(kh) {}
        
        bool RutTien(double amt) override {
            if (DaKhoa || amt <= 0) return false;
            if (SoDu + SoDuVay < amt) return false;
            if (SoDu >= amt) { SoDu -= amt; } else { double rem = amt - SoDu; SoDu = 0; SoDuVay -= rem; }
            return true;
        }
        double TinhLai() override { return SoDu * LaiSuat; }
        string GetLoai() const override { return "TietKiem"; }
        double GetLaiSuat() const { return LaiSuat; }
        int GetKyHan() const { return KyHan; }
    };

    class TaiKhoanTinDung : public TaiKhoan {
        double HanMuc;
        double NoTinDung;
    public:
        TaiKhoanTinDung(string stk, string ten, double du, double hm, string pin = "1234", bool k = false, string h = "Thành viên", double dv = 0, double no = 0)
            : TaiKhoan(stk, ten, du, pin, k, h, dv), HanMuc(hm), NoTinDung(no) {}

        bool RutTien(double amt) override {
            // Logic for "main balance" withdrawal if any
            if (DaKhoa || amt <= 0 || SoDu < amt) return false;
            SoDu -= amt; return true;
        }

        bool RutTienTinDung(double amt) override {
            if (DaKhoa || HanMuc <= 0 || amt <= 0) return false;
            if (NoTinDung + amt > HanMuc) return false;
            NoTinDung += amt; return true;
        }

        void NapTienTinDung(double amt) override {
            if (amt > 0) { if (NoTinDung >= amt) NoTinDung -= amt; else NoTinDung = 0; }
        }

        double TinhLai() override { return NoTinDung > 0 ? NoTinDung * 0.2 : 0; }
        string GetLoai() const override { return "TinDung"; }
        double GetBalanceDisplay() const override { return HanMuc - NoTinDung; }
        
        double GetHanMucTinDung() const override { return HanMuc; }
        void SetHanMucTinDung(double h) override { HanMuc = h; }
        double GetNoTinDung() const override { return NoTinDung; }
        void SetNoTinDung(double n) override { NoTinDung = n; }
    };

    struct KhoanVay {
        string stk; double soTien; long long hanTra; double laiSuat;
        KhoanVay(string s, double st, long long ht, double ls) : stk(s), soTien(st), hanTra(ht), laiSuat(ls) {}
    };

    struct LichSuVay {
        string stk, ten, thoiGian, hanVay, trangThai;
        double soTien, laiSuat;
        LichSuVay(string s, string t, string tg, string hv, double st, double ls, string tt)
            : stk(s), ten(t), thoiGian(tg), hanVay(hv), soTien(st), laiSuat(ls), trangThai(tt) {}
    };

}
