#include "TaiKhoan.h"
#include <sstream>
#include <iomanip>

using namespace Bank::Models;

string FormatTien(long long n) {
    string s = to_string(n);
    int pos = s.length() - 3;
    while (pos > 0) { s.insert(pos, ","); pos -= 3; }
    return s;
}

// ===== TaiKhoan =====
TaiKhoan::TaiKhoan(string soTK, string tenKH, double soDu, string pin, bool khoa)
    : SoTaiKhoan(soTK), TenKhachHang(tenKH), SoDu(soDu), MaPIN(pin), DaKhoa(khoa) {}

TaiKhoan::~TaiKhoan() {}

void TaiKhoan::NapTien(double soTien) { if (soTien > 0) SoDu += soTien; }

string TaiKhoan::HienThiThongTin() const {
    stringstream ss;
    ss << "STK: " << SoTaiKhoan << " | KH: " << TenKhachHang << " | PIN: " << MaPIN << " | So du: " << FormatTien((long long)SoDu) << " VND" << (DaKhoa ? " [KHOA]" : " [MO]");
    return ss.str();
}

// ===== TaiKhoanThanhToan =====
TaiKhoanThanhToan::TaiKhoanThanhToan(string a, string b, double c, string p, bool k) : TaiKhoan(a, b, c, p, k) {}
bool TaiKhoanThanhToan::RutTien(double s) { if (!DaKhoa && s > 0 && SoDu - s >= 50000) { SoDu -= s; return true; } return false; }
double TaiKhoanThanhToan::TinhLai() { return SoDu * 0.001; }
double TaiKhoanThanhToan::PhiDuyTri() { return PHI_DUY_TRI; }
string TaiKhoanThanhToan::HienThiThongTin() const { return "[ThanhToan] " + TaiKhoan::HienThiThongTin(); }

// ===== TaiKhoanTietKiem =====
TaiKhoanTietKiem::TaiKhoanTietKiem(string a, string b, double c, double ls, int kh, string p) : TaiKhoan(a, b, c, p, false), LaiSuat(ls), KyHan(kh) {}
bool TaiKhoanTietKiem::RutTien(double s) { if (!DaKhoa && s == SoDu) { SoDu = 0; return true; } return false; }
double TaiKhoanTietKiem::TinhLai() { return SoDu * LaiSuat; }
double TaiKhoanTietKiem::PhiDuyTri() { return 0; }
string TaiKhoanTietKiem::HienThiThongTin() const { return "[TietKiem] " + TaiKhoan::HienThiThongTin(); }

// ===== TaiKhoanTinDung =====
TaiKhoanTinDung::TaiKhoanTinDung(string a, string b, double c, double hm, string p) : TaiKhoan(a, b, c, p, false), HanMuc(hm) {}
bool TaiKhoanTinDung::RutTien(double s) { if (!DaKhoa && s > 0 && SoDu + s <= HanMuc) { SoDu += s; return true; } return false; }
double TaiKhoanTinDung::TinhLai() { return SoDu > 0 ? SoDu * 0.2 : 0; }
double TaiKhoanTinDung::PhiDuyTri() { return PHI; }
string TaiKhoanTinDung::HienThiThongTin() const { return "[TinDung] " + TaiKhoan::HienThiThongTin(); }
