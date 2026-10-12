# BUSINESS REQUIREMENTS DOCUMENT (BRD)

## HỆ THỐNG QUẢN LÝ RẠP CHIẾU PHIM
*Cinema Management System – Offline Box Office & Cinema Operations*

| Thông tin | Nội dung |
|---|---|
| Mã tài liệu | BRD-CMS-001 |
| Phiên bản | 0.1 – Draft |
| Ngày xây dựng | 10/10/2026 |
| Loại dự án | Bài tập môn học |
| Mô hình triển khai | Ứng dụng offline trên một máy tính |
| Phạm vi | Một rạp chiếu phim, nhiều phòng chiếu |
| Trạng thái | Chờ rà soát và phê duyệt yêu cầu |

---

# 1. Executive Summary – Tổng quan dự án

## 1.1. Giới thiệu

Dự án **Cinema Management System (CMS)** hướng đến xây dựng một hệ thống hỗ trợ các hoạt động quản lý và bán vé tại một rạp chiếu phim.

Hệ thống cho phép người quản lý tổ chức các hoạt động chiếu phim thông qua việc quản lý danh sách phim, phòng chiếu, lịch chiếu và giá vé. Nhân viên bán vé sử dụng hệ thống để tra cứu suất chiếu, lựa chọn ghế, thực hiện giao dịch và phát hành vé cho khách hàng.

Bên cạnh đó, hệ thống cung cấp thông tin về tình trạng bán vé và doanh thu, hỗ trợ người quản lý theo dõi kết quả kinh doanh của rạp.

Sản phẩm được giới hạn ở mô hình **một rạp chiếu phim hoạt động offline**, không yêu cầu khách hàng đặt vé trực tuyến hay tích hợp các dịch vụ bên ngoài.

## 1.2. Mục đích của tài liệu

BRD được xây dựng nhằm:

- Xác định vấn đề kinh doanh mà sản phẩm cần giải quyết.
- Làm rõ mục tiêu và giá trị kỳ vọng của hệ thống.
- Xác định các bên liên quan và trách nhiệm nghiệp vụ.
- Thống nhất phạm vi triển khai.
- Mô tả các quy trình nghiệp vụ chính.
- Xác định yêu cầu và quy tắc kinh doanh.
- Đưa ra tiêu chí đánh giá sản phẩm đã đáp ứng nhu cầu.

Tài liệu này là cơ sở để tiếp tục xây dựng đặc tả yêu cầu phần mềm (SRS).

# 2. Business Context – Bối cảnh kinh doanh

## 2.1. Mô hình hoạt động của rạp

Rạp chiếu phim giả định có nhiều phòng chiếu với số lượng và sơ đồ ghế khác nhau.

Mỗi ngày, rạp tổ chức nhiều suất chiếu cho những bộ phim đang được khai thác. Mỗi suất chiếu gắn với một bộ phim, một phòng chiếu và thời gian xác định.

Khách hàng đến quầy bán vé để lựa chọn phim, suất chiếu và ghế ngồi. Nhân viên tiếp nhận yêu cầu, thu tiền và phát hành vé.

Người quản lý chịu trách nhiệm tổ chức lịch chiếu, giám sát hoạt động bán vé và theo dõi doanh thu.

Các thông tin về phim, phòng chiếu, suất chiếu, vé và giao dịch có mối liên hệ với nhau, cần được quản lý nhất quán.

**Lưu ý:** Đây là mô hình rạp giả định phục vụ bài tập, không phải mô tả hiện trạng của một doanh nghiệp đã được khảo sát.

## 2.2. Business Problems – Những vấn đề cần giải quyết

Trong kịch bản quản lý thủ công hoặc thiếu một hệ thống dữ liệu thống nhất, rạp có thể gặp những vấn đề sau:

| ID | Vấn đề kinh doanh | Tác động |
|---|---|---|
| BP-01 | Thông tin phim, phòng chiếu và lịch chiếu được theo dõi riêng lẻ | Khó tra cứu và điều phối hoạt động |
| BP-02 | Các suất chiếu có thể được bố trí trùng thời gian trong cùng phòng | Gây xung đột lịch sử dụng phòng |
| BP-03 | Tình trạng ghế chưa được kiểm soát nhất quán giữa các giao dịch | Có nguy cơ bán trùng ghế |
| BP-04 | Việc ghi nhận giao dịch bán vé thủ công dễ nhầm lẫn | Sai lệch số vé và số tiền thu |
| BP-05 | Khó tổng hợp lượng vé bán và doanh thu theo từng phim hoặc suất chiếu | Nhà quản lý thiếu thông tin đánh giá kinh doanh |
| BP-06 | Thiếu cơ chế kiểm soát quyền hạn của nhân viên | Có nguy cơ thực hiện những thay đổi không được phép |

## 2.3. Business Opportunity – Cơ hội cải thiện

Một hệ thống quản lý tập trung có thể giúp rạp:

