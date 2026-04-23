#include "KhachHang.h"
#include <sstream>

namespace BankManagementSystem {
namespace Models {

KhachHang::KhachHang(std::string ma, std::string ten, std::string sdt, std::string dc)
    : MaKH(ma), HoTen(ten), SoDienThoai(sdt), DiaChi(dc) {}

std::string KhachHang::GetMaKH() const { return MaKH; }
std::string KhachHang::GetHoTen() const { return HoTen; }
std::string KhachHang::GetSoDienThoai() const { return SoDienThoai; }
std::string KhachHang::GetDiaChi() const { return DiaChi; }

std::string KhachHang::ToCSV() const {
    return MaKH + ";" + HoTen + ";" + SoDienThoai + ";" + DiaChi;
}

KhachHang KhachHang::FromCSV(const std::string& line) {
    std::stringstream ss(line);
    std::string ma, ten, sdt, dc;
    std::getline(ss, ma, ';');
    std::getline(ss, ten, ';');
    std::getline(ss, sdt, ';');
    std::getline(ss, dc, ';');
    return KhachHang(ma, ten, sdt, dc);
}

}
}
