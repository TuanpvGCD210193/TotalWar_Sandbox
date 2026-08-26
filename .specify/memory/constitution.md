# TotalWar_Sandbox Constitution

## Core Principles

### I. Clean Architecture & SOLID (NON-NEGOTIABLE)
- **Single Responsibility Principle (SRP)**: Each Component and Subsystem must have one and only one reason to change.
  - `FormationMovementComponent`: Pure slot navigation, Boids separation, About-Face, and Charge kinematics.
  - `CombatSubsystem`: Frontline melee collision detection, Hit/Damage formulas, and Auto-Brace.
  - `TWHUD`: Canvas rendering, 2D screen projections, selection boxes, and tactical arrows.
  - `SpatialGridSubsystem`: O(1) Spatial Hash Grid queries and Obstacle Avoidance.
- **Dependency Inversion Principle (DIP)**: High-level systems depend on abstract types/subsystems, never on direct circular dependencies.

### II. Zero Hardcoding Standard
- 100% of game design variables, unit stats, movement speeds, combat constants, meshes, and materials MUST be exposed via `UPROPERTY(EditAnywhere, BlueprintReadWrite)` or configured through `UUnitDataAsset` / Config Data.
- Zero hardcoded magic numbers in business logic.

### III. Micro-Steps & Pair Programming Workflow
- **No Mass Code Generation**: Never generate large blocks of unreviewed code.
- **Micro-Step Protocol**: Analyze architecture -> Present technical solution -> Clearly state *"chưa code nhé"* -> Wait for user approval before modifying code.
- **Verification Guidance**: Provide step-by-step verification instructions (Live Coding `Ctrl+Alt+F11`, Play in Editor) for every single step.

### IV. Specification-Driven Development (SDD)
- Every major feature must follow the 4-phase lifecycle:
  1. **Spec** (`.specify/specs/XXX/spec.md`): Requirements, math formulas, edge cases.
  2. **Plan** (`.specify/specs/XXX/plan.md`): Architectural boundaries, affected files, API contracts.
  3. **Tasks** (`.specify/specs/XXX/tasks.md`): Granular, testable micro-tasks.
  4. **Implement**: Controlled, rule-abiding execution.

### V. Total War Battleline & Kinematic Integrity
- Squad center is always defined dynamically by the **Centroid of its living soldiers**.
- Movement (Move / Fallback) commands are strictly isolated from Attack commands.
- Terrain height ($Z$) is sampled via squad-level raycast and locked to ground level (0% flying or sinking).
- Inter-squad boundary forces prevent friendly and enemy soldiers from clipping/penetrating.

## Governance
- This Constitution is the Single Source of Truth for the project.
- All code changes, refactors, and feature implementations must strictly adhere to these principles.

**Version**: 1.0.0 | **Ratified**: 2026-08-26 | **Project**: TotalWar_Sandbox (Unreal Engine 5)
