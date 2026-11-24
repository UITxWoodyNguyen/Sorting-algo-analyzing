# Báo cáo phân tích các thuật toán sắp xếp

## Tóm tắt

Dự án cài đặt và so sánh năm thuật toán sắp xếp bằng C++17: Insertion Sort,
Selection Sort, Heap Sort, Merge Sort và Quick Sort. Hệ thống hỗ trợ giao
diện dòng lệnh, nhập/xuất bằng file, benchmark tự động, kiểm thử
known-answer, sinh dữ liệu, vẽ biểu đồ và giao diện PyQt6.

Kết quả benchmark xác nhận sự khác biệt giữa nhóm thuật toán bậc hai và nhóm
`O(n log n)`. Trong 19 file input chuẩn, Quick Sort nhanh nhất ở 16 kích
thước, Merge Sort nhanh nhất ở 2 kích thước và Insertion Sort nhanh nhất ở
kích thước nhỏ nhất. Insertion Sort và Selection Sort trở nên không thực tế
ở kích thước lớn theo giới hạn thời gian đã cấu hình, trong khi Heap Sort,
Merge Sort và Quick Sort vẫn hoàn thành với mảng một triệu phần tử.

## 1. Mục tiêu

Dự án có bốn mục tiêu chính:

1. Cài đặt năm thuật toán sắp xếp trong một giao diện thống nhất.
2. Hỗ trợ chạy tương tác qua stdin/stdout và kiểm thử bằng file có thể tái lập.
3. Đo runtime và sinh báo cáo CSV, biểu đồ.
4. Kiểm tra tính đúng đắn bằng known-answer test và output kỳ vọng.

Báo cáo này dựa trên mã nguồn hiện tại và các artifact trong `Output/`.
Các nội dung cũ chỉ được sử dụng khi còn phù hợp với implementation và dữ
liệu hiện tại.

## 2. Kiến trúc dự án

```text
Sorting-algo-analyzing/
├── CMakeLists.txt              # Cấu hình CMake đa nền tảng
├── dataset/
│   ├── generator.cpp           # Sinh 50 file input
│   ├── input/                  # Các file test .inp
│   └── output/<algorithm>/     # Output sắp xếp kỳ vọng
├── src/
│   ├── main.cpp                # CLI thống nhất
│   ├── bench.cpp               # Cài đặt benchmark
│   ├── kat.cpp                 # Cài đặt known-answer test
│   ├── sort_common.hpp         # Parse, dispatch và tiện ích dùng chung
│   ├── app_modes.hpp           # Khai báo mode dùng chung
│   └── include/                # Interface thuật toán
├── scripts/
│   ├── build.bat / build.sh
│   ├── run_all_bench.bat / run_all_bench.sh
│   └── plot_bench.py
├── gui/
│   └── sortapp_gui.py          # Giao diện PyQt6
├── test-vector/
│   └── vectors.json            # Các ca kiểm thử KAT
├── Output/                     # CSV, biểu đồ và báo cáo sinh ra
└── docs/
    └── report.md               # Báo cáo này
```

Các implementation được tổ chức theo template và được include thông qua
`src/sort_common.hpp`. Mã thuật toán được chọn theo ID:

| ID | Thuật toán |
|---:|---|
| 1 | Insertion Sort |
| 2 | Selection Sort |
| 3 | Heap Sort |
| 4 | Merge Sort |
| 5 | Quick Sort |

## 3. Thuật toán và độ phức tạp

| Thuật toán | Tốt nhất | Trung bình | Xấu nhất | Bộ nhớ phụ | Ổn định |
|---|---:|---:|---:|---:|---|
| Insertion Sort | `O(n)` | `O(n^2)` | `O(n^2)` | `O(1)` | Có |
| Selection Sort | `O(n^2)` | `O(n^2)` | `O(n^2)` | `O(1)` | Không |
| Heap Sort | `O(n log n)` | `O(n log n)` | `O(n log n)` | `O(1)` | Không |
| Merge Sort | `O(n log n)` | `O(n log n)` | `O(n log n)` | `O(n)` | Có |
| Quick Sort | `O(n log n)` | `O(n log n)` | `O(n^2)` | `O(log n)` stack đệ quy TB | Không |

Quick Sort sử dụng phần tử ở giữa làm pivot. Cách này giảm khả năng phân
hoạch cực đoan trên dữ liệu ngẫu nhiên nhưng không loại bỏ trường hợp xấu
nhất. Merge Sort dùng vùng nhớ phụ, còn Heap Sort sắp xếp tại chỗ.

