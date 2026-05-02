#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <iomanip>

namespace BankManagementSystem {
namespace UI {
using namespace std;

class Components {
public:
    // 1. Textbox: Nhập thông tin
    static string TextBox(string label) {
        string value;
        cout << " [?] " << left << setw(20) << label << ": ";
        // Xóa bộ nhớ đệm trước khi nhập chuỗi
        if (cin.peek() == '\n') cin.ignore();
        getline(cin, value);
        return value;
    }

    // 2. Button / Menu: Lựa chọn hành động
    static int Menu(vector<string> options) {
        cout << "\n --- LUA CHON HANH DONG ---\n";
        for (size_t i = 0; i < options.size(); ++i) {
            cout << "  " << i + 1 << ". " << options[i] << "\n";
        }
        cout << "  0. Quay lai / Thoat\n";
        cout << " >> Nhap so: ";
        int choice;
        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(1000, '\n');
            return -1;
        }
        return choice;
    }

    // 3. DataGridView / Table: Hiển thị bảng dữ liệu chuyên nghiệp
    static void Table(vector<string> headers, vector<vector<string>> rows) {
        cout << "\n" << string(90, '=') << "\n";
        for (const auto& h : headers) cout << "| " << left << setw(18) << h;
        cout << "|\n" << string(90, '-') << "\n";

        if (rows.empty()) {
            cout << "| " << setw(86) << " (Khong co du lieu) " << "|\n";
        } else {
            for (const auto& row : rows) {
                for (const auto& cell : row) cout << "| " << left << setw(18) << cell;
                cout << "|\n";
            }
        }
        cout << string(90, '=') << "\n";
    }

    // 4. ComboBox: Chon tu danh sach
    static int ComboBox(string label, vector<string> items) {
        cout << "\n [v] " << label << ":\n";
        for (size_t i = 0; i < items.size(); ++i) cout << "     " << i + 1 << ". " << items[i] << "\n";
        cout << " >> Chon (1-" << items.size() << ") hoac 0 de huy: ";
        int c;
        if (!(cin >> c)) {
            cin.clear();
            cin.ignore(1000, '\n');
            return -1;
        }
        return c;
    }

    // 5. Label: Thông báo lỗi hoặc thành công
    static void Label(string msg, bool isError = false) {
        if (isError) cout << " [!] LOI: " << msg << " !!!\n";
        else cout << " [i] THONG BAO: " << msg << ".\n";
    }

    static void Header(string title) {
        system("cls");
        cout << "\n" << string(50, '*') << "\n";
        cout << "   " << title << "\n";
        cout << string(50, '*') << "\n";
    }
};

}
}
