#pragma once
#include <string>
#include <vector>

namespace Bank {
namespace Models {
    using namespace std;

    class TaiKhoan {
    protected:
        string SoTaiKhoan;
        string TenKhachHang;
        string MaPIN;
        double SoDu;
        bool DaKhoa;

    public:
        TaiKhoan(string soTK, string tenKH, double soDu, string pin = "1234", bool khoa = false);
        virtual ~TaiKhoan();

        virtual void NapTien(double soTien);
        virtual bool RutTien(double soTien) = 0;
        virtual double TinhLai() = 0;
        virtual double PhiDuyTri() = 0;

        virtual string HienThiThongTin() const;

        string GetSoTaiKhoan() const { return SoTaiKhoan; }
        string GetTenKhachHang() const { return TenKhachHang; }
        string GetMaPIN() const { return MaPIN; }
        bool IsLocked() const { return DaKhoa; }
        void SetLocked(bool status) { DaKhoa = status; }
        double GetSoDu() const { return SoDu; }
        void SetTenKhachHang(string ten) { TenKhachHang = ten; }
        void SetMaPIN(string pin) { MaPIN = pin; }
    };

    class TaiKhoanThanhToan : public TaiKhoan {
    private:
        const double PHI_DUY_TRI = 11000.0;
    public:
        TaiKhoanThanhToan(string stk, string ten, double du, string pin = "1234", bool khoa = false);
        bool RutTien(double amt) override;
        double TinhLai() override;
        double PhiDuyTri() override;
        string HienThiThongTin() const override;
    };

    class TaiKhoanTietKiem : public TaiKhoan {
    private:
        double LaiSuat;
        int KyHan;
    public:
        TaiKhoanTietKiem(string stk, string ten, double du, double ls, int kh, string pin = "1234");
        double GetLaiSuat() const { return LaiSuat; }
        int GetKyHan() const { return KyHan; }
        bool RutTien(double amt) override;
        double TinhLai() override;
        double PhiDuyTri() override;
        string HienThiThongTin() const override;
    };

    class TaiKhoanTinDung : public TaiKhoan {
    private:
        double HanMuc;
        const double PHI = 50000.0;
    public:
        TaiKhoanTinDung(string stk, string ten, double du, double hm, string pin = "1234");
        double GetHanMuc() const { return HanMuc; }
        bool RutTien(double amt) override;
        double TinhLai() override;
        double PhiDuyTri() override;
        string HienThiThongTin() const override;
    };
}
}
