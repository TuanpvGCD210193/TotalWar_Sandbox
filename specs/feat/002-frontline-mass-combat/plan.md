# Technical Plan: 004-frontline-mass-combat

## 1. Các Module Cần Cập Nhật

### 1. `FormationMovementComponent.cpp`:
- **Mass Ratio & Impact Calculation**:
  - `MassRatio = AttackerMass / DefenderMass;`
  - Nếu `MassRatio <= 1.05f` $\rightarrow$ `KnockbackDist = 0.0f;` (Bộ binh vs Bộ binh đứng im $100\%$).
- **Frontline Contact & Stop**:
  - Khi Hàng 1 của quân tấn công chạm Hàng 1 của quân phòng thủ ($Dist \le 85\text{cm}$):
    - Đặt `bIsMoving = false;` (Dừng bước hoàn toàn).
    - Cả 2 squad chuyển `EFormationState::Engage`.
    - Trong trạng thái `Engage`, vận tốc `DesiredVelocity = FVector::ZeroVector;` và `SeparationForce = FVector::ZeroVector;` $\rightarrow$ Triệt tiêu $100\%$ trôi lùi và xé đội hình!
- **Rotation Blending**:
  - Lính Hàng 1 tự xoay mặt đối diện với kẻ thù trước mặt để vung kiếm.

### 2. `CombatSubsystem.cpp`:
- Duy trì nhịp đánh 10Hz.
- Khi toàn bộ quân Đỏ chết: Chuyển quân Xanh về `Idle` và gọi `RebuildFormationAndRegroup()`.

---

## 2. Verification Plan
- Live Coding $\rightarrow$ Play $\rightarrow$ Click tấn công $\rightarrow$ Kiểm tra:
  1. Quân Đỏ đứng im như tường đá, không bị đẩy lùi.
  2. Quân Xanh lao tới chạm mặt là dừng, 2 bên tạo thành 2 khối chữ nhật áp sát nhau.
  3. Không có bất kỳ lính nào tách đôi hay trôi dạt.
  4. Đánh xong tự dàn lại hàng ngũ.
