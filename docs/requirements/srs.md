# SOFTWARE REQUIREMENTS SPECIFICATION (SRS)

## HỆ THỐNG QUẢN LÝ RẠP CHIẾU PHIM
*Cinema Management System - Offline Box Office & Cinema Operations*

| Thông tin | Nội dung |
|---|---|
| Mã tài liệu | SRS-CMS-001 |
| Phiên bản | 0.1 - Draft |
| Ngày xây dựng | 10/10/2026 |
| Tài liệu nguồn | `docs/requirements/brd.md` - BRD-CMS-001 |
| Mô hình triển khai | Ứng dụng offline trên một máy tính |
| Phạm vi | Một rạp chiếu phim, nhiều phòng chiếu |
| Trạng thái | Chờ rà soát và phê duyệt |

---

# 1. Mục đích và phạm vi

## 1.1. Mục đích

Tài liệu này đặc tả các yêu cầu phần mềm cho Cinema Management System (CMS). SRS chuyển các mục tiêu và quy tắc trong BRD thành các yêu cầu có thể thiết kế, triển khai và kiểm thử.

SRS tập trung mô tả **hệ thống phải làm gì**, không khóa vào một ngôn ngữ lập trình hoặc giao diện triển khai cụ thể.

## 1.2. Phạm vi sản phẩm

CMS hỗ trợ một rạp chiếu phim:

- Quản lý phim, phòng chiếu và sơ đồ ghế.
- Tạo, kiểm tra và công bố suất chiếu.
- Tra cứu lịch chiếu và tình trạng ghế.
- Bán một hoặc nhiều vé tại quầy bằng tiền mặt.
- Phát hành vé sau khi giao dịch được xác nhận thành công.
- Tra cứu vé, giao dịch và báo cáo doanh thu.
- Kiểm soát nghiệp vụ theo vai trò người dùng.
- Lưu và đọc lại dữ liệu từ các file TXT khi ứng dụng đóng và mở lại.

## 1.3. Ngoài phạm vi phiên bản 1.0

Hệ thống không bao gồm đặt vé trực tuyến, nhiều chi nhánh, nhiều máy sử dụng đồng thời, thanh toán điện tử, hủy/hoàn vé, soát vé, quản lý đồ ăn thức uống, quản lý ca thu ngân hoặc chính sách giá nâng cao.

---

# 2. Mô tả tổng quan

## 2.1. Bối cảnh hệ thống

CMS là ứng dụng độc lập, hoạt động không phụ thuộc Internet hoặc dịch vụ bên ngoài. Dữ liệu nghiệp vụ được lưu cục bộ dưới dạng các file TXT theo cơ chế do nhóm xây dựng.

Hệ thống có một phiên sử dụng tại một thời điểm. Người dùng đăng nhập, thực hiện nghiệp vụ theo quyền, sau đó đăng xuất hoặc đóng ứng dụng.

## 2.2. Người dùng và vai trò

| Vai trò | Mô tả | Quyền chính |
|---|---|---|
| Quản lý rạp | Quản lý hoạt động và dữ liệu vận hành | Quản lý phim, phòng, ghế, suất chiếu, giá vé, tài khoản, báo cáo; có thể bán vé nếu được cấp quyền |
| Nhân viên bán vé | Thực hiện nghiệp vụ tại quầy | Tra cứu phim/suất chiếu, xem ghế, bán vé, tra cứu vé và giao dịch |
| Khách hàng | Người mua vé tại quầy | Không đăng nhập và không thao tác trực tiếp trên phần mềm |

## 2.3. Điều kiện vận hành

- Máy tính có thể đọc và ghi thư mục dữ liệu của ứng dụng.
- Hệ thống không yêu cầu kết nối mạng.
- Dữ liệu đầu vào do người dùng nhập tại màn hình ứng dụng.
- Thanh toán được xác nhận bằng thao tác của nhân viên; không kết nối thiết bị thanh toán.

## 2.4. Ràng buộc cấu trúc dữ liệu triển khai

