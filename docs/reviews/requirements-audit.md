# Đối chiếu BRD/SRS với sản phẩm hiện tại

> **Cập nhật 12/10/2026:** nội dung từ mục 1 trở đi là bằng chứng lịch sử
> của bản trước sửa, không phải kết luận về mã hiện tại. Các lỗi P1-01,
> P1-02, P1-03, P2-01, P2-02 và P2-03 đã được sửa và có regression test.
> Ma trận hiện tại, bằng chứng nghiệm thu và giới hạn còn lại nằm trong
> `docs/product/01-requirements.md` và `docs/product/07-verification.md`.
> BRD/SRS không bị sửa để hợp thức hóa triển khai.

## Trạng thái sau sửa — 12/10/2026

- Đã nối đầy đủ luồng Qt danh mục/phòng/lịch/quầy/lookup/report/tài khoản,
  giữ login chỉ username/password và quyền bán tách khỏi role.
- Load/save đối soát vị trí ghế, capacity, thời gian/overlap, trạng thái
  giao dịch, đủ vé và tổng tiền; lỗi ghi không tạo receipt thành công.
- Cấp Admin bằng chế độ triển khai riêng `--setup-admin`, demo có suất tương
  lai bằng `--prepare-demo`; thư mục cũ không bị ghi đè, app giữ QLockFile.
- Debug, Release và ASan/UBSan đều 16/16 CTest đạt; core-only 8/8 đạt.
  Leak detection tắt trong sanitizer, chưa có cam kết crash/mất điện.
- Đã đo fixture 1000 phim/100 suất/500 vé; search/query/report và sale dưới
  ngưỡng 2s/3s trên máy hiện tại. Không suy rộng sang mọi quy mô.
- Các giới hạn/chỗ cần chốt còn lại: genre BRD khác model SRS; trạng thái
  phê duyệt tài liệu; legacy plaintext migrate-on-login; TXT không fsync;
  search chưa Trie/n-gram/bỏ dấu; containers raw chưa bảo đảm trước OOM.
- Tài liệu vấn đáp tại `docs/product/README.md`, giải thích ý nghĩa thiết kế,
  thuật toán, vòng đời và kiểm thử bằng lời kết hợp sơ đồ Mermaid; không
  chép lại mã nguồn vào tài liệu.

**Các mục 1–9 dưới đây lưu bằng chứng trước sửa ngày 11/10/2026.**

Ngày rà soát: 11/10/2026. Nguồn yêu cầu: `docs/requirements/brd.md` và `docs/requirements/srs.md` trong workspace.

Các số dòng trong nhận xét lịch sử thuộc bản tài liệu lúc rà soát; đường dẫn
đã cập nhật theo cấu trúc `docs/`, số dòng không phải vị trí của bản hiện tại.
Phạm vi: mã C++/Qt hiện tại, lớp service, DataEngine, TXT và kiểm thử hiện có.
Đây là báo cáo rà soát, không phải thay đổi yêu cầu hoặc phê duyệt nghiệp vụ mới.
Không sửa mã sản phẩm, tài khoản hay dữ liệu vận hành trong lần rà soát này.

## 1. Phát hiện ưu tiên

### P1-01. Mất file vé vẫn được coi là mở dữ liệu thành công

- Yêu cầu: BR-13, BR-14; FR-DATA-02..04; NFR-03, NFR-07; ERR-10.
- `src/repository/txt/TxtDatabase.cpp:176`: file không tồn tại
  hoặc file 0 byte được chấp nhận; `readRows` trả thành công với bảng rỗng.
- `src/repository/txt/TxtDatabase.cpp:437`: chỉ kiểm tra tham
  chiếu của vé có mặt; không kiểm tra giao dịch COMPLETED phải có đủ vé.
- Tái hiện trên bản sao tạm: bán một vé, xóa `tickets.txt`, mở lại. Kết quả:
  `open=1, transactions=1, tickets=0, sold=0`.
- Hệ quả: một giao dịch đã xác nhận còn tồn tại nhưng ghế trở lại trống;
  ứng dụng có thể tiếp tục vận hành trên dữ liệu thiếu. Lần lưu sau ghi cả
  bảy bảng, nên có nguy cơ cố định trạng thái thiếu vào TXT/backup mới.
- Cần phân biệt bộ dữ liệu mới hoàn toàn với bộ đã vận hành bị mất/trống file;
  không thể áp dụng quy tắc "thiếu file là bảng rỗng" cho cả hai.

### P1-02. Đọc TXT không kiểm tra tính toàn vẹn giao dịch

- Yêu cầu: BR-09, BR-14; BRD RULE-14..17, RULE-21; SRS RULE-08..10;
  NFR-03, NFR-10; SRS mục 12.3.
- `src/repository/txt/TxtDatabase.cpp:463`: giao dịch chỉ
  kiểm tra phim/suất/người tham chiếu; không so tổng tiền với các vé.
- `src/repository/txt/TxtDatabase.cpp:472`: vé chỉ kiểm tra
  quan hệ, không từ chối vé ISSUED thuộc giao dịch FAILED/UNCONFIRMED.
