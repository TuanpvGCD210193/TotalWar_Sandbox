# Feature Specification: Sửa Lỗi Tấn Công, Truy Kích & Va Chạm Tiền Tuyến (Attack Pursuit & Frontline Impact Fix)

**Feature Branch**: `002-attack-pursuit-fix`  
**Created**: 2026-08-26  
**Status**: Draft (Spec Ready for Review)  

---

## 1. Bối Cảnh & Mô Tả Lỗi Thực Tế (Bug Description)

### ❌ Lỗi 1: Đạo quân Xanh đứng yên / dừng khựng lại không tiến lên tấn công
- **Hiện tượng**: Khi chọn quân Xanh và click chuột phải vào quân Đỏ (lệnh `Attack`), mũi tên Đỏ xuất hiện nhưng quân Xanh bị nén cụm lại và đứng im, không chịu hành quân tiến lên húc quân Đỏ.
- **Nguyên nhân cốt lõi**: Trong `FormationMovementComponent::ExecuteMoveCommand`, nhánh `CommandType == Attack` bị `return;` sớm trước khi gọi `SlotComponent->RebuildSlots(...)` và `AssignSlotsByRankAndFileProjection(...)`, khiến các con lính không nhận được tọa độ slot đích mới về phía quân địch.

### ❌ Lỗi 2: Quân Đỏ bị dịch chuyển (Teleport/Shift) ngay khi vừa click chuột
- **Hiện tượng**: Ngay khi click chuột vào quân Đỏ từ xa, quân Đỏ bị giật lùi/dịch chuyển một đoạn ngay lập tức dù quân Xanh chưa hề lao tới chạm vào.
- **Nguyên nhân cốt lõi**: Điều kiện kiểm tra cự ly va chạm `DistToEnemy <= MeleeThreshold` trong `UpdateMovement` bị kích hoạt quá sớm trên frame 1 $\rightarrow$ Lập tức gọi vòng lặp Knockback làm quân Đỏ bị dịch chuyển vị trí và gọi `StopMovement` làm quân Xanh dừng lại ngay lập tức!

---

## 2. Tiêu Chí Nghiệm Thu Chuẩn Total War (Acceptance Criteria)

1. **Truy Kích Mượt Mà (Smooth Pursuit & March)**:
   - Khi ra lệnh `Attack`: Quân Xanh ngay lập tức bước đều trong đội hình thẳng hàng, tiến về phía đạo quân Đỏ.
   - Khi cách quân Đỏ $< 30\text{m}$: Tự động chuyển sang `EFormationState::Charge` tăng tốc $+80\%$ lao vào.
2. **Khoảnh Khắc Va Chạm Thực Tế (True Physical Impact at Frontline)**:
   - Knockback **CHỈ ĐƯỢC PHÉP KÍCH HOẠT** khi hàng lính đầu tiên của quân Xanh thực sự tiếp xúc với hàng lính đầu tiên của quân Đỏ ở cự ly $< 100\text{cm}$ (Front-to-Front distance).
   - Tuyệt đối $0\%$ dịch chuyển quân Đỏ khi đang ở khoảng cách xa!
3. **Căn Chỉnh Tiền Tuyến Thẳng Hàng (Aligned Frontline)**:
   - Hai đạo quân dừng lại đối mặt nhau ở tiền tuyến, không bị chui vào bụng nhau và không bị bay lên trời.

---

## 3. Phân Tích Kịch Bản Lỗi (Edge Cases Analysis)

- [ ] **Case 1: Click Attack Từ Khoảng Cách Xa ($> 30\text{m}$)**:
  - Quân Xanh xây dựng slot đích ở mép chiến tuyến của quân Đỏ, hành quân `March` tiếp cận, tuyệt đối không trigger Knockback sớm.
- [ ] **Case 2: Chuyển Sang Xung Phong Nước Rút ($< 30\text{m}$)**:
  - Tốc độ tăng lên $540\text{cm/s}$, lính giữ vững hàng ngũ, mắt nhìn thẳng vào mục tiêu.
- [ ] **Case 3: Chạm Tiền Tuyến Thực Tế ($< 100\text{cm}$)**:
  - Kích hoạt sự kiện Impact: Áp dụng Knockback đẩy lùi lính Đỏ, bắt đầu đếm ngược 10s Charge Bonus, chuyển state sang `Engage`.
- [ ] **Case 4: Đổi Mục Tiêu Đột Ngột (Target Switch)**:
  - Khi đang lao vào Squad Đỏ 1, người chơi click sang Squad Đỏ 2 $\rightarrow$ Reset trạng thái Impact, tính toán lại slot đích về phía Squad Đỏ 2.
- [ ] **Case 5: Hủy Lệnh Tấn Công Bằng Lệnh Di Chuyển (Move Override)**:
  - Người chơi click ra bãi đất trống $\rightarrow$ Lập tức hủy bỏ `TargetEnemyFormation`, tốc độ trở về bình thường và đi đến điểm chỉ định.
