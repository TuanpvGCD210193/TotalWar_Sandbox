# Technical Plan: Refactor Hệ Thống Giao Tranh "Combat Envelope"

## 1. Ranh Giới Module & Các Phần Code Cần Xóa Bỏ / Viết Mới

### ❌ Xóa Bỏ Các Logic Cũ Gây Lỗi (Legacy Code Removal):
1. **Xóa bỏ việc xây dựng slot giả lập trước mặt địch trong `ExecuteMoveCommand`**.
2. **Xóa bỏ các công thức tính khoảng cách cộng dồn `MyHalfDepth + EnemyHalfDepth`**.
3. **Xóa bỏ lực cản `SeparationForce` quá mức ($260\text{cm/s}$) gây hiện tượng "khiên chắn vô hình"**.
4. **Xóa bỏ lệnh gán cứng `Position = SlotPosition` khi có lính chết trong `KillSoldier` / `RebuildFormationAndRegroup`**.

---

### ✨ Viết Mới Kiến Trúc Combat Envelope:

1. **`FormationMovementComponent.h/.cpp`**:
   - Khi nhận lệnh `Attack`:
     - Điểm đến tổng quát của cả đạo quân là tâm của quân địch: `TargetDestination = EnemyFormation->GetActorLocation()`.
     - Xây dựng slot di chuyển tổng quát hướng về phía quân địch.
     - Trong lúc tiếp cận: Lính chạy theo slot bình thường.
     - Khi cách quân địch $< 30\text{m}$: Kích hoạt `Charge` tăng tốc $+80\%$.
     - Khi lính hàng 1 chạm cự ly $< 2.5\text{m}$ với lính địch: Chuyển sang `EFormationState::Engage` (Combat Envelope).
   - Trong chế độ `Engage` (Combat Envelope):
     - Mỗi con lính tìm con lính địch gần nhất qua `SpatialGridSubsystem`.
     - Nếu khoảng cách tới địch $> 100\text{cm}$: Bước tới với vận tốc nhẹ ($120\text{cm/s}$).
     - Nếu khoảng cách $< 80\text{cm}$: Đứng lại trong tư thế chém kiếm (`ESoldierState::Fighting`), xoay mặt vào đối thủ.
     - Lực tách đồng đội mềm mại ($40\text{cm}$) giữ lính không dính chùm vào nhau.

2. **`CombatSubsystem.h/.cpp`**:
   - Chạy vòng lặp 10Hz, tìm lính địch trong tầm đánh $150\text{cm}$.
   - Thực hiện tính toán trúng đòn, giáp, sát thương và Charge bonus.
   - Khi mục tiêu bị tiêu diệt hết: Gọi `AttackerSquad->SetFormationState(EFormationState::Idle)` và tự động tái lập hàng ngũ tại chỗ (`RebuildFormationAndRegroup`).

3. **`FormationActor.h/.cpp`**:
   - `Tick()` chạy liên tục, đồng bộ `SyncVisualTransforms()` mỗi khung hình để đảm bảo hình ảnh 100% mượt mà trên GPU.

---

## 2. Kế Hoạch Kiểm Thử Nghiệm Thu (Verification Plan)

1. **Test 1: Tiếp cận & Xung phong**:
   - Chọn quân Xanh $\rightarrow$ Click chuột phải vào quân Đỏ.
   - **Kỳ vọng**: Quân Xanh chạy thẳng hàng, tăng tốc khi cách $30\text{m}$, lao vào tiếp xúc mượt mà $100\%$, không có giật cục.
2. **Test 2: Giao chiến & Tiêu diệt**:
   - Hai bên chém nhau nảy lửa, lính Đỏ ngã xuống liên tục, lính Xanh tự bước tới trám chỗ.
3. **Test 3: Tái lập đội hình**:
   - Khi quân Đỏ chết hết, quân Xanh đứng lại, tự động dàn lại khối chữ nhật nghiêm chỉnh ngay tại chỗ.
