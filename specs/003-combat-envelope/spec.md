# Feature Specification: Hệ Thống Giao Tranh "Combat Envelope" (Vùng Giao Tranh Tự Do Chuẩn Total War)

**Feature Branch**: `003-combat-envelope`  
**Created**: 2026-08-26  
**Status**: Draft (Ready for Review)  

---

## 1. Triết Lý Thiết Kế & Mục Tiêu (Vision & Rationale)

Thay vì ép lính phải chạy vào các "Slot cứng" giả lập trước mặt quân địch (gây méo mó, giật cục, lệch góc khi giao tranh), kiến trúc **Combat Envelope** phân chia rõ ràng 2 chế độ chuyển động:

1. **Chế Độ Di Chuyển Bình Thường (`March` / `Idle`)**:
   - Lính tuân thủ $100\%$ theo Slot đội hình vuông vức, di chuyển đồng bộ, thẳng hàng như trong sách giáo khoa.
2. **Chế Độ Giao Tranh Tự Do (`Engage` - Combat Envelope)**:
   - Khi tiếp cận quân địch ($< 3\text{m}$ hoặc có va chạm), lính **tạm thời thoát ly khỏi Slot cứng**.
   - Mỗi con lính trở thành một Agent vi mô độc lập:
     - Dùng `SpatialGridSubsystem` tìm con lính địch gần nhất trong bán kính $2.5\text{m}$.
     - Tự động bước tới áp sát mục tiêu ở cự ly chém kiếm ($70 - 130\text{cm}$).
     - Xoay mặt trực diện vào đối thủ và vung kiếm chém nhau.
     - Lính hàng sau tự động tiến lên trám vào các khoảng trống tiền tuyến.
3. **Chế Độ Tái Lập Đội Hình Khi Trận Đánh Kết Thúc (Post-Battle Regroup)**:
   - Khi toàn bộ quân địch bị tiêu diệt: Hệ thống tự động tính tâm trung bình của các con lính sống sót, xây lại Slot vuông vức và lính tự động bước vào hàng ngũ!

---

## 2. Tiêu Chí Nghiệm Thu (Acceptance Criteria)

- [ ] **1. Tiếp Cận Mượt Mà**: Quân Xanh chạy đều đặn tới gần quân Đỏ $\rightarrow$ Khi cách $< 30\text{m}$ tăng tốc Charge $\rightarrow$ Lao sầm vào quân Đỏ mà không có bất kỳ con lính nào bị giật, nhảy cóc hay tách đôi đội hình.
- [ ] **2. Tự Động Bắt Cặp Giao Chiến (Dynamic 1-1 & Local Clashing)**: Lính hàng đầu 2 bên chạm nhau, tự động khóa mục tiêu và chém nhau liên tục.
- [ ] **3. Lính Hàng Sau Trám Chỗ (Battleline Reinforcement)**: Khi lính hàng đầu phe Đỏ chết, lính phe Xanh bước tới hoặc lính phe Đỏ phía sau dồn lên ép chiến tuyến.
- [ ] **4. Không Rác Dữ Liệu & 0 Ghost Instances**: Lính chết lập tức biến mất sạch sẽ, không để lại bóng ma trên màn hình.
- [ ] **5. Tự Động Thu Hồi Đội Hình Sau Khi Thắng**: Khi toàn bộ quân Đỏ chết hết, quân Xanh tự động đứng nghiêm trang, dàn lại đội hình chữ nhật ngay tại bãi chiến trường.

---

## 3. Danh Sách 6 Kịch Bản Lỗi Cần Triệt Tiêu (Edge Cases)

- [ ] **Case 1: Click Tấn Công Từ Xa ($> 50\text{m}$)**: Hành quân bằng Slot cứng, khi tới $< 30\text{m}$ bật Charge, khi $< 3\text{m}$ mở Combat Envelope.
- [ ] **Case 2: Click Tấn Công Khi Đã Ở Sát Mặt ($< 3\text{m}$)**: Ngay lập tức mở Combat Envelope mà không bị giật lùi vị trí.
- [ ] **Case 3: Bị Đánh Sườn Hoặc Đánh Sau Lưng (Flank / Rear Attack)**: Lính bị đánh sườn tự động xoay mặt $90^\circ$ về phía kẻ thù để đỡ đòn, không bị gò bó theo hướng quay của cờ hiệu.
- [ ] **Case 4: Rút Lui Giữa Trận (Disengage / Fallback)**: Khi người chơi ra lệnh Move chạy ra chỗ khác $\rightarrow$ Lập tức thoát khỏi Combat Envelope, khóa lại Slot cứng và chạy thẳng về điểm chỉ định.
- [ ] **Case 5: Quân Địch Chết Sạch (Enemy Wiped Out)**: Trạng thái tự động chuyển từ `Engage` $\rightarrow$ `Idle`, tái lập đội hình sống tại chỗ.
- [ ] **Case 6: Đổi Mục Tiêu Tấn Công Đột Ngột**: Chuyển ngay Combat Envelope sang mục tiêu mới mà không bị đơ.