- Chuẩn hóa quy trình tổ chức lịch chiếu và bán vé.
- Hạn chế những sai sót có thể kiểm tra bằng các quy tắc nghiệp vụ.
- Rút ngắn công việc tra cứu và ghi nhận thông tin tại quầy.
- Theo dõi chính xác tình trạng bán vé theo suất chiếu.
- Tổng hợp dữ liệu để hỗ trợ người quản lý đánh giá hoạt động kinh doanh.

# 3. Business Objectives – Mục tiêu kinh doanh

| ID | Mục tiêu | Kết quả kỳ vọng |
|---|---|---|
| BO-01 | Quản lý tập trung hoạt động chiếu phim | Phim, phòng và suất chiếu được quản lý nhất quán |
| BO-02 | Ngăn ngừa xung đột trong việc tổ chức suất chiếu | Không có hai suất chiếu trùng thời gian sử dụng cùng một phòng |
| BO-03 | Chuẩn hóa quy trình bán vé tại quầy | Mỗi giao dịch được ghi nhận đầy đủ, chính xác |
| BO-04 | Đảm bảo tính chính xác của việc phân bổ ghế | Không bán trùng ghế trong cùng một suất chiếu |
| BO-05 | Hỗ trợ quản lý và tra cứu thông tin bán vé | Có thể xác định vé đã bán và giao dịch liên quan |
| BO-06 | Hỗ trợ theo dõi doanh thu | Số liệu báo cáo phản ánh đúng các giao dịch hợp lệ |
| BO-07 | Kiểm soát trách nhiệm và quyền hạn | Người dùng chỉ thực hiện các nghiệp vụ được cấp quyền |

## 3.1. Chỉ số đánh giá thành công

Do đây là dự án học tập chưa có dữ liệu vận hành thực tế, các chỉ số được đề xuất dưới dạng tiêu chí kiểm chứng khi nghiệm thu, không phải cam kết cải thiện kinh doanh so với số liệu quá khứ.

| Chỉ số | Mục tiêu nghiệm thu |
|---|---|
| Xung đột lịch chiếu được chấp nhận | 0 trường hợp trong bộ kiểm thử |
| Bán trùng ghế | 0 trường hợp trong bộ kiểm thử |
| Giao dịch thành công không có vé tương ứng | 0 trường hợp |
| Sai lệch giữa doanh thu báo cáo và dữ liệu giao dịch hợp lệ | 0 đồng |
| Mất dữ liệu giao dịch đã xác nhận khi đóng và mở lại chương trình bình thường | 0 trường hợp |
| Truy cập nghiệp vụ trái quyền trong bộ kiểm thử | 0 trường hợp được chấp nhận |

# 4. Stakeholder Analysis – Các bên liên quan

## 4.1. Danh sách Stakeholders

| Stakeholder | Vai trò | Nhu cầu chính |
|---|---|---|
| Quản lý rạp | Người chịu trách nhiệm vận hành và kinh doanh | Tổ chức lịch chiếu, kiểm soát bán vé, xem báo cáo |
| Nhân viên bán vé | Người trực tiếp sử dụng hệ thống hằng ngày | Tra cứu, chọn ghế, bán vé, kiểm tra giao dịch |
| Khách hàng | Người sử dụng dịch vụ của rạp | Mua đúng vé, đúng suất chiếu, đúng ghế và giá tiền |
| Nhóm phát triển | Người xây dựng sản phẩm | Có phạm vi và quy tắc nghiệp vụ rõ ràng |
| Giảng viên | Bên đánh giá bài tập | Hệ thống đáp ứng đề bài và thể hiện các kiến thức đã học |

## 4.2. Vai trò sử dụng hệ thống

### Cinema Manager – Quản lý rạp

Quản lý rạp có trách nhiệm:

- Duy trì thông tin những bộ phim đang được khai thác.
- Quản lý thông tin phòng chiếu và ghế.
- Tổ chức, điều chỉnh lịch chiếu theo các quy tắc được phép.
- Thiết lập giá vé cơ bản.
- Giám sát hoạt động bán vé.
- Tra cứu giao dịch và theo dõi doanh thu.
- Quản lý tài khoản và quyền truy cập cơ bản.

### Ticket Seller – Nhân viên bán vé

Nhân viên bán vé có trách nhiệm:

- Tra cứu phim và các suất chiếu đang mở bán.
- Xem tình trạng ghế của từng suất.
- Tiếp nhận yêu cầu mua một hoặc nhiều vé.
- Xác định số tiền cần thu.
- Xác nhận đã nhận thanh toán.
- Phát hành vé và tra cứu giao dịch.

Nhân viên bán vé không có quyền tự ý sửa lịch chiếu, giá vé hoặc thay đổi dữ liệu doanh thu đã ghi nhận.

### Customer – Khách hàng

Khách hàng không trực tiếp đăng nhập hoặc thao tác trên phần mềm.

Khách hàng tương tác với nhân viên tại quầy, đưa ra yêu cầu lựa chọn phim, suất chiếu, ghế và nhận vé sau khi hoàn tất thanh toán.

