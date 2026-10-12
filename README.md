# FRAME - Quản lý rạp chiếu phim

Ứng dụng desktop C++17 / Qt Widgets, lưu dữ liệu bằng file TXT qua
ProjectDataEngine. Mã nguồn chia theo trách nhiệm; cấu trúc dữ liệu tự xây dựng
được giữ trong `src/DataStructures/`.

## Cấu trúc dự án

| Thư mục | Nội dung |
|---|---|
| `src/` | Mã nguồn C++, chia theo trách nhiệm, không lồng package Java |
| `src/ui/` | Giao diện Qt: cửa sổ, hộp thoại và trang chức năng |
| `src/service/` | Mỗi nghiệp vụ có cặp `.h/.cpp` riêng; request/result đặt cạnh service |
| `tests/` | Kiểm thử core, lưu trữ, nghiệp vụ và Qt |
| `src/DataStructures/` | Các cấu trúc dữ liệu tự xây dựng |
| `data/` | Dữ liệu mẫu và thư mục dự phòng; không xóa khi dọn dự án |
| `resources/` | Vị trí dành cho tài nguyên ứng dụng |
| `docs/requirements/` | BRD, SRS và use case |
| `docs/design/` | Kiến trúc, thiết kế dữ liệu và sơ đồ ERD |
| `docs/design/wireframes/` | Wireframe HTML, ảnh và kiểm tra wireframe |
| `docs/reviews/` | Kết quả rà soát yêu cầu |
| `docs/product/` | Giải thích sản phẩm hiện tại, sơ đồ Mermaid và hướng dẫn vấn đáp |
| `build/` | Kết quả build cục bộ, không phải mã nguồn |

## Bắt đầu

- [Tài liệu sản phẩm hiện tại](docs/product/README.md)
- [Lộ trình học và ôn vấn đáp từ đầu tới cuối](docs/product/11-learning-roadmap.md)
- [Build, cấp Admin và chạy demo](docs/product/02-build-and-demo.md)
- [BRD](docs/requirements/brd.md), [SRS](docs/requirements/srs.md) và [Use case](docs/requirements/use-cases.md)
- [Kiến trúc](docs/design/ARCHITECTURE.md) và [Thiết kế dữ liệu](docs/design/data-design.md)
- [Sơ đồ ERD](docs/design/database-erd.svg) và [Wireframe](docs/design/wireframes/wireframe.html)
- [Rà soát yêu cầu](docs/reviews/requirements-audit.md)

Build và kiểm thử từ thư mục gốc, sau khi cài CMake, Qt Widgets và libcrypt:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j2
ctest --test-dir build --output-on-failure
./build/cinema_app --help
```

Nếu CMake không tìm thấy Qt, chỉ core được build. Xem hướng dẫn build để
cấu hình Qt trên máy hiện tại. Kiểm tra wireframe riêng bằng
`node docs/design/wireframes/wireframe.check.cjs`.

Dữ liệu vận hành mặc định nằm ngoài repository, do Qt xác định; `data/demo/`
chỉ là seed. Không chạy ứng dụng trực tiếp vào seed hoặc xóa dữ liệu để sửa lỗi.
