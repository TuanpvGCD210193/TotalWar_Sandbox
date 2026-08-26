# 🚀 CẨM NANG KHỞI ĐẦU DỰ ÁN MỚI VỚI GITHUB SPEC KIT (SPECIFICATION-DRIVEN DEVELOPMENT)

> **Mục tiêu**: Hướng dẫn trọn vẹn từ A-Z cách thiết lập và vận hành **GitHub Spec Kit** ngay từ ngày đầu tiên cho mọi dự án mới (Unreal Engine, Unity, Web, App, Backend, Embedded), giúp dự án đạt chuẩn **0% Bug, 0% Nợ Kỹ Thuật, và Có Tài Liệu Sống (Living Docs) Vĩnh Viễn**.

---

# 🌟 1. Tổng Quan Triết Lý: Tại Sao Phải Áp Dụng Spec Kit Từ Ngày Đầu?

Trong quy trình phát triển truyền thống với AI (*"Vibe Coding"*):
- Lập trình viên chat các câu lệnh rời rạc, AI vội vã sinh code $\rightarrow$ Code ban đầu chạy được nhưng càng về sau càng rối, các tính năng mới bắt đầu đè logic vào tính năng cũ và sinh ra lỗi hàng loạt (Cascading Bugs).

**Quy trình Specification-Driven Development (SDD) của GitHub Spec Kit** giải quyết triệt để vấn đề này bằng triết lý **"No Spec, No Code"**:

```
[ 1. CONSTITUTION (Hiến pháp) ] ──► [ 2. SPEC (Đặc tả & Toán) ] ──► [ 3. PLAN (Kiến trúc SOLID) ] ──► [ 4. TASKS (Micro-Steps) ] ──► [ 5. IMPLEMENT (Thi công an toàn) ]
```

---

# 📋 2. QUY TRÌNH 4 BƯỚC KHỞI ĐẦU DỰ ÁN MỚI (STEP-BY-STEP)

---

### 📌 BƯỚC 1: Khởi Tạo Bộ Khung Spec Kit Ngay Từ Đầu (1 Lần Duy Nhất)
Khi vừa tạo một project mới:
1. Mở Terminal / Git Bash tại thư mục gốc của project đó.
2. Chạy đúng 1 dòng lệnh duy nhất:
   ```bash
   specify init . --ignore-agent-tools
   ```
3. Lệnh này tự động sinh ra thư mục `.specify/` chứa toàn bộ:
   - `templates/`: Mẫu đặc tả (`spec-template.md`), kế hoạch (`plan-template.md`), tasks (`tasks-template.md`).
   - `scripts/powershell/`: Bộ công cụ tự động hóa (`create-new-feature.ps1`, `setup-plan.ps1`, `setup-tasks.ps1`).
   - `memory/constitution.md`: Hiến pháp của dự án.

---

### 📌 BƯỚC 2: Thiết Lập "Hiến Pháp Dự Án" (`constitution.md`)
Mở file `.specify/memory/constitution.md` để quy định ngay các **Luật Bất Biến** của dự án:

1. **Điều 1: Clean Architecture & SOLID (SRP / DIP)**:
   - Mỗi Component/Subsystem chỉ làm đúng 1 nhiệm vụ duy nhất (Ví dụ: Movement lo di chuyển, Combat lo đánh nhau, HUD lo vẽ).
2. **Điều 2: 0% Hardcoding**:
   - 100% các biến số (tốc độ, sát thương, tầm xa, asset mesh, material) đều phải expose qua Config / DataAsset / UPROPERTY, tuyệt đối không hardcode trong logic C++.
3. **Điều 3: Kỷ Luật Thi Công Micro-Steps ("Chưa code nhé")**:
   - Ép AI phải phân tích giải pháp $\rightarrow$ nói rõ *"chưa code nhé"* $\rightarrow$ chờ anh duyệt từng bước mới được sửa code $\rightarrow$ hướng dẫn test Live Coding.

---

