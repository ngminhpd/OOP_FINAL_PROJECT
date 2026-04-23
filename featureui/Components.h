#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <iomanip>

namespace BankManagementSystem {
namespace UI {

class Components {
public:
    // 1. Textbox: Nhập thông tin
    static std::string TextBox(std::string label) {
        std::string value;
        std::cout << " [?] " << std::left << std::setw(20) << label << ": ";
        // Xóa bộ nhớ đệm trước khi nhập chuỗi
        if (std::cin.peek() == '\n') std::cin.ignore();
        std::getline(std::cin, value);
        return value;
    }

    // 2. Button / Menu: Lựa chọn hành động
    static int Menu(std::vector<std::string> options) {
        std::cout << "\n --- LUA CHON HANH DONG ---\n";
        for (size_t i = 0; i < options.size(); ++i) {
            std::cout << "  " << i + 1 << ". " << options[i] << "\n";
        }
        std::cout << "  0. Quay lai / Thoat\n";
        std::cout << " >> Nhap so: ";
        int choice;
        if (!(std::cin >> choice)) {
            std::cin.clear();
            std::cin.ignore(1000, '\n');
            return -1;
        }
        return choice;
    }

    // 3. DataGridView / Table: Hiển thị bảng dữ liệu chuyên nghiệp
    static void Table(std::vector<std::string> headers, std::vector<std::vector<std::string>> rows) {
        std::cout << "\n" << std::string(90, '=') << "\n";
        for (const auto& h : headers) std::cout << "| " << std::left << std::setw(18) << h;
        std::cout << "|\n" << std::string(90, '-') << "\n";

        if (rows.empty()) {
            std::cout << "| " << std::setw(86) << " (Khong co du lieu) " << "|\n";
        } else {
            for (const auto& row : rows) {
                for (const auto& cell : row) std::cout << "| " << std::left << std::setw(18) << cell;
                std::cout << "|\n";
            }
        }
        std::cout << std::string(90, '=') << "\n";
    }

    // 4. ComboBox: Chọn loại tài khoản
    static int ComboBox(std::string label, std::vector<std::string> items) {
        std::cout << " [v] " << label << ":\n";
        for (size_t i = 0; i < items.size(); ++i) std::cout << "     " << i + 1 << ". " << items[i] << "\n";
        std::cout << " >> Chon (1-" << items.size() << "): ";
        int c; std::cin >> c;
        return c;
    }

    // 5. Label: Thông báo lỗi hoặc thành công
    static void Label(std::string msg, bool isError = false) {
        if (isError) std::cout << " [!] LOI: " << msg << " !!!\n";
        else std::cout << " [i] THONG BAO: " << msg << ".\n";
    }

    static void Header(std::string title) {
        system("cls");
        std::cout << "\n" << std::string(50, '*') << "\n";
        std::cout << "   " << title << "\n";
        std::cout << std::string(50, '*') << "\n";
    }
};

}
}
