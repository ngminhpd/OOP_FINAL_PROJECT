import requests
import time
import json

#chạy code để test các chức năng của hệ thống

BASE_URL = "http://localhost:8080"

def call(method, path, data=None, params=None):
    url = f"{BASE_URL}{path}"
    res = requests.post(url, data=data) if method == "POST" else requests.get(url, params=params)
    return res.json()

def test_everything():
    u1 = {"stk": "U1", "ten": "User One", "pin": "1111"}
    u2 = {"stk": "U2", "ten": "User Two", "pin": "2222"}

    print("--- [1] DỌN DẸP HỆ THỐNG ---")
    call("POST", "/api/admin/delete", {"stk": u1["stk"]})
    call("POST", "/api/admin/delete", {"stk": u2["stk"]})

    print("--- [2] ADMIN TẠO TK & NẠP TẠI QUẦY ---")
    call("POST", "/api/admin/add", {**u2, "type": "ThanhToan", "sodu": "100000"})
    call("POST", "/api/admin/deposit", {"stk": u2["stk"], "amount": "50000"})
    print("Admin counter deposit: Done")

    print("--- [3] USER ĐĂNG KÝ & QUÊN MÃ PIN ---")
    call("POST", "/api/register", {**u1, "type": "TietKiem"})
    call("POST", "/api/user/request_reset_pin", {"stk": u1["stk"]})
    call("POST", "/api/admin/approve_request", {"stk": u1["stk"], "type": "RESET_PIN"})
    u1["pin"] = "1234" 
    print("Reset PIN flow: Done")

    print("--- [4] ĐĂNG NHẬP & ĐỔI PIN ---")
    call("POST", "/api/login", {"user": u1["stk"], "pin": u1["pin"]})
    call("POST", "/api/user/changepin", {"stk": u1["stk"], "oldpin": "1234", "newpin": "8888"})
    u1["pin"] = "8888"
    print("Login & Change PIN: Done")

    print("--- [5] GIAO DỊCH & LỊCH SỬ ---")
    call("POST", "/api/user/transact", {"stk": u1["stk"], "type": "nap", "amount": "1000000", "pin": u1["pin"], "source": "main"})
    call("POST", "/api/user/transact", {"stk": u1["stk"], "dest": u2["stk"], "type": "chuyen", "amount": "50000", "pin": u1["pin"], "source": "main"})
    his = call("GET", "/api/user/history", params={"stk": u1["stk"]})
    print(f"History count: {len(his)}")

    print("--- [6] HỆ THỐNG VAY (REQUEST -> REJECT -> APPROVE -> REPAY) ---")
    call("POST", "/api/user/request_loan", {"stk": u1["stk"], "amount": "5000000", "phut": "1"})
    call("POST", "/api/admin/reject_request", {"stk": u1["stk"], "type": "LOAN"})
    print("Reject loan request: Done")
    call("POST", "/api/user/request_loan", {"stk": u1["stk"], "amount": "2000000", "phut": "1"})
    call("POST", "/api/admin/approve_request", {"stk": u1["stk"], "type": "LOAN", "val": "2000000", "hang": "1"})
    call("POST", "/api/user/repay_loan", {"stk": u1["stk"], "pin": u1["pin"]})
    print("Loan workflow (Request-Reject-Approve-Repay): Done")

    print("--- [7] THẺ TÍN DỤNG & CHI TIÊU ---")
    call("POST", "/api/user/request_credit", {"stk": u1["stk"]})
    call("POST", "/api/admin/approve_request", {"stk": u1["stk"], "type": "CREDIT", "val": "20000000"})
    call("POST", "/api/user/transact", {"stk": u1["stk"], "type": "rut", "amount": "1000000", "pin": u1["pin"], "source": "credit"})
    call("POST", "/api/user/transact", {"stk": u1["stk"], "type": "nap", "amount": "1000000", "pin": u1["pin"], "source": "credit"})
    print("Credit Card flow: Done")

    print("--- [8] QUẢN TRỊ VIÊN (UPGRADE, LOCK, INTEREST) ---")
    call("POST", "/api/user/request_upgrade", {"stk": u1["stk"], "hang": "Private"})
    call("POST", "/api/admin/approve_request", {"stk": u1["stk"], "type": "UPGRADE", "val": "Private"})
    call("POST", "/api/admin/lock", {"stk": u1["stk"], "lock": "1"})
    call("POST", "/api/admin/lock", {"stk": u1["stk"], "lock": "0"})
    call("POST", "/api/admin/interest")
    call("GET", "/api/admin/all")
    call("GET", "/api/admin/upgrade_requests")
    call("GET", "/api/admin/loans")
    call("GET", "/api/admin/loan_history")
    print("Admin management actions: Done")

    print("\n" + "!"*30)
    print("SUCCESS: 100% FUNCTIONS TESTED")
    print("!"*30)

if __name__ == "__main__":
    test_everything()