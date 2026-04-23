#include "TaiKhoan.h"
#include <sstream>
#include <iomanip>

using namespace BankManagementSystem::Models;

std::string FormatTien(long long n) {
    std::string s = std::to_string(n);
    int pos = s.length() - 3;
    while (pos > 0) { s.insert(pos, ","); pos -= 3; }
    return s;
}

// ===== TaiKhoan =====
TaiKhoan::TaiKhoan(std::string soTK, std::string tenKH, double soDu, std::string pin, bool khoa)
    : SoTaiKhoan(soTK), TenKhachHang(tenKH), SoDu(soDu), MaPIN(pin), DaKhoa(khoa) {}

TaiKhoan::~TaiKhoan() {}

void TaiKhoan::NapTien(double soTien) { if (soTien > 0) SoDu += soTien; }

std::string TaiKhoan::HienThiThongTin() const {
    std::stringstream ss;
    ss << "STK: " << SoTaiKhoan << " | KH: " << TenKhachHang << " | PIN: " << MaPIN << " | So du: " << FormatTien((long long)SoDu) << " VND" << (DaKhoa ? " [KHOA]" : " [MO]");
    return ss.str();
}

std::string TaiKhoan::GetSoTaiKhoan() const { return SoTaiKhoan; }
std::string TaiKhoan::GetTenKhachHang() const { return TenKhachHang; }
std::string TaiKhoan::GetMaPIN() const { return MaPIN; }
void TaiKhoan::SetMaPIN(std::string pin) { MaPIN = pin; }
double TaiKhoan::GetSoDu() const { return SoDu; }
void TaiKhoan::SetTenKhachHang(std::string ten) { TenKhachHang = ten; }

// ===== TaiKhoanThanhToan =====
TaiKhoanThanhToan::TaiKhoanThanhToan(std::string a, std::string b, double c, std::string p, bool k) : TaiKhoan(a, b, c, p, k) {}
bool TaiKhoanThanhToan::RutTien(double s) { if (!DaKhoa && s > 0 && SoDu - s >= 50000) { SoDu -= s; return true; } return false; }
double TaiKhoanThanhToan::TinhLai() { return SoDu * 0.001; }
double TaiKhoanThanhToan::PhiDuyTri() { return PHI_DUY_TRI; }
std::string TaiKhoanThanhToan::HienThiThongTin() const { return "[ThanhToan] " + TaiKhoan::HienThiThongTin(); }

// ===== TaiKhoanTietKiem =====
TaiKhoanTietKiem::TaiKhoanTietKiem(std::string a, std::string b, double c, double ls, int kh, std::string p) : TaiKhoan(a, b, c, p, false), LaiSuat(ls), KyHan(kh) {}
double TaiKhoanTietKiem::GetLaiSuat() const { return LaiSuat; }
int TaiKhoanTietKiem::GetKyHan() const { return KyHan; }
bool TaiKhoanTietKiem::RutTien(double s) { if (!DaKhoa && s == SoDu) { SoDu = 0; return true; } return false; }
double TaiKhoanTietKiem::TinhLai() { return SoDu * LaiSuat; }
double TaiKhoanTietKiem::PhiDuyTri() { return 0; }
std::string TaiKhoanTietKiem::HienThiThongTin() const { return "[TietKiem] " + TaiKhoan::HienThiThongTin(); }

// ===== TaiKhoanTinDung =====
TaiKhoanTinDung::TaiKhoanTinDung(std::string a, std::string b, double c, double hm, std::string p) : TaiKhoan(a, b, c, p, false), HanMuc(hm) {}
double TaiKhoanTinDung::GetHanMuc() const { return HanMuc; }
bool TaiKhoanTinDung::RutTien(double s) { if (!DaKhoa && s > 0 && SoDu + s <= HanMuc) { SoDu += s; return true; } return false; }
double TaiKhoanTinDung::TinhLai() { return SoDu > 0 ? SoDu * 0.2 : 0; }
double TaiKhoanTinDung::PhiDuyTri() { return PHI; }
std::string TaiKhoanTinDung::HienThiThongTin() const { return "[TinDung] " + TaiKhoan::HienThiThongTin(); }
