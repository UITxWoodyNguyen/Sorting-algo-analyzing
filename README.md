# Sorting Algorithm Analyzing

Dự án C++17 dùng để cài đặt, kiểm thử và đo hiệu năng năm thuật toán sắp xếp:

1. Insertion sort
2. Selection sort
3. Heap sort
4. Merge sort
5. Quick sort

## Cấu trúc dự án

```text
.
├── CMakeLists.txt
├── dataset/
│   ├── generator.cpp
│   ├── input/                  # Các file .inp
│   └── output/<algorithm>/     # Expected output theo từng thuật toán
├── gui/
│   └── sortapp_gui.py          # GUI PyQt6
├── scripts/
│   ├── build.bat               # Build Windows
│   ├── build.sh                # Build Ubuntu
│   ├── plot_bench.py           # Vẽ biểu đồ và tạo bảng runtime
│   ├── run_all_bench.bat       # Benchmark Windows
│   └── run_all_bench.sh        # Benchmark Ubuntu
├── src/
│   ├── main.cpp                # Executable chính
│   ├── bench.cpp               # Benchmark
│   ├── kat.cpp                 # Known-answer test
│   └── include/                # Header của các thuật toán
└── test-vector/
    └── vectors.json            # Dữ liệu KAT
```

## Kiến trúc dự án

### Tổng quan các lớp

![alt text](img/image.png)

### Luồng chạy chính

1. `dataset/generator.cpp` tạo file input gồm `n` và mảng số nguyên. Khi
   dùng tùy chọn `--out` hoặc `--all`, generator dùng `std::sort` để tạo
   expected output cho từng thư mục thuật toán.
2. `src/main.cpp` là entry point của `sort_app`. Nó phân tích mode:
   - Không có tham số: đọc stdin, chọn thuật toán, ghi stdout.
   - `--file`: đọc một file `.inp`, chạy sort, ghi một file `.out`.
   - `--benchmark`: chuyển quyền xử lý cho `runBenchmark`.
   - `--kat`: chuyển quyền xử lý cho `runKnownAnswerTests`.
3. `src/sort_common.hpp` cung cấp các hàm dùng chung:
   - `readArray`/`writeArray` xử lý định dạng input/output.
   - `runSort` dispatch thuật toán theo mã `1..5`.
   - `readExpected` đọc expected output.
   - `algoDir` và `toOutputPath` ánh xạ tên thuật toán với thư mục output.
4. `src/bench.cpp` đọc các file `.inp`, chạy cả năm thuật toán, so sánh với
   expected output tương ứng, đo thời gian và ghi từng kết quả vào CSV.
5. `scripts/plot_bench.py` chỉ lọc các dòng tương ứng với `01.inp` đến
   `19.inp`, sau đó tạo biểu đồ runtime và bảng CSV/Markdown. GUI gọi các
   executable CLI qua `QProcess`, nên không chứa lại logic benchmark/KAT.

### Quan hệ build

`CMakeLists.txt` tạo một thư viện interface `sort_core` để chia sẻ include
path `src/` cho các executable. Các target chính là:

```text
sort_app  = main.cpp + bench.cpp + kat.cpp
bench     = bench.cpp + bench_main.cpp
kat       = kat.cpp + kat_main.cpp
generator = dataset/generator.cpp
```

Các file `*-sort.cpp` chứa triển khai template và được include trực tiếp bởi
`sort_common.hpp`; chúng không được compile thành object riêng. Cách tổ chức
này bảo đảm compiler nhìn thấy đầy đủ định nghĩa template khi các target gọi
`runSort`.

### Phân tách trách nhiệm

| Thành phần | Trách nhiệm | Không phụ trách |
|---|---|---|
| `sort_app` | Chạy sort tương tác, file I/O, benchmark và KAT qua CLI | Vẽ biểu đồ |
| `gui/sortapp_gui.py` | Giao diện sort, benchmark và KAT | Cài đặt thuật toán |
| `bench`/`bench.cpp` | Đo runtime, kiểm tra expected output | Tạo biểu đồ |
| `kat`/`kat.cpp` | Kiểm tra vector đáp án tăng/giảm | Đo benchmark dataset |
| `scripts/plot_bench.py` | Đọc CSV, tạo biểu đồ và bảng cho test `01..19` | Chạy thuật toán sort |
| `dataset/generator` | Tạo input và expected output | Đo runtime |