- Tái hiện: đổi tổng tiền giao dịch 80.000 thành 1, ứng dụng vẫn `open=1`.
- Tái hiện: đổi giao dịch thành FAILED/UNCONFIRMED nhưng giữ vé ISSUED,
  vẫn `open=1`. Sơ đồ bỏ qua vé này, trong khi `SaleService::preview` xét
  mọi vé của suất để từ chối ghế đã bán: các lớp diễn giải dữ liệu khác nhau.
- Kiểm tra tốt khi tạo giao dịch mới không thay thế được kiểm tra khi nạp lại.

### P1-03. Qt chưa có chuỗi nghiệp vụ cốt lõi để vận hành rạp

- Yêu cầu: BR-03..11; FR-SHOW, FR-SEARCH, FR-SALE, FR-QUERY, FR-REPORT;
  SRS mục 7 và SAT-01..12.
- `src/ui/mainwindow/MainWindow.cpp:126` chỉ
  tạo ba trang: Phim, Tài khoản & quyền, Phòng & ghế.
- Chưa có tổng quan, quản lý suất chiếu/giá, tra cứu suất, bán vé, kết quả
  phát hành vé, tra cứu vé/giao dịch hay báo cáo trong Qt.
- `src/CinemaApplication.cpp:43` chỉ nối Auth, Film,
  Auditorium tới controller/Qt; Screening/Sale/Report chưa được nối.
- Có service và unit test không có nghĩa nhân viên đã làm được nghiệp vụ
  trên app. Hiện chưa thể nghiệm thu MVP theo BRD/SRS.

### P2-01. Cấu hình phòng/ghế và thời gian sai vẫn lọt qua lúc khởi động

- Yêu cầu: FR-ROOM-02..03, FR-SHOW-02..05, NFR-10; ERR-01, ERR-10.
- `src/repository/txt/TxtDatabase.cpp:503`: parser kiểm tra
  kiểu số/trạng thái nhưng không áp đầy đủ điều kiện miền dữ liệu khi load.
- `src/repository/txt/TxtDatabase.cpp:375`: kiểm tra trùng
  seat_code, chưa kiểm tra trùng cặp vị trí row_label/seat_number khi load.
- Tái hiện độc lập: phòng 6x8 nhưng capacity=49; hai ghế khác mã ở cùng A1;
  start_time=`invalid`. Cả ba bộ đều được mở thành công.
- `AuditoriumController::details` chặn một phần kích thước/số ghế tại màn hình
  xem phòng, nhưng đó không phải kiểm tra toàn bộ database lúc khởi động;
  kiểm tra này cũng không phát hiện trùng vị trí cùng số lượng ghế.

### P2-02. Công thức tỷ lệ lấp đầy khác SRS

- Yêu cầu: FR-REPORT-05 tại `docs/requirements/srs.md:191`: số ghế đã bán / sức chứa phòng.
- Bản trước sửa, `ReportService::occupancy` (nay trong `src/service/ReportService.cpp`) dùng số ghế ACTIVE
  làm mẫu số thay vì `Auditorium.capacity`.
- Tái hiện bằng API hợp lệ: phòng capacity=48, ngừng một ghế trước khi lập
  suất, bán một vé. Service trả 1/47 = 2,12766%; theo SRS là 1/48 = 2,08333%.
- Cần sửa công thức theo SRS, hoặc xin chốt lại định nghĩa sức chứa có thể
  bán. Không tự thay định nghĩa nghiệp vụ.

### P2-03. Phòng/ghế mới chỉ xem; công cụ tra cứu và lịch còn thiếu ở service

- FR-ROOM-01, FR-ROOM-05: Qt chưa có tạo phòng, tạo sơ đồ, sửa trạng thái;
  `AuditoriumController` chỉ có list/details dù service có save.
- FR-SHOW-07: service chỉ có danh sách suất OPEN còn ở tương lai, chưa có
  danh sách quản lý/tìm kiếm DRAFT, CLOSED, COMPLETED qua API service.
- FR-QUERY-03..04: `SaleService::findTicket/findTransaction` chỉ trả entity
  hoặc false; chưa có kết quả đầy đủ tên phim, phòng, ghế, người bán và lỗi
  tìm kiếm rõ ràng để phục vụ màn hình nghiệp vụ.

## 2. Cách đánh giá

- **Đạt hiện tại**: chức năng có trong phần sản phẩm tương ứng và không phát
  hiện sai lệch cụ thể trong lần kiểm tra; không phải chứng nhận mọi đầu vào.
- **Có lõi, thiếu Qt**: có xử lý ở service/storage, người dùng chưa thao tác
  được trên app. Không tính là hoàn thành yêu cầu cấp sản phẩm.
- **Một phần**: thiếu hành vi, bằng chứng hoặc có lỗi khiến chưa thể nghiệm thu.
- **Không đạt**: tái hiện được hành vi trái yêu cầu hoặc chưa có chức năng.
- **Ngoài MVP**: BRD xác định mở rộng, SRS loại khỏi phiên bản đầu.

Không quy đổi số service, số file hoặc số test xanh thành phần trăm hoàn thiện.

