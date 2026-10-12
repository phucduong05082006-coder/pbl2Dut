# Du lieu mau FRAME

3 phim gia lap theo wireframe, 3 phong (6x8, 5x8, 6x8), 136 ghe STANDARD.
Hang A gan man hinh, ma ghe A01...F08. Khong co lich chieu, ve hay giao dich.
Co hai tai khoan mau theo wireframe:

- Admin: `manager` / `demo123`.
- Nhan vien: `seller01` / `demo123`.

Mat khau trong users.txt duoc bam, khong luu plaintext. Day la tai khoan demo
cong khai, chi dung voi thu muc mau rieng, khong dung cho du lieu van hanh.
Dang nhap chi nhap username va mat khau; khong tao tai khoan khi dang nhap.

Sao chep 7 file TXT sang mot thu muc rieng, sau do chay:

```bash
mkdir -p "$HOME/.local/share/FRAME/CinemaDemo"
cp -n data/demo/*.txt "$HOME/.local/share/FRAME/CinemaDemo/"
/tmp/cinema-qt-build/cinema_app --data-dir "$HOME/.local/share/FRAME/CinemaDemo"
```

`cp -n` khong ghi de file da co. Neu da co ban mau cu voi users.txt rong,
hay dung mot thu muc demo moi; khong ghi de tai khoan da duoc tao.
Khong sao chep vao thu muc du lieu that.
App doc du lieu TXT, khong doc du lieu JavaScript trong docs/design/wireframes/wireframe.html.
Chon Phim de xem phim; Phong & ghe de xem so do tung phong.
So do phong khong the hien ghe da ban: trang thai do phai gan voi suat chieu.

## Demo day du luong ban ve

Dung thu muc moi/trong; lenh sau tu tao 6 suat mo ban luc 19:00 ngay mai va
ngay kia, ban mau 2 ve o moi phong trong ngay mai (6 ve, 480000 VND).
Ngay duoc tinh luc chay lenh, khong hardcode ngay het han trong source.

```bash
DEMO_DIR="$(mktemp -d /tmp/frame-demo.XXXXXX)"
/tmp/cinema-mvp-release/cinema_app --prepare-demo --data-dir "$DEMO_DIR"
/tmp/cinema-mvp-release/cinema_app --data-dir "$DEMO_DIR"
```

Lenh prepare tu choi neu thu muc da co du lieu, hoac dang duoc app su dung.
Chi account demo co mat khau cong khai; tao Admin van hanh rieng bang
`cinema_app --setup-admin --data-dir <thu-muc-van-hanh>`.
Tai lieu day du: `docs/product/README.md`.
