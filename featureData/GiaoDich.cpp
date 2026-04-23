#include "GiaoDich.h"
#include <sstream>

namespace BankManagementSystem {
namespace Models {

GiaoDich::GiaoDich(std::string ma, std::string stk, std::string loai, double tien, std::string ngay)
    : MaGD(ma), SoTaiKhoan(stk), LoaiGD(loai), SoTien(tien), NgayGiaoDich(ngay) {}

std::string GiaoDich::GetMaGD() const { return MaGD; }
std::string GiaoDich::GetSoTaiKhoan() const { return SoTaiKhoan; }
std::string GiaoDich::GetLoaiGD() const { return LoaiGD; }
double GiaoDich::GetSoTien() const { return SoTien; }
std::string GiaoDich::GetNgayGiaoDich() const { return NgayGiaoDich; }

std::string GiaoDich::ToCSV() const {
    return MaGD + ";" + SoTaiKhoan + ";" + LoaiGD + ";" + std::to_string(SoTien) + ";" + NgayGiaoDich;
}

GiaoDich GiaoDich::FromCSV(const std::string& line) {
    std::stringstream ss(line);
    std::string ma, stk, loai, tien, ngay;
    std::getline(ss, ma, ';');
    std::getline(ss, stk, ';');
    std::getline(ss, loai, ';');
    std::getline(ss, tien, ';');
    std::getline(ss, ngay, ';');
    return GiaoDich(ma, stk, loai, std::stod(tien + "0"), ngay);
}

}
}