## 3. Ma trận toàn bộ yêu cầu BRD

### 3.1. Mục tiêu và quy trình

| Mục tiêu | Đánh giá cấp sản phẩm |
|---|---|
| BO-01 | Chưa quản lý tập trung trọn luồng: phim có, phòng chỉ xem, lịch chưa có Qt. |
| BO-02 | Lõi chống chồng lịch có, chưa vận hành qua giao diện, load thời gian còn yếu. |
| BO-03 | Chưa có quy trình bán vé tại quầy trên Qt. |
| BO-04 | Giao dịch mới chống trùng có; mất file vé lúc mở lại vẫn lọt. |
| BO-05 | Chưa tra cứu thông tin bán vé đầy đủ trên Qt. |
| BO-06 | Chưa có báo cáo Qt; còn lỗi đối soát dữ liệu và mẫu số lấp đầy. |
| BO-07 | Có vai trò/guard; nghiệm thu tổng thể còn phụ thuộc module chưa triển khai. |

| Quy trình ở mục 7 BRD | Đánh giá |
|---|---|
| BP-01 Danh mục | Phim có; tạo/cấu hình phòng chưa có UI. |
| BP-02 Lịch chiếu | Lõi có, chưa có quy trình lập/công bố lịch trên Qt. |
| BP-03 Bán vé | Chưa có quy trình Qt, dù lõi preview/commit có. |
| BP-04 Tra cứu vé/giao dịch | Mới có lookup entity, chưa đủ thông tin/màn hình. |
| BP-05 Báo cáo | Mới có phép tổng hợp lõi, chưa có báo cáo Qt và còn lệch công thức. |

BRD dùng tiền tố BP hai lần: mục 2 là Business Problems BP-01..06,
mục 7 là Business Processes BP-01..05. Bảng này ghi rõ mục 7, không gộp hai
bộ mã. Các vấn đề ban đầu về lịch/ghế/giao dịch/doanh thu vẫn chưa giải quyết
trọn ở cấp sản phẩm; chỉ phần phân quyền và danh mục đã thấy trên Qt.

SC-01..10 được đối chiếu lần lượt qua BR-01..14; SC-03..08 thiếu luồng Qt.
RISK-01..05 còn cần kiểm soát qua validation lịch, ghế, tiền, load/restore và
guard; RISK-06 cần giữ phạm vi MVP, RISK-07 cần chốt các điểm ở mục 8 báo cáo.

### 3.2. BR-01..18

| ID | Nhu cầu | Sản phẩm hiện tại |
|---|---|---|
| BR-01 | Quản lý tập trung phim | Có danh sách, tìm, chi tiết, tạo, sửa, ngừng phim trên Qt. |
| BR-02 | Phòng, sức chứa, vị trí ghế | Một phần: xem trên Qt; tạo/sửa chỉ ở service; load còn lỗi cấu hình. |
| BR-03 | Tổ chức/công bố lịch | Có lõi tạo/chuyển trạng thái; thiếu controller/Qt. |
| BR-04 | Không chồng lịch | Có kiểm tra trong lõi; dữ liệu thời gian hỏng vẫn có thể được load. |
| BR-05 | Tra cứu suất và ghế của suất | Có lõi; Qt hiện chỉ có sơ đồ phòng, không phải sơ đồ suất. |
| BR-06 | Giá bán từng suất | Có trường giá và kiểm tra trong lõi; không nhập/xem qua màn hình suất. |
| BR-07 | Bán nhiều vé một giao dịch | Có lõi và kiểm tra lưu; không có quầy bán vé Qt. |
| BR-08 | Không bán trùng ghế | Có kiểm tra lõi; mất tickets.txt khi mở lại có thể làm ghế thành trống. |
| BR-09 | Thanh toán thành công mới có vé/doanh thu | Có kiểm tra giao dịch mới; load chấp nhận trạng thái không nhất quán. |
| BR-10 | Tra cứu vé/giao dịch | Lookup entity có; thiếu kết quả đầy đủ và màn hình tra cứu. |
| BR-11 | Báo cáo theo thời gian/phim/suất | Có lọc/tổng hợp lõi; thiếu Qt, tỷ lệ lấp đầy sai định nghĩa. |
| BR-12 | Phân quyền nghiệp vụ | Đúng với thao tác đã nối; module vận hành còn thiếu để nghiệm thu tổng thể. |
| BR-13 | Bảo toàn cho lần sử dụng sau | Lưu/đọc hợp lệ có, rollback/backup có; không đạt xử lý mất/trống file. |
| BR-14 | Giao dịch/vé/ghế/doanh thu nhất quán | Một phần khi ghi mới; không đạt kiểm tra khi nạp dữ liệu hỏng. |
| BR-15 | Hủy/hoàn vé | Ngoài MVP; không có là đúng phạm vi hiện tại. |
| BR-16 | Soát vé | Ngoài MVP. |
| BR-17 | Chính sách giá linh hoạt | Ngoài MVP; một giá mỗi suất là đúng. |
| BR-18 | Đối soát ca thu ngân | Ngoài MVP. |

## 4. Ma trận toàn bộ 60 yêu cầu chức năng SRS

