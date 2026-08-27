# Technical Plan: Khóa Cố Định Góc Quay Frontline Anchor

## 1. Ranh Giới Module & Các Tệp Cần Cập Nhật

### 1. `FormationMovementComponent.cpp`:
- Trong hàm `StopMovement(Soldiers, SlotComponent, VisualComponent)`:
  - Khi đạo quân hoàn thành di chuyển:
    - Gọi `OwnerFormation->SetFacingDirection(FinalDesiredFacing);`
    - Cập nhật `OwnerFormation->SetActorRotation(FinalDesiredFacing.Rotation());`
    - Cập nhật SlotComponent: `SlotComponent->RebuildSlots(Soldiers.Num(), OwnerFormation->GetActorLocation(), FinalDesiredFacing);`
    - Gán lại lính vào slot: `SlotComponent->AssignSlotsByRankAndFileProjection(Soldiers, OwnerFormation->GetActorLocation(), FinalDesiredFacing);`
    - Đặt trạng thái về `EFormationState::Idle`.

### 2. `TWCommandComponent.cpp`:
- Đảm bảo khi người chơi nhả chuột kéo dàn trận (Drag Placement), `Command.FacingDirection` được chuẩn hóa `GetSafeNormal2D()` và truyền chính xác vào `ExecuteMoveCommand`.

### 3. `TWHUD.cpp`:
- Đảm bảo khi vẽ Space Bar (`IsSpaceBarHeld()`), hệ thống truy xuất `Squad->GetFacingDirection()` đã được đồng bộ chuẩn xác.

---

## 2. Verification Plan
- Biên dịch Live Coding (`Ctrl + Alt + F11`).
- Play game $\rightarrow$ Chọn 1 đạo quân $\rightarrow$ Giữ chuột phải kéo xoay chéo $45^\circ$ $\rightarrow$ Chờ lính bước tới đích $\rightarrow$ Giữ phím **Space Bar**.
- **Kỳ vọng**: Lưới chấm vàng của Space Bar xoay chéo đúng $45^\circ$, trùng khít từng chấm vàng vào từng con lính!