Khách hàng là một bên tham gia quy trình nghiệp vụ nhưng không phải người dùng trực tiếp của ứng dụng.

# 5. Project Scope – Phạm vi dự án

## 5.1. In Scope – Phạm vi bắt buộc

| ID | Nhóm nghiệp vụ | Phạm vi |
|---|---|---|
| SC-01 | Quản lý phim | Quản lý những bộ phim rạp được khai thác |
| SC-02 | Quản lý phòng chiếu | Quản lý phòng, sức chứa và sơ đồ ghế |
| SC-03 | Quản lý suất chiếu | Lập lịch, kiểm tra xung đột, quản lý trạng thái mở bán |
| SC-04 | Quản lý giá vé | Một mức giá cơ bản áp dụng cho mỗi suất chiếu |
| SC-05 | Tra cứu lịch chiếu | Tìm kiếm phim và các suất chiếu liên quan |
| SC-06 | Bán vé tại quầy | Chọn ghế, tính tiền, ghi nhận giao dịch và phát hành vé |
| SC-07 | Quản lý vé và giao dịch | Tra cứu, theo dõi tình trạng bán vé và các giao dịch thành công |
| SC-08 | Báo cáo kinh doanh | Thống kê số vé bán, doanh thu và tỷ lệ lấp đầy |
| SC-09 | Phân quyền | Phân biệt quyền quản lý và nhân viên bán vé |
| SC-10 | Lưu trữ | Bảo toàn và khôi phục dữ liệu nghiệp vụ từ TXT |

## 5.2. Future Scope – Phạm vi mở rộng

Những nghiệp vụ sau được xem là hướng phát triển, **không nằm trong cam kết nghiệm thu phiên bản 1.0**:

| ID | Nghiệp vụ | Giá trị mang lại |
|---|---|---|
| FS-01 | Hủy vé và hoàn tiền | Giải quyết các yêu cầu thay đổi sau khi mua |
| FS-02 | Soát vé | Kiểm tra tính hợp lệ và trạng thái sử dụng vé |
| FS-03 | Chính sách giá linh hoạt | Giá theo ngày, loại ghế hoặc đối tượng khách |
| FS-04 | Quản lý ca thu ngân | Đối soát tiền mặt theo nhân viên và ca |
| FS-05 | Báo cáo nâng cao | Phân tích xu hướng bán vé và hiệu quả khai thác phòng |

## 5.3. Out of Scope – Ngoài phạm vi

Dự án không triển khai:

- Website hoặc ứng dụng đặt vé trực tuyến.
- Hệ thống hoạt động đồng thời trên nhiều máy.
- Quản lý chuỗi rạp và nhiều chi nhánh.
- Tích hợp thanh toán ngân hàng, ví điện tử hoặc cổng thanh toán.
- Quản lý kho, đồ ăn và thức uống.
- Quản lý nhân sự, chấm công và tính lương.
- Khách hàng thành viên và tích điểm.
- Quản lý hợp đồng phát hành phim.
- Quản lý kỹ thuật máy chiếu hoặc nội dung phim kỹ thuật số.
- Xử lý đổi phòng, đổi suất chiếu đã bán vé trong phiên bản cơ bản.

# 6. Business Concepts – Các khái niệm nghiệp vụ

Để tránh hiểu nhầm giữa các thành phần, dự án sử dụng những khái niệm sau:

| Khái niệm | Định nghĩa |
|---|---|
| Film – Phim | Một tác phẩm được rạp đăng ký để tổ chức chiếu |
| Auditorium – Phòng chiếu | Không gian vật lý có sức chứa và sơ đồ ghế xác định |
| Seat – Ghế | Một vị trí ngồi cụ thể thuộc một phòng chiếu |
| Screening – Suất chiếu | Một lần trình chiếu một bộ phim tại một phòng vào một thời điểm |
| Ticket – Vé | Quyền tham dự một suất chiếu tại một ghế xác định |
| Sale Transaction – Giao dịch bán vé | Hoạt động bán một hoặc nhiều vé và ghi nhận số tiền thanh toán |
| Ticket Price – Giá vé | Số tiền khách hàng phải trả để mua một vé theo chính sách áp dụng |
| Revenue – Doanh thu bán vé | Tổng giá trị các giao dịch bán vé hợp lệ trong phạm vi báo cáo |

### Những quan hệ nghiệp vụ quan trọng

- Một phim có thể được tổ chức thành nhiều suất chiếu.
- Một phòng chiếu có thể phục vụ nhiều suất chiếu ở các thời điểm khác nhau.
- Một phòng chiếu có nhiều ghế.
- Một suất chiếu có nhiều vé được bán.
- Một giao dịch bán vé có thể bao gồm nhiều vé.
- Một vé tương ứng với một ghế trong một suất chiếu cụ thể.
- Cùng một ghế vật lý có thể được bán ở những suất chiếu khác nhau.