## Yêu cầu môi trường

### Windows

- CMake
- Ninja
- MinGW g++ tại `C:\msys64\mingw64\bin\g++.exe`
- Python 3
- Các package Python: `PyQt6`, `matplotlib`

### Ubuntu

```bash
sudo apt update
sudo apt install -y build-essential cmake ninja-build python3 python3-pip
```

Compiler mặc định được sử dụng là `/usr/bin/g++`.

## Build

### Windows

```bat
scripts\build.bat
```

Hoặc chạy trực tiếp bằng CMake:

```powershell
cmake -S . -B build -G Ninja `
  -DCMAKE_CXX_COMPILER=C:\msys64\mingw64\bin\g++.exe
cmake --build build --parallel
```

### Ubuntu

```bash
chmod +x scripts/build.sh
./scripts/build.sh
```

Hoặc:

```bash
cmake -S . -B build -G Ninja -DCMAKE_CXX_COMPILER=/usr/bin/g++
cmake --build build --parallel
```

Các executable được tạo trong `build/`:

- `sort_app`: chương trình chính, tích hợp mọi chức năng.
- `bench`: benchmark độc lập.
- `kat`: known-answer test độc lập.
- `generator`: sinh dữ liệu test.

## Sinh dữ liệu test

Biên dịch project trước, sau đó chạy:

```powershell
.\build\generator.exe --all
```

Ubuntu:

```bash
./build/generator --all
```

Các tùy chọn:

```text
--inp, -i    Sinh 50 file input
--out, -o    Sinh expected output từ input hiện có
--all, -a    Sinh cả input và expected output
--help, -h   Hiển thị trợ giúp
```

Generator tạo:

- `dataset/input/01.inp` đến `dataset/input/50.inp`.
- Test `01` đến `19` dùng các size mặc định:
  `100`, `1000`, `10000` đến `80000`, `100000` đến `800000`,
  và `1000000`.
- Test `20` đến `50` dùng size ngẫu nhiên, tối đa `1000000`.
- Expected output tại:
  `dataset/output/<algorithm>/01.out` đến `50.out`.

Seed mặc định là `20261101`, vì vậy dữ liệu có thể tái tạo.

## Chạy chương trình chính

Xem toàn bộ trợ giúp:

```powershell
.\build\sort_app.exe --help
```

### 1. stdin/stdout

Input gồm `n` và `n` số nguyên. Nếu không truyền tham số, chương trình hỏi
thuật toán từ `1` đến `5`, sau đó in `n` và dãy đã sắp xếp.

```powershell
@"
3
5 1 4
"@ | .\build\sort_app.exe
```

Kết quả:

```text
3
1 4 5
```

Ubuntu:

```bash
printf "3\n5 1 4\n" | ./build/sort_app
```

### 2. File input/output

```powershell
.\build\sort_app.exe --file `
  dataset\input\01.inp `
  Output\01.out `
  heap
```

Thuật toán có thể truyền bằng số hoặc tên:

```text
1 / insertion
2 / selection
3 / heap
4 / merge
5 / quick
```

File output gồm dòng đầu là `n`, tiếp theo là các phần tử đã sắp xếp tăng dần.

## Benchmark

Benchmark chạy năm thuật toán trên toàn bộ file `.inp`, kiểm tra kết quả với
expected output và ghi runtime vào CSV.

```powershell
.\build\sort_app.exe --benchmark `
  --in dataset\input `
  --out dataset\output `
  --csv Output\benchmark.csv `
  --repeat 3 `
  --timeout 180
```

Ubuntu:

```bash
./build/sort_app --benchmark \
  --in dataset/input \
  --out dataset/output \
  --csv Output/benchmark.csv \
  --repeat 3 \
  --timeout 180