## 4. Phương pháp thực nghiệm

### 4.1. Bộ dữ liệu

`dataset/generator.cpp` tạo 50 file input có thể tái lập:

- `01.inp` đến `19.inp` sử dụng các kích thước chuẩn
  `100`, `1.000`, `10.000`, `20.000`, ..., `100.000`,
  `200.000`, ..., `1.000.000`.
- `20.inp` đến `50.inp` sử dụng kích thước được random.
- Các giá trị là số nguyên, sử dụng seed cố định `20261101`.
- Mỗi input có format gồm `n`, sau đó là `n` số nguyên.
- Output kỳ vọng được lưu riêng theo từng thuật toán.

Script plotting cố ý chỉ phân tích `01.inp` đến `19.inp`, vì vậy các kết quả
trong báo cáo so sánh cùng 19 kích thước chuẩn.

### 4.2. Quy trình benchmark

Benchmark ghi các cột:

```text
file,n,algorithm,avg_ms,min_ms,status
```

Mỗi dòng thành công chứa runtime trung bình và nhỏ nhất, tính bằng
millisecond qua số lần lặp được cấu hình. Kết quả sắp xếp được đối chiếu
với output kỳ vọng. Các trạng thái gồm:

- `ok`: chạy xong và tạo kết quả đúng;
- `TLE`: vượt quá giới hạn thời gian khi thực thi;
- `TLE_predicted`: được bỏ qua vì runtime bậc hai dự kiến vượt giới hạn.

CSV benchmark có 250 dòng cho 50 input và 5 thuật toán. Báo cáo này sử dụng
95 dòng tương ứng với 19 input đầu tiên.

### 4.3. Môi trường kiểm thử

Lần kiểm thử được xác nhận trên Windows với:

```text
Compiler: MinGW g++.exe 16.2.0
CMake:    4.3.5
Python:   3.14.7
```

Cấu hình CMake cũng hỗ trợ `/usr/bin/g++` trên Ubuntu. Runtime phụ thuộc vào
môi trường, vì vậy các con số dưới đây chỉ đại diện cho lần đo này, không
phải hằng số chung cho mọi máy.

## 5. Kết quả kiểm tra tính đúng đắn

File vector KAT hiện tại có 46 ca kiểm thử:

| Nhóm | Số ca |
|---|---:|
| Best | 7 |
| Worst | 11 |
| Normal | 10 |
| Edge | 18 |
| **Tổng** | **46** |

Lần chạy KAT đầy đủ thực hiện 380 tổ hợp thuật toán-ca kiểm thử:

```text
380/380 pass
FAIL 0
CRASH 0
TIMEOUT 0
```

Kết quả theo từng thuật toán:

| Thuật toán | Kết quả |
|---|---:|
| Insertion | 76/76 |
| Selection | 74/74 |
| Heap | 74/74 |
| Merge | 76/76 |
| Quick | 80/80 |

Các output benchmark ở trạng thái `ok` cũng được đối chiếu với output kỳ
vọng tương ứng.

## 6. Kết quả runtime đo được

Các giá trị dưới đây là `avg_ms` lấy từ `Output/benchmark.csv`. `TLE` bao
gồm cả timeout quan sát được và timeout dự đoán, trong đó benchmark không
khởi chạy phép đo bậc hai không thực tế.

| n | Insertion | Selection | Heap | Merge | Quick |
|---:|---:|---:|---:|---:|---:|
| 100 | 0.007 | 0.015 | 0.020 | 0.014 | 0.009 |
| 1,000 | 0.546 | 1.064 | 0.147 | 0.139 | 0.101 |
| 10,000 | 54.332 | 106.385 | 1.803 | 1.189 | 1.302 |
| 20,000 | 240.154 | 415.215 | 3.846 | 2.955 | 2.516 |
| 30,000 | 507.878 | 914.595 | 6.140 | 4.187 | 3.669 |
| 40,000 | 890.503 | 1,604.909 | 8.495 | 5.979 | 5.112 |
| 50,000 | 1,404.799 | 2,542.464 | 11.681 | 7.072 | 6.324 |
| 60,000 | 2,031.012 | 3,723.187 | 12.706 | 8.386 | 8.268 |
| 70,000 | 2,790.251 | 6,261.593 | 24.212 | 18.000 | 14.699 |
| 80,000 | 4,008.245 | 7,468.784 | 23.719 | 14.533 | 12.983 |
| 100,000 | 5,932.612 | 10,717.341 | 30.919 | 18.192 | 17.854 |
| 200,000 | 26,845.772 | 57,899.230 | 87.155 | 52.859 | 48.657 |
| 300,000 | 73,158.630 | 112,157.839 | 91.675 | 54.146 | 49.604 |
| 400,000 | 111,150.093 | TLE | 145.323 | 83.091 | 78.804 |
| 500,000 | TLE | TLE | 375.966 | 173.368 | 162.702 |
| 600,000 | TLE | TLE | 299.546 | 156.783 | 136.396 |
| 700,000 | TLE | TLE | 294.464 | 159.826 | 143.875 |
| 800,000 | TLE | TLE | 341.926 | 214.308 | 215.020 |
| 1,000,000 | TLE | TLE | 396.430 | 204.959 | 186.889 |