- Phần lõi phải sử dụng các cấu trúc dữ liệu tự xây dựng trong `src/DataStructures/`.
- Không dùng container STL hoặc container Qt làm nơi lưu trữ chính cho các thực thể nghiệp vụ.
- `DoublyLinkedList`, `HashTable`, `BST` và `Matrix2D` được dùng theo thiết kế trong `docs/design/data-design.md`.
- `Queue`, `Stack` và `Graph` chỉ được dùng khi có nghiệp vụ phù hợp; không thêm chức năng chỉ để ép sử dụng cấu trúc.
- Qt chịu trách nhiệm giao diện, sự kiện, hộp thoại và đọc/ghi hỗ trợ; không chứa logic nghiệp vụ cốt lõi trong widget.

---

# 3. Quyết định nghiệp vụ đã chốt

Các quyết định dưới đây được chuyển từ phần Open Questions của BRD thành cơ sở bắt buộc cho SRS:

| ID | Quyết định |
|---|---|
| OQ-01 | Không bán vé khi thời điểm hiện tại đã bằng hoặc sau thời điểm bắt đầu suất chiếu. |
| OQ-02 | Khoảng nghỉ tối thiểu giữa hai suất chiếu trong cùng phòng là 15 phút. |
| OQ-03 | Phiên bản đầu chỉ sử dụng ghế tiêu chuẩn; chưa triển khai ghế VIP hoặc loại ghế khác. |
| OQ-04 | Lưu phân loại độ tuổi của phim; hệ thống không kiểm tra giấy tờ, nhân viên chịu trách nhiệm kiểm tra thực tế. |
| OQ-05 | Không hỗ trợ hủy vé và hoàn tiền trong phiên bản đầu. |
| OQ-06 | Không giữ ghế chưa thanh toán; ghế chỉ được đánh dấu đã bán sau khi giao dịch thành công. |
| OQ-07 | Không quản lý ca thu ngân; giao dịch chỉ lưu người thực hiện và thời điểm thực hiện. |
| OQ-08 | Quản lý được phép bán vé nếu tài khoản được cấp quyền bán vé. |
| OQ-09 | Không ngừng khai thác phim/phòng nếu còn suất chiếu tương lai bị ảnh hưởng chưa được xử lý. |

---

# 4. Yêu cầu chức năng

Mức ưu tiên: **Must** là bắt buộc cho phiên bản 1.0; **Should** là mở rộng, chưa dùng làm điều kiện nghiệm thu cốt lõi.

## 4.1. Đăng nhập và phân quyền

| ID | Yêu cầu | Ưu tiên |
|---|---|---|
| FR-AUTH-01 | Hệ thống phải cho phép người dùng đăng nhập bằng tài khoản và mật khẩu hợp lệ. | Must |
| FR-AUTH-02 | Hệ thống phải xác định vai trò và các quyền nghiệp vụ của người dùng sau khi đăng nhập. | Must |
| FR-AUTH-03 | Hệ thống phải từ chối đăng nhập khi tài khoản hoặc mật khẩu không hợp lệ và không tiết lộ thông tin nhạy cảm. | Must |
| FR-AUTH-04 | Hệ thống phải cho phép người dùng đăng xuất; sau khi đăng xuất, màn hình nghiệp vụ phải yêu cầu đăng nhập lại. | Must |
| FR-AUTH-05 | Quản lý phải có thể tạo, cập nhật trạng thái và phân quyền cơ bản cho tài khoản người dùng. | Must |
| FR-AUTH-06 | Người dùng không được truy cập hoặc thực hiện thao tác vượt quá quyền được cấp. | Must |

## 4.2. Quản lý phim

| ID | Yêu cầu | Ưu tiên |
|---|---|---|
| FR-FILM-01 | Quản lý phải có thể tạo phim với mã phim duy nhất, tên phim, thời lượng, phân loại độ tuổi và trạng thái khai thác. | Must |
| FR-FILM-02 | Hệ thống phải kiểm tra các trường bắt buộc, thời lượng dương và mã phim không trùng. | Must |
| FR-FILM-03 | Quản lý phải có thể xem danh sách, tìm kiếm và xem chi tiết phim. | Must |
| FR-FILM-04 | Quản lý phải có thể cập nhật thông tin phim khi thay đổi không làm mất tính hợp lệ của suất chiếu đã có vé. | Must |
| FR-FILM-05 | Quản lý phải có thể chuyển trạng thái phim sang ngừng khai thác khi không còn suất chiếu tương lai bị ảnh hưởng. | Must |
| FR-FILM-06 | Hệ thống không được xóa hoặc thay đổi thông tin quan trọng làm mất tính hợp lệ của suất chiếu đã có vé. | Must |

