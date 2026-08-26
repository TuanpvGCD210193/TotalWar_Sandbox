# ⚔️ TOTAL WAR: SANDBOX SIMULATION (UE5)

> **Dự án Mô phỏng Chiến trường Chiến thuật Thời gian thực Quy mô lớn (Large-Scale RTS Battle Simulator)**  
> Được xây dựng trên nền tảng **Unreal Engine 5** với kiến trúc **C++ Thuần túy (Pure Modern C++)**, tối ưu hóa hiệu năng bằng **GPU Instancing** và **Thuật toán Phân hoạch Không gian (Spatial Hash Grid)**.

---

## 📖 1. Giới Thiệu Học Thuật & Tầm Nhìn Dự Án (Academic Description & Vision)

Dự án **Total War Sandbox** là một công trình nghiên cứu và phát triển hệ thống mô phỏng chiến thuật thời gian thực (Real-Time Tactics - RTT). Mục tiêu trọng tâm của dự án là giải quyết bài toán mô phỏng đồng thời hàng nghìn đơn vị tác chiến độc lập (**Multi-Agent Simulation**) nhưng vẫn duy trì tính kỷ luật đội hình (**Rigid Formation Discipline**), tính chân thực trong va chạm vật lý khối lượng (**Mass-based Momentum Collision**) và cơ chế giao tranh mặt tiền tuyến (**Frontline Engagement**) tương tự dòng game chiến thuật kinh điển *Total War*.

Dự án áp dụng triệt để các nguyên lý thiết kế phần mềm hiện đại:
- **Nguyên lý SOLID & Phân rã trách nhiệm đơn lẻ (SRP)**: Mỗi Component chỉ chịu trách nhiệm cho một miền logic duy nhất (Di chuyển, Quản lý Slot, Render hiển thị, Giao chiến).
- **Kiến trúc Hướng Dữ liệu (Data-Driven Architecture)**: $100\%$ chỉ số chiến đấu và thông số mô hình được quản lý qua `UDataAsset`, không hardcode bất kỳ giá trị nào trong mã nguồn C++.
- **Kiến trúc Hướng Tối ưu (Data-Oriented Mindset)**: Phân tách dữ liệu tính toán logic (`FSoldierEntity` - Lightweight POD Struct) hoàn toàn độc lập với Actor/Object cồng kềnh, giảm thiểu tối đa bộ nhớ đệm và Overhead của CPU.

---

## 🛠️ 2. Nền Tảng Công Nghệ & Ngôn Ngữ (Engine & Tech Stack)

| Hạng mục | Công nghệ sử dụng | Vai trò & Mục đích |
| :--- | :--- | :--- |
| **Game Engine** | **Unreal Engine 5 (UE 5.5)** | Nền tảng dựng hình, quản lý World, Viewport và hệ thống Camera chiến thuật. |
| **Ngôn ngữ Lập trình** | **Modern C++ (C++20)** | Toàn bộ $100\%$ logic mô phỏng, tính toán toán học, bầy đàn và va chạm được viết bằng C++ thuần để đạt tốc độ thực thi tối đa. |
| **Hệ thống Hiển thị** | **GPU Instanced Static Mesh (ISM)** | Gom toàn bộ $120+$ binh sĩ trong một đạo quân thành $1$ Draw Call duy nhất trên GPU, cho phép hiển thị hàng chục nghìn binh sĩ ở mức $60+$ FPS. |
| **Cấu trúc Không gian** | **2D Spatial Hash Grid Subsystem** | Chia bản đồ thành lưới ô vuông kích thước $3\text{m} \times 3\text{m}$, giúp truy vấn tìm kiếm kẻ thù xung quanh với độ phức tạp thuật toán lý tưởng $\mathcal{O}(1)$. |

---

## 🧠 3. Các Thuật Toán Hiện Tại (Giải Thích Dễ Hiểu Cho Mọi Người Đọc)

Hệ thống chiến trường vận hành mượt mà nhờ sự phối hợp nhịp nhàng của 4 thuật toán cốt lõi:

