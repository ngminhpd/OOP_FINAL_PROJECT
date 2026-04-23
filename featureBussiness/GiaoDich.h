#pragma once
#include <string>
#include <ctime>
#include <iomanip>
#include <sstream>

namespace BankManagementSystem {
namespace Business {

struct GiaoDich {
    std::string thoiGian;
    std::string loai;
    double soTien;
    std::string noiDung;

    GiaoDich(std::string l, double st, std::string nd) : loai(l), soTien(st), noiDung(nd) {
        std::time_t t = std::time(nullptr);
        std::tm* now = std::localtime(&t);
        char buf[80];
        std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", now);
        thoiGian = std::string(buf);
    }

    std::string ToString() const {
        std::stringstream ss;
        ss << "[" << thoiGian << "] " << loai << ": " 
           << (soTien > 0 ? "+" : "") << soTien << " VND | ND: " << noiDung;
        return ss.str();
    }
};

}
}