## 4.3. Quản lý phòng chiếu và ghế

| ID | Yêu cầu | Ưu tiên |
|---|---|---|
| FR-ROOM-01 | Quản lý phải có thể tạo phòng với mã phòng duy nhất, tên phòng, trạng thái hoạt động và sơ đồ ghế. | Must |
| FR-ROOM-02 | Mỗi ghế trong một phòng phải có mã/vị trí duy nhất; hệ thống phải từ chối ghế trùng hoặc cấu hình không hợp lệ. | Must |
| FR-ROOM-03 | Hệ thống phải tính hoặc kiểm tra sức chứa từ danh sách ghế hợp lệ của phòng. | Must |
| FR-ROOM-04 | Quản lý phải có thể xem danh sách phòng, sức chứa và sơ đồ ghế. | Must |
| FR-ROOM-05 | Quản lý phải có thể cập nhật trạng thái phòng khi không làm ảnh hưởng đến suất chiếu tương lai chưa được xử lý. | Must |
| FR-ROOM-06 | Hệ thống không được cho ngừng sử dụng phòng nếu còn suất chiếu tương lai bị ảnh hưởng. | Must |

## 4.4. Quản lý suất chiếu

| ID | Yêu cầu | Ưu tiên |
|---|---|---|
| FR-SHOW-01 | Quản lý phải có thể tạo suất chiếu bằng cách chọn một phim, một phòng, thời điểm bắt đầu và giá vé cơ bản. | Must |
| FR-SHOW-02 | Hệ thống phải tính thời điểm kết thúc từ thời điểm bắt đầu và thời lượng phim. | Must |
| FR-SHOW-03 | Hệ thống phải tính khoảng sử dụng phòng bao gồm thời lượng phim và khoảng nghỉ tối thiểu 15 phút. | Must |
| FR-SHOW-04 | Hệ thống phải từ chối suất chiếu nếu khoảng sử dụng phòng chồng lấn với suất chiếu khác trong cùng phòng. | Must |
| FR-SHOW-05 | Hệ thống phải từ chối suất chiếu nếu phim hoặc phòng không tồn tại, không hoạt động hoặc thông tin thời gian/giá không hợp lệ. | Must |
| FR-SHOW-06 | Quản lý phải có thể lưu suất chiếu ở trạng thái nháp hoặc mở bán theo thiết kế giao diện, nhưng chỉ trạng thái mở bán mới nhận giao dịch. | Must |
| FR-SHOW-07 | Quản lý phải có thể xem, tìm kiếm và điều chỉnh trạng thái suất chiếu khi chưa vi phạm quy tắc dữ liệu đã bán. | Must |
| FR-SHOW-08 | Khi suất chiếu đã có vé bán, hệ thống không được cho thay đổi phim, phòng hoặc thời điểm bắt đầu. | Must |
| FR-SHOW-09 | Hệ thống phải tự động hoặc cho phép quản lý đóng bán vé khi suất chiếu đã bắt đầu. | Must |

## 4.5. Tra cứu lịch chiếu và ghế

| ID | Yêu cầu | Ưu tiên |
|---|---|---|
| FR-SEARCH-01 | Nhân viên có quyền phải có thể tra cứu các phim và suất chiếu đang mở bán theo ngày, phim hoặc phòng. | Must |
| FR-SEARCH-02 | Hệ thống phải hiển thị các thông tin tối thiểu của suất chiếu: phim, phòng, thời điểm bắt đầu, thời điểm kết thúc và giá vé. | Must |
| FR-SEARCH-03 | Hệ thống phải hiển thị sơ đồ ghế của suất chiếu với trạng thái còn trống hoặc đã bán. | Must |
| FR-SEARCH-04 | Hệ thống phải phân biệt ghế không tồn tại, ghế còn trống và ghế đã bán. | Must |