SRS có 59 Must và 1 Should. Ma trận phân biệt backend với sản phẩm Qt.

### 4.1. Xác thực và tài khoản

| ID | Đánh giá | Bằng chứng / khoảng thiếu |
|---|---|---|
| FR-AUTH-01 | Đạt hiện tại | LoginDialog chỉ username/password; không đăng ký hoặc yêu cầu display_name. |
| FR-AUTH-02 | Đạt hiện tại | Vai trò/quyền lấy từ tài khoản, không chọn vai trò lúc đăng nhập. |
| FR-AUTH-03 | Đạt hiện tại | Sai username/password hoặc INACTIVE bị từ chối; lỗi chung, xóa ô mật khẩu. |
| FR-AUTH-04 | Đạt hiện tại | Logout xóa session, đóng workspace và quay lại đăng nhập. |
| FR-AUTH-05 | Đạt hiện tại | Admin tạo/sửa/khóa tài khoản, cấp/thu hồi quyền bán qua AccountPage. |
| FR-AUTH-06 | Đạt trong phần đã có | Service kiểm tra quyền mỗi thao tác; chưa nghiệm thu module Qt chưa triển khai. |

### 4.2. Phim

| ID | Đánh giá | Bằng chứng / khoảng thiếu |
|---|---|---|
| FR-FILM-01 | Đạt hiện tại | Tạo phim đủ trường và mã riêng qua FilmPage/FilmService::create. |
| FR-FILM-02 | Đạt hiện tại | Chặn rỗng, thời lượng không dương, mã tạo trùng. |
| FR-FILM-03 | Đạt hiện tại | Danh sách, tìm theo mã/tên, bộ lọc trạng thái, chi tiết. |
| FR-FILM-04 | Đạt, có hạn chế | Có sửa; chặn đổi thời lượng với mọi lịch, chặt hơn trường hợp đã bán vé. |
| FR-FILM-05 | Đạt hiện tại | Chặn ngừng phim còn lịch chưa hoàn tất bị ảnh hưởng. |
| FR-FILM-06 | Đạt qua service hiện tại | Không có xóa phim; bảo vệ thông tin ảnh hưởng lịch đã tồn tại. |

### 4.3. Phòng và ghế

| ID | Đánh giá | Bằng chứng / khoảng thiếu |
|---|---|---|
| FR-ROOM-01 | Có lõi, thiếu Qt | Service save được phòng/ghế; không có form tạo hoặc adapter create riêng. |
| FR-ROOM-02 | Một phần | Save kiểm tra vị trí/mã; load bỏ lọt trùng vị trí. |
| FR-ROOM-03 | Một phần | Save kiểm tra rows*columns=capacity và số ghế; load bỏ lọt sai capacity. |
| FR-ROOM-04 | Đạt với bộ hợp lệ | Qt xem danh sách phòng, sức chứa, sơ đồ và trạng thái. |
| FR-ROOM-05 | Có lõi, thiếu Qt | Có save trạng thái với bảo vệ lịch; chưa có thao tác quản lý phòng trên Qt. |
| FR-ROOM-06 | Có lõi, thiếu Qt | Service chặn ngừng phòng còn lịch bị ảnh hưởng. |

### 4.4. Suất chiếu

| ID | Đánh giá | Bằng chứng / khoảng thiếu |
|---|---|---|
| FR-SHOW-01 | Có lõi, thiếu Qt | Tạo bằng phim/phòng/thời gian/giá qua ScreeningService::create. |
| FR-SHOW-02 | Một phần | Create tính end từ duration; load không kiểm tra lại thời gian/thời lượng. |
| FR-SHOW-03 | Một phần | Create cộng 15 phút; load không kiểm tra đủ khoảng nghỉ. |
| FR-SHOW-04 | Có lõi, thiếu Qt | Kiểm tra chồng khoảng sử dụng phòng khi lưu; chưa có màn hình. |
| FR-SHOW-05 | Một phần | Create kiểm tra phim/phòng ACTIVE, thời gian và giá; load yếu hơn. |
| FR-SHOW-06 | Có lõi, thiếu Qt | Có DRAFT/OPEN và chặn bán không OPEN; chưa có thao tác công bố trên app. |
| FR-SHOW-07 | Một phần | Có changeStatus; danh sách hiện chỉ OPEN tương lai, thiếu màn hình/danh sách quản lý. |
| FR-SHOW-08 | Có lõi, thiếu Qt | Storage chặn đổi phim/phòng/thời gian của suất có vé; không có thao tác sửa lịch trên Qt. |
| FR-SHOW-09 | Có lõi, thiếu Qt | Cho manager chuyển CLOSED; sale chặn now>=start; không tự đổi status trong TXT. |

### 4.5. Tra cứu lịch và ghế

