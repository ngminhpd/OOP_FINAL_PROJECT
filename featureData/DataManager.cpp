#include "DataManager.h"
#include <fstream>
#include <sstream>
#include <typeinfo>

namespace BankManagementSystem {
namespace Data {

void DataManager::SaveKhachHang(const std::vector<KhachHang>& danhSach, const std::string& filename) {
    std::ofstream outFile(filename);
    for (const auto& kh : danhSach) {
        outFile << kh.ToCSV() << "\n";
    }
}

std::vector<KhachHang> DataManager::LoadKhachHang(const std::string& filename) {
    std::vector<KhachHang> danhSach;
    std::ifstream inFile(filename);
    std::string line;
    while (std::getline(inFile, line)) {
        if (!line.empty()) {
            danhSach.push_back(KhachHang::FromCSV(line));
        }
    }
    return danhSach;
}

void DataManager::SaveTaiKhoan(const std::vector<TaiKhoan*>& danhSach, const std::string& filename) {
    std::ofstream outFile(filename);
    for (auto tk : danhSach) {
        std::string type;
        std::string extraData = "";

        if (dynamic_cast<TaiKhoanThanhToan*>(tk)) {
            type = "ThanhToan";
        } else if (auto tkTK = dynamic_cast<TaiKhoanTietKiem*>(tk)) {
            type = "TietKiem";
            extraData = "," + std::to_string(tkTK->GetLaiSuat()) + "," + std::to_string(tkTK->GetKyHan());
        } else if (auto tkTD = dynamic_cast<TaiKhoanTinDung*>(tk)) {
            type = "TinDung";
            extraData = "," + std::to_string(tkTD->GetHanMuc());
        }

        outFile << type << "," << tk->GetSoTaiKhoan() << "," << tk->GetTenKhachHang() << "," << tk->GetSoDu() << extraData << "\n";
    }
}

std::vector<TaiKhoan*> DataManager::LoadTaiKhoan(const std::string& filename) {
    std::vector<TaiKhoan*> danhSach;
    std::ifstream inFile(filename);
    std::string line;
    while (std::getline(inFile, line)) {
        if (line.empty()) continue;
        std::stringstream ss(line);
        std::string type, stk, ten, sodu_str;
        std::getline(ss, type, ',');
        std::getline(ss, stk, ',');
        std::getline(ss, ten, ',');
        std::getline(ss, sodu_str, ',');

        double sodu = std::stod(sodu_str);

        if (type == "ThanhToan") {
            danhSach.push_back(new TaiKhoanThanhToan(stk, ten, sodu));
        } else if (type == "TietKiem") {
            std::string ls_str, kh_str;
            std::getline(ss, ls_str, ',');
            std::getline(ss, kh_str, ',');
            danhSach.push_back(new TaiKhoanTietKiem(stk, ten, sodu, std::stod(ls_str), std::stoi(kh_str)));
        } else if (type == "TinDung") {
            std::string hm_str;
            std::getline(ss, hm_str, ',');
            danhSach.push_back(new TaiKhoanTinDung(stk, ten, sodu, std::stod(hm_str)));
        }
    }
    return danhSach;
}

void DataManager::SaveGiaoDich(const std::vector<GiaoDich>& danhSach, const std::string& filename) {
    std::ofstream outFile(filename);
    for (const auto& gd : danhSach) {
        outFile << gd.ToCSV() << "\n";
    }
}

std::vector<GiaoDich> DataManager::LoadGiaoDich(const std::string& filename) {
    std::vector<GiaoDich> danhSach;
    std::ifstream inFile(filename);
    std::string line;
    while (std::getline(inFile, line)) {
        if (!line.empty()) {
            danhSach.push_back(GiaoDich::FromCSV(line));
        }
    }
    return danhSach;
}

}
}