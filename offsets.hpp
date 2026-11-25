#pragma once

#define U_WORLD 0x1B2578F8  // UPDATED


#define GAME_INSTANCE          0x238   // UWorld::OwningGameInstance
#define LOCAL_PLAYERS          0x38    // UGameInstance::LocalPlayers
#define PLAYER_CONTROLLER      0x30    // ULocalPlayer::PlayerController
#define LOCAL_PAWN             0x318   // APlayerController::AcknowledgedPawn
#define PAWN_PRIVATE           0x2E8   // APlayerState::PawnPrivate
#define ROOT_COMPONENT         0x1B0   // AActor::RootComponent
#define RELATIVE_LOCATION      0x140   // USceneComponent::RelativeLocation
#define ACKNOWLEDGEDPAWN       0x318   // APlayerController::AcknowledgedPawn
#define PLAYER_STATE           0x290   // APawn::PlayerState  (or 0x278 in AController)
#define TEAM_INDEX             0xF69   // AFortPlayerStateAthena::TeamIndex
#define GAME_STATE             0x1C0   // UWorld::GameState
#define PLAYER_ARRAY           0x288   // AGameStateBase::PlayerArray
#define MESH                   0x2F0   // ACharacter::Mesh
#define COMPONENT_TO_WORLD     0x1E0   // (common / still used – not listed as named UPROPERTY in this dump)
#define BONE_ARRAY             0x9C0   // USkeletalMeshComponent::CachedBoneSpaceTransforms
#define BONE_ARRAY_CACHE       0x9D0   // USkeletalMeshComponent::CachedComponentSpaceTransforms
#define LAST_SUBMIT_TIME       0x2E8   // (not found under this name – may have moved)
#define LAST_SUBMIT_TIME_ON_SCREEN 0x2F0 // (not found under this name)
#define COMPONENT_VELOCITY     0x188   // USceneComponent::ComponentVelocity
#define TARGETED_FORT_PAWN     0x16C0  // AFortPlayerController::TargetedFortPawn
#define ITEM_NAME              0x40    // (relative – still commonly used)
#define WEAPON_DATA            0x628   // AFortWeapon::WeaponData
#define WEAPON_NAME            0x40    // (relative)
#define AMMO_COUNT             0x1100  // AFortWeapon::AmmoCount
#define RARITY                 0xa2    // (relative – not renamed in dump)
#define CURRENT_WEAPON         0x9D0   // AFortPawn::CurrentWeapon
#define PLATFORM               0x400   // AFortPlayerStateAthena::Platform
#define HABANERO_COMPONENT     0x918   // AFortPlayerStateAthena::HabaneroComponent
#define RANKED_PROGRESS        0xd0    // (relative to HabaneroComponent – still commonly used)
