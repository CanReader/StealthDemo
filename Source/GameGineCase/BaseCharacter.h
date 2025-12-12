// © 2024 Gamegine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Weapon.h"
#include <Components/SphereComponent.h>
#include "CombatComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "BaseCharacter.generated.h"

class UStealthComponent;
class UCombatComponent;

UCLASS()
class GAMEGINECASE_API ABaseCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ABaseCharacter();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	void EquipWeapon(AWeapon* EquipWeapon);	


	#pragma region Camera
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USpringArmComponent> CameraArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UCameraComponent> Camera;
#pragma endregion

	#pragma region Movement Settings
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float BaseWalkSpeed = 300;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float BaseSprintSpeed = 650;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float BaseAimSpeed = 150;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float BaseCrouchSpeed = 200;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cover", meta = (AllowPrivateAccess = "true"))
	float BaseCoverSpeed = 180.0f;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cover", meta = (AllowPrivateAccess = "true"))
	float TargetSpeed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cover", meta = (AllowPrivateAccess = "true"))
	float InterpSpeed = 2.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Crouch", meta = (AllowPrivateAccess = "true"))
	float DefaultCapsuleHalfHeight = 96;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Crouch", meta = (AllowPrivateAccess = "true"))
	float CrouchCapsuleHalfHeight = 44.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Crouch", meta = (AllowPrivateAccess = "true"))
	float CrouchCameraHeightOffset = 18.0f;
#pragma endregion

protected:
	
	float MaxHealth = 100;
	float Health = 60;

	float AO_Yaw;
	float AO_Pitch;
	float Direction;
	bool bIsAI;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Logging", meta = (AllowPrivateAcess = "true"))
	bool bIsLoggingAOEnabled;

#pragma region Components

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCombatComponent> Combat;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<AWeapon> Weapon;

	
	UPROPERTY(EditAnywhere, Category = Components, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStealthComponent> Stealth;


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
	TObjectPtr < UArrowComponent> CoverArrow;


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USphereComponent> CoverCollider;

#pragma endregion

public:
	UFUNCTION(BlueprintCallable)
	FORCEINLINE bool GetIsInAir() {
		return GetCharacterMovement()->IsFalling();
	}

	FORCEINLINE TObjectPtr<UCombatComponent> GetCombatComponent() {
		return Combat;
	}

	UFUNCTION(BlueprintCallable)
	FORCEINLINE AWeapon* GetWeapon() {
		return Combat ? Combat->GetEquippedWeapon() : nullptr;
	}

	UFUNCTION(BlueprintCallable)
	FORCEINLINE bool GetIsAiming() {
		return Combat && Combat->GetEquippedWeapon() && Combat->GetIsAiming();
	}

	FORCEINLINE USpringArmComponent* GetCameraArm() {
		return CameraArm;
	}

	FORCEINLINE UArrowComponent* GetCoverArrowComponent() {
		return CoverArrow;
	}

	UFUNCTION(BlueprintCallable)
	FORCEINLINE bool GetIsAI() {
		return bIsAI;
	}

	FORCEINLINE float GetAO_Yaw() {
		return AO_Yaw;
	}

	FORCEINLINE float GetAO_Pitch() {
		return AO_Pitch;
	}

	FORCEINLINE bool GetIsAOLoggingEnabled() {
		return bIsLoggingAOEnabled;
	}

	FORCEINLINE EWeaponType GetEquippedWeaponType() { return Combat && Combat->GetEquippedWeapon() ? Combat->GetEquippedWeapon()->GetType() : EWeaponType::EWT_None; }

	FORCEINLINE void ChangeSpeed(float NewSpeed) { TargetSpeed = NewSpeed; }
};