```
┌─────────────────────────┐     ┌─────────────────────────┐     ┌─────────────────────────┐
│ 1. CHIẾU HÀNG NGŨ       │ ──► │ 2. ĐÀN CHIM LÁI ĐỘNG HỌC│ ──► │ 3. KHÓA TIỀN TUYẾN      │
│ (Rank & File Projection)│     │ (Boids Flocking Steering)│    │ (Frontline Hybrid Lock) │
└─────────────────────────┘     └─────────────────────────┘     └─────────────────────────┘
```

---

### 🏛️ Thuật Toán 1: Chiếu Hàng Ngũ (Rank & File Projection)
- **Tên học thuật**: *Rank & File Planar Coordinate Projection Matching*.
- **Cách hiểu đơn giản**: 
  - Hãy tưởng tượng một trung đội 120 người lính đang đứng lộn xộn. Khi người chỉ huy chỉ vào một bãi đất mới và bảo *"Tập hợp đội hình 10 hàng dọc, 12 hàng ngang!"*, làm sao để lính không chạy đâm sầm vào nhau?
  - Thuật toán sẽ vẽ sẵn 120 chiếc ghế (Slot) tại điểm đích. Sau đó, nó nhìn xem người lính nào đang đứng bên trái nhất thì cho vào ghế bên trái, người lính nào đứng phía trước nhất thì cho vào ghế hàng đầu.
  - **Kết quả**: Mỗi người lính đều có một đường đi thẳng tắp, song song và bước đi đều đặn như duyệt binh mà không bao giờ bị chéo chân hay chen lấn.

---

### 🐦 Thuật Toán 2: Đàn Chim Lái Động Học (Boids Flocking Kinematics)
- **Tên học thuật**: *Continuous Kinematic Flocking with Soft Distance Constraint*.
- **Cách hiểu đơn giản**:
  - Giống như một đàn chim bay trên bầu trời hay đàn cá bơi dưới biển, mỗi người lính khi bước đi đều có một "vùng không gian cá nhân" hình tròn quanh mình (bán kính $\approx 38\text{cm}$).
  - Nếu một đồng đội bước quá sát làm lấn vào vòng tròn này, một lực đẩy nhẹ nhàng và êm ái sẽ đẩy cả hai ra xa một chút.
  - **Kết quả**: Binh lính di chuyển uyển chuyển, tự nhiên như dòng nước chảy qua chướng ngại vật mà không bao giờ bị dính chặt vào nhau hay dịch chuyển nhảy cóc (Teleport/Snapping).

---

### 🛡️ Thuật Toán 3: Khóa Tiền Tuyến & Khối Lượng Va Chạm (Hybrid Frontline & Mass-Ratio Combat)
- **Tên học thuật**: *Mass-Momentum Frontline Engagement with Dynamic Rank Reinforcement*.
- **Cách hiểu đơn giản**:
  - **Khi 2 đạo bộ binh bằng cân nhau lao vào nhau ($Mass = 1:1$)**: 
    - Đạo phòng thủ hạ khiên ghì chân xuống đất (Auto-Brace) tạo thành bức tường thép vững chắc, không bị lùi dù chỉ $1\text{cm}$.
    - Đạo tấn công lao tới, chạm vào hàng đầu của địch là dừng bước ngay lập tức. Hai đạo quân áp sát nhau phẳng phiu và khóa chặt mặt trận (**Frontline Lock**).
  - **Quy tắc "Chỉ có tiến lên trám chỗ — $0\%$ đi lùi"**:
    - Lính Hàng 1 của 2 bên trực tiếp vung kiếm chém nhau.
    - Lính Hàng sau của bên tấn công tự động tràn lên phía trước ép sát lưng đồng đội để duy trì sức ép chiến tuyến.
    - Khi một người lính Hàng 1 tử trận, người lính ngay phía sau sẽ bước lên đúng 1 bước chân để lấp kín lỗ hổng.

---

