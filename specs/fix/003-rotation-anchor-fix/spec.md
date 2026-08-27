# Bug Fix Specification: Khóa Cố Định Góc Quay Frontline Anchor & Đồng Bộ Space Bar

**Category**: `fix`  
**Feature Branch / Path**: `specs/fix/003-rotation-anchor-fix`  
**Created**: 2026-08-27  
**Status**: Ready for Review  

---

## 1. Triết Lý & Vấn Đề Cốt Lõi (Core Problem & Philosophy)

### 📌 Bản Chất Lỗi:
- Khi người chơi giữ chuột phải kéo rê trên mặt đất để điều động quân $\rightarrow$ Hướng kéo chuột xác định **Góc Quay Mặt (Target Facing / Rotation)** và **Độ Rộng Hàng Ngũ (Formation Width)**.
- Khi lính hành quân tới đích và dừng lại (`StopMovement`) $\rightarrow$ Hàm dừng lại không cập nhật góc quay mới này vào biến `FacingDirection` của `AFormationActor`.
- **Hậu quả**:
  - `AFormationActor::FacingDirection` vẫn giữ góc quay cũ.
  - Khi người chơi bấm giữ **Space Bar** $\rightarrow$ Hệ thống vẽ các chấm vàng theo góc quay cũ, khiến lưới chấm vàng bị xoay chéo lệch góc đâm xuyên qua người lính như trong các bức ảnh nghiệm thu!

---

## 2. Tiêu Chí Nghiệm Thu (Acceptance Criteria)

- [ ] **1. Khóa Cứng Góc Quay Frontline Anchor**: Bất kể người chơi kéo chuột xoay góc bao nhiêu độ ($30^\circ, 45^\circ, 90^\circ, 180^\circ$), khi đạo quân tới đích, `FacingDirection` của đạo quân được cập nhật chính xác $100\%$ theo góc quay đã kéo.
- [ ] **2. Đồng Bộ Hoàn Hảo Với Space Bar Marker**: Khi bấm giữ phím Space Bar ở trạng thái đứng yên (Idle) hoặc đang đi (Moving), lưới chấm vàng luôn xoay đúng hướng mặt của đạo quân và bao trọn vừa khít các con lính.
- [ ] **3. Bảo Toàn Logic Hành Quân & Giao Tranh**: Việc cập nhật này hoàn toàn không làm gián đoạn chuyển động động học liên tục (Kinematics) hay cơ chế giao tranh mặt tiền tuyến (Frontline Lock).

---

## 3. Ma Trận 5 Kịch Bản Lỗi Biên (Edge Cases)

- [ ] **Case 1: Kéo chuột đảo ngược $180^\circ$ (Quay lưng lại)**: Lính quay đầu $180^\circ$ tại chỗ, chấm vàng Space Bar đảo chiều theo đúng góc $180^\circ$.
- [ ] **Case 2: Kéo chuột chéo $45^\circ$ với đội hình hẹp (2 columns, nhiều rows)**: Lưới chấm vàng xoay chéo $45^\circ$ song song tuyệt đối với hàng ngũ.
- [ ] **Case 3: Ra lệnh di chuyển liên tiếp khi chưa tới đích (Chained Move Orders)**: Mỗi lệnh mới lập tức ghi đè góc quay đích mới mà không bị xung đột với góc quay cũ.
- [ ] **Case 4: Dàn trận nhiều đạo quân cùng lúc (Multi-Squad Drag Battleline)**: Tất cả các đạo quân trong phòng tuyến đều nhận chung hướng quay và khóa góc quay đồng bộ khi tới đích.
- [ ] **Case 5: Giao chiến cận chiến (Engage State)**: Khi lính cận chiến, góc quay của lính tạm thời xoay vào đối thủ trước mặt, nhưng khi trận đánh kết thúc, đạo quân tự dàn lại theo hướng nhìn chiến thuật.
