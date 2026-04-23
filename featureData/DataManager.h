#pragma once
#include <vector>
#include <string>
#include "../featuremodel/TaiKhoan.h"
#include "KhachHang.h"
#include "GiaoDich.h"

namespace BankManagementSystem {
namespace Data {

using namespace Models;

class DataManager {
public:
    static void SaveKhachHang(const std::vector<KhachHang>& danhSach, const std::string& filename = "customers.txt");
    static std::vector<KhachHang> LoadKhachHang(const std::string& filename = "customers.txt");

    static void SaveTaiKhoan(const std::vector<TaiKhoan*>& danhSach, const std::string& filename = "accounts.txt");
    static std::vector<TaiKhoan*> LoadTaiKhoan(const std::string& filename = "accounts.txt");

    static void SaveGiaoDich(const std::vector<GiaoDich>& danhSach, const std::string& filename = "transactions.txt");
    static std::vector<GiaoDich> LoadGiaoDich(const std::string& filename = "transactions.txt");
};

}
}