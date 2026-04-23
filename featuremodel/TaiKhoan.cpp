#include "TaiKhoan.h"
#include <sstream>
#include <iomanip>

using namespace BankManagementSystem::Models;

// ===== HÀM FORMAT TIỀN (1,000,000) =====
std::string FormatTien(long long n) {
    std::string s = std::to_string(n);
    int pos = s.length() - 3;
    while (pos > 0) {
        s.insert(pos, ",");
        pos -= 3;
    }
    return s;
}

// ===== TaiKhoan =====
TaiKhoan::TaiKhoan(std::string soTK, std::string tenKH, double soDu)
    : SoTaiKhoan(soTK), TenKhachHang(tenKH), SoDu(soDu) {}

TaiKhoan::~TaiKhoan() {}

void TaiKhoan::NapTien(double soTien) {
    if (soTien > 0) SoDu += soTien;
}

std::string TaiKhoan::HienThiThongTin() const {
    std::stringstream ss;
    ss << "STK: " << SoTaiKhoan
       << " | KH: " << TenKhachHang
       << " | So du: " << FormatTien((long long)SoDu) << " VND";
    return ss.str();
}

std::string TaiKhoan::GetSoTaiKhoan() const { return SoTaiKhoan; }
std::string TaiKhoan::GetTenKhachHang() const { return TenKhachHang; }
double TaiKhoan::GetSoDu() const { return SoDu; }

// ===== Thanh Toán =====
TaiKhoanThanhToan::TaiKhoanThanhToan(std::string a, std::string b, double c)
    : TaiKhoan(a, b, c) {}

bool TaiKhoanThanhToan::RutTien(double soTien) {
    if (soTien > 0 && SoDu - soTien >= 50000) {
        SoDu -= soTien;
        return true;
    }
    return false;
}

double TaiKhoanThanhToan::TinhLai() {
    return SoDu * 0.001;
}

double TaiKhoanThanhToan::PhiDuyTri() {
    return PHI_DUY_TRI;
}

std::string TaiKhoanThanhToan::HienThiThongTin() const {
    return "[ThanhToan] " + TaiKhoan::HienThiThongTin();
}

// ===== Tiết Kiệm =====
TaiKhoanTietKiem::TaiKhoanTietKiem(std::string a, std::string b, double c, double ls, int kh)
    : TaiKhoan(a, b, c), LaiSuat(ls), KyHan(kh) {}

double TaiKhoanTietKiem::GetLaiSuat() const { return LaiSuat; }
int TaiKhoanTietKiem::GetKyHan() const { return KyHan; }

bool TaiKhoanTietKiem::RutTien(double soTien) {
    if (soTien == SoDu) {
        SoDu = 0;
        return true;
    }
    return false;
}

double TaiKhoanTietKiem::TinhLai() {
    return SoDu * LaiSuat;
}

double TaiKhoanTietKiem::PhiDuyTri() {
    return 0;
}

std::string TaiKhoanTietKiem::HienThiThongTin() const {
    std::stringstream ss;
    ss << "[TietKiem] STK: " << SoTaiKhoan
       << " | KH: " << TenKhachHang
       << " | So du: " << FormatTien((long long)SoDu) << " VND"
       << " | Lai suat: " << LaiSuat * 100 << "%"
       << " | Ky han: " << KyHan << " thang";
    return ss.str();
}

// ===== Tín Dụng =====
TaiKhoanTinDung::TaiKhoanTinDung(std::string a, std::string b, double c, double hm)
    : TaiKhoan(a, b, c), HanMuc(hm) {}

double TaiKhoanTinDung::GetHanMuc() const { return HanMuc; }

bool TaiKhoanTinDung::RutTien(double soTien) {
    if (soTien > 0 && SoDu + soTien <= HanMuc) {
        SoDu += soTien;
        return true;
    }
    return false;
}

double TaiKhoanTinDung::TinhLai() {
    return SoDu > 0 ? SoDu * 0.2 : 0;
}

double TaiKhoanTinDung::PhiDuyTri() {
    return PHI;
}

std::string TaiKhoanTinDung::HienThiThongTin() const {
    std::stringstream ss;
    ss << "[TinDung] STK: " << SoTaiKhoan
       << " | KH: " << TenKhachHang
       << " | Du no (so tien da su dung): " << FormatTien((long long)SoDu) << " VND"
       << " | Han muc: " << FormatTien((long long)HanMuc) << " VND";
    return ss.str();
}