Một số giá trị lớn không tăng hoàn toàn đơn điệu là hiện tượng có thể xảy ra
với phép đo wall-clock. Nguyên nhân có thể là lịch CPU, trạng thái cache,
cấp phát bộ nhớ hoặc tiến trình nền. Điều này không làm thay đổi xu hướng
độ phức tạp tổng quát.

### 6.1. Thuật toán nhanh nhất theo kích thước

Số lần đứng đầu trong 19 kích thước:

| Thuật toán | Số lần thắng | Tỷ lệ |
|---|---:|---:|
| Quick | 16 | 84,2% |
| Merge | 2 | 10,5% |
| Insertion | 1 | 5,3% |
| Heap | 0 | 0,0% |
| Selection | 0 | 0,0% |

Một số mốc tiêu biểu:

| n | Thuật toán nhanh nhất | Runtime (ms) |
|---:|---|---:|
| 100 | Insertion | 0.007400 |
| 1,000 | Quick | 0.101000 |
| 10,000 | Merge | 1.189100 |
| 100,000 | Quick | 17.853600 |
| 1,000,000 | Quick | 186.888633 |

Bảng đầy đủ theo từng kích thước được sinh tại
`Output/performance_comparison.md`. Crossover đo được nằm giữa `n = 100`
và `n = 1.000` cho cả Insertion so với Quick và Insertion so với Merge.
Đây là khoảng crossover của lần đo này, không phải giá trị cố định trên mọi
máy.

## 7. Phân tích kết quả

### 7.1. Nhóm thuật toán bậc hai

Insertion Sort và Selection Sort tăng runtime rất nhanh khi kích thước tăng.
Tại `n = 100.000`, runtime tương ứng xấp xỉ 5,93 giây và 10,72 giây.
Selection Sort chậm hơn xuyên suốt bộ dữ liệu vì luôn thực hiện kiểu tìm
kiếm bậc hai, trong khi Insertion Sort có thể hưởng lợi từ dữ liệu gần có
thứ tự.

Selection Sort timeout tại `n = 400.000`. Insertion Sort hoàn thành ở kích
thước này nhưng timeout tại `n = 500.000`. Các kích thước bậc hai lớn hơn
được đánh dấu `TLE_predicted`.

### 7.2. Nhóm `O(n log n)`

Quick Sort, Merge Sort và Heap Sort hoàn thành cả 19 kích thước chuẩn. Quick
Sort thường nhanh nhất, phù hợp với partition tại chỗ và hằng số chi phí
thấp trên bộ input này. Merge Sort có kết quả gần Quick Sort và thắng tại
`n = 10.000` và `n = 800.000`. Heap Sort chậm hơn Quick Sort và Merge Sort
trong lần đo này.

Các quan sát trên phù hợp với phân loại độ phức tạp, nhưng không chứng minh
một implementation luôn nhanh nhất. Kết quả còn phụ thuộc phân phối dữ
liệu, compiler, phần cứng, memory hierarchy và nhiễu đo.

### 7.3. Độ ổn định của phép đo

`Output/measurement_stability_proxy.csv` tổng hợp tỷ lệ `avg_ms / min_ms`:

| Thuật toán | Tỷ lệ trung bình | Tỷ lệ lớn nhất |
|---|---:|---:|
| Insertion | 1.041 | 1.231 |
| Quick | 1.053 | 1.174 |
| Heap | 1.054 | 1.282 |
| Selection | 1.056 | 1.212 |
| Merge | 1.094 | 1.472 |

Đây chỉ là proxy cho overhead giữa các lần lặp, không phải độ lệch chuẩn,
phương sai, khoảng tin cậy hay bằng chứng rằng một thuật toán vốn dĩ dễ dự
đoán hơn. CSV hiện không lưu từng lần lặp riêng lẻ nên chưa thể đưa ra kết
luận thống kê mạnh.