```

Tùy chọn benchmark:

```text
--in DIR              Thư mục input, mặc định dataset/input
--out DIR             Thư mục expected output, mặc định dataset/output
--csv FILE            File CSV kết quả
--repeat N            Số lần lặp mỗi test, mặc định 3
--timeout SEC         Giới hạn mỗi lần chạy, mặc định 180 giây
--skip-after-tle      Suy ra TLE cho các size lớn tiếp theo
```

CSV có các cột:

```text
file,n,algorithm,avg_ms,min_ms,status
```

## Biểu đồ và bảng runtime

Sau khi có `Output/benchmark.csv`, chạy. Công cụ plot chỉ sử dụng 19 test đầu,
từ `01.inp` đến `19.inp`; các dòng `20.inp` đến `50.inp` sẽ bị bỏ qua:

```bash
python3 scripts/plot_bench.py \
  --input Output/benchmark.csv \
  --output-dir Output
```

Windows:

```powershell
py scripts\plot_bench.py `
  --input Output\benchmark.csv `
  --output-dir Output
```

Các file sinh ra:

- `Output/runtime_comparison.png`: runtime theo 19 test đầu.
- `Output/runtime_comparison_selected_sizes.png`: so sánh tại `1e3`, `1e4`,
  `1e5`, `1e6`.
- `Output/runtime_table.csv` và `Output/runtime_table.md`: bảng của 19 test đầu.
- `Output/runtime_selected_sizes.csv` và
  `Output/runtime_selected_sizes.md`: bảng bốn size được chọn.

Biểu đồ dùng trục log để dễ quan sát đồng thời các thuật toán `O(n^2)` và
`O(n log n)`.

## Chạy benchmark tự động

Windows:

```bat
scripts\run_all_bench.bat
```

Ubuntu:

```bash
chmod +x scripts/run_all_bench.sh
./scripts/run_all_bench.sh
```

Script sẽ:

1. Build nếu executable chưa tồn tại.
2. Chạy benchmark năm thuật toán.
3. Ghi `Output/benchmark.csv`.
4. Sinh biểu đồ và bảng runtime.

## Known-answer test

KAT đọc `test-vector/vectors.json`, kiểm tra cả thứ tự tăng và giảm.

```powershell
.\build\sort_app.exe --kat `
  --vectors test-vector\vectors.json `
  --timeout 10
```

Ubuntu:

```bash
./build/sort_app --kat \
  --vectors test-vector/vectors.json \
  --timeout 10
```

Lọc theo thuật toán hoặc nhóm test:

```text
--algo heap|insertion|selection|merge|quick|1..5
--category best|worst|normal|edge
-v                         In cả test PASS
```

Ví dụ:

```bash
./build/kat --algo quick --category edge -v
```

## GUI PyQt6

Cài dependency:

```bash
python3 -m pip install PyQt6 matplotlib
```

Windows:

```powershell
py -m pip install PyQt6 matplotlib
py gui\sortapp_gui.py
```

Ubuntu:

```bash
python3 gui/sortapp_gui.py
```

GUI tích hợp toàn bộ chức năng CLI của `sort_app`:

- Chạy stdin/stdout với mảng nhập trực tiếp.
- Chạy file `.inp` và ghi kết quả ra file `.out`.
- Chọn một trong năm thuật toán cho chế độ sort.
- Chạy benchmark toàn bộ thuật toán, cấu hình repeat/timeout và xuất CSV.
- Chạy known-answer test, lọc theo thuật toán/category và bật verbose.
- Sinh biểu đồ/bảng benchmark bằng `plot_bench.py`.
- Build `sort_app` từ nút `Build sort_app`.

GUI sử dụng các tab `Sort`, `Benchmark` và `Known-answer test`. Benchmark vẫn chỉ plot các file `01.inp` đến `19.inp`, theo quy tắc của `scripts/plot_bench.py`.

Giao diện:

![alt text](img/image_1.png)

## Lưu ý

- Chạy các lệnh từ thư mục gốc repository.
- `insertion` và `selection` có độ phức tạp `O(n^2)`, có thể rất chậm ở
  payload lớn.
- Trên Windows, cơ chế timeout của benchmark dùng thread; process đang chạy
  có thể tiếp tục dùng CPU đến khi benchmark kết thúc.
- Các file trong `Output/`, thư mục `build/` và file executable không nên commit.
