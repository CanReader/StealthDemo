// © 2024 Gamegine. All Rights Reserved.
// Author: Canberk Pitirli
// Project: GameGine Case Project
// Description: Manages the combat system, including weapon handling, attack logic, and player interactions. Provides methods for equipping and using primary and secondary weapons, ensuring a seamless and dynamic combat experience.


#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Weapon.h"
#include "Sound/SoundCue.h"
#include "CombatComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GAMEGINECASE_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCombatComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void EquipWeapon(AWeapon*);
	void Reload();

	void InterpFOV(float);

	UFUNCTION(BlueprintCallable)
	void AttachWeapon(AWeapon* Object);
	UFUNCTION(BlueprintCallable)
	void StartAim(bool pressed);
	
	float DefaultFOV;

	UPROPERTY(EditAnywhere, Category = Combat)
	float ZoomedFOV = 30.f;

	UPROPERTY(EditAnywhere, Category = Combat)
	float ZoomInterpSpeed = 20.f;

	float CurrentFOV;

private:
	class AAgentCharacter* Character;

	AWeapon* EquippedWeapon;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Weapons, meta = (AllowPrivateAccess = "true"))
	AWeapon* PrimaryWeapon;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Weapons, meta = (AllowPrivateAccess = "true"))
	AWeapon* SecondaryWeapon;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Aiming, meta = (AllowPrivateAccess = "true"))
	bool bIsAiming;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Ammo, meta = (AllowPrivateAccess = "true"))
	TMap<EWeaponType,int32> CarriedAmmo;
	
	void WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit);

	UPROPERTY(EditAnywhere)
	class UParticleSystem* ImpactParticles;
	UPROPERTY(EditAnywhere)
	UParticleSystem* BeamParticles;

	UPROPERTY(EditAnywhere)
	UParticleSystem* MuzzleFlash;

	UPROPERTY(EditAnywhere)
	USoundBase* HitSound;

	UPROPERTY(EditAnywhere)
	USoundBase* FireSound;

public:
	FORCEINLINE bool GetIsWeaponEquipped() {
		return EquippedWeapon ? true : false;
	}
	
	FORCEINLINE AWeapon* GetEquippedWeapon() {
		return EquippedWeapon;
	}

	FORCEINLINE	bool GetIsAiming() {
		return bIsAiming;
	}
};
