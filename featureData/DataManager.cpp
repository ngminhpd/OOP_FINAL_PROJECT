#include "DataManager.h"
#include <fstream>
#include <sstream>
#include <typeinfo>

namespace BankManagementSystem {
namespace Data {

void DataManager::SaveKhachHang(const std::vector<KhachHang>& danhSach, const std::string& filename) {
    std::ofstream outFile(filename);
    for (const auto& kh : danhSach) outFile << kh.ToCSV() << "\n";
}

std::vector<KhachHang> DataManager::LoadKhachHang(const std::string& filename) {
    std::vector<KhachHang> danhSach;
    std::ifstream inFile(filename);
    std::string line;
    while (std::getline(inFile, line)) { if (!line.empty()) danhSach.push_back(KhachHang::FromCSV(line)); }
    return danhSach;
}

void DataManager::SaveTaiKhoan(const std::vector<TaiKhoan*>& danhSach, const std::string& filename) {
    std::ofstream outFile(filename);
    for (auto tk : danhSach) {
        std::string type;
        std::string extraData = "";
        if (dynamic_cast<TaiKhoanThanhToan*>(tk)) type = "ThanhToan";
        else if (auto tkTK = dynamic_cast<TaiKhoanTietKiem*>(tk)) { type = "TietKiem"; extraData = ";" + std::to_string(tkTK->GetLaiSuat()) + ";" + std::to_string(tkTK->GetKyHan()); }
        else if (auto tkTD = dynamic_cast<TaiKhoanTinDung*>(tk)) { type = "TinDung"; extraData = ";" + std::to_string(tkTD->GetHanMuc()); }
        
        // Cấu trúc: Type;STK;Ten;PIN;SoDu;DaKhoa;Extra
        outFile << type << ";" << tk->GetSoTaiKhoan() << ";" << tk->GetTenKhachHang() << ";" << tk->GetMaPIN() << ";" << tk->GetSoDu() << ";" << (tk->IsLocked() ? "1" : "0") << extraData << "\n";
    }
}

std::vector<TaiKhoan*> DataManager::LoadTaiKhoan(const std::string& filename) {
    std::vector<TaiKhoan*> danhSach;
    std::ifstream inFile(filename);
    std::string line;
    while (std::getline(inFile, line)) {
        if (line.empty()) continue;
        std::stringstream ss(line);
        std::string type, stk, ten, pin, sodu_s, khoa_s;
        std::getline(ss, type, ';'); std::getline(ss, stk, ';'); std::getline(ss, ten, ';');
        std::getline(ss, pin, ';'); std::getline(ss, sodu_s, ';'); std::getline(ss, khoa_s, ';');

        double sodu = std::stod(sodu_s);
        bool isLocked = (khoa_s == "1");

        if (type == "ThanhToan") danhSach.push_back(new TaiKhoanThanhToan(stk, ten, sodu, pin, isLocked));
        else if (type == "TietKiem") {
            std::string ls, kh;
            std::getline(ss, ls, ';'); std::getline(ss, kh, ';');
            danhSach.push_back(new TaiKhoanTietKiem(stk, ten, sodu, std::stod(ls), std::stoi(kh), pin));
            if(isLocked) danhSach.back()->SetLocked(true);
        } else if (type == "TinDung") {
            std::string hm; std::getline(ss, hm, ';');
            danhSach.push_back(new TaiKhoanTinDung(stk, ten, sodu, std::stod(hm), pin));
            if(isLocked) danhSach.back()->SetLocked(true);
        }
    }
    return danhSach;
}

void DataManager::SaveGiaoDich(const std::vector<GiaoDich>& ds, const std::string& f) {
    std::ofstream o(f);
    for (const auto& gd : ds) o << gd.ToCSV() << "\n";
}

std::vector<GiaoDich> DataManager::LoadGiaoDich(const std::string& f) {
    std::vector<GiaoDich> ds; std::ifstream i(f); std::string l;
    while (std::getline(i, l)) { if (!l.empty()) ds.push_back(GiaoDich::FromCSV(l)); }
    return ds;
}

}
}
