// Copyright Total War Sandbox. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "TWTypes.generated.h"

// ============================================================================
// ENUMS
// ============================================================================

/**
 * Defines the archetype/category of a unit.
 * Prototype v1 only uses MeleeInfantry, but all types are declared
 * so the architecture is ready for expansion.
 */
UENUM(BlueprintType)
enum class EUnitType : uint8
{
	MeleeInfantry    UMETA(DisplayName = "Melee Infantry"),
	SpearInfantry    UMETA(DisplayName = "Spear Infantry"),
	Cavalry          UMETA(DisplayName = "Cavalry"),
	Archer           UMETA(DisplayName = "Archer"),
	Gunner           UMETA(DisplayName = "Gunner"),
	Monster          UMETA(DisplayName = "Monster"),
	Hero             UMETA(DisplayName = "Hero"),
	Artillery        UMETA(DisplayName = "Artillery"),
};

/**
 * Team identifier. Used for visual color, target filtering, and faction logic.
 */
UENUM(BlueprintType)
enum class ETeamID : uint8
{
	Blue   UMETA(DisplayName = "Blue (Player)"),
	Red    UMETA(DisplayName = "Red (Enemy)"),
};

/**
 * Formation-level state machine.
 * This is the PRIMARY state — bMaintainFormation is a RESULT of this state, not a driver.
 */
UENUM(BlueprintType)
enum class EFormationState : uint8
{
	Idle       UMETA(DisplayName = "Idle"),        // Standing in formation slots
	March      UMETA(DisplayName = "March"),       // Moving while maintaining formation
	Charge     UMETA(DisplayName = "Charge"),      // Charging toward enemy, loosened slots
	Engage     UMETA(DisplayName = "Engage"),      // In melee combat, combat envelope
	Rout       UMETA(DisplayName = "Rout"),        // Fleeing, no formation (future)
	Shattered  UMETA(DisplayName = "Shattered"),   // Cannot rally, removed from battle (future)
};

/**
 * Individual soldier state within a formation.
 */
UENUM(BlueprintType)
enum class ESoldierState : uint8
{
	Idle       UMETA(DisplayName = "Idle"),
	Moving     UMETA(DisplayName = "Moving"),
	Charging   UMETA(DisplayName = "Charging"),
	Fighting   UMETA(DisplayName = "Fighting"),
	Dead       UMETA(DisplayName = "Dead"),
};

/**
 * Types of commands a player can issue to a formation.
 */
UENUM(BlueprintType)
enum class EFormationCommandType : uint8
{
	Move           UMETA(DisplayName = "Move"),            // Right-click ground (quick)
	Attack         UMETA(DisplayName = "Attack"),          // Right-click enemy formation
	PlacementMove  UMETA(DisplayName = "Placement Move"),  // Right-hold + drag placement
};

/**
 * Formation shape type. Prototype only uses Rectangle.
 * Extensible for wedge, circle, etc.
 */
UENUM(BlueprintType)
enum class EFormationShape : uint8
{
	Rectangle  UMETA(DisplayName = "Rectangle"),
};

// ============================================================================
// CONSTANTS
// ============================================================================

/**
 * Global gameplay constants.
 * All values tunable — change here, affects entire project.
 */
namespace TWConstants
{
	// --- Formation ---
	constexpr float  DefaultSoldierSpacing   = 100.0f;   // cm between soldier slots
	constexpr int32  DefaultFormationWidth    = 20;       // columns (soldiers per rank)
	constexpr int32  MinFormationWidth        = 3;
	constexpr int32  MaxFormationWidth        = 40;
	constexpr int32  DefaultEntityCount       = 120;

	// --- Movement ---
	constexpr float  DefaultMoveSpeed         = 300.0f;   // cm/s base march speed
	constexpr float  ChargeSpeedMultiplier    = 1.8f;     // charge speed = base * this
	constexpr float  SlotArrivalThreshold     = 15.0f;    // cm — "close enough" to slot
	constexpr float  FormationArrivalThreshold = 50.0f;   // cm — formation reached target

	// --- Combat ---
	constexpr float  CombatTickInterval       = 0.1f;     // 10 Hz combat tick
	constexpr float  DefaultAttackInterval    = 1.2f;     // seconds between attacks
	constexpr float  DefaultMeleeRange        = 150.0f;   // cm — melee attack range
	constexpr int32  BaseHitChance            = 35;       // base % before MA/MD
	constexpr int32  MinHitChance             = 8;        // floor
	constexpr int32  MaxHitChance             = 90;       // ceiling

	// --- Charge ---
	constexpr float  ChargeDecayDuration      = 5.0f;     // seconds for charge bonus to fade
	constexpr float  MinChargeDistance         = 300.0f;   // cm — min distance to build charge

	// --- Spatial Grid ---
	constexpr float  SpatialGridCellSize      = 200.0f;   // cm per cell (2m)

	// --- Input ---
	constexpr float  SelectionDragThreshold   = 5.0f;     // pixels before drag-select starts
	constexpr float  PlacementDragThreshold   = 10.0f;    // pixels before placement drag starts

	// --- Camera ---
	constexpr float  CameraMoveSpeed          = 1500.0f;  // cm/s
	constexpr float  CameraZoomSpeed          = 200.0f;   // cm per scroll tick
	constexpr float  CameraRotateSpeed        = 2.0f;     // degrees per pixel
	constexpr float  CameraMinHeight          = 500.0f;   // cm
	constexpr float  CameraMaxHeight          = 5000.0f;  // cm
	constexpr float  CameraDefaultHeight      = 2000.0f;  // cm
	constexpr float  CameraDefaultPitch       = -50.0f;   // degrees

	// --- Visual ---
	constexpr float  SoldierCubeSize          = 40.0f;    // cm — cube half-extent for visual
}

// ============================================================================
// FORWARD DECLARATIONS
// ============================================================================

class AFormationActor;
class UUnitDataAsset;
struct FSoldierEntity;
struct FSoldierHandle;
struct FFormationSlot;