| ID | Đánh giá | Bằng chứng / khoảng thiếu |
|---|---|---|
| FR-SEARCH-01 | Có lõi, thiếu Qt | openScreeningsFor lọc ngày/phim/phòng; chưa có tra cứu suất trên app. |
| FR-SEARCH-02 | Không đạt ở Qt | Entity có trường nhưng chưa có màn hình hiển thị đầy đủ suất chiếu. |
| FR-SEARCH-03 | Có lõi, thiếu Qt | Matrix2D theo screening có O/X/-; AuditoriumPage không dùng sơ đồ này. |
| FR-SEARCH-04 | Có lõi, thiếu Qt | Map phân biệt vị trí không dùng/trống/đã bán; chưa render theo suất. |

### 4.6. Bán vé

| ID | Đánh giá | Bằng chứng / khoảng thiếu |
|---|---|---|
| FR-SALE-01 | Có lõi, thiếu Qt | Kiểm tra quyền bán, OPEN và chưa bắt đầu; không chọn suất được trên app. |
| FR-SALE-02 | Có lõi, thiếu Qt | SaleRequest chứa nhiều seatIds, preview kiểm tra; chưa có chọn ghế. |
| FR-SALE-03 | Một phần | Giao dịch mới kiểm tra ghế trùng/sai phòng/đã bán; dữ liệu lỗi có thể làm map và preview bất đồng. |
| FR-SALE-04 | Có lõi, thiếu Qt | Cộng đơn giá từng ghế, kiểm tra tràn số. |
| FR-SALE-05 | Không đạt ở Qt | Preview chỉ trả giao dịch/vé; chưa có trang kiểm tra phim/phòng/suất/ghế/tiền. |
| FR-SALE-06 | Có lõi hỗ trợ, thiếu Qt | Preview không ghi dữ liệu; chưa có quy trình hủy lựa chọn trên màn hình bán. |
| FR-SALE-07 | Có lõi, thiếu Qt | paymentConfirmed bắt buộc; chưa có bước xác nhận đã nhận tiền. |
| FR-SALE-08 | Có lõi, thiếu Qt | Sinh mã giao dịch/vé, lưu thành công; chưa có kết quả vé cho nhân viên/khách. |
| FR-SALE-09 | Một phần | Mutation có rollback/backup; load vẫn chấp nhận giao dịch thành công thiếu vé. |
| FR-SALE-10 | Có lõi, thiếu Qt | Không thanh toán/ghi lỗi không trả receipt thành công; thiếu nghiệm thu UI. |
| FR-SALE-11 | Có lõi, thiếu Qt | Người bán từ session, thời điểm từ clock; không lấy giả từ input UI. |
| FR-SALE-12 | Có lõi, thiếu Qt | Không giữ ghế khi preview/thất bại; thiếu màn hình để nghiệm thu hủy. |

### 4.7. Tra cứu vé và giao dịch

| ID | Đánh giá | Bằng chứng / khoảng thiếu |
|---|---|---|
| FR-QUERY-01 | Có lookup, thiếu Qt | findTransaction theo ID, yêu cầu login. |
| FR-QUERY-02 | Có lookup, thiếu Qt | findTicket theo ID, yêu cầu login. |
| FR-QUERY-03 | Không đạt | Chưa có kết quả join/DTO và màn hình đủ phim, phòng, ghế, giá, người bán. |
| FR-QUERY-04 | Không đạt | Lookup trả false, chưa có phân biệt input lỗi/không tìm thấy và thông báo UI. |
| FR-QUERY-05 | Đạt ở API sản phẩm | Không cung cấp xóa/sửa giao dịch xác nhận; sửa tay TXT còn cần phát hiện dữ liệu sai. |

### 4.8. Báo cáo

| ID | Đánh giá | Bằng chứng / khoảng thiếu |
|---|---|---|
| FR-REPORT-01 | Có lõi, thiếu Qt | revenueByFilm nhận from/to và kiểm tra khoảng thời gian. |
| FR-REPORT-02 | Có lõi, thiếu Qt | RevenueQuery nhận filmId/screeningId. |
| FR-REPORT-03 | Một phần | Có tổng vé/doanh thu và occupancy riêng; chưa có báo cáo hiển thị tổng hợp. |
| FR-REPORT-04 | Có lõi, thiếu Qt | Chỉ cộng vé thuộc transaction COMPLETED + CONFIRMED; cần chặn dữ liệu tiền sai khi load. |
| FR-REPORT-05 | Không đạt | Mẫu số là số ghế ACTIVE, khác capacity theo SRS. |
| FR-REPORT-06 | Đạt giới hạn lõi | Không tính lợi nhuận/chi phí; giao dịch FAILED bị loại khỏi tổng hợp. |

### 4.9. Dữ liệu và khôi phục

| ID | Đánh giá | Bằng chứng / khoảng thiếu |
|---|---|---|
| FR-DATA-01 | Đạt cơ chế lõi | Bảy bảng TXT cục bộ; không DBMS/network. |
| FR-DATA-02 | Một phần | Bộ hợp lệ đọc lại được; không từ chối đầy đủ dữ liệu đã mất/hỏng. |
| FR-DATA-03 | Một phần | Commit mới có bảo vệ; mở lại không đối soát đủ giao dịch/vé/tiền. |
| FR-DATA-04 | Không đạt | File thiếu/0 byte được xem là bảng rỗng, không báo lỗi phù hợp. |
| FR-DATA-05 | Có lõi, thiếu Qt bán vé | Save kiểm tra lỗi và rollback; chưa có luồng bán Qt để nghiệm thu thông báo. |
| FR-DATA-06 | Có cơ chế lõi (Should) | Có .bak, write_pending.txt, khôi phục; chưa có thao tác backup/restore cho người dùng. |

