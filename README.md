# CinemaDataStructures

Dự án này xây dựng một thư viện cấu trúc dữ liệu tự cài đặt bằng C++ cho hệ thống quản lý rạp chiếu phim.

## Cấu trúc dự án

- DataStructures/: chứa các lớp template cho danh sách liên kết kép, bảng băm, cây tìm kiếm nhị phân, hàng đợi, ngăn xếp và hàng đợi ưu tiên.
- main.cpp: minh họa cách khởi tạo và gọi các cấu trúc dữ liệu.
- docs/CauTrucDuLieu.md: giải thích nguyên lý, độ phức tạp, sơ đồ ERD và sơ đồ chức năng cho từng cấu trúc.
- docs/example.md: ví dụ ứng dụng các cấu trúc dữ liệu vào tình huống quản lý rạp chiếu phim.

Các thư mục Models và Tests đã được loại bỏ. Các lớp dữ liệu thực tế có thể được bổ sung trong module ứng dụng khi cần; `main.cpp` hiện dùng kiểu cơ bản để minh họa API.

## Cách tổ chức template

Trong mỗi file `DataStructures/*.h`, phần khai báo hàm nằm trong lớp và phần định nghĩa được viết bên ngoài lớp. Định nghĩa template vẫn nằm trong file header để compiler có thể tạo mã cho các kiểu dữ liệu được sử dụng.

## Biên dịch

Mở terminal trong thư mục gốc của dự án và chạy:

```bash
g++ -std=c++11 -Wall -Wextra -pedantic main.cpp -o CinemaDataStructures
```

## Chạy

```bash
./CinemaDataStructures
```

Hoặc trên Windows PowerShell:

```powershell
.\CinemaDataStructures.exe
```

## Mô tả ngắn về từng cấu trúc

- DoublyLinkedList: lưu trữ dữ liệu bằng danh sách liên kết kép với khả năng thêm, xóa, duyệt và truy cập theo vị trí.
- HashTable: lưu trữ theo khóa, xử lý va chạm bằng bucket và cho phép cập nhật giá trị khi khóa đã tồn tại.
- BinarySearchTree: thực hiện chèn, xóa, tìm kiếm và duyệt theo thứ tự.
- Queue: theo nguyên tắc FIFO.
- Stack: theo nguyên tắc LIFO.
- PriorityQueue: dùng Max-Heap dạng cây, giá trị lớn hơn được lấy trước.

## Quy định sử dụng

- Không sử dụng std::vector, std::list, std::map, std::unordered_map, std::queue và std::priority_queue trong phần cài đặt CTDL.
- Không dùng mảng động hay mảng tĩnh để thay thế cấu trúc dữ liệu chính.
- Mỗi cấu trúc đều được cài đặt độc lập trong một file header riêng.
