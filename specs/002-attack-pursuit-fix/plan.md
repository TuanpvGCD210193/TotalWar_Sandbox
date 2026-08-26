# Technical Plan: 002-attack-pursuit-fix

## 1. Ranh Giới Module & Files Chịu Trách Nhiệm

- **`FormationMovementComponent.h` & `.cpp`**:
  - `ExecuteMoveCommand`:
    - Khi nhận lệnh `Attack`: Tính toán `TargetDestination` ở vị trí tiếp xúc tiền tuyến (`EnemyLocation - DirToEnemy * FrontlineOffset`).
    - Gọi `SlotComponent->RebuildSlots(...)` và `SlotComponent->AssignSlotsByRankAndFileProjection(...)` ngay lập tức để toàn bộ lính nhận slot đích và bước đi!
  - `UpdateMovement`:
    - Chỉ kích hoạt Knockback và `StopMovement` khi **Khoảng cách giữa mép tiền tuyến quân ta và mép tiền tuyến quân địch $< 100\text{cm}$**.
    - Sử dụng biến `bHasImpacted` và kiểm tra cự ly thực tế giữa các hàng lính đầu tiên.

- **`FormationActor.cpp`**:
  - Đảm bảo `SetActorLocation` luôn bám sát tâm trung bình (Centroid) của các con lính sống sót.

---

## 2. API Contract & Thay Đổi Cấu Trúc Dữ Liệu

- Không thêm biến rác, tận dụng 100% các biến đã có sẵn (`TargetDestination`, `FinalDesiredFacing`, `TargetEnemyFormation`, `bHasImpacted`).

---

## 3. Kế Hoạch Kiểm Thử (Verification Plan)

1. **Test 1: Lệnh Tấn Công Từ Xa**:
   - Chọn quân Xanh $\rightarrow$ Click chuột phải vào quân Đỏ từ xa ($> 30\text{m}$).
   - **Kỳ vọng**: Mũi tên Đỏ xuất hiện, quân Đỏ đứng yên **KHÔNG bị dịch chuyển**, quân Xanh bước đều tiến lên.
2. **Test 2: Xung Phong & Va Chạm**:
   - Khi cách $< 30\text{m}$, quân Xanh tăng tốc Charge.
   - Khi chạm sát hàng lính Đỏ, cú húc đẩy giật lùi lính Đỏ và 2 bên giáp lá cà đối mặt nhau phẳng phiu trên mặt đất.
