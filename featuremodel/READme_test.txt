CHẠY FILE build.bat ĐỂ TEST HỆ THỐNG!!!

TÓM TẮT NGHIỆP VỤ HỆ THỐNG TÀI KHOẢN NGÂN HÀNG

Trong hệ thống này, mỗi tài khoản đều có các khái niệm chung:

* Số dư (SoDu): số tiền hiện có (hoặc số tiền đang nợ đối với tài khoản tín dụng)
* Rút tiền (RutTien): hành động sử dụng tiền (ý nghĩa khác nhau tùy loại tài khoản)
* Lãi (TinhLai): khoản tiền phát sinh thêm (có thể là tiền bạn nhận hoặc phải trả)
* Phí (PhiDuyTri): phí ngân hàng thu định kỳ (thường theo tháng)

1. TÀI KHOẢN THANH TOÁN (ATM)

* Rút tiền: là lấy tiền từ tài khoản ra để sử dụng
* Điều kiện: sau khi rút phải còn tối thiểu 50,000 VND
* Lãi: ngân hàng trả cho bạn, nhưng rất thấp (~0.1%)
* Phí: phí duy trì tài khoản mỗi tháng (ví dụ: 11,000 VND)

2. TÀI KHOẢN TIẾT KIỆM

* Rút tiền: không được rút từng phần, chỉ được rút toàn bộ
* Lãi: ngân hàng trả cho bạn, cao hơn tài khoản thanh toán (ví dụ: 5%)
* Phí: không có phí duy trì

3. TÀI KHOẢN TÍN DỤNG (CREDIT)

* Số dư (SoDu): KHÔNG phải tiền bạn có, mà là số tiền bạn đã tiêu (nợ ngân hàng)
* Rút tiền: thực chất là tiêu tiền của ngân hàng (tăng khoản nợ)
* Điều kiện: không được vượt quá hạn mức tín dụng
* Lãi: là tiền bạn phải trả thêm cho ngân hàng nếu có dư nợ (ví dụ: 20%)
* Phí: phí duy trì thẻ tín dụng (ví dụ: 50,000 VND/tháng)

KẾT LUẬN QUAN TRỌNG:

* RutTien():

  * Thanh toán → rút tiền thật
  * Tiết kiệm → rút toàn bộ
  * Tín dụng → tiêu tiền (tăng nợ)

* TinhLai():

  * Thanh toán → tiền bạn nhận (ít)
  * Tiết kiệm → tiền bạn nhận (nhiều)
  * Tín dụng → tiền bạn phải trả (phạt)

* PhiDuyTri():

  * Thanh toán → phí tháng
  * Tiết kiệm → không có
  * Tín dụng → phí thẻ

Lưu ý: cùng một hàm nhưng ý nghĩa thay đổi theo loại tài khoản → đây chính là đa hình trong hệ thống.
