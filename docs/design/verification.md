# ĐỐI CHIẾU TRIỂN KHAI VÀ KIỂM THỬ

**Phiên bản:** 1.0 - Hoàn thiện | **Ngày:** 19/10/2026 | **Căn cứ:** SRS 1.0.

## 1. Ma trận yêu cầu

| Phạm vi | Triển khai | Bằng chứng kiểm thử chính |
|---|---|---|
| `FR-FILM-*` | `CatalogService`, tab `Phim` | `ServiceTest::testCatalog`, `MainWindowTest::testFilmsAndClose` |
| `FR-ROOM-*` | `CatalogService`, tab `Phòng & ghế` | `ServiceTest::testCatalog`, `testAllocationRollback`, `MainWindowTest::testCompleteWorkflow` |
| `FR-SHOW-*` | `ScreeningService`, tab `Suất chiếu` | `ServiceTest::testScheduling`, `testCalendarAndOverflow`, `MainWindowTest::testCompleteWorkflow` |
| `FR-SEARCH-*`, `FR-SALE-*` | `TicketService`, tab `Bán vé` | `ServiceTest::testSales`, `testInvalidMapsAndReconfirmation`, `MainWindowTest::testCompleteWorkflow` |
| `FR-QUERY-*`, `FR-REPORT-*` | `TicketService`, tab `Vé và doanh thu` | `ServiceTest::testSales`, `MainWindowTest::testCompleteWorkflow` |
| `FR-DATA-*` | `TxtPersistence`, `ApplicationData`, `MainWindow::closeEvent` | Toàn bộ `ApplicationDataTest`, `MainWindowTest::testStartup`, `testFilmsAndClose` |
| `NFR-02` | Duyệt DSA trên RAM | `ServiceTest::testPerformanceBaseline` với đúng tải SRS và giới hạn 2 giây |
| `NFR-03`, `NFR-11` | Rollback, kiểm tra tràn, RAII | `RepositoryTest`, `ServiceTest::testAllocationRollback`, các test lỗi cấp phát/lifetime |
| `NFR-04`, `NFR-07` | Lưu đủ năm file, phục hồi lỗi, mở lại | `ApplicationDataTest`, `MainWindowTest::testCompleteWorkflow` |

Toàn bộ kịch bản SAT được liệt kê trong SRS 1.0 đều được các nhóm trên bao phủ.

## 2. Tám chương trình kiểm thử CTest

| CTest | Nội dung |
|---|---|
| `LinkedList` | Thêm, tìm, xóa, lifetime, exception safety và tái sử dụng template. |
| `HashTable` | Va chạm, khóa chuỗi, giá trị con trỏ, lỗi cấp phát và chỉ mục repository. |
| `Matrix2D` | Biên chỉ số, kích thước sai, lifetime và ánh xạ ghế hai chiều. |
| `Entity` | Dữ liệu hợp lệ/sai, chuẩn hóa và ngày giờ. |
| `Repository` | CRUD, ID, năm repository cụ thể và rollback khi cập nhật/cấp phát lỗi. |
| `Service` | Danh mục, xếp lịch, bán vé, tra cứu, doanh thu, rollback và hiệu năng. |
| `ApplicationData` | TXT, UTF-8, round-trip, bộ file, lỗi I/O, phục hồi và dữ liệu demo. |
| `MainWindow` | Khởi động, biểu mẫu, năm tab, luồng bán vé, danh sách vé theo ngày và lưu khi đóng. |

Các hàm kiểm thử trọng tâm:

- `ServiceTest`: `testCatalog`, `testScheduling`, `testCalendarAndOverflow`,
  `testSales`, `testAllocationRollback`, `testInvalidMapsAndReconfirmation`,
  `testPerformanceBaseline`.
- `ApplicationDataTest`: `testEmptyAndRoundTrip`, `testBadInput`,
  `testFileSetAndEncoding`, `testSaveFailures`, `testFailedRecoveryAndValidation`,
  `testRealIoFailures`, `testAllocationFailures`, `testDemoData`.
- `MainWindowTest`: `testStartup`, `testFilmsAndClose`, `testCompleteWorkflow`.

`data/demo/` được nạp thật trong `ApplicationDataTest::testDemoData`; không phải dữ
liệu được mã hóa cứng trong chương trình. Test Qt chạy bằng platform `offscreen`.

## 3. Lệnh xác minh bản bàn giao

Build đầy đủ Qt 6 ở ngoài thư mục nguồn:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

Kết quả xác minh sạch gần nhất: cấu hình `Release`, **8/8 test Passed**.

Nếu máy chỉ có toolchain C++ và chưa cài Qt, có thể kiểm tra bảy bộ test phần lõi:

```bash
cmake -S . -B build -DBUILD_GUI=OFF -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

Hướng dẫn cài Qt, build và chạy trên Ubuntu/Debian và Windows nằm trong `README.md`.