Các khái niệm này mô tả nghiệp vụ, chưa quy định thiết kế bảng dữ liệu hoặc cấu trúc file.

# 7. Business Processes – Quy trình nghiệp vụ

## BP-01. Quản lý danh mục phim và phòng chiếu

**Mục tiêu:** Đảm bảo rạp có đầy đủ thông tin cần thiết để tổ chức suất chiếu.

**Người chịu trách nhiệm:** Quản lý rạp.

**Điều kiện đầu vào:** Quản lý đã đăng nhập và có quyền thực hiện nghiệp vụ.

**Quy trình chính:**

1. Quản lý đăng ký các bộ phim được rạp khai thác.
2. Quản lý ghi nhận thời lượng, thể loại, phân loại độ tuổi và thông tin cần thiết của phim.
3. Quản lý khai báo các phòng chiếu đang hoạt động.
4. Quản lý xác định sức chứa và vị trí các ghế trong từng phòng.
5. Rạp có thể sử dụng những thông tin này để tổ chức suất chiếu.

**Tình huống ngoại lệ:**

- Phim thiếu những thông tin cần thiết để lập lịch.
- Phòng chiếu có cấu hình ghế không hợp lệ.
- Yêu cầu ngừng sử dụng phòng đang có suất chiếu trong tương lai.

**Kết quả:** Danh mục phim và phòng chiếu hợp lệ, sẵn sàng phục vụ việc lập lịch.

## BP-02. Tổ chức và công bố lịch chiếu

**Mục tiêu:** Sử dụng hợp lý các phòng chiếu, tránh xung đột lịch và công bố những suất chiếu có thể bán vé.

**Người chịu trách nhiệm:** Quản lý rạp.

**Điều kiện đầu vào:**

- Phim đã được đăng ký.
- Phòng chiếu tồn tại và có thể sử dụng.
- Thời gian chiếu được xác định.

**Quy trình chính:**

1. Quản lý lựa chọn phim cần tổ chức chiếu.
2. Xác định ngày và thời gian bắt đầu.
3. Lựa chọn phòng chiếu.
4. Xác định thời gian sử dụng phòng dựa trên thời lượng phim và khoảng thời gian chuẩn bị phòng.
5. Kiểm tra khả năng sử dụng phòng tại khoảng thời gian đó.
6. Xác định giá vé áp dụng cho suất chiếu.
7. Xác nhận lịch chiếu.
8. Công bố suất chiếu để nhân viên có thể bán vé.

**Tình huống ngoại lệ:**

- Phòng đã có suất chiếu chồng lấn thời gian.
- Phim hoặc phòng không còn trong trạng thái khai thác.
- Thông tin thời gian hoặc giá vé không hợp lệ.

**Kết quả:** Một suất chiếu hợp lệ được tạo và có thể mở bán.

## BP-03. Bán vé tại quầy

**Mục tiêu:** Hoàn tất việc bán một hoặc nhiều vé đúng phim, đúng suất chiếu, đúng ghế và đúng số tiền.

**Người chịu trách nhiệm:** Nhân viên bán vé.

**Điều kiện đầu vào:**

- Nhân viên có quyền bán vé.
- Suất chiếu đang mở bán.
- Các ghế khách muốn mua chưa được bán.

**Quy trình chính:**

1. Khách hàng đưa ra yêu cầu xem phim.
2. Nhân viên tra cứu các phim và suất chiếu phù hợp.
3. Khách hàng lựa chọn suất chiếu.
4. Nhân viên kiểm tra tình trạng ghế.
5. Khách hàng lựa chọn một hoặc nhiều ghế.
6. Nhân viên thông báo số tiền cần thanh toán.
7. Khách hàng thanh toán tiền mặt.
8. Nhân viên xác nhận đã nhận đủ tiền.
9. Giao dịch được ghi nhận thành công.
10. Vé được phát hành cho khách hàng.
11. Những ghế thuộc giao dịch được xác định là đã bán.

**Tình huống ngoại lệ:**

- Không còn đủ ghế theo yêu cầu.
- Ghế được chọn đã bán.
- Suất chiếu không còn mở bán.
- Khách hàng thay đổi lựa chọn trước khi xác nhận thanh toán.
- Khách hàng không thanh toán hoặc từ chối giao dịch.
- Giao dịch không thể hoàn tất do lỗi ghi nhận dữ liệu.

**Kết quả khi thành công:**

- Giao dịch hợp lệ được ghi nhận.
- Khách hàng nhận đủ số vé đã mua.
- Các ghế được bán không thể tiếp tục bán trong cùng suất chiếu.
- Doanh thu được phản ánh theo giao dịch.

**Kết quả khi thất bại:** Không được tồn tại tình trạng giao dịch chỉ hoàn tất một phần khiến số vé, ghế và tiền không nhất quán.

## BP-04. Tra cứu thông tin vé và giao dịch