## 4.6. Bán vé tại quầy

| ID | Yêu cầu | Ưu tiên |
|---|---|---|
| FR-SALE-01 | Người dùng có quyền bán vé phải có thể chọn một suất chiếu đang mở bán. | Must |
| FR-SALE-02 | Người dùng phải có thể chọn một hoặc nhiều ghế còn trống trong cùng suất chiếu. | Must |
| FR-SALE-03 | Hệ thống phải từ chối ghế đã bán, ghế không thuộc phòng hoặc ghế bị chọn trùng trong cùng giao dịch. | Must |
| FR-SALE-04 | Hệ thống phải tính tổng tiền bằng số lượng vé nhân với giá vé của suất chiếu. | Must |
| FR-SALE-05 | Hệ thống phải hiển thị phim, suất chiếu, phòng, ghế, đơn giá và tổng tiền trước khi xác nhận. | Must |
| FR-SALE-06 | Nhân viên phải có thể hủy lựa chọn trước khi xác nhận mà không tạo giao dịch, vé hoặc doanh thu. | Must |
| FR-SALE-07 | Nhân viên phải có thể xác nhận đã nhận đủ tiền mặt để hoàn tất giao dịch. | Must |
| FR-SALE-08 | Khi giao dịch thành công, hệ thống phải tạo mã giao dịch duy nhất, tạo vé cho từng ghế, cập nhật trạng thái ghế và ghi nhận doanh thu. | Must |
| FR-SALE-09 | Việc ghi nhận giao dịch, vé, ghế đã bán và doanh thu phải thành công toàn bộ hoặc không ghi nhận giao dịch thành công. | Must |
| FR-SALE-10 | Hệ thống không được phát hành vé hoặc cộng doanh thu khi nhân viên không xác nhận thanh toán hoặc quá trình ghi nhận thất bại. | Must |
| FR-SALE-11 | Hệ thống phải lưu người thực hiện và thời điểm giao dịch. | Must |
| FR-SALE-12 | Hệ thống không được giữ ghế sau khi màn hình bán vé bị hủy hoặc giao dịch thất bại. | Must |

## 4.7. Tra cứu vé và giao dịch

| ID | Yêu cầu | Ưu tiên |
|---|---|---|
| FR-QUERY-01 | Nhân viên có quyền phải có thể tra cứu giao dịch bằng mã giao dịch. | Must |
| FR-QUERY-02 | Người dùng có quyền phải có thể tra cứu vé bằng mã vé. | Must |
| FR-QUERY-03 | Kết quả tra cứu phải hiển thị tối thiểu phim, suất chiếu, phòng, ghế, giá vé, trạng thái, mã giao dịch và người thực hiện. | Must |
| FR-QUERY-04 | Hệ thống phải thông báo rõ trường hợp không tìm thấy hoặc dữ liệu tìm kiếm không hợp lệ. | Must |
| FR-QUERY-05 | Hệ thống không cung cấp chức năng xóa hoặc sửa tùy ý giao dịch đã xác nhận. | Must |

## 4.8. Báo cáo doanh thu

| ID | Yêu cầu | Ưu tiên |
|---|---|---|
| FR-REPORT-01 | Quản lý có quyền phải có thể xem báo cáo theo khoảng thời gian. | Must |
| FR-REPORT-02 | Quản lý phải có thể lọc báo cáo theo phim và suất chiếu. | Must |
| FR-REPORT-03 | Báo cáo phải hiển thị số lượng vé bán, số ghế đã bán, tỷ lệ lấp đầy và doanh thu. | Must |
| FR-REPORT-04 | Doanh thu chỉ được tính từ các giao dịch đã xác nhận thành công. | Must |
| FR-REPORT-05 | Tỷ lệ lấp đầy phải được tính bằng số ghế đã bán chia cho sức chứa phòng của suất chiếu, nhân 100%. | Must |
| FR-REPORT-06 | Báo cáo không bao gồm lợi nhuận, chi phí hoạt động hoặc giao dịch thất bại. | Must |