## 5. Quy tắc bắt buộc và các quyết định đã chốt

### 5.1. Toàn bộ quy tắc SRS

| ID | Đối chiếu |
|---|---|
| RULE-01 | ID được kiểm tra trùng khi tạo/load; room save hiện là upsert, cần create riêng khi nối UI. |
| RULE-02 | Ghi mới kiểm tra ghế; load kiểm tra mã nhưng bỏ lọt trùng vị trí trong phòng. |
| RULE-03 | Có khóa ngoại suất -> phim/phòng và kiểm tra khi load. |
| RULE-04 | Create tính +15 phút và kiểm tra overlap; load không kiểm tra định dạng/phép tính thời gian đầy đủ. |
| RULE-05 | SaleService chặn không OPEN hoặc đã bắt đầu. |
| RULE-06 | Có kiểm tra cặp suất/ghế; mất file vé sau bán không được phát hiện. |
| RULE-07 | Chặn seatIds trùng trong một sale và cặp suất/ghế trùng trong TXT. |
| RULE-08 | Commit so tổng vé/giao dịch; load không so lại. |
| RULE-09 | Commit mới cần paymentConfirmed; load cho ISSUED thuộc giao dịch chưa xác nhận. |
| RULE-10 | Giao dịch mới thất bại rollback; load vẫn cho trạng thái FAILED kèm vé. |
| RULE-11 | Không có API UI/service xóa/sửa giao dịch thành công. |
| RULE-12 | Ticket lưu unitPrice; không tính lại vé cũ theo giá suất. |
| RULE-13 | Kiểm tra manager ở ScreeningService/ReportService; chưa có Qt tương ứng. |
| RULE-14 | canSellTicket độc lập role; manager không mặc nhiên được bán. |
| RULE-15 | saveScreening chặn thay phim/phòng/thời gian khi có vé. |

BRD RULE-01..27 đều được bao quát bởi các nhóm trên: mã/ghế/phim/phòng
(01..06), lịch (07..10), bán vé (11..18), tiền/truy vết/bảo toàn giá
(19..23), phân quyền (24..27). Những lỗ hổng load ảnh hưởng trực tiếp các quy
tắc này dù đường tạo mới đã có kiểm tra.

### 5.2. OQ-01..09 trong SRS

| ID | Đối chiếu |
|---|---|
| OQ-01 | Đúng trong sale: now>=start bị từ chối. |
| OQ-02 | Đúng khi tạo: khoảng nghỉ cố định 15 phút; load chưa đối soát. |
| OQ-03 | Đúng: chỉ STANDARD. |
| OQ-04 | Đúng giới hạn: lưu ageRating, không kiểm tra giấy tờ trong app. |
| OQ-05 | Đúng phạm vi: không hủy/hoàn vé. |
| OQ-06 | Đúng trong lõi: preview không giữ ghế, commit mới bán ghế. |
| OQ-07 | Đúng: lưu người/thời điểm, không quản lý ca. |
| OQ-08 | Đúng: manager bán khi canSellTicket=true. |
| OQ-09 | Có guard ngừng phim/phòng trong service; Qt chỉ có ngừng phim. |

## 6. Chất lượng, cấu trúc và dữ liệu logic

| Nhóm | Kết quả |
|---|---|
| NFR-01 Offline | Kiến trúc không gọi network; các phần đã triển khai chạy cục bộ. Nghiệp vụ Qt còn thiếu. |
| NFR-02 Hiệu năng | Chưa có benchmark có quy mô dữ liệu xác định cho ngưỡng 2s/3s. Test xanh không chứng minh ngưỡng này. |
| NFR-03 Toàn vẹn | Không đạt đầy đủ: load cho COMPLETED thiếu vé hoặc tiền không khớp. |
| NFR-04 Bảo toàn | Có kiểm thử đóng/mở lại hợp lệ, rollback lỗi ghi; chưa đủ trước mất/trống file. |
| NFR-05 Phân quyền | Có guard ở service cho thao tác hiện tại, không chỉ ẩn widget. |
| NFR-06 Truy vết | Có transactionId/createdAt/createdBy; thiếu màn hình tra cứu thông tin đầy đủ. |
| NFR-07 Lỗi dữ liệu | Có báo lỗi header/enum/FK/ghi; file thiếu/trống lại bị bỏ qua. |
| NFR-08 Dễ bán vé | Không đạt ở sản phẩm: chưa có quy trình quầy bán vé Qt. |
| NFR-09 Mật khẩu | Tạo mới dùng yescrypt, UI không đưa hash ra; còn nhận plaintext legacy và chỉ migrate khi login thành công. Chưa thể nói mọi dữ liệu TXT luôn đã hash. |
| NFR-10 Kiểm tra TXT | Có header, số trường, enum, ID/FK; thiếu vị trí ghế, miền số/thời gian và đối soát tiền/trạng thái. |