**Mục tiêu:** Hỗ trợ nhân viên xác minh thông tin bán vé và phục vụ công việc tại quầy.

**Người chịu trách nhiệm:** Nhân viên bán vé hoặc quản lý rạp.

**Quy trình chính:**

1. Nhân viên tiếp nhận yêu cầu tra cứu.
2. Xác định thông tin tìm kiếm phù hợp, chẳng hạn mã vé hoặc mã giao dịch.
3. Tra cứu thông tin đã ghi nhận.
4. Đối chiếu phim, suất chiếu, ghế, giá vé và trạng thái giao dịch.
5. Cung cấp kết quả cho người có quyền truy cập.

**Tình huống ngoại lệ:**

- Không tìm thấy giao dịch hoặc vé.
- Dữ liệu tìm kiếm không hợp lệ.
- Người dùng không có quyền xem thông tin cần tra cứu.

**Kết quả:** Thông tin vé và giao dịch được tra cứu chính xác.

## BP-05. Theo dõi và báo cáo doanh thu

**Mục tiêu:** Cung cấp thông tin để quản lý đánh giá kết quả bán vé.

**Người chịu trách nhiệm:** Quản lý rạp.

**Quy trình chính:**

1. Quản lý xác định phạm vi cần theo dõi.
2. Lựa chọn ngày, phim hoặc suất chiếu cần xem.
3. Tổng hợp số lượng vé đã bán.
4. Xác định doanh thu từ những giao dịch thành công.
5. Xác định lượng ghế đã bán và tỷ lệ lấp đầy.
6. Quản lý sử dụng kết quả để theo dõi hoạt động kinh doanh.

**Kết quả:** Có thông tin bán vé và doanh thu chính xác trong phạm vi yêu cầu.

*Phiên bản đầu chỉ báo cáo doanh thu bán vé, không tính lợi nhuận hoặc chi phí hoạt động của rạp.*

# 8. Business Requirements – Yêu cầu kinh doanh

Các yêu cầu dưới đây mô tả nhu cầu mà hoạt động kinh doanh cần được đáp ứng. Chúng chưa quy định giao diện, cấu trúc dữ liệu hoặc thuật toán triển khai.

| ID | Yêu cầu kinh doanh | Mục tiêu liên quan | Ưu tiên |
|---|---|---|---|
| BR-01 | Rạp cần quản lý tập trung thông tin các bộ phim đang được khai thác | BO-01 | Must |
| BR-02 | Rạp cần quản lý phòng chiếu, sức chứa và vị trí ghế | BO-01 | Must |
| BR-03 | Người quản lý cần tổ chức và công bố lịch chiếu theo phim, phòng và thời gian | BO-01, BO-02 | Must |
| BR-04 | Rạp cần ngăn chặn việc bố trí những suất chiếu chồng lấn trong cùng phòng | BO-02 | Must |
| BR-05 | Nhân viên cần tra cứu các suất chiếu đang mở bán và tình trạng ghế tương ứng | BO-03 | Must |
| BR-06 | Rạp cần xác định được giá bán cho từng suất chiếu | BO-03 | Must |
| BR-07 | Nhân viên cần bán một hoặc nhiều vé trong một giao dịch | BO-03 | Must |
| BR-08 | Rạp cần ngăn chặn việc bán trùng ghế trong cùng một suất chiếu | BO-04 | Must |
| BR-09 | Rạp cần đảm bảo chỉ những giao dịch được xác nhận thanh toán thành công mới phát hành vé và ghi nhận doanh thu | BO-03, BO-06 | Must |
| BR-10 | Nhân viên có thẩm quyền cần tra cứu được vé và giao dịch đã ghi nhận | BO-05 | Must |
| BR-11 | Người quản lý cần theo dõi số lượng vé bán và doanh thu theo thời gian, phim và suất chiếu | BO-06 | Must |
| BR-12 | Rạp cần kiểm soát việc thực hiện nghiệp vụ dựa trên trách nhiệm và quyền hạn người sử dụng | BO-07 | Must |
| BR-13 | Rạp cần bảo toàn thông tin nghiệp vụ đã xác nhận để có thể tiếp tục hoạt động ở lần sử dụng sau | BO-01, BO-05 | Must |
| BR-14 | Rạp cần duy trì tính nhất quán giữa giao dịch, vé, ghế đã bán và số liệu doanh thu | BO-03, BO-04, BO-06 | Must |

### Yêu cầu mở rộng không bắt buộc

| ID | Yêu cầu kinh doanh | Ưu tiên |
|---|---|---|
| BR-15 | Rạp có thể xử lý hủy vé, hoàn tiền theo chính sách được phê duyệt | Should |
| BR-16 | Nhân viên được cấp quyền có thể kiểm tra trạng thái sử dụng của vé | Should |
| BR-17 | Rạp có thể áp dụng nhiều chính sách giá tùy theo suất chiếu và loại ghế | Should |
| BR-18 | Quản lý có thể theo dõi và đối soát giao dịch theo ca thu ngân | Should |