## 4.9. Lưu trữ và khôi phục dữ liệu

| ID | Yêu cầu | Ưu tiên |
|---|---|---|
| FR-DATA-01 | Hệ thống phải lưu các dữ liệu nghiệp vụ cần thiết vào file TXT cục bộ. | Must |
| FR-DATA-02 | Hệ thống phải đọc lại dữ liệu đã xác nhận khi khởi động ứng dụng. | Must |
| FR-DATA-03 | Hệ thống phải lưu dữ liệu giao dịch, vé, ghế đã bán và doanh thu nhất quán sau giao dịch thành công. | Must |
| FR-DATA-04 | Hệ thống phải phát hiện file dữ liệu không tồn tại, không đọc được hoặc sai định dạng và hiển thị lỗi phù hợp. | Must |
| FR-DATA-05 | Lỗi ghi dữ liệu không được làm cho giao dịch được hiển thị là thành công khi vé hoặc doanh thu chưa được lưu đầy đủ. | Must |
| FR-DATA-06 | Hệ thống phải có cơ chế tạo bản sao hoặc khôi phục dữ liệu theo khả năng triển khai của nhóm trước khi thao tác có nguy cơ làm mất dữ liệu. | Should |

---

# 5. Quy tắc xử lý bắt buộc

| ID | Quy tắc |
|---|---|
| RULE-01 | Mã phim, mã phòng, mã suất chiếu, mã giao dịch và mã vé phải duy nhất trong phạm vi tương ứng. |
| RULE-02 | Mỗi ghế chỉ duy nhất trong một phòng; cùng vị trí ghế có thể được bán ở các suất chiếu khác nhau. |
| RULE-03 | Một suất chiếu gắn với đúng một phim và một phòng. |
| RULE-04 | Hai suất chiếu trong cùng phòng không được chồng lấn khoảng sử dụng; khoảng nghỉ tối thiểu là 15 phút. |
| RULE-05 | Chỉ suất chiếu đang mở bán và chưa bắt đầu mới được bán vé. |
| RULE-06 | Một ghế chỉ được bán một lần trong cùng suất chiếu. |
| RULE-07 | Một giao dịch có thể có nhiều vé nhưng không được chứa cùng một ghế hai lần. |
| RULE-08 | Tổng tiền giao dịch bằng tổng giá trị các vé trong giao dịch. |
| RULE-09 | Vé chỉ được phát hành sau khi thanh toán được xác nhận thành công. |
| RULE-10 | Giao dịch thất bại không tạo vé đã bán và không tạo doanh thu. |
| RULE-11 | Giao dịch thành công không được xóa hoặc sửa tùy ý. |
| RULE-12 | Giá vé của vé đã bán phải được lưu tại thời điểm giao dịch, không phụ thuộc thay đổi giá về sau. |
| RULE-13 | Chỉ quản lý được tổ chức/điều chỉnh lịch chiếu và xem báo cáo doanh thu. |
| RULE-14 | Chỉ tài khoản có quyền bán vé được xác nhận giao dịch; quản lý có thể được cấp quyền này. |
| RULE-15 | Không được thay đổi phim, phòng hoặc thời gian của suất chiếu đã có vé bán. |

---

# 6. Yêu cầu dữ liệu

SRS chỉ quy định dữ liệu logic. Cấu trúc file TXT cụ thể do thiết kế kỹ thuật quyết định nhưng phải bảo toàn các trường bắt buộc và quan hệ dưới đây.

## 6.1. Thực thể và thuộc tính

| Thực thể | Thuộc tính tối thiểu |
|---|---|
| User | `user_id`, `username`, `password` hoặc giá trị bảo vệ tương đương, `display_name`, `role`, `permissions`, `status` |
| Film | `film_id`, `title`, `duration_minutes`, `age_rating`, `status` |
| Auditorium | `auditorium_id`, `name`, `capacity`, `status` |
| Seat | `seat_id`, `auditorium_id`, `row`, `number`, `type` |
| Screening | `screening_id`, `film_id`, `auditorium_id`, `start_time`, `end_time`, `room_available_until`, `ticket_price`, `status` |
| Transaction | `transaction_id`, `created_at`, `created_by`, `screening_id`, `total_amount`, `payment_status`, `status` |
| Ticket | `ticket_id`, `transaction_id`, `screening_id`, `seat_id`, `unit_price`, `status` |

