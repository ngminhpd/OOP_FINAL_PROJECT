#pragma once
#include <string>

namespace BankManagementSystem {
namespace Models {

class GiaoDich {
private:
    std::string MaGD;
    std::string SoTaiKhoan;
    std::string LoaiGD; // "Nap" or "Rut"
    double SoTien;
    std::string NgayGiaoDich;

public:
    GiaoDich(std::string ma, std::string stk, std::string loai, double tien, std::string ngay);
    
    std::string GetMaGD() const;
    std::string GetSoTaiKhoan() const;
    std::string GetLoaiGD() const;
    double GetSoTien() const;
    std::string GetNgayGiaoDich() const;

    std::string ToCSV() const;
    static GiaoDich FromCSV(const std::string& line);
};

}
}