# 9. Business Rules – Quy tắc nghiệp vụ

## 9.1. Quy tắc quản lý phim và phòng chiếu

| ID | Quy tắc |
|---|---|
| RULE-01 | Mỗi phim phải có mã định danh riêng để phân biệt các bộ phim |
| RULE-02 | Mỗi phòng chiếu phải có mã định danh riêng |
| RULE-03 | Mỗi ghế phải được xác định duy nhất trong phạm vi phòng chiếu |
| RULE-04 | Không được lập suất chiếu với phim hoặc phòng không tồn tại |
| RULE-05 | Không được xóa hoặc thay đổi thông tin quan trọng làm mất tính hợp lệ của những suất chiếu đã có vé bán |

## 9.2. Quy tắc tổ chức suất chiếu

| ID | Quy tắc |
|---|---|
| RULE-06 | Mỗi suất chiếu gắn với đúng một phim và một phòng chiếu |
| RULE-07 | Một phòng không thể được sử dụng bởi hai suất chiếu có khoảng thời gian sử dụng chồng lấn |
| RULE-08 | Thời gian sử dụng phòng phải tính đến thời lượng trình chiếu và khoảng nghỉ theo chính sách của rạp |
| RULE-09 | Chỉ suất chiếu đang mở bán mới được phép phát sinh giao dịch bán vé |
| RULE-10 | Khi suất chiếu đã có vé bán, không được tùy ý thay đổi phim, phòng hoặc thời gian của suất chiếu trong phiên bản cơ bản |

## 9.3. Quy tắc bán vé

| ID | Quy tắc |
|---|---|
| RULE-11 | Mỗi vé phải gắn với một suất chiếu và một ghế xác định |
| RULE-12 | Một ghế chỉ được bán thành công một lần trong một suất chiếu |
| RULE-13 | Một giao dịch có thể bao gồm nhiều vé nhưng không được chứa hai vé cho cùng một ghế trong cùng suất |
| RULE-14 | Giá trị giao dịch bằng tổng giá trị các vé thuộc giao dịch |
| RULE-15 | Vé chỉ được phát hành khi giao dịch đã được xác nhận thanh toán |
| RULE-16 | Nếu giao dịch không thành công, không được ghi nhận vé đã bán hoặc doanh thu từ giao dịch đó |
| RULE-17 | Một giao dịch bán nhiều vé phải được hoàn tất đầy đủ hoặc không được ghi nhận là thành công |
| RULE-18 | Các ghế đã bán của một suất chiếu không ảnh hưởng đến khả năng bán cùng vị trí ghế trong suất chiếu khác |

## 9.4. Quy tắc quản lý giao dịch và doanh thu

| ID | Quy tắc |
|---|---|
| RULE-19 | Mỗi giao dịch thành công có mã định danh riêng |
| RULE-20 | Các giao dịch thành công cần lưu lại thời điểm thực hiện và người thực hiện |
| RULE-21 | Doanh thu bán vé được xác định từ các giao dịch hợp lệ |
| RULE-22 | Các giao dịch đã xác nhận không được tùy ý xóa hoặc thay đổi giá trị để điều chỉnh báo cáo |
| RULE-23 | Giá trị vé đã bán phải được bảo toàn khi chính sách giá thay đổi về sau |

## 9.5. Quy tắc phân quyền

| ID | Quy tắc |
|---|---|
| RULE-24 | Chỉ người dùng có quyền quản lý mới được tổ chức và điều chỉnh lịch chiếu |
| RULE-25 | Chỉ người dùng được cấp quyền bán vé mới được xác nhận giao dịch bán vé |
| RULE-26 | Chỉ người dùng có quyền quản lý mới được truy cập báo cáo doanh thu tổng hợp |
| RULE-27 | Người dùng không được thực hiện nghiệp vụ vượt quá phạm vi quyền hạn được cấp |

Các quy tắc trên là đề xuất chính sách cho rạp giả định. Chúng cần được nhóm xác nhận trước khi khóa phiên bản BRD chính thức.

# 10. Business-Level Quality Requirements – Yêu cầu chất lượng

| ID | Yêu cầu | Kỳ vọng |
|---|---|---|
| BQR-01 | Chính xác | Không bán trùng ghế, không tính sai doanh thu |
| BQR-02 | Nhất quán | Thông tin vé, giao dịch và ghế luôn phù hợp với nhau |
| BQR-03 | Bảo toàn dữ liệu | Dữ liệu đã xác nhận được lưu và sử dụng lại |
| BQR-04 | Kiểm soát truy cập | Các vai trò chỉ thực hiện nghiệp vụ được phép |
| BQR-05 | Dễ sử dụng | Nhân viên có thể tra cứu và bán vé theo quy trình rõ ràng |
| BQR-06 | Truy vết | Có thể biết giao dịch nào đã diễn ra, thời điểm và nhân viên thực hiện |
| BQR-07 | Độc lập môi trường mạng | Các nghiệp vụ cơ bản không phụ thuộc kết nối Internet |