## 6.2. Quan hệ dữ liệu

- Một `Film` có nhiều `Screening`.
- Một `Auditorium` có nhiều `Seat` và nhiều `Screening` theo các thời điểm không chồng lấn.
- Một `Screening` có nhiều `Ticket`.
- Một `Transaction` có một hoặc nhiều `Ticket`.
- Một `Ticket` gắn với đúng một `Screening` và một `Seat`.
- Một `User` có thể tạo nhiều `Transaction`.

## 6.3. Trạng thái chính

| Đối tượng | Trạng thái tối thiểu |
|---|---|
| User | `ACTIVE`, `INACTIVE` |
| Film | `ACTIVE`, `INACTIVE` |
| Auditorium | `ACTIVE`, `INACTIVE` |
| Screening | `DRAFT`, `OPEN`, `CLOSED`, `COMPLETED` |
| Transaction | `PENDING`, `COMPLETED`, `FAILED` |
| Ticket | `ISSUED` |

Phiên bản đầu không dùng trạng thái `CANCELLED` hoặc `REFUNDED` vì chưa hỗ trợ hủy và hoàn tiền.

---

# 7. Yêu cầu giao diện và tương tác

Hệ thống phải cung cấp tối thiểu các màn hình hoặc khu vực chức năng sau:

| Màn hình | Người dùng | Nội dung chính |
|---|---|---|
| Đăng nhập | Tất cả người dùng | Tài khoản, mật khẩu, thông báo lỗi |
| Trang tổng quan | Quản lý | Lối vào quản lý danh mục, lịch chiếu, bán vé và báo cáo |
| Quản lý phim | Quản lý | Danh sách, tìm kiếm, thêm, sửa, ngừng khai thác |
| Quản lý phòng và ghế | Quản lý | Danh sách phòng, sức chứa, sơ đồ ghế, trạng thái |
| Quản lý suất chiếu | Quản lý | Tạo lịch, kiểm tra xung đột, mở/đóng bán |
| Bán vé | Người có quyền bán vé | Chọn suất, chọn ghế, xem tổng tiền, xác nhận thanh toán |
| Tra cứu | Nhân viên, quản lý | Tra cứu mã vé/mã giao dịch |
| Báo cáo | Quản lý | Bộ lọc và thống kê vé, ghế, doanh thu |

Yêu cầu tương tác:

- Trường bắt buộc phải được đánh dấu và kiểm tra trước khi lưu.
- Hệ thống phải hiển thị lỗi gần thao tác hoặc trường gây lỗi bằng thông báo dễ hiểu.
- Thao tác xác nhận giao dịch phải hiển thị lại dữ liệu quan trọng trước khi hoàn tất.
- Ghế đã bán phải có cách hiển thị khác với ghế còn trống.
- Các thao tác bị từ chối do quyền hạn phải giải thích ngắn gọn lý do.
- Không dùng thao tác xóa để làm mất dữ liệu giao dịch đã xác nhận.

---

# 8. Yêu cầu phi chức năng

