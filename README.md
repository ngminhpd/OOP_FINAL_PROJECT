# Đồ án cuối kỳ Thực hành OOP
## Thành viên
Nguyễn Hồng Minh
Nguyễn Hoàng Khang
Nguyễn Minh Quân
Huỳnh Tấn Đạt

## Lịch sử commit
| Ngày      |                     Nội dung                   | Thành viên        | Xác nhận      | Ghi chú                                     |
|-----------|------------------------------------------------|-------------------|---------------|---------------------------------------------|
|  15/4/26  |            Tạo REAME.md                        |   **Tất cả **     | Hoàn thành    | e335c56860e9619a0fe3ea4bfc98521b0258f9bb    |
|  23/4/26  |       Thiết kế hệ thống, class, đa hình        |   **Khang**       | Hoàn thành    | b81bb2e28271032202937ac6570878dd835e3f36    |
|  23/4/26  |             ...                                |   **Minh**        | Hoàn thành    | ee0c64e1778a1a23e19102cab0f213ac7a2575a2    |
|  23/4/26  |       File dữ liệu, test, tích hợp             |   **Minh**        | Hoàn thành    | 17f23c915aa3b2642d867176a2f4894316a180cf    |
|  23/4/26  |       Chức năng ngân hàng                      |   **Quân**        | Hoàn thành    | d22e96ff7d64212efc5640a254f64d3ee5cc9e55    |
|  23/4/26  |       Đồng bộ hệ thống, class, đa hình         |   **Quân**        | Hoàn thành    | 4e22a0b5822b244bad7068b9039ec3788a859f2d    |
|  23/4/26  |  Thực hiện các logic của UI, các trường hợp    |   **Đạt**         | Hoàn thành    | a6f20c1870a20a4765a8e8d7e70e88708a54e03e    |
|  23/4/26  |       Bổ sung các nội dung UI còn thiếu        |   **Đạt**         | Hoàn thành    | ceaf7ed3c984b7a0e628851fac46403f9567cb5c    |
|  24/4/26  |       Lên ý tưởng thiết kế giao diện           |   **Tất cả **     | Hoàn thành    | Họp     |
|  25/4/26  |       Commit và Push lên nhánh main            |   **Minh**        | Hoàn thành    | 7a780ba7ca39d3937cef7273abe5abbe976cd0d8    |
|  25/4/26  | Sửa chức năng đổi mã pin, hiện chữ cho số tiền |   **Quân/Minh **  | Hoàn thành    | 766f184335611cd8bf006f8dcbdf31c1a7a4cd3e    |
|  25/4/26  |       Sửa lệnh điều kiện tín dụng              |   **Khang**       | Hoàn thành    | Push    |
|  25/4/26  |       Thêm chức năng vay tiền và đếm ngược     |   **Minh/Đạt**    | Hoàn thành    | 5d4f00068def26d9127196368e11d26a9dfff51d    |
|  26/4/26  |       Thêm chức năng refresh trang             |   **Quân**        | Hoàn thành    | 83647cc801fb757976f87b088c2f62da80f6d9a9    |
|  26/4/26  |       Sửa chức năng hiển thị số dư             |  ** Quân **       | Hoàn thành    | a3d82638913a1d9f203f4e420c8124f0df012a68    |
|  26/4/26  |       Hiển thị số dư khoản vay riêng biệt.     |   **Minh **       | Hoàn thành    | 43b79ac6c54e520a8bfceb7fb2bc22f582f56873    |
|           |                                                |               |               |         |

## Nhiệm vụ
1. **`feature/model`** Thiết kế hệ thống, class, đa hình  
    - TaiKhoan hoặc BankAccount là lớp cha
    - Các lớp con:
    - TaiKhoanTietKiem
    - TaiKhoanThanhToan
    - TaiKhoanDoanhNghiep hoặc TaiKhoanTinDung
    - Các hàm ảo:
    - rutTien()
    - tinhLai()
    - phiDuyTri()
    - hienThiThongTin()
2. **`feature/ui`** Các form nên làm                   
    - Form đăng nhập
    - Form menu chính
    - Form quản lý khách hàng
    - Form quản lý tài khoản
    - Form giao dịch
    - Form tra cứu / thống kê
    - Thành phần giao diện
    - textbox nhập thông tin
    - button thêm / sửa / xóa
    - datagridview hoặc table
    - combobox chọn loại tài khoản
    - label thông báo lỗi
3. **`feature/business`** Chức năng ngân hàng
    - thêm tài khoản
    - xóa tài khoản
    - sửa thông tin tài khoản
    - nạp tiền
    - rút tiền
    - chuyển tiền
    - tra cứu số dư
    - tính lãi
    - kiểm tra điều kiện rút/chuyển
    - Các rule nên có
    - không cho rút quá số dư
    - tài khoản tiết kiệm có điều kiện rút
    - tài khoản khác nhau có cách tính phí/lãi khác nhau
    - đây chính là chỗ dùng đa hình
4. **`feature/data`** File dữ liệu, test, tích hợp
    - lưu dữ liệu ra file .txt, .json, hoặc .xml
    - đọc dữ liệu từ file khi mở chương trình
    - viết test case
    - sửa lỗi khi ghép code
    - chuẩn bị demo
    - tổng hợp báo cáo, slide
    - Dữ liệu nên lưu
    - danh sách khách hàng
    - danh sách tài khoản
    - lịch sử giao dịch

| Nhiệm vụ |    Thành viên          |
|----------|------------------------|
|   1      |      Khang             |
|   2      |      Đạt               |
|   3      |      Quân              |
|   4      |      Minh              |


