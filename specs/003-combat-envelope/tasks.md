# Tasks: 003-combat-envelope (Refactor Giao Tranh Combat Envelope)

## Phase 1 — Dọn Dẹp Logic Cũ & Viết Mới Combat Envelope
- [x] **Task 1.1**: Dọn dẹp các logic cũ trong `FormationMovementComponent.cpp` (xóa bỏ tính toán slot giả lập, xóa bỏ cộng dồn half-depth, xóa bỏ khiên chắn vô hình) và thiết lập lệnh `Attack` di chuyển thẳng về phía quân địch. ✅
- [x] **Task 1.2**: Triển khai logic vi mô **Combat Envelope** trong `FormationMovementComponent::UpdateMovement`: khi lính chạm cự ly giao chiến ($< 2.5\text{m}$), từng con lính tự động bước tới con lính địch gần nhất và xoay mặt chém nhau. ✅
- [x] **Task 1.3**: Chuẩn hóa vòng lặp `CombatSubsystem.cpp` & `FormationActor.cpp`: đảm bảo chiến đấu diễn ra liên tục cho đến khi địch bị tiêu diệt sạch, lính tử trận biến mất sạch sẽ và tự động tái lập hàng ngũ khi thắng trận. ✅
- [ ] **Task 1.4**: Biên dịch Live Coding (`Ctrl + Alt + F11`) và test nghiệm thu toàn diện trên Unreal Editor.