| ID | Nhóm | Yêu cầu kiểm thử được |
|---|---|---|
| NFR-01 | Offline | Các chức năng cốt lõi phải hoạt động khi máy không có kết nối Internet. |
| NFR-02 | Hiệu năng | Với dữ liệu trong phạm vi bài tập, các thao tác tra cứu thông thường phản hồi trong tối đa 2 giây; xác nhận bán vé phản hồi trong tối đa 3 giây, không tính thời gian người dùng nhập liệu. |
| NFR-03 | Toàn vẹn | Không được tạo trạng thái giao dịch thành công nếu thiếu vé, ghế đã bán hoặc dữ liệu doanh thu tương ứng. |
| NFR-04 | Bảo toàn | Dữ liệu đã xác nhận phải còn nguyên sau khi đóng và mở lại ứng dụng bình thường. |
| NFR-05 | Phân quyền | Mọi thao tác nghiệp vụ phải được kiểm tra quyền ở thời điểm thực hiện, không chỉ ẩn nút giao diện. |
| NFR-06 | Truy vết | Giao dịch thành công phải truy được mã giao dịch, thời điểm và người thực hiện. |
| NFR-07 | Khả dụng | Ứng dụng phải hiển thị lỗi có thể xử lý khi file dữ liệu bị thiếu, hỏng hoặc không có quyền ghi. |
| NFR-08 | Dễ sử dụng | Quy trình bán vé phải hoàn thành qua các bước rõ ràng: chọn suất, chọn ghế, kiểm tra tiền, xác nhận thanh toán. |
| NFR-09 | Bảo mật cục bộ | Mật khẩu không được hiển thị công khai trong giao diện hoặc lưu dưới dạng dễ đọc nếu công cụ/ngôn ngữ cho phép bảo vệ phù hợp. |
| NFR-10 | Bảo trì | Các mã định danh và trạng thái trong dữ liệu phải được kiểm tra nhất quán khi đọc file TXT. |

---

# 9. Xử lý lỗi và trường hợp ngoại lệ

| Mã | Tình huống | Hệ thống phải |
|---|---|---|
| ERR-01 | Dữ liệu bắt buộc rỗng hoặc sai định dạng | Từ chối lưu và chỉ rõ trường cần sửa. |
| ERR-02 | Mã định danh bị trùng | Từ chối lưu, giữ nguyên dữ liệu cũ. |
| ERR-03 | Phòng có suất chiếu chồng lấn | Từ chối tạo/cập nhật suất chiếu và không thay đổi lịch hiện tại. |
| ERR-04 | Phim/phòng không hoạt động | Từ chối tạo hoặc mở bán suất chiếu liên quan. |
| ERR-05 | Suất chiếu đã bắt đầu hoặc đóng bán | Từ chối giao dịch mới. |
| ERR-06 | Ghế đã bán hoặc chọn trùng | Từ chối xác nhận và yêu cầu chọn lại. |
| ERR-07 | Người dùng không đủ quyền | Từ chối thao tác và không thay đổi dữ liệu. |
| ERR-08 | Người dùng không xác nhận thanh toán | Hủy quy trình bán, không phát hành vé và không ghi doanh thu. |
| ERR-09 | Ghi file thất bại | Báo lỗi, không hiển thị giao dịch là thành công và tránh ghi nhận một phần. |
| ERR-10 | File TXT thiếu hoặc sai cấu trúc | Báo lỗi dữ liệu, không tự động bỏ qua bản ghi làm mất tính nhất quán. |

---

# 10. Ma trận truy vết

| Yêu cầu BRD | Yêu cầu SRS chính | Tiêu chí nghiệm thu liên quan |
|---|---|---|
| BR-01, BR-02 | FR-FILM-01..06, FR-ROOM-01..06 | BAC-01 |
| BR-03, BR-04 | FR-SHOW-01..09 | BAC-02, BAC-03 |
| BR-05, BR-06 | FR-SEARCH-01..04, FR-SHOW-01..09 | BAC-02, BAC-04 |
| BR-07, BR-08 | FR-SALE-01..12 | BAC-04, BAC-05, BAC-06 |
| BR-09, BR-14 | FR-SALE-07..12, FR-DATA-03..05 | BAC-07, BAC-08 |
| BR-10 | FR-QUERY-01..05 | BAC-12 |
| BR-11 | FR-REPORT-01..06 | BAC-11 |
| BR-12 | FR-AUTH-01..06, NFR-05 | BAC-10 |
| BR-13 | FR-DATA-01..06, NFR-04 | BAC-09 |

---

# 11. Kịch bản kiểm thử chấp nhận cấp hệ thống