- BQR-01/BQR-02 còn vướng tính đúng/nhất quán khi nạp lại; BQR-03 có cơ chế
  lưu nhưng thiếu bảo vệ mất file; BQR-04 có guard lõi; BQR-05 chưa có quầy vé;
  BQR-06 có dữ liệu truy vết nhưng thiếu UI; BQR-07 đúng hướng offline.
- C++17 + Qt Widgets đúng lựa chọn dự án. Core dùng DLL/HashTable/BST/Matrix2D
  tự xây dựng, không dùng container STL/Qt làm kho thực thể chính.
- Các service đi qua một ProjectDataEngine; không có DBMS hay cơ chế query
  riêng trong mỗi widget. Search chung hiện Exact/Prefix/Contains, chỉ fold
  hoa/thường ASCII. Trie/N-gram/fuzzy không được ghi thành Must trong BRD/SRS;
  không tự coi chúng là điều kiện nghiệm thu.
- Bảy entity và enum hiện bao quát trường tối thiểu/quan hệ SRS mục 6.
  Có model đúng không đồng nghĩa các ràng buộc của model được kiểm tra lúc load.
- BST thời gian được dựng nhưng query thông thường vẫn duyệt DLL; chưa có
  bằng chứng hiệu năng. Giới hạn một máy/một phiên là đúng phạm vi; chưa thử
  crash/mất điện để cam kết độ bền vật lý. Không có fsync trong đường ghi TXT.

## 7. Giao diện, lỗi và nghiệm thu

### 7.1. Màn hình tối thiểu theo SRS mục 7

| Màn hình | Hiện trạng |
|---|---|
| Đăng nhập | Có, chỉ username/password. |
| Trang tổng quan quản lý | Chưa có; mặc định mở Phim cho cả manager/seller. |
| Quản lý phim | Có thao tác chính. |
| Quản lý phòng/ghế | Có xem/lọc, chưa tạo/sửa/ngừng phòng. |
| Quản lý suất chiếu/giá | Chưa có. |
| Bán vé | Chưa có. |
| Tra cứu vé/giao dịch | Chưa có. |
| Báo cáo | Chưa có. |
| Tài khoản & quyền | Có theo FR-AUTH-05, chỉ manager quản lý. |

Sơ đồ phòng không thay cho trạng thái ghế của suất. Bộ dữ liệu demo hiện có
phim/phòng/ghế và tài khoản, nhưng không có suất/ve/giao dịch để đi hết BP-03..05.
Nhân viên xem phim/phòng không phải tự thân vi phạm phân quyền; vấn đề chính
là chưa có quầy vé/tra cứu đúng vai trò. Nút quản trị và service quản trị được
bảo vệ; không có bằng chứng nhân viên đang sửa được dữ liệu quản trị qua Qt.

### 7.2. ERR-01..10

| ID | Kết quả |
|---|---|
| ERR-01 | Có validation thao tác mới, chưa đầy đủ khi load; dấu trường bắt buộc/lỗi theo từng trường chưa đồng nhất trên form. |
| ERR-02 | Có create riêng cho user/film/show; room save vẫn upsert, chưa có form tạo để phân biệt. |
| ERR-03 | Lõi từ chối overlap và giữ dữ liệu cũ; thiếu UI lịch. |
| ERR-04 | Lõi create/open kiểm tra ACTIVE; thiếu UI lịch. |
| ERR-05 | Lõi sale chặn đóng/đã bắt đầu; thiếu UI bán. |
| ERR-06 | Lõi sale kiểm tra sold/duplicate; thiếu UI chọn lại ghế. |
| ERR-07 | Có guard service và lỗi quyền. |
| ERR-08 | paymentConfirmed=false không commit; thiếu nút hủy/xác nhận trên Qt. |
| ERR-09 | Có rollback và lỗi ghi; chưa nghiệm thu sale Qt. |
| ERR-10 | Không đạt: thiếu/0 byte không báo; một số bản ghi không hợp lệ vẫn được nạp. |

### 7.3. SAT-01..12 và BAC-01..12

| SAT | BAC liên quan | Kết quả nghiệm thu trên Qt hiện tại |
|---|---|---|
| SAT-01 | BAC-01 | Chưa đạt trọn: tạo phim được, tạo phòng chưa có UI. |
| SAT-02 | BAC-02 | Chưa thực hiện được: không có tạo/công bố suất trên Qt. |
| SAT-03 | BAC-03 | Có kiểm tra lõi, chưa thử được bằng màn hình lịch. |
| SAT-04 | BAC-04, BAC-08 | Chưa thực hiện được: không có bán hai vé trên Qt. |
| SAT-05 | BAC-05 | Có kiểm tra lõi; chưa có quầy vé Qt, load còn lỗ hổng mất vé. |
| SAT-06 | BAC-06 | Thiết kế cặp suất/ghế hỗ trợ; chưa có nghiệm thu Qt hai suất. |
| SAT-07 | BAC-07 | Lõi không ghi khi chưa xác nhận; chưa có nghiệm thu UI hủy. |
| SAT-08 | BAC-09 | Test storage/service mở lại hợp lệ có; chưa có end-to-end bán/đóng/mở Qt. |
| SAT-09 | BAC-10 | Lõi từ chối staff sửa lịch/xem report; Qt chưa có các module này để thử trọn. |
| SAT-10 | BAC-11 | Lõi lọc có; thiếu báo cáo Qt và công thức occupancy không khớp SRS. |
| SAT-11 | Quy tắc thời điểm bán | Lõi chặn start, chưa có nghiệm thu Qt. |
| SAT-12 | Quy tắc ngừng khai thác | UI phim có guard; UI ngừng phòng chưa có. |
| Chưa có SAT tương ứng trực tiếp | BAC-12 | Lookup lõi có, chưa có màn hình đủ thông tin giao dịch. |

