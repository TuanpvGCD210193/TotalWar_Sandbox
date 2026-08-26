# 🚀 HƯỚNG DẪN SETUP & SỬ DỤNG CHÍNH THỨC GITHUB SPEC KIT (`github/spec-kit`)

> Tài liệu này hướng dẫn cài đặt và sử dụng **công cụ chính thức từ GitHub**: [github.com/github/spec-kit](https://github.com/github/spec-kit) áp dụng cho mọi dự án phần mềm.

---

# 📦 1. Điều Kiện Cần Chuẩn Bị (Prerequisites)

Để sử dụng CLI chính thức của GitHub, máy tính cần:
1. **Python 3.10+** và **Git** (Máy của anh đã có sẵn: Python 3.13 và Git 2.45).
2. Công cụ quản lý package **`uv`** (khuyên dùng bởi GitHub) hoặc **`pip`**.

---

# 🛠️ 2. Hướng Dẫn Cài Đặt Chính Thức (Installation)

### Cách A: Cài bằng `uv` (Khuyên dùng chính thức bởi GitHub)
```bash
# 1. Cài đặt uv (nếu chưa có)
powershell -ExecutionPolicy ByPass -c "irm https://astral.sh/uv/install.ps1 | iex"

# 2. Cài đặt specify-cli chính thức từ GitHub Spec Kit
uv tool install specify-cli --from git+https://github.com/github/spec-kit.git
```

### Cách B: Cài bằng `pip` (Có sẵn trên máy)
```bash
pip install git+https://github.com/github/spec-kit.git
```

---

# ⚙️ 3. Khởi Tạo Dự Án Bằng GitHub Spec Kit (`specify init`)

Mở Terminal / PowerShell tại thư mục dự án cần áp dụng (Ví dụ: `d:\UE Project\TotalWar_Sandbox`), chạy lệnh:

### 🔹 Khởi tạo cho dự án hiện tại (In-place):
```bash
specify init .
```
*(Hoặc dùng flag `--here`)*:
```bash
specify init --here
```

### 🔹 Khởi tạo kèm Agent Integration (Copilot, Claude, Gemini, Cursor...):
```bash
specify init . --integration gemini
# hoặc
specify init . --integration copilot
# hoặc
specify init . --integration claude
```

---

# 📂 4. Cấu Trúc Do GitHub Spec Kit Tự Động Sinh Ra

Sau khi chạy `specify init`, công cụ sẽ tự động tạo ra thư mục chuẩn của GitHub:

```
Your_Project/
  └── .specify/
       ├── constitution.md       ◄── Hiến pháp dự án (Quy định kiến trúc & coding standards)
       ├── templates/            ◄── Bộ templates chính thức của GitHub
       │    ├── spec-template.md
       │    ├── plan-template.md
       │    └── tasks-template.md
       ├── scripts/              ◄── Các script tự động hóa (PowerShell / Bash / Python)
       └── specs/                ◄── Nơi lưu các tính năng (Features)
```

---

# 🔄 5. Vòng Lặp Làm Việc Chính Thức Với GitHub Spec Kit

GitHub Spec Kit vận hành qua **5 bước chuẩn mực**:

```
[ 1. CONSTITUTION ] ──► [ 2. SPECIFY ] ──► [ 3. PLAN ] ──► [ 4. TASKS ] ──► [ 5. IMPLEMENT ]
```

### 1️⃣ Bước 1: Thiết lập Hiến pháp (`Constitution`)
Chỉnh sửa file `.specify/constitution.md` để quy định các nguyên tắc bất biến (Không hardcode, Clean Architecture, Micro-Steps).

### 2️⃣ Bước 2: Tạo Đặc tả tính năng mới (`Specify`)
Khi muốn làm tính năng mới (ví dụ: `tactical-ai` hoặc `magic-system`):
```bash
specify new "tactical-ai"
```
CLI sẽ tự động tạo folder `.specify/specs/001-tactical-ai/spec.md` từ template chuẩn.

### 3️⃣ Bước 3: Lập Bản vẽ Kỹ thuật (`Plan`)
```bash
specify plan
```
CLI sẽ sinh ra file `plan.md` xác định kiến trúc, các file C++ bị ảnh hưởng, API contract và verification plan.

### 4️⃣ Bước 4: Phân rã công việc (`Tasks`)
```bash
specify tasks
```
CLI sẽ sinh ra file `tasks.md` chia nhỏ thành các nhiệm vụ độc lập, đánh số thứ tự.

### 5️⃣ Bước 5: Thực thi cùng AI Agent (`Implement`)
Mở chat với AI Agent (Antigravity, Copilot, Claude), trỏ vào `.specify/specs/001-tactical-ai/tasks.md` để Agent code từng task một cách an toàn và có kiểm soát!

---

# ⚡ 6. Cheat Sheet Các Lệnh GitHub Spec Kit Thông Dụng

| Lệnh | Ý Nghĩa |
| :--- | :--- |
| `specify init .` | Khởi tạo Spec Kit trực tiếp vào dự án hiện tại |
| `specify init <ProjectName>` | Tạo dự án mới kèm bộ khung Spec Kit |
| `specify new "<feature-name>"` | Tạo một Feature Spec mới |
| `specify plan` | Tạo bản kế hoạch kỹ thuật cho feature hiện tại |
| `specify tasks` | Phân rã feature thành danh sách tasks |
| `specify check` | Kiểm tra tính toàn vẹn của các file spec |