Những chỉ tiêu kỹ thuật cụ thể về thời gian phản hồi, xử lý lỗi và định dạng dữ liệu sẽ được xác định ở giai đoạn SRS.

# 11. Business Acceptance Criteria – Tiêu chí chấp nhận nghiệp vụ

| ID | Kịch bản nghiệm thu | Kết quả mong đợi |
|---|---|---|
| BAC-01 | Quản lý khai báo phim và phòng hợp lệ | Thông tin có thể được sử dụng để lập lịch |
| BAC-02 | Quản lý tạo một suất chiếu hợp lệ | Suất chiếu được ghi nhận và có thể mở bán |
| BAC-03 | Quản lý tạo suất chiếu trùng thời gian sử dụng cùng phòng | Yêu cầu bị từ chối, lịch cũ không bị ảnh hưởng |
| BAC-04 | Nhân viên mua hai ghế trong một giao dịch | Ghi nhận một giao dịch với hai vé đúng ghế |
| BAC-05 | Nhân viên thử bán ghế đã bán trong cùng suất | Giao dịch bị từ chối |
| BAC-06 | Nhân viên bán cùng vị trí ghế ở suất chiếu khác | Được phép nếu ghế còn trống |
| BAC-07 | Giao dịch không được xác nhận thanh toán | Không phát hành vé và không cộng doanh thu |
| BAC-08 | Giao dịch bán vé thành công | Vé, tình trạng ghế và doanh thu phù hợp với nhau |
| BAC-09 | Đóng rồi mở lại chương trình sau giao dịch thành công | Các thông tin đã xác nhận vẫn tồn tại và chính xác |
| BAC-10 | Nhân viên bán vé cố gắng thay đổi lịch chiếu | Không được phép |
| BAC-11 | Quản lý xem báo cáo doanh thu | Số liệu khớp với dữ liệu giao dịch hợp lệ |
| BAC-12 | Quản lý truy vấn một giao dịch đã thực hiện | Tìm thấy đầy đủ thông tin nghiệp vụ cần thiết |

# 12. Assumptions & Constraints – Giả định và ràng buộc

## 12.1. Giả định nghiệp vụ

Các giả định tạm dùng để giới hạn sản phẩm:

- Rạp chỉ có một địa điểm kinh doanh.
- Rạp có nhiều phòng chiếu nhưng không quản lý các chi nhánh khác.
- Khách hàng mua vé trực tiếp tại quầy.
- Nhân viên xác nhận việc nhận tiền mặt, không tích hợp thanh toán điện tử.
- Một suất chiếu có một mức giá vé cơ bản.
- Không quản lý hồ sơ hoặc tài khoản khách hàng.
- Không cho phép hủy vé và hoàn tiền trong phạm vi cốt lõi.
- Không thay đổi phim, phòng và thời gian của suất chiếu đã phát sinh vé bán.
- Dữ liệu chỉ được sử dụng trên một máy tính tại một thời điểm.

## 12.2. Ràng buộc dự án

- Hệ thống chạy offline.
- Không sử dụng các thành phần yêu cầu giao tiếp mạng.
- Không sử dụng DBMS.
- Dữ liệu cần được lưu trong các file văn bản TXT.
- Các cơ chế quản lý dữ liệu phải được tự xây dựng theo yêu cầu học tập.
- Không sử dụng dịch vụ trực tuyến bên ngoài cho quy trình chính.
- Phạm vi triển khai phải phù hợp năng lực và thời gian của nhóm sinh viên.

Ngôn ngữ lập trình và mức độ cho phép sử dụng thư viện cần được đối chiếu với đề bài chính thức trước giai đoạn thiết kế. BRD không tự quyết định lựa chọn này.

# 13. Business Risks – Rủi ro nghiệp vụ

| ID | Rủi ro | Tác động | Định hướng kiểm soát |
|---|---|---|---|
| RISK-01 | Tổ chức lịch chiếu chồng lấn | Không thể vận hành phòng theo kế hoạch | Kiểm tra xung đột trước khi xác nhận |
| RISK-02 | Bán trùng ghế | Tranh chấp giữa khách hàng | Đảm bảo ghế chỉ được bán thành công một lần mỗi suất |
| RISK-03 | Giao dịch ghi nhận không đầy đủ | Sai vé hoặc doanh thu | Chỉ xác nhận giao dịch khi toàn bộ dữ liệu nhất quán |
| RISK-04 | Mất hoặc hỏng dữ liệu TXT | Không thể tra cứu chính xác hoạt động trước đó | Cơ chế lưu, kiểm tra và khôi phục dữ liệu phù hợp |
| RISK-05 | Nhân viên thao tác không đúng quyền | Dữ liệu quan trọng bị thay đổi sai | Kiểm soát nghiệp vụ theo vai trò |
| RISK-06 | Phạm vi dự án mở rộng quá nhiều | Không đủ thời gian hoàn thành | Ưu tiên nghiệp vụ cốt lõi trước |
| RISK-07 | Chính sách kinh doanh chưa rõ | Các chức năng xử lý không nhất quán | Làm rõ quy tắc trước khi đặc tả chi tiết |

