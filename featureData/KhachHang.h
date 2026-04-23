#pragma once
#include <string>

namespace BankManagementSystem {
namespace Models {

class KhachHang {
private:
    std::string MaKH;
    std::string HoTen;
    std::string SoDienThoai;
    std::string DiaChi;

public:
    KhachHang(std::string ma, std::string ten, std::string sdt, std::string dc);
    
    std::string GetMaKH() const;
    std::string GetHoTen() const;
    std::string GetSoDienThoai() const;
    std::string GetDiaChi() const;

    std::string ToCSV() const;
    static KhachHang FromCSV(const std::string& line);
};

}
}