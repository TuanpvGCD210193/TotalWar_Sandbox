# Feature Specification: Đồng Nhất Tâm Đội Hình & Khớp Chấm Vàng 100% (Formation Origin Alignment)

**Feature Branch**: `005-formation-origin-alignment`  
**Created**: 2026-08-27  
**Status**: In Progress  

---

## 1. Bản Chất Vấn Đề (Root Cause Analysis)

Khi di chuyển số lượng lớn đạo quân, các chấm vàng (Yellow Dots / Space Bar Grid / Move Preview) bị lệch lùi ra sau nửa thân đội hình so với vị trí lính đứng thực tế.

### 📌 Nguyên nhân toán học gốc rễ:
1. **Sự bất đồng nhất về Định nghĩa Gốc Tọa độ (Origin Mismatch)**:
   - `ActorLocation` & `GetFormationCenter()`: Được định nghĩa là **Tâm hình học (Geometric Centroid / Midpoint)** của 120 con lính.
   - Nhưng `CalculateSlotLayout()` và `RebuildSlots()`: Lại lấy gốc $(0, 0)$ là **Hàng 1 (Front Row)** của đạo quân và tính các hàng sau lùi dần về phía sau (`LocalX = -Row * Spacing`).
2. **Hậu quả**:
   - Khi truyền `Squad->GetActorLocation()` (Tâm ở giữa) vào hàm tính chấm vàng, hàm này hiểu nhầm Tâm ở giữa là Hàng 1, rồi vẽ tiếp các hàng sau lùi sâu ra phía sau!
   - 👉 **Dẫn đến toàn bộ lưới chấm vàng bị trôi tụt lùi ra sau đúng bằng một nửa độ sâu của đạo quân!**

---

## 2. Giải Pháp Chuẩn Hóa (Unification Architecture)

Đồng nhất **Gốc Tọa độ của Đạo quân (Formation Origin)** trên toàn bộ dự án về chuẩn **Tâm Hình Học (Centroid Origin)**:
- **Tâm $(0, 0)$** luôn là điểm chính giữa của khối chữ nhật:
  - Hàng 0 (Hàng đầu): Nằm ở $LocalX = +((Rows - 1) * Spacing) * 0.5f$.
  - Hàng cuối: Nằm ở $LocalX = -((Rows - 1) * Spacing) * 0.5f$.
- **Lợi ích**:
  - `ActorLocation`, `TargetDestination`, `Drag Placement Center`, `Space Bar Grid`, `Move Preview Grid` và `Soldier Positions` sẽ **khớp nhau từng milimet $100\%$ không bao giờ lệch dù chỉ $1$ pixel**!

---

## 3. Tiêu Chí Nghiệm Thu (Acceptance Criteria)

- [ ] **1. Khớp Chấm Vàng Tuyệt Đối**: Khi chọn 1 hoặc 20 đạo quân di chuyển đến bất kỳ đâu, khi tới đích, lính đứng khớp $100\%$ khít từng chấm vàng của Space Bar.
- [ ] **2. Move Preview & Group Drag Đồng Bộ**: Mũi tên và ô lưới chấm vàng trong lúc kéo rê chuột và lúc di chuyển trùng khớp hoàn hảo với vị trí lính sẽ dừng chân.