# 14. Open Questions – Những quyết định cần xác nhận

Các vấn đề sau chưa có quyết định cuối cùng. Đề xuất mặc định được đưa ra để thuận tiện cho việc chốt phạm vi.

| ID | Vấn đề cần làm rõ | Đề xuất mặc định |
|---|---|---|
| OQ-01 | Có được bán vé khi suất chiếu đã bắt đầu không? | Không |
| OQ-02 | Thời gian nghỉ giữa hai suất chiếu là bao lâu? | 15 phút, có thể điều chỉnh |
| OQ-03 | Có cần quản lý nhiều loại ghế, như VIP không? | Chưa, chỉ ghế tiêu chuẩn |
| OQ-04 | Có cần kiểm tra giấy tờ và điều kiện độ tuổi trong phần mềm không? | Lưu phân loại độ tuổi phim, nhân viên chịu trách nhiệm kiểm tra thực tế |
| OQ-05 | Có cần hủy và hoàn vé trong bản đầu không? | Không, để mở rộng |
| OQ-06 | Có cho phép đặt giữ ghế mà chưa thanh toán không? | Không, chỉ chọn ghế trong quá trình bán tại quầy |
| OQ-07 | Có cần quản lý ca thu ngân không? | Không, chỉ ghi nhận nhân viên thực hiện giao dịch |
| OQ-08 | Có cho phép quản lý kiêm thao tác bán vé không? | Có, nếu được cấp quyền phù hợp |
| OQ-09 | Khi ngừng khai thác phim hoặc phòng, các lịch chiếu tương lai sẽ xử lý ra sao? | Không cho phép ngừng khai thác nếu còn suất chiếu bị ảnh hưởng chưa được xử lý |

Các đề xuất mặc định chưa phải chính sách đã được phê duyệt. Khi nhóm đồng ý, chúng sẽ được chuyển thành Business Rules chính thức.

# 15. Requirements Traceability – Liên kết mục tiêu và yêu cầu

| Mục tiêu kinh doanh | Yêu cầu hỗ trợ | Tiêu chí nghiệm thu |
|---|---|---|
| BO-01 – Quản lý tập trung | BR-01, BR-02, BR-03, BR-13 | BAC-01, BAC-02, BAC-09 |
| BO-02 – Không xung đột lịch | BR-03, BR-04 | BAC-03 |
| BO-03 – Chuẩn hóa bán vé | BR-05, BR-06, BR-07, BR-09, BR-14 | BAC-04, BAC-07, BAC-08 |
| BO-04 – Không bán trùng ghế | BR-08, BR-14 | BAC-05, BAC-06 |
| BO-05 – Tra cứu thông tin | BR-10, BR-13 | BAC-09, BAC-12 |
| BO-06 – Báo cáo chính xác | BR-09, BR-11, BR-14 | BAC-07, BAC-08, BAC-11 |
| BO-07 – Kiểm soát quyền hạn | BR-12 | BAC-10 |

Bảng này giúp kiểm tra mỗi mục tiêu kinh doanh có yêu cầu đáp ứng và có cách nghiệm thu tương ứng.

# 16. Approval & Next Steps – Phê duyệt và bước tiếp theo

## 16.1. Điều kiện phê duyệt BRD

BRD có thể chuyển sang trạng thái được phê duyệt khi nhóm thống nhất:

- Mục tiêu và phạm vi sản phẩm.
- Người dùng và trách nhiệm của các vai trò.
- Các quy trình nghiệp vụ chính.
- Những quy tắc kinh doanh bắt buộc.
- Chính sách đối với các trường hợp ngoại lệ.
- Tiêu chí nghiệm thu nghiệp vụ.

## 16.2. Kết quả cần đạt

Sau khi BRD được thống nhất, nhóm phát triển có đủ cơ sở để xây dựng SRS, đặc tả cụ thể các chức năng, điều kiện xử lý, dữ liệu nghiệp vụ và yêu cầu chất lượng của phần mềm.

## 16.3. Kết luận

Cinema Management System được xác định là một ứng dụng offline hỗ trợ quản lý lịch chiếu và bán vé tại quầy của một rạp chiếu phim.

Sản phẩm tập trung giải quyết những vấn đề cốt lõi: tổ chức suất chiếu hợp lệ, kiểm soát ghế và vé, ghi nhận giao dịch chính xác, quản lý quyền hạn và theo dõi doanh thu.

Phạm vi này vừa đủ để mô phỏng một chu trình kinh doanh thực tế, đồng thời phù hợp với mục tiêu học tập là tự xây dựng các cơ chế lưu trữ và quản lý dữ liệu.

**Trạng thái tài liệu: Draft v0.1 – Chờ rà soát nghiệp vụ.**