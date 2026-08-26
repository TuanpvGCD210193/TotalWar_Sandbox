# Tasks: 002-attack-pursuit-fix

## Phase 1 — Fix Attack Command & Movement Pursuit
- [x] **Task 1.1**: Cập nhật `FormationMovementComponent::ExecuteMoveCommand` để tính toán slot tiền tuyến và gán slot cho toàn bộ lính khi nhận lệnh `Attack` (Xóa bỏ lệnh `return;` sớm). ✅
- [x] **Task 1.2**: Sửa cự ly tiếp xúc tiền tuyến (`CombatGap = 65cm`) và bổ sung Anti-Snap khi click lại lúc đang giao chiến. ✅
- [x] **Task 1.3**: Khắc phục hiện tượng False-Disengage trong `CombatSubsystem` (chỉ dừng giao tranh khi toàn bộ lính địch tử trận). ✅
- [ ] **Task 1.4**: Kiểm tra biên dịch Live Coding (`Ctrl + Alt + F11`) và test nghiệm thu trên Unreal Editor.
