#pragma once
#include <string>

namespace BankManagementSystem {
namespace Models {

class TaiKhoan {
protected:
    std::string SoTaiKhoan;
    std::string TenKhachHang;
    double SoDu;

public:
    TaiKhoan(std::string soTK, std::string tenKH, double soDu);
    virtual ~TaiKhoan();

    virtual void NapTien(double soTien);

    virtual bool RutTien(double soTien) = 0;
    virtual double TinhLai() = 0;
    virtual double PhiDuyTri() = 0;

    virtual std::string HienThiThongTin() const;

    std::string GetSoTaiKhoan() const;
    std::string GetTenKhachHang() const;
    double GetSoDu() const;
    void SetTenKhachHang(std::string ten);
};

class TaiKhoanThanhToan : public TaiKhoan {
private:
    const double PHI_DUY_TRI = 11000.0;

public:
    TaiKhoanThanhToan(std::string, std::string, double);

    bool RutTien(double) override;
    double TinhLai() override;
    double PhiDuyTri() override;
    std::string HienThiThongTin() const override;
};

class TaiKhoanTietKiem : public TaiKhoan {
private:
    double LaiSuat;
    int KyHan;

public:
    TaiKhoanTietKiem(std::string, std::string, double, double, int);

    double GetLaiSuat() const;
    int GetKyHan() const;

    bool RutTien(double) override;
    double TinhLai() override;
    double PhiDuyTri() override;
    std::string HienThiThongTin() const override;
};

class TaiKhoanTinDung : public TaiKhoan {
private:
    double HanMuc;
    const double PHI = 50000.0;

public:
    TaiKhoanTinDung(std::string, std::string, double, double);

    double GetHanMuc() const;

    bool RutTien(double) override;
    double TinhLai() override;
    double PhiDuyTri() override;
    std::string HienThiThongTin() const override;
};

}
}
