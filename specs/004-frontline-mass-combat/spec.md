# Feature Specification: Giao Tranh Tiền Tuyến & Khối Lượng Va Chạm (Total War Frontline Mass Combat)

**Feature Branch**: `004-frontline-mass-combat`  
**Created**: 2026-08-26  
**Status**: In Progress  

---

## 1. Triết Lý Thiết Kế (Vision & Principles)

Triển khai cơ chế va chạm tiền tuyến chuẩn $100\%$ Total War:
1. **Mass Ratio Va Chạm (Mass-Based Impulse)**:
   - Khi 2 đạo quân có Mass bằng nhau ($Mass_A / Mass_B \le 1.05$): Lực đẩy lùi $= 0$. Quân phòng thủ đứng im $100\%$, quân tấn công dừng bước và khóa chặt tiền tuyến.
   - Khi Mass chênh lệch (Kỵ binh húc Bộ binh, $Mass_A / Mass_B > 1.2$): Lực húc Knockback được kích hoạt đúng 1 lần tại khoảnh khắc va chạm.
2. **Khóa Tiền Tuyến (Frontline Lock & Zero Drift)**:
   - Hai đạo quân áp sát nhau ở cự ly $\approx 80\text{cm}$ và dừng chuyển động.
   - Tuyệt đối không có lực đẩy trôi lùi hay xé lẻ đội hình.
3. **Tiếp Ứng Tiền Tuyến (Rank Reinforcement)**:
   - Hàng 1 chém nhau, các hàng sau đứng nghiêm trang giữ chiều sâu.
   - Khi lính Hàng 1 chết, lính Hàng 2 bước lên trám chỗ.
4. **Tái Lập Đội Hình Chiến Thắng (Post-Battle Regroup)**:
   - Khi địch bị tiêu diệt hết, quân chiến thắng tự động xếp lại hình chữ nhật ngay tại chỗ.

---

## 2. Tiêu Chí Nghiệm Thu (Acceptance Criteria)

- [ ] **1. Dừng Đúng Cự Ly & Khóa Tiền Tuyến**: Quân Xanh lao tới, chạm Hàng 1 quân Đỏ là dừng ngay, hai bên khóa chặt mặt tiếp xúc phẳng phiu.
- [ ] **2. 0% Trôi Lùi & 0% Xé Đội Hình**: Quân Đỏ đứng im như một bức tường đá, không bị trôi lùi hay tách đôi lính ra sau.
- [ ] **3. Chém Nhau Liên Tục & Trám Chỗ**: Lính hàng đầu giao chiến, lính ngã xuống thì lính sau bước lên trám chỗ.
- [ ] **4. Tự Động Thu Hồi Sau Trận Đánh**: Quét sạch quân Đỏ thì quân Xanh tự động đứng nghiêm trang, dàn lại hình chữ nhật phẳng phiu.
