# 🚀 GITHUB SPEC KIT (SDD) — HƯỚNG DẪN BẮT ĐẦU DỰ ÁN MỚI TỪ ĐẦU (PROJECT STARTER GUIDE)

> **Cẩm Nang Chuẩn Mực Dành Cho Mọi Dự Án Unreal Engine & Phần Mềm Hiện Đại**  
> Kết hợp hoàn hảo giữa phương pháp luận **Phát triển Hướng Đặc tả (Spec-Driven Development - SDD)** và **Bộ 5 Nguyên Tắc Vàng của Antigravity (AGY)**.

---

## 🏛️ 1. BỘ 5 NGUYÊN TẮC VÀNG BẮT BUỘC

1. **🌟 Phong Cách Đồng Hành (Senior Technical Lead)**: Xưng hô "Em" - "Anh", giải thích bản chất gốc rễ toán học / kiến trúc, hướng dẫn chi tiết từng bước trên Unreal Editor.
2. **🛡️ Kỷ Luật Micro-Steps ("Chưa Code Nhé")**: Phân tích trước $\rightarrow$ Chia nhỏ bước $\rightarrow$ Nhắc rõ *"chưa code nhé"* $\rightarrow$ Chờ anh duyệt mới code $\rightarrow$ Cập nhật ngay `[x]` vào `tasks.md`.
3. **📐 Kiến Trúc Hướng Dữ Liệu & SOLID (Zero Hardcoding & DRY)**: Phân rã SRP, $100\%$ cấu hình qua `UPROPERTY`/`UDataAsset`, không hardcode asset path hay magic numbers.
4. **🚀 Quy Trình Spec Kit Phân Cấp (Hierarchical SDD)**: Phân chia rõ `specs/feat/`, `specs/fix/`, `specs/refactor/`, nguyên tắc "One Spec, One Purpose", ma trận 5 Edge Cases.
5. **🧠 Tích Lũy Tri Thức (/learn)**: Dùng lệnh `/learn` để lưu vết các bài học kiến trúc vĩnh viễn vào hệ thống.

---

## 📂 2. CẤU TRÚC THƯ MỤC CHUẨN MỰC CỦA SPEC KIT

```
MyProject/
├── .agents/
│   └── rules/
│       └── user_workflow.md            # Bộ 5 Nguyên tắc vàng của Antigravity
│
├── .specify/
│   ├── memory/
│   │   └── constitution.md             # Hiến pháp kỹ thuật của dự án
│   └── scripts/
│       └── powershell/
│           ├── create-new-feature.ps1  # Script tự động tạo Spec mới
│           ├── check-prerequisites.ps1
│           └── common.ps1
│
└── specs/
    ├── feat/                           # TÍNH NĂNG MỚI (New Features)
    │   └── 001-archer-unit/
    │       ├── spec.md                 # Đặc tả mục tiêu & 5 Edge Cases
    │       ├── plan.md                 # Bản vẽ kiến trúc module
    │       └── tasks.md                # Danh sách Micro-Tasks thi công
    │
    ├── fix/                            # SỬA LỖI & TINH CHỈNH (Bug Fixes)
    │   └── 001-rotation-anchor-fix/
    │
    └── refactor/                       # TÁI CẤU TRÚC KIẾN TRÚC (Architecture Refactoring)
        └── 001-spatial-grid-opt/
```

---

## 🛠️ 3. QUY TRÌNH 4 BƯỚC KHỞI TẠO DỰ ÁN MỚI

### Bước 1: Khởi Tạo Môi Trường Spec Kit
```powershell
# Cài đặt specify CLI (nếu máy mới chưa có)
pip install git+https://github.com/github/spec-kit.git

# Khởi tạo Spec Kit trong thư mục dự án mới
cd "D:\UE Project\NewProject"
specify init . --ignore-agent-tools
```

### Bước 2: Tạo Thư Mục Phân Cấp Chuẩn
```powershell
New-Item -ItemType Directory -Force -Path 'specs/feat', 'specs/fix', 'specs/refactor', '.agents/rules'
```

### Bước 3: Tạo Spec Mới Theo Chuẩn Phân Cấp
```powershell
# Khi làm tính năng mới:
powershell -File .specify/scripts/powershell/create-new-feature.ps1 "Archer Unit" -ShortName "feat/001-archer-unit"

# Khi sửa lỗi phát sinh:
powershell -File .specify/scripts/powershell/create-new-feature.ps1 "Rotation Anchor Fix" -ShortName "fix/001-rotation-anchor"
```

### Bước 4: Vận Hành Theo Chu Trình SDD 3 Giai Đoạn
1. **Giai đoạn Thiết kế**: Viết `spec.md` (chốt công thức & 5 Edge Cases) $\rightarrow$ Viết `plan.md` $\rightarrow$ Viết `tasks.md`.
2. **Giai đoạn Thi công (Micro-Steps)**: Báo cáo giải pháp $\rightarrow$ Nhắc *"chưa code nhé"* $\rightarrow$ Chờ duyệt $\rightarrow$ Viết code $\rightarrow$ Hướng dẫn test engine $\rightarrow$ Tick `[x]` vào `tasks.md`.
3. **Giai đoạn Nghiệm thu & Tích lũy (/learn)**: Test hoàn tất $\rightarrow$ Dùng lệnh `/learn` để ghi nhớ tri thức vĩnh viễn.
