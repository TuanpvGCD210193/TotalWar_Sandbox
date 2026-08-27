# Technical Plan: 005-formation-origin-alignment

## 1. Các Module Cần Đồng Bộ Hóa

### 1. `FormationSlotComponent.cpp` & `FormationSubsystem.cpp`:
- Chuẩn hóa công thức sinh Slot (`RebuildSlots` & `CalculateSlotLayout`):
  ```cpp
  const float HalfDepthOffset = ((Rows - 1) * Spacing) * 0.5f;
  const float LocalX = HalfDepthOffset - (Row * Spacing);
  ```
  - Khi $Row = 0 \rightarrow LocalX = +HalfDepthOffset$ (Nửa trước).
  - Khi $Row = Rows-1 \rightarrow LocalX = -HalfDepthOffset$ (Nửa sau).
  - Điểm $(0, 0)$ nằm **chính xác ở tâm hình học của đạo quân**!

### 2. `TWHUD.cpp`:
- Đảm bảo Space Bar và Move Order Indicator lấy trực tiếp Slots thực tế hoặc gọi `CalculateSlotLayout` với tâm đồng nhất.

---

## 2. Verification Plan
- Live Coding $\rightarrow$ Chọn cả đại quân (20 đạo quân) $\rightarrow$ Giữ chuột phải kéo dàn trận $\rightarrow$ Thả chuột $\rightarrow$ Giữ Space Bar.
- **Kỳ vọng**: 20 đạo quân hành quân và dừng lại khớp $100\%$ từng chấm vàng của Space Bar, không lệch một ly!