### 📌 BƯỚC 3: Vòng Lặp Phát Triển Từng Tính Năng (Feature Workflow)
Mỗi khi bắt đầu làm một tính năng mới (ví dụ: `tactical-ai`, `inventory-system`, `magic-spell`...):

#### 1️⃣ Tạo Đặc Tả Tính Năng (`spec.md`):
- Chạy lệnh:
  ```bash
  powershell -File .specify/scripts/powershell/create-new-feature.ps1 "Tên mô tả tính năng" -ShortName "ten-ngan"
  ```
  *(Hoặc bảo AI: "Em tạo spec cho tính năng X đi em")*.
- Trong file `spec.md`:
  - Chốt rõ **Mục tiêu tính năng** & Trải nghiệm kỳ vọng.
  - Chốt rõ **Công thức toán học & Quy trình trạng thái**.
  - **Bắt buộc phân tích tối thiểu 5-6 kịch bản lỗi (Edge Cases)** trước khi đụng vào code.

#### 2️⃣ Lập Bản Vẽ Kỹ Thuật (`plan.md`):
- Chạy lệnh `setup-plan.ps1` (hoặc để AI tạo):
  - Khoanh vùng chính xác file C++/Blueprint nào chịu trách nhiệm, đảm bảo không đè logic sang file khác.
  - Thiết kế API Contract (Hàm, Biến, Event).

#### 3️⃣ Phân Rã Danh Sách Việc Cần Làm (`tasks.md`):
- Chạy lệnh `setup-tasks.ps1`:
  - Chia nhỏ tính năng thành các Micro-tasks độc lập `Task 1.1`, `Task 1.2`, `Task 1.3`...

---

### 📌 BƯỚC 4: Thi Công An Toàn Cùng AI Agent (Rule Execution)
1. Yêu cầu AI đọc trực tiếp từ file `specs/XXX/tasks.md`.
2. AI trình bày giải pháp cho từng task $\rightarrow$ *"Chưa code nhé"* $\rightarrow$ Chờ anh duyệt $\rightarrow$ Cập nhật code $\rightarrow$ Hướng dẫn test Live Coding $\rightarrow$ Đánh dấu tick `[x]` vào `tasks.md`.
3. Khi hoàn thành tính năng: Sử dụng lệnh `/learn` để lưu lại các kinh nghiệm quý báu vào bộ nhớ dài hạn của dự án!

---

# 💡 3. BÍ QUYẾT "VÀNG" ĐỂ ĐẠT HIỆU QUẢ TUYỆT ĐỐI 100%

| Bí Quyết | Ý Nghĩa Thực Chiến |
| :--- | :--- |
| **1. "No Spec, No Code"** | Tuyệt đối không bao giờ cho phép AI viết code trước khi có file `spec.md` đã được duyệt. |
| **2. Giải quyết Edge Cases trên giấy trước** | 90% lỗi trong lập trình xuất phát từ việc không lường trước các trường hợp góc (va chạm, chia cho 0, null pointer, mạng lag). Giải quyết nó trong `spec.md` giúp code viết ra chạy mượt mà ngay lần đầu tiên! |
| **3. Thư mục `specs/` là tài sản lớn nhất** | Nó là **Living Documentation** lưu lại toàn bộ lịch sử kiến trúc, giúp anh và cộng sự dễ dàng bảo trì, nâng cấp dự án nhiều năm sau mà không bao giờ sợ quên code! |

---

# ⚡ 4. BẢNG TRA CỨU LỆNH NHANH (CHEAT SHEET)

```bash
# 1. Khởi tạo dự án mới:
specify init . --ignore-agent-tools

# 2. Tạo Spec tính năng mới:
powershell -File .specify/scripts/powershell/create-new-feature.ps1 "Feature Description" -ShortName "feature-name"

# 3. Tạo Plan kỹ thuật:
powershell -File .specify/scripts/powershell/setup-plan.ps1

# 4. Tạo Danh sách Tasks:
powershell -File .specify/scripts/powershell/setup-tasks.ps1
```
