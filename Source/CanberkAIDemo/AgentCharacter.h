// © 2025 Canberk. All Rights Reserved.
// Author: Canberk Pitirli
// Project: CanberkAIDemo
// Description: This class defines the main character, AgentCharacter, with unique abilities and gameplay mechanics.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include <GameFramework/SpringArmComponent.h>
#include <Camera/CameraComponent.h>
#include "EnhancedInputSubsystemInterface.h"
#include "CombatComponent.h"
#include "StealthComponent.h"
#include "BaseCharacter.h"
#include "CharacterHUD.h"
#include <Components/SphereComponent.h>

class AThrownRock;
class APickupRock;

#include "AgentCharacter.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogMainCharacter, Log, All);

UCLASS()
class CANBERKAIDEMO_API AAgentCharacter : public ABaseCharacter
{
	GENERATED_BODY()

public:
	AAgentCharacter();
	
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	
	virtual void PostInitializeComponents() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void CalculateAimOffset(float DeltaTime);

	void MoveControl(const FInputActionValue& Value);

	void LookControl(const FInputActionValue& Value);

	void CrouchAction(const FInputActionValue& Value);

	void CoverAction(const FInputActionValue& Value);

	void SprintAction(const FInputActionValue& Value);

	void EquipAction(const FInputActionValue& Value);

	void FireAction(const FInputActionValue& Value);
	
	void AimAction(const FInputActionValue& Value);

	void SetOverlappingPickupRock(APickupRock* PickupRock);

	virtual void Jump() override;

	UFUNCTION(BlueprintCallable)
	void EquipArmor(bool bIsEquip);
	
	UFUNCTION(BlueprintCallable)
	void ShowHUD(bool bShow = true, float ShowSpeed = 0.5f);

	void SetOverlappingWeapon(AWeapon* weapon);

	void PlayReloadMontage(EWeaponType type);
	void PlayFireMontage();
	void PlayHitMontage();

	void UpdateHealth();

private:
#pragma region Mesh Modules
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = MeshModule, meta = (AllowPrivateAccess = "true"))
	TMap<FName, TObjectPtr<USkeletalMeshComponent>> MeshModules;

	UPROPERTY(EditAnywhere, Category = MeshModule, meta = (AllowPrivateAccess = "true"))
	int32 ArmorsThreshold = 3;

#pragma endregion

#pragma region Input properties

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputHandlerComponent* InputHandler;

#pragma endregion

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Animation, meta = (AllowPrivateAccess = "true"))
	UAnimMontage* ReloadMontage;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Animation, meta = (AllowPrivateAccess = "true"))
	UAnimMontage* FireMontage;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Animation, meta = (AllowPrivateAccess = "true"))
	UAnimMontage* HitMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = AimOffset, meta = (AllowPrivateAccess = "true"))
	float AimSensivity = 2;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = AimOffset, meta = (AllowPrivateAccess = "true"))
	bool bIsMovementAllowed;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = AimOffset, meta = (AllowPrivateAccess = "true"))
	bool bIsAiming;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = AimOffset, meta = (AllowPrivateAccess = "true"))
	bool bIsSprinting;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = AimOffset, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<AWeapon> OverlappingWeapon;
	FRotator StartingAim;

#pragma region Rock Throwing
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rock", meta = (AllowPrivateAccess = "true"))
	bool bIsHoldingRock = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rock", meta = (AllowPrivateAccess = "true"))
	bool bIsAimingRock = false;

	UPROPERTY(VisibleAnywhere, Category = "Rock", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<APickupRock> OverlappingPickupRock;

	UPROPERTY(EditDefaultsOnly, Category = "Rock", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AThrownRock> ThrownRockClass;

	UPROPERTY(EditDefaultsOnly, Category = "Rock", meta = (AllowPrivateAccess = "true"))
	float ThrowSpeed = 2000.f;

	UPROPERTY(EditDefaultsOnly, Category = "Rock", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USoundBase> ThrowSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rock|Animation", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* ThrowMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rock", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> HeldRockMesh;

	void DrawThrowTrajectory();
	void ThrowRock();
#pragma endregion
	

	UPROPERTY(EditDefaultsOnly, Category = "Camera|Zoom")
	float DefaultFOV = 90.0f;
	UPROPERTY(EditDefaultsOnly, Category = "Camera|Zoom")
	float ZoomedFOV = 50.0f;
	UPROPERTY(EditDefaultsOnly, Category = "Camera|Zoom")
	float ZoomInterpSpeed = 20.0f;

	bool bIsZooming;

public:
	FORCEINLINE	TObjectPtr<UArrowComponent> GetCoverArrowComponent() {
		return CoverArrow;
	}

	FORCEINLINE	TObjectPtr<APlayerController> GetPlayerController() 
	{
		auto player = Cast<APlayerController>(Controller);
		return player;
	}

	FORCEINLINE TObjectPtr<ACharacterHUD> GetHUD() {
		return GetPlayerController() ? Cast<ACharacterHUD>(GetPlayerController()->GetHUD())  : nullptr;
	}

	FORCEINLINE TObjectPtr<UCameraComponent> GetCamera() { return Camera;}

	FORCEINLINE bool GetIsHoldingRock() const { return bIsHoldingRock; }
	FORCEINLINE bool GetIsAimingRock() const { return bIsAimingRock; }
};
