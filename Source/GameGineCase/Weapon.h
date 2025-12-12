// © 2024 Gamegine. All Rights Reserved.
// Author: Canberk Pitirli
// Project: GameGine Case Project
// Description: Defines the base class for all weapon types, providing core functionality and properties. Utilizes HitScan technology for immediate hit detection, ensuring precise and instantaneous weapon impact.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Animation/AnimationAsset.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Weapon.generated.h"

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	EWS_None UMETA(DisplayName = "Initial State"),
	EWS_Equipped UMETA(DisplayName = "Equipped"),
	EWS_EquippedSecondary UMETA(DisplayName = "Equipped Secondary"),
	EWS_Dropped UMETA(DisplayName = "Dropped"),

	EWS_MAX UMETA(DisplayName = "DefaultMAX")
};

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	EWT_None UMETA(DisplayName = "Initial State"),
	EWT_Rock UMETA(DisplayName = "Rock"),
	EWT_Pistol UMETA(DisplayName = "Pistol"),
	EWT_Rifle UMETA(DisplayName = "Rifle"),
	EWT_SubmachineGun UMETA(DisplayName = "Submachine")
};

UCLASS()
class GAMEGINECASE_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	AWeapon();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	void SetHUDAmmo();
	void ShowPickupWidget(bool bShowWidget);
	virtual void Fire(const FVector& HitTarget);
	virtual void Dropped();

	void AddAmmo(int32 AmmoToAdd);
	FVector TraceEndWithScatter(const FVector& HitTarget);
	void WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit);

	UPROPERTY(EditAnywhere, Category = Sounds)
	class USoundBase* EquipSound;
	UPROPERTY(EditAnywhere, Category = Sounds)
	class USoundBase* ReloadSound;
	UPROPERTY(EditAnywhere, Category = Sounds)
	class USoundBase* FireSound;
protected:
	UPROPERTY(EditDefaultsOnly, Category = Sounds)
	float SoundIntensity;
	UPROPERTY(EditDefaultsOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float FireDelay = .15f;
	UPROPERTY(EditDefaultsOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float Damage = 10;
	UPROPERTY(EditDefaultsOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float HeadshotDamage = 100;
	UPROPERTY(EditDefaultsOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	bool bAutomatic = false;


	UFUNCTION()
	virtual void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	UFUNCTION()
	void OnSphereEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	);


private:
#pragma region Components
	UPROPERTY(EditAnywhere, BlueprintReadOnly,meta = (AllowPrivateAccess = "true"))
		TObjectPtr<USkeletalMeshComponent> Mesh;
	UPROPERTY(EditAnywhere, BlueprintReadOnly,meta = (AllowPrivateAccess = "true"))
		TObjectPtr<UStaticMeshComponent> Silencer;
	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
		TObjectPtr<USphereComponent> AreaSphere;
	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
		TObjectPtr<UWidgetComponent> PickupWidget;
#pragma endregion
	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
		EWeaponType type;
	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
		EWeaponState WeaponState;
		
		int32 Ammo;
		int32 MagCapacity;

public:	
	FORCEINLINE USoundBase* GetEquipSound() { return EquipSound; }
	FORCEINLINE bool HasSilencer() {return Silencer == nullptr;}
	FORCEINLINE EWeaponState GetWeaponState() { return WeaponState; }
	FORCEINLINE void SetWeaponState(EWeaponState state) { WeaponState = state; }
	FORCEINLINE EWeaponType GetType() { return type; }
	FORCEINLINE int32 GetAmmo() { return Ammo; }
	FORCEINLINE int32 GetMaxAmmo() { return MagCapacity; }
};