| ID | Tiền điều kiện | Thao tác | Kết quả mong đợi |
|---|---|---|---|
| SAT-01 | Có quyền quản lý | Tạo phim và phòng với dữ liệu hợp lệ | Dữ liệu được lưu và dùng được khi tạo suất chiếu. |
| SAT-02 | Phim, phòng hợp lệ | Tạo suất chiếu không xung đột và mở bán | Suất chiếu được lưu, hiển thị khi tra cứu. |
| SAT-03 | Đã có suất chiếu trong phòng | Tạo suất chiếu chồng lấn | Hệ thống từ chối, suất cũ không bị thay đổi. |
| SAT-04 | Suất chiếu mở bán, ghế còn trống | Chọn hai ghế, xác nhận thanh toán | Một giao dịch và hai vé được tạo; hai ghế chuyển sang đã bán; doanh thu tăng đúng tổng tiền. |
| SAT-05 | Một ghế đã bán | Chọn lại ghế trong cùng suất | Không thể hoàn tất giao dịch. |
| SAT-06 | Hai suất chiếu khác nhau | Chọn cùng vị trí ghế nếu còn trống | Giao dịch được phép. |
| SAT-07 | Suất chiếu đang mở bán | Không xác nhận thanh toán hoặc hủy lựa chọn | Không có vé, giao dịch thành công hoặc doanh thu mới. |
| SAT-08 | Có giao dịch thành công | Đóng và mở lại ứng dụng | Giao dịch, vé, ghế đã bán và doanh thu vẫn chính xác. |
| SAT-09 | Đăng nhập bằng tài khoản nhân viên | Thử sửa lịch chiếu hoặc xem báo cáo tổng hợp | Thao tác bị từ chối theo quyền. |
| SAT-10 | Có dữ liệu giao dịch hợp lệ | Quản lý lọc báo cáo theo ngày/phim/suất | Số vé, ghế, tỷ lệ lấp đầy và doanh thu khớp dữ liệu. |
| SAT-11 | Suất chiếu đã bắt đầu | Thử bán vé | Giao dịch bị từ chối. |
| SAT-12 | Phòng hoặc phim có suất chiếu tương lai bị ảnh hưởng | Thử ngừng khai thác | Thao tác bị từ chối và dữ liệu lịch không thay đổi. |

---

# 12. Giả định, ràng buộc và rủi ro kỹ thuật

## 12.1. Giả định

- Một máy tính được sử dụng tại một thời điểm.
- Khách hàng mua vé trực tiếp tại quầy.
- Thanh toán là tiền mặt và do nhân viên xác nhận.
- Mỗi suất chiếu dùng một giá vé cơ bản.
- Nhân viên chịu trách nhiệm kiểm tra điều kiện độ tuổi ngoài hệ thống.

## 12.2. Ràng buộc

- Không sử dụng DBMS.
- Dữ liệu nghiệp vụ phải lưu bằng TXT.
- Không phụ thuộc Internet hoặc dịch vụ bên ngoài.
- Không xử lý đồng thời nhiều máy hoặc nhiều phiên ghi dữ liệu.

## 12.3. Rủi ro cần kiểm soát trong thiết kế

- Ghi nhiều file TXT có thể tạo trạng thái cập nhật một phần; thiết kế phải có thứ tự ghi, kiểm tra và phục hồi lỗi.
- File bị sửa thủ công có thể làm sai quan hệ; khi khởi động phải kiểm tra mã, quan hệ và trạng thái tối thiểu.
- Không có DBMS nên tính duy nhất của ghế/vé/giao dịch phải được kiểm tra trong logic ứng dụng và khi ghi file.

---

# 13. Tiêu chí hoàn thành SRS

SRS được xem là đủ để chuyển sang thiết kế kỹ thuật khi nhóm đã xác nhận:

- Tất cả yêu cầu Must và quy tắc nghiệp vụ trong tài liệu.
- Các trường dữ liệu và quan hệ chính.
- Vai trò, quyền hạn và các màn hình tối thiểu.
- Các trường hợp lỗi khi lập lịch, bán vé và lưu dữ liệu.
- Ma trận truy vết từ BRD sang SRS và kịch bản nghiệm thu.

Sau khi SRS được phê duyệt, bước tiếp theo là thiết kế cấu trúc file TXT, sơ đồ luồng xử lý, wireframe giao diện và kế hoạch triển khai từng module.
