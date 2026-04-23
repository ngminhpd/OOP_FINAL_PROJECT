#include "NganHang.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

namespace BankManagementSystem {
namespace Business {

NganHang::NganHang() {}

NganHang::~NganHang() {
    // Smart pointers se tu dong giai phong bo nho
}

std::shared_ptr<Models::TaiKhoan> NganHang::TimTaiKhoan(std::string soTK) {
    for (auto& tk : danhSachTaiKhoan) {
        if (tk->GetSoTaiKhoan() == soTK) {
            return tk;
        }
    }
    return nullptr;
}

void NganHang::GhiLog(std::string soTK, std::string loai, double soTien, std::string nd) {
    dsLichSu[soTK].emplace_back(loai, soTien, nd);
}

void NganHang::KiemTraBaoMat(std::string soTK, std::string pin) {
    if (dsKhoa[soTK]) throw TaiKhoanKhoaException();
    if (dsPin[soTK] != pin) throw SaiPinException();
}

void NganHang::ThemTaiKhoan(Models::TaiKhoan* tk, std::string pin) {
    if (tk != nullptr && TimTaiKhoan(tk->GetSoTaiKhoan()) == nullptr) {
        std::shared_ptr<Models::TaiKhoan> ptr(tk);
        danhSachTaiKhoan.push_back(ptr);
        dsPin[ptr->GetSoTaiKhoan()] = pin;
        dsKhoa[ptr->GetSoTaiKhoan()] = false;
        GhiLog(ptr->GetSoTaiKhoan(), "KHOI_TAO", ptr->GetSoDu(), "Mo tai khoan moi");
    } else {
        std::cout << "Loi: Tai khoan da ton tai hoac du lieu khong hop le.\n";
    }
}

bool NganHang::XoaTaiKhoan(std::string soTK) {
    for (auto it = danhSachTaiKhoan.begin(); it != danhSachTaiKhoan.end(); ++it) {
        if ((*it)->GetSoTaiKhoan() == soTK) {
            danhSachTaiKhoan.erase(it);
            dsPin.erase(soTK);
            dsKhoa.erase(soTK);
            dsLichSu.erase(soTK);
            return true;
        }
    }
    return false;
}

bool NganHang::SuaThongTin(std::string soTK, std::string tenMoi) {
    auto tk = TimTaiKhoan(soTK);
    if (tk) {
        tk->SetTenKhachHang(tenMoi);
        return true;
    }
    return false;
}

void NganHang::KhoaTaiKhoan(std::string soTK, bool trangThai) {
    if (dsPin.count(soTK)) {
        dsKhoa[soTK] = trangThai;
        std::cout << (trangThai ? "Da khoa " : "Da mo khoa ") << "tai khoan " << soTK << "\n";
    }
}

void NganHang::NapTien(std::string soTK, double soTien) {
    auto tk = TimTaiKhoan(soTK);
    if (!tk) throw TaiKhoanKhongTonTaiException();
    if (dsKhoa[soTK]) throw TaiKhoanKhoaException();

    tk->NapTien(soTien);
    GhiLog(soTK, "NAP_TIEN", soTien, "Nap tien tai quay");
}

void NganHang::RutTien(std::string soTK, double soTien, std::string pin) {
    auto tk = TimTaiKhoan(soTK);
    if (!tk) throw TaiKhoanKhongTonTaiException();
    
    KiemTraBaoMat(soTK, pin);

    if (tk->RutTien(soTien)) {
        GhiLog(soTK, "RUT_TIEN", -soTien, "Rut tien mat");
    } else {
        throw ThieuSoDuException();
    }
}

void NganHang::ChuyenTien(std::string soTK_Nguon, std::string soTK_Dich, double soTien, std::string pin) {
    auto tkNguon = TimTaiKhoan(soTK_Nguon);
    auto tkDich = TimTaiKhoan(soTK_Dich);

    if (!tkNguon || !tkDich) throw TaiKhoanKhongTonTaiException();
    
    KiemTraBaoMat(soTK_Nguon, pin);
    if (dsKhoa[soTK_Dich]) throw BankException("Loi: Tai khoan dich dang bi khoa.");

    if (tkNguon->RutTien(soTien)) {
        tkDich->NapTien(soTien);
        GhiLog(soTK_Nguon, "CHUYEN_DI", -soTien, "Chuyen den " + soTK_Dich);
        GhiLog(soTK_Dich, "NHAN_TIEN", soTien, "Nhan tu " + soTK_Nguon);
    } else {
        throw ThieuSoDuException();
    }
}

std::string NganHang::LietKeDanhSach() const {
    std::string res = "--- DANH SACH TAI KHOAN ---\n";
    for (const auto& tk : danhSachTaiKhoan) {
        res += tk->HienThiThongTin() 
            + (dsKhoa.at(tk->GetSoTaiKhoan()) ? " [DANG KHOA]" : " [ACTIVE]") + "\n";
    }
    return res;
}

void NganHang::NapLichSu(std::string stk, std::string loai, double tien, std::string nd) {
    dsLichSu[stk].emplace_back(loai, tien, nd);
}

void NganHang::InSaoKe(std::string soTK) {
    if (dsLichSu.count(soTK)) {
        std::cout << "--- SAO KE TAI KHOAN " << soTK << " ---\n";
        for (const auto& gd : dsLichSu[soTK]) {
            std::cout << gd.ToString() << "\n";
        }
    } else {
        std::cout << "Khong co lich su cho tai khoan nay.\n";
    }
}

void NganHang::TinhLaiVaCapNhat() {
    for (auto& tk : danhSachTaiKhoan) {
        double lai = tk->TinhLai();
        if (lai > 0) {
            tk->NapTien(lai);
            GhiLog(tk->GetSoTaiKhoan(), "TINH_LAI", lai, "Lai suat dinh ky");
        }
    }
}

void NganHang::LuuDuLieu(std::string filename) {
    std::ofstream f(filename);
    if (f.is_open()) {
        for (const auto& tk : danhSachTaiKhoan) {
            f << tk->GetSoTaiKhoan() << "|" << dsPin[tk->GetSoTaiKhoan()] << "|" << dsKhoa[tk->GetSoTaiKhoan()] << "\n";
        }
        f.close();
        std::cout << "Da luu thong tin bao mat vao " << filename << "\n";
    }
}

void NganHang::DocDuLieu(std::string filename) {
    std::ifstream f(filename);
    std::string line;
    if (f.is_open()) {
        while (std::getline(f, line)) {
            std::stringstream ss(line);
            std::string stk, pin, khoa;
            std::getline(ss, stk, '|');
            std::getline(ss, pin, '|');
            std::getline(ss, khoa, '|');
            
            if (!stk.empty()) {
                dsPin[stk] = pin;
                dsKhoa[stk] = (khoa == "1");
            }
        }
        f.close();
        std::cout << "Da khoi phuc thong tin bao mat tu " << filename << "\n";
    }
}

}
}
