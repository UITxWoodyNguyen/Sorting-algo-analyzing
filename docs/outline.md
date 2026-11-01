# Outline báo cáo 

## Chương 1. Cơ sở lý thuyết
1.1. Khái niệm: sắp xếp, ổn định (stable), in-place, Big-O (trường hợp tốt nhất / trung bình / xấu nhất)
1.2. Trình bày từng thuật toán (ý tưởng, mã giả, source code (basic), độ phức tạp, ưu/nhược điểm)
 - 1.2.1. Selection Sort
 - 1.2.2. Insertion Sort
 - 1.2.3. Heap Sort
 - 1.2.4. Quick Sort (nêu cách chọn pivot)
 - 1.2.5. Merge Sort
1.3. **Bảng tổng hợp lý thuyết**

| Thuật toán | Tốt nhất | Trung bình | Xấu nhất | Bộ nhớ phụ | Stable | In-place |
|---|---|---|---|---|---|---|
| Selection | | | | | | |
| Insertion | | | | | | |
| Heap | | | | | | |
| Quick | | | | | | |
| Merge | | | | | | |

## Chương 2. Phương pháp thực nghiệm
2.1. Môi trường: CPU, RAM, hệ điều hành, ngôn ngữ, trình biên dịch, cờ tối ưu
2.2. Bộ dữ liệu (TestCase)
 - Sinh ngẫu nhiên: khoảng giá trị, seed
 - 21 kích thước: 100; 1.000; 10.000 → 100.000 (bước 10.000); 150.000, 200.000, 250.000, 300.000; 500.000 → 1.000.000 (bước 100.000)
 - Lưu file (quy ước tên, định dạng); cả 5 thuật toán dùng chung một bộ dữ liệu
2.3. Cài đặt và cách đo
 - Cấu trúc source code, lưu ý cài đặt (Quick Sort tránh tràn stack, Merge Sort dùng mảng phụ)
 - Hàm đo thời gian, chỉ đo phần sắp xếp, sao chép mảng gốc trước mỗi lần chạy
 - Số lần lặp và cách lấy kết quả
2.4. Cơ chế timeout: quá 5 phút thì ghi chú "> 5 phút"
2.5. Kiểm tra tính đúng đắn (mảng sau sắp xếp tăng dần)

## Chương 3. Kết quả thực nghiệm
3.1. Bảng thống kê (đúng mẫu đề bài: 21 dòng × 5 cột, đơn vị giây)
3.2. Biểu đồ
 - Đường: thời gian theo kích thước, cả 5 thuật toán
 - Đường thang log hoặc tách riêng 3 thuật toán O(n log n) vì Selection/Insertion sẽ lấn át
 - (Tùy chọn) Cột cho các mốc 10.000; 100.000; 1.000.000
3.3. Bảng tỉ lệ so với thuật toán nhanh nhất (ví dụ tại n = 100.000)

## Chương 4. Phân tích và so sánh độ phức tạp
4.1. Đối chiếu lý thuyết và thực nghiệm: n tăng gấp đôi thì thời gian tăng khoảng 4 lần (O(n²)) hay hơn 2 lần một chút (O(n log n))
4.2. So sánh theo nhóm
 - O(n²): Selection vs Insertion, vì sao Insertion thường nhanh hơn trên dữ liệu ngẫu nhiên
 - O(n log n): Quick vs Merge vs Heap (cache locality, cấp phát bộ nhớ, hằng số ẩn)
4.3. Kích thước n mà thuật toán nâng cao bắt đầu vượt trội
4.4. Đánh đổi thời gian / bộ nhớ / tính ổn định, gợi ý chọn thuật toán theo tình huống
4.5. Hạn chế (một cấu hình máy, một kiểu dữ liệu, sai số đo)

## Chương 5. Kết luận
- Tóm tắt kết quả chính, bài học rút ra, hướng mở rộng (dữ liệu đã sắp xếp/đảo ngược, `std::sort`, Shell/Radix Sort)

## Tài liệu tham khảo

## Phụ lục
- **A.** Cấu trúc thư mục nộp: `Sorting_MASV1_MASV2_MASV3/` (source, testcase, báo cáo, PPT)
- **B.** Hướng dẫn biên dịch và chạy
- **C.** **Bảng phân công công việc**

| Thành viên | MSSV | Công việc | % đóng góp |
|---|---|---|---|