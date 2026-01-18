# 🚀 Hướng dẫn Nhanh - Petri Net Analyzer

## 📦 Cài đặt Thư viện

### Ubuntu/Debian
```bash
# Cài đặt compiler và tools cơ bản
sudo apt-get update
sudo apt-get install g++ make

# Cài đặt thư viện CUDD (cho Task 3, 4, 5)
sudo apt-get install libcudd-dev

# Cài đặt ILP solver (cho Task 4, 5)
sudo apt-get install coinor-cbc
# Hoặc
sudo apt-get install glpk-utils
```

### macOS
```bash
# Cài đặt Homebrew nếu chưa có
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Cài đặt tools
brew install gcc make

# Cài đặt CUDD
brew install cudd

# Cài đặt CBC solver
brew install coin-or-tools/coinor/cbc
```

### Kiểm tra cài đặt
```bash
g++ --version        # Kiểm tra compiler
make --version       # Kiểm tra make
make check-solver    # Kiểm tra ILP solver
```

---

## ⚡ Chạy Nhanh (3 bước)

```bash
# Bước 1: Build tất cả
make build_all

# Bước 2: Chạy test nhanh (compact)
make test

# Bước 3: Xem kết quả chi tiết
make testv
```

---

## 🎯 Chạy từng Task

### Task 1 - Phân tích Petri Net
```bash
make task1          # Build
make test1          # Test (compact)
make test1v         # Test (verbose)
```

### Task 2 - Reachability Analysis
```bash
make task2          # Build
make test2          # Test BFS + DFS (compact)
make test2v         # Test chi tiết
```

### Task 3 - BDD Analysis
```bash
make task3          # Build
make test3          # Test (compact)
make test3v         # Test chi tiết
```

### Task 4 - Deadlock Detection
```bash
make task4          # Build
make test4          # Test (compact)
make test4v         # Test chi tiết
```

### Task 5 - Optimization
```bash
make task5          # Build
make test5          # Test (compact)
make test5v         # Test chi tiết
```

---

## 📁 Xem Kết quả

```bash
# Kết quả test được lưu trong thư mục output/
cat output/test_task1.txt
cat output/test_bfs.txt
cat output/test_dfs.txt
cat output/test_task3.txt
cat output/test_deadlock.txt
cat output/test_optimization.txt
```

---

## 🧹 Dọn dẹp

```bash
make clean          # Xóa tất cả file build
```

---

## 🆘 Xử lý Lỗi

### Lỗi: "CUDD library not found"
```bash
# Ubuntu
sudo apt-get install libcudd-dev

# macOS
brew install cudd

# Rebuild
make clean && make build_all
```

### Lỗi: "No ILP solver found"
```bash
# Cài CBC (khuyên dùng)
sudo apt-get install coinor-cbc

# Hoặc GLPK
sudo apt-get install glpk-utils

# Kiểm tra
make check-solver
```

### Lỗi: "Command not found: make"
```bash
# Ubuntu
sudo apt-get install make

# macOS
xcode-select --install
```

---

## 📋 Lệnh hay dùng

| Lệnh | Mô tả |
|------|-------|
| `make help` | Xem tất cả lệnh |
| `make build_all` | Build tất cả tasks |
| `make test` | Test tất cả (compact) |
| `make testv` | Test tất cả (verbose) |
| `make clean` | Dọn dẹp |
| `make check-solver` | Kiểm tra ILP solver |

---

## 💡 Tips

- Luôn chạy `make clean` trước khi build lại
- Dùng `testv` để xem chi tiết khi debug
- Kết quả lưu trong `output/` để review sau
- Chạy `make help` để xem đầy đủ lệnh

---

## 📞 Cấu trúc Project

```
PetriNetAnalyzer/
├── src/
│   ├── core/           # PetriNet cơ bản
│   ├── parser/         # PNML Parser
│   ├── explicit/       # BFS/DFS
│   ├── symbolic/       # BDD
│   ├── ILP/            # Deadlock & Optimization
│   └── tests/          # Test files
├── tests/              # Test cases (test_1 → test_6)
├── output/             # Kết quả test
└── Makefile            # Build automation
```