## 8. Các điểm cần chốt, không tự thêm nghiệp vụ

1. Cấp tài khoản quản lý đầu tiên cho dữ liệu vận hành: SRS yêu cầu tài khoản
   có sẵn để đăng nhập, chưa đặc tả quy trình cài đặt/cấp tài khoản đầu tiên.
   App không được tự biến đăng nhập thành đăng ký. Demo và dữ liệu thật phải
   tách rõ; không đưa tài khoản/mật khẩu demo vào dữ liệu vận hành.
2. BRD BP-01 có nhắc thể loại phim (`docs/requirements/brd.md:239`) nhưng SRS FR-FILM-01 và model
   tối thiểu không có genre. Đây là chỗ lệch giữa hai tài liệu cần xác nhận,
   không tự coi genre đã được chốt là trường bắt buộc.
3. BRD OQ-02 nói 15 phút "có thể điều chỉnh", SRS chốt tối thiểu 15 phút.
   Code hiện cố định 15 phút, không trái mức tối thiểu SRS; chưa có yêu cầu
   Must rõ ràng cho trang cấu hình khoảng nghỉ.
4. BRD còn ghi OQ là đề xuất/chưa phê duyệt, SRS lại ghi đã chốt; cả hai còn
   trạng thái Draft. Cần đồng bộ trạng thái tài liệu, không dùng điều này để
   hợp thức hóa hành vi tự thêm vào sản phẩm.
5. NFR-02 cần bộ dữ liệu/quy mô đo cụ thể trước khi tuyên bố đạt 2s/3s.
6. SRS FR-DATA-04/ERR-10 yêu cầu lỗi file thiếu; use-cases cho phép tạo header
   cho file khởi tạo. Cần một chính sách rõ cho "mới hoàn toàn" và "đã vận hành".
7. `docs/design/ARCHITECTURE.md:196` còn nói UI tạo admin đầu tiên; đoạn sau đã nói chỉ
   đăng nhập. `docs/design/ARCHITECTURE.md:212` còn nói phòng chưa có trong khi đã có viewer.
   Đây là tài liệu triển khai bị trôi; không sửa BRD/SRS để chạy theo code.

## 9. Kiểm chứng và thứ tự xử lý đề xuất

- Chạy lại CTest hiện tại: 11/11 đạt. Bộ test này không bao phủ toàn bộ 59 Must
  hay tất cả SAT/BAC. Nó vẫn xanh trong khi các lỗi P1/P2 trên tái hiện được.
- Probe độc lập chạy bằng cms_core hiện tại trên bản sao trong
  `/tmp/cinema-audit.Y3HPEu/fixtures`; không sửa TXT vận hành.
- Đã tái hiện: mất tickets.txt, sai tổng tiền, FAILED kèm ISSUED, thời gian
  không hợp lệ, sai capacity, trùng vị trí và mẫu số occupancy khác SRS.
- Không chạy benchmark quy mô lớn; không thử mất điện/crash vật lý; không
  khẳng định đã nghiệm thu Qt cho module chưa tồn tại.

Thứ tự đề xuất, chưa triển khai trong lần rà soát:

1. Chặn dữ liệu thiếu/hỏng tại load, đối soát giao dịch/vé/tiền/trạng thái;
   bổ sung regression cho các ca đã tái hiện và giữ dữ liệu cũ nguyên vẹn.
2. Chốt cách cấp bộ dữ liệu/tài khoản vận hành; giữ login đúng hai trường.
3. Hoàn thiện quản lý phòng/ghế và quản lý suất chiếu/giá trên Qt.
4. Nối tra cứu suất -> sơ đồ ghế theo suất -> preview -> xác nhận tiền mặt -> vé.
5. Hoàn thiện tra cứu giao dịch/vé và báo cáo, sửa occupancy theo định nghĩa chốt.
6. Chạy đầy đủ SAT/BAC qua Qt, đóng/mở lại và lỗi ghi; đo NFR-02 rồi đóng gói.

**Kết luận:** hiện là phần lõi nghiệp vụ kèm một số màn hình quản trị, chưa phải
MVP vận hành rạp hoàn chỉnh theo BRD/SRS. Ưu tiên sửa tính toàn vẹn và hoàn
thiện quy trình bắt buộc, không thêm nghiệp vụ ngoài phạm vi để lấp khoảng thiếu.
