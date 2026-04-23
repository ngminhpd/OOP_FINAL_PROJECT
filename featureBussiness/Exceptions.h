#pragma once
#include <exception>
#include <string>

namespace BankManagementSystem {
namespace Business {

class BankException : public std::exception {
protected:
    std::string message;
public:
    BankException(const std::string& msg) : message(msg) {}
    virtual const char* what() const noexcept override {
        return message.c_str();
    }
};

class SaiPinException : public BankException {
public:
    SaiPinException() : BankException("Loi: Ma PIN khong chinh xac.") {}
};

class TaiKhoanKhoaException : public BankException {
public:
    TaiKhoanKhoaException() : BankException("Loi: Tai khoan hien dang bi khoa.") {}
};

class ThieuSoDuException : public BankException {
public:
    ThieuSoDuException() : BankException("Loi: So du khong du de thuc hien giao dich.") {}
};

class TaiKhoanKhongTonTaiException : public BankException {
public:
    TaiKhoanKhongTonTaiException() : BankException("Loi: So tai khoan khong ton tai.") {}
};

}
}