## 8. Các artifact phân tích

Quy trình benchmark và plotting tạo ra:

- `Output/benchmark.csv`: dữ liệu benchmark tổng hợp.
- `Output/runtime_comparison.png`: biểu đồ runtime tổng quát.
- `Output/runtime_comparison_selected_sizes.png`: so sánh tại các kích
  thước tiêu biểu.
- `Output/runtime_crossover_analysis.png`: biểu đồ crossover.
- `Output/algorithm_wins.png`: số lần mỗi thuật toán thắng.
- `Output/performance_comparison.md`: bảng thuật toán nhanh nhất và speedup.
- `Output/crossover_analysis.md`: crossover đầu tiên được đo.
- `Output/measurement_stability_proxy.csv`: proxy overhead giữa các lần lặp.
- `Output/algorithms/<algorithm>/`: biểu đồ và bảng riêng từng thuật toán.

Nên đọc biểu đồ cùng với CSV vì hình ảnh không thể hiện đầy đủ một giá trị
là phép đo thành công, timeout hay timeout dự đoán.

## 9. Hạn chế

Thực nghiệm hiện tại có các hạn chế sau:

1. Bộ sinh tạo dữ liệu số nguyên nhưng CSV chưa gắn nhãn phân phối ngữ nghĩa
   như sorted, reverse, gần sorted hoặc nhiều phần tử trùng.
2. Benchmark chỉ đo wall-clock time, chưa đếm phép so sánh, ghi, swap, cấp
   phát bộ nhớ, cache miss hoặc branch misprediction.
3. CSV lưu giá trị trung bình và nhỏ nhất, không lưu toàn bộ lần lặp. Muốn
   tính variance và confidence interval cần thay đổi format benchmark.
4. Runtime phụ thuộc môi trường Windows/MinGW đã ghi nhận, không nên xem là
   hằng số độc lập phần cứng.
5. Biểu đồ được chọn chỉ dùng 19 file đầu. 31 file kích thước random còn lại
   có thể dùng cho phân tích bổ sung nhưng không thuộc các bảng trên.
6. Quick Sort vẫn có worst case `O(n^2)` dù sử dụng pivot ở giữa.

## 10. Kết luận và hướng phát triển

Dự án cung cấp quy trình có thể tái lập từ sinh dữ liệu, kiểm tra tính đúng
đắn, benchmark, plotting đến chạy bằng GUI. Implementation vượt qua toàn bộ
KAT hiện tại, đồng thời dữ liệu đo thể hiện rõ sự khác biệt giữa nhóm thuật
toán bậc hai và nhóm `O(n log n)`.

Với dữ liệu số nguyên ngẫu nhiên trong lần đo này, Quick Sort là lựa chọn
nhanh nhất về runtime tổng thể. Merge Sort là lựa chọn ổn định và có hiệu
năng cạnh tranh, còn Heap Sort có độ phức tạp dự đoán được và sắp xếp tại
chỗ. Insertion Sort chỉ cạnh tranh ở kích thước nhỏ nhất, trong khi
Selection Sort chậm nhất trong phạm vi đã đo.

Các hướng mở rộng:

- gắn nhãn rõ ràng cho dữ liệu sorted, reverse, gần sorted, nhiều phần tử
  trùng và random;
- lưu toàn bộ lần lặp để tính mean, median, standard deviation và confidence
  interval;
- đếm số phép so sánh và số lần ghi;
- bổ sung hardware counter khi môi trường cho phép;
- so sánh với các thuật toán lai như introsort hoặc TimSort;
- lặp lại thực nghiệm trên cả Windows và Ubuntu.

## 11. Lệnh tái lập

Chạy từ thư mục gốc dự án:

```text
# Build
scripts\build.bat
# hoặc trên Ubuntu:
./scripts/build.sh

# Chạy known-answer test
build\kat.exe --vectors test-vector\vectors.json --timeout 2

# Chạy benchmark
scripts\run_all_bench.bat
# hoặc trên Ubuntu:
./scripts/run_all_bench.sh

# Sinh biểu đồ và phân tích
python scripts/plot_bench.py --input Output/benchmark.csv --output-dir Output
```

Khởi chạy giao diện PyQt6:

```text
python gui/sortapp_gui.py
```

GUI cung cấp các mode sort, benchmark và known-answer test, đồng thời sử
dụng cùng các executable và artifact dòng lệnh được mô tả trong báo cáo.