### 🎲 Thuật Toán 4: Tính Toán Sát Thương & Giáp Chuẩn Total War
- **Tên học thuật**: *D100 Probability Hit-Chance & Stochastic Armour Absorption Formula*.
- **Cách hiểu đơn giản**:
  - Khi một người lính vung kiếm chém đối thủ, máy tính tung một con xúc xắc 100 mặt:
    $$\text{Tỷ lệ trúng} = 35\% + (\text{Công cận chiến} - \text{Thủ cận chiến}) + \text{Thưởng Xung phong}$$
  - Nếu chém trúng, lớp giáp sắt của đối thủ sẽ phát huy tác dụng: Giáp càng dày thì sát thương thông thường bị triệt tiêu càng nhiều, chỉ có sát thương xuyên giáp (Armour-Piercing) là chém xuyên thẳng vào máu.

---

## 🎯 4. Lộ Trình Phát Triển Các Binh Chủng Tương Lai (Unit Roadmap)

Hệ thống được thiết kế mở rộng sẵn sàng để đón nhận các binh chủng chiến thuật đặc trưng:

```
                  ┌────────────────────────────────────────┐
                  │       TỔNG THỂ BINH CHỦNG TÁC CHIẾN   │
                  └────────────────────────────────────────┘
                                       │
         ┌─────────────────────────────┼─────────────────────────────┐
         ▼                             ▼                             ▼
┌──────────────────┐          ┌──────────────────┐          ┌──────────────────┐
│  🏹 CUNG THỦ     │          │  🐎 KỴ BINH      │          │  🛡️ GIÁO BINH    │
│  (Archer/Ranged) │          │  (Shock Cavalry) │          │  (Spear/Pike)    │
└──────────────────┘          └──────────────────┘          └──────────────────┘
```

### 🏹 1. Cung Thủ / Xạ Thủ (Archer & Ranged Unit) — *Giai đoạn tiếp theo*
- **Quỹ đạo Đạn đạo Parabol (Parabolic Ballistics)**: Mũi tên bay theo đường cong vật lý chịu ảnh hưởng của trọng lực và sức gió.
- **Góc Bắn & Tầm Nhìn (Arc of Fire & Line of Sight)**: Cung thủ cần có tầm nhìn thoáng đãng; có thể bắn gián tiếp qua đầu đồng đội nếu đứng ở hàng sau.
- **Chu kỳ Nạp Tên & Rút Kiếm Phòng Vệ**: Bắn tên từ xa, nhưng nếu bị quân địch áp sát cận chiến sẽ tự động chuyển sang rút dao găm chống cự.

### 🐎 2. Kỵ Binh Xung Kích (Shock Cavalry)
- **Khối Lượng Khổng Lồ (Huge Mass Ratio $> 3.5\times$)**: Cú húc xung phong (Charge) có sức xuyên phá cực mạnh, húc văng và xé toạc các hàng ngũ bộ binh mỏng manh.
- **Đánh Sườn & Đánh Tập Hậu (Flanking & Rear Charge)**: Đánh từ sau lưng hoặc bên sườn đối phương gây sát thương nhân đôi và làm sụp đổ tinh thần (Morale Shock).

### 🛡️ 3. Giáo Binh / Trường Thương (Spearmen & Pikemen)
- **Khắc Chế Kỵ Binh (Anti-Cavalry Brace)**: Khi đứng yên dựng giáo, toàn bộ lực húc của kỵ binh sẽ bị phản ngược lại chính kỵ binh.
- **Bức Tường Giáo Phalanx**: Tạo ra tầm sát thương dài hơn kiếm binh, chặn đứng kẻ thù từ khoảng cách an toàn.

### ☄️ 4. Vũ Khí Công Thành & Pháo Binh (Artillery & Siege Engines)
- **Sát Thương Diện Rộng (Area of Effect - AoE)**: Bắn phá đá lửa hoặc đại bác tầm xa, tạo ra các vụ nổ phá vỡ cấu trúc khối của toàn bộ một đạo quân.

---

## 👨‍💻 Tác giả & Định hướng Phát triển
- **Dự án**: Total War Sandbox Clone (UE5)
- **Định hướng**: Tạo dựng một Framework chiến trường thời gian thực mạnh mẽ, tối ưu hóa tối đa cho các nhà phát triển game chiến thuật trên Unreal Engine.
