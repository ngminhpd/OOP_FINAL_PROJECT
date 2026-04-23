#pragma once
#include "../featuremodel/TaiKhoan.h"
#include "GiaoDich.h"
#include "Exceptions.h"
#include <vector>
#include <string>
#include <memory>
#include <map>

namespace BankManagementSystem {
namespace Business {

class NganHang {
private:
    std::vector<std::shared_ptr<Models::TaiKhoan>> danhSachTaiKhoan;
    
    // Cac map quan ly rieng de khong can sua lop Model
    std::map<std::string, std::string> dsPin;
    std::map<std::string, bool> dsKhoa; // true la bi khoa
    std::map<std::string, std::vector<GiaoDich>> dsLichSu;

    std::shared_ptr<Models::TaiKhoan> TimTaiKhoan(std::string soTK);
    void GhiLog(std::string soTK, std::string loai, double soTien, std::string nd);
    void KiemTraBaoMat(std::string soTK, std::string pin);

public:
    NganHang();
    ~NganHang();

    // Chức năng quản lý
    void ThemTaiKhoan(Models::TaiKhoan* tk, std::string pin);
    bool XoaTaiKhoan(std::string soTK);
    bool SuaThongTin(std::string soTK, std::string tenMoi);
    void KhoaTaiKhoan(std::string soTK, bool trangThai);

    // Chức năng giao dịch (Su dung Exception)
    void NapTien(std::string soTK, double soTien);
    void RutTien(std::string soTK, double soTien, std::string pin);
    void ChuyenTien(std::string soTK_Nguon, std::string soTK_Dich, double soTien, std::string pin);

    // Tra cuu
    void LietKeDanhSach() const;
    void InSaoKe(std::string soTK);
    void NapLichSu(std::string stk, std::string loai, double tien, std::string nd);
    
    // Luu file
    const std::map<std::string, std::vector<GiaoDich>>& GetLichSu() const { return dsLichSu; }
    
    // Tính lãi
    void TinhLaiVaCapNhat();

    // Getter phục vụ lưu file (Thống nhất logic)
    const std::vector<std::shared_ptr<Models::TaiKhoan>>& GetDanhSach() const { return danhSachTaiKhoan; }

    // File I/O
    void LuuDuLieu(std::string filename);
    void DocDuLieu(std::string filename);
};

}
}
