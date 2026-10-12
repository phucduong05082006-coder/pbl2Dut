# 02. Build, triển khai và sử dụng

## 1. Điều kiện và build

Bản hiện tại được kiểm chứng trên Linux, C++17, CMake ≥3.21, Qt Widgets 5,
libcrypt có yescrypt. CMake tìm Qt6 trước Qt5; điều này **không đồng nghĩa**
đã kiểm thử Qt6/Windows/macOS. Không có crypto backend Windows. Không cần
Internet khi chạy nghiệp vụ; build cần thư viện phát triển đã cài sẵn.

Từ thư mục gốc dự án:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j2
ctest --test-dir build --output-on-failure
./build/cinema_app --help
```

Nếu CMake/Qt chưa nằm trong PATH, cấu hình thực tế trên máy hiện tại:

```bash
"$HOME/.local/bin/cmake" -S . -B /tmp/cinema-mvp-release \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_PREFIX_PATH="$HOME/.local/opt/qt5-dev/usr"
"$HOME/.local/bin/cmake" --build /tmp/cinema-mvp-release -j2
"$HOME/.local/bin/ctest" --test-dir /tmp/cinema-mvp-release --output-on-failure
```

Không tìm được Qt: CMake vẫn build core và core checks, **không tạo**
`cinema_app`. Kiểm tra thông báo cấu hình thay vì tưởng đã build GUI.
`-UNDEBUG` trên các check giữ `assert` hoạt động cả Release; nếu bỏ tùy chọn
này, các câu lệnh đặt bên trong assert có thể bị loại hoàn toàn.

## 2. Dữ liệu vận hành và Admin đầu tiên

Mặc định Qt chọn `QStandardPaths::AppLocalDataLocation`; trên Linux hiện tại
là `$HOME/.local/share/FRAME/CinemaManagement`. Thanh đầu cửa sổ ghi đường
dẫn thực tế. `--data-dir` chọn một thư mục khác, không thay mọi thư mục khác.

```bash
./build/cinema_app --setup-admin --data-dir "$HOME/.local/share/FRAME/CinemaManagement"
./build/cinema_app --data-dir "$HOME/.local/share/FRAME/CinemaManagement"
```

Chế độ setup là bước **triển khai một lần**: username, tên hiển thị, mật
khẩu/nhập lại và checkbox quyền bán riêng (mặc định tắt). Nó bị từ chối nếu
bảng User đã có bất kỳ tài khoản nào. Không có tài khoản mặc định của dữ liệu
thật; không nhét tài khoản demo vào bộ vận hành. Lần chạy bình thường chỉ
có màn hình đăng nhập username/password, không buộc tên hiển thị.

Nếu tất cả bảy bảng chưa tồn tại và không có backup/journal, engine tạo
header rỗng. Nếu một bảng mất giữa bộ dữ liệu, file 0 byte, header sai, FK
sai hoặc tổng vé không khớp: dừng mở dữ liệu và báo lỗi, không coi là “rỗng”.
Không xóa dữ liệu hoặc sửa header để lách lỗi; sao lưu nguyên thư mục rồi
phục hồi từ **một bộ nhất quán**, xem chương 04/07.

`QLockFile` chặn hai app/setup/demo cùng một thư mục. Khóa tồn tại suốt vòng
đời ứng dụng, có canonical path chống hai đường symlink cùng đích. Thông báo
khóa/lỗi thư mục được in ra stderr khi mở từ terminal. Chỉ khóa entrypoint
app; chương trình tự viết gọi trực tiếp cms_core phải tuân thủ một writer.

## 3. Demo đầy đủ để trình bày

```bash
DEMO_DIR="$(mktemp -d /tmp/frame-demo.XXXXXX)"
./build/cinema_app --prepare-demo --data-dir "$DEMO_DIR"
./build/cinema_app --data-dir "$DEMO_DIR"
```

- Admin: `manager` / `demo123`; nhân viên: `seller01` / `demo123`.
- 3 phim; 3 phòng 6×8, 5×8, 6×8; 136 ghế STANDARD.
- 6 suất OPEN lúc 19:00 ngày mai/ngày kia tính theo ngày chạy lệnh.
- 3 giao dịch mẫu, mỗi phòng ngày mai bán A01/A02: 6 vé, 480.000 VND.
- Chọn ngày báo cáo **ngày tạo demo** để thấy doanh thu mẫu, không chọn
  ngày chiếu rồi cho rằng doanh thu biến mất.
- Lệnh chỉ chấp nhận thư mục trống; không ghi đè hoặc xóa dữ liệu đã có.
  `CMS_DEMO_SOURCE` trỏ tới seed trong checkout lúc build. Nếu chuyển mỗi
  executable sang máy khác mà không có checkout, cần kèm seed hoặc build
  lại; bộ dữ liệu demo đã tạo không còn phụ thuộc đường source này.

`data/demo` bản tĩnh chỉ là seed (không có suất/vé). Không chạy app trực tiếp
vào seed rồi sửa nó nếu muốn giữ bộ mẫu. Lệnh prepare tạo bản độc lập.

## 4. Thao tác quản lý

1. Đăng nhập Admin, vào tổng quan. Tạo phim (ID/title/duration/age/status).
2. Tạo phòng với ID/name/hàng/cột; capacity tự tính, ghế tự sinh A01…
3. Tạo lịch chọn phim/phòng ACTIVE, giờ tương lai, giá dương, DRAFT/OPEN.
4. Xem end và room available. Thử giờ chồng lịch để thấy bị từ chối.
5. Mở/đóng bán. Hoàn tất chỉ sau end; suất COMPLETED không mở lại.
6. Tạo nhân viên và cấp quyền bán trong Tài khoản & quyền.

UI tạo phòng giới hạn 26 hàng/100 cột; backend bảo vệ số nguyên và rectangular
grid. Khi sửa phòng đã load có >100 cột, UI giữ được giá trị cũ, không tự
ép nhỏ. Đã có lịch thì không đổi layout; có lịch tương lai chưa xử lý thì
không ngừng phòng/phim. Không có xóa lịch/cancel/di dời tự động trong MVP.

## 5. Thao tác nhân viên

1. Chọn film/room/date, bấm tải/tìm suất. Date trống là mọi ngày; nhập
   `YYYY-MM-DD` hợp lệ. Chỉ OPEN, chưa bắt đầu, phim/phòng ACTIVE xuất hiện.
2. Chọn suất, xem thời lượng, giờ kết thúc, giá, ageRating.
3. Chọn ghế Trống; dùng Ctrl/click hoặc phím chọn của bảng Qt để chọn nhiều.
   “Đã bán”/“Không dùng” không chọn được; trạng thái không chỉ dựa màu.
4. Bấm kiểm tra tiền. Dialog có phim/suất/phòng/ghế/giá/tổng và người bán.
5. Chỉ tick “đã nhận đủ tiền mặt” khi thật sự đã nhận; confirm mặc định tắt.
6. Thành công nhận transactionId/ticketId; quay map sẽ thấy ghế Đã bán.
7. Hủy thì không ghi và không giữ ghế; lỗi ghi thì không hiện receipt thành
   công. Chỉ kiểm tra/chọn lại, không gửi bán lần nữa một cách mù quáng.
8. Tra mã giao dịch để thấy tất cả vé, hoặc mã vé để thấy riêng vé ấy.
   “Tổng giao dịch” ở lookup một vé vẫn là tổng transaction, không phải giá
   riêng của vé (giá riêng nằm ở dòng vé).

## 6. Báo cáo và kết thúc phiên

Manager chọn từ/đến **ngày bán** (createdAt), film và screening. Revenue chỉ
giao dịch COMPLETED/CONFIRMED; bảng lấp đầy là tất cả vé của suất, không cắt
theo ngày bán, đúng nhãn giải thích trên màn hình. Mẫu số là capacity vật lý.
Khoảng from>to bị từ chối và kết quả cũ bị xóa để tránh nhìn nhầm.

Đăng xuất xóa phiên và quay login; đóng cửa sổ ứng dụng thì thoát. Dữ liệu
được lưu khi xác nhận mutation, không chờ nút “save khi thoát”. Trước khi
nộp bài, tạo demo mới, bán thêm một vé, đóng/mở lại cùng `--data-dir`, tra
lại mã vé và kiểm tra